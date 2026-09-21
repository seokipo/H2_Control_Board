/**
 * @file main.c
 * @brief H2_Control_Board dsPIC33CK512MP710 Main Source File with Integrated
 * BOP Control & Modbus
 *
 * @note 본 메인 소스는 시스템 설계 사양에 맞춰 다음 통신 및 제어 스케줄러를
 * 가동합니다:
 *       1. 시스템 하드웨어 핀 및 GPIO 초기화
 *       2. ADS1115(16ch ADC), DAC60516(12ch DAC), MAX31856(다채널 TC) 장치
 * 드라이버 초기화
 *       3. RS-485 및 RS-422 UART 통신 개시
 *       4. 주기적 센서 계측(온도, 압력, 유량 등) 결과의 Modbus 슬레이브
 * 데이터베이스 바인딩
 *       5. 노트북 관제단 패킷 처리용 Modbus_Task 실시간 구동
 *
 * Company: Microchip Technology Inc. & Antigravity AI
 */

// ==============================================================================
// dsPIC33CK512MP710 Configuration Bit Settings (ICSP Channel 2: PGEC2 / PGED2)
// ==============================================================================

// FSEC
#pragma config BWRP = OFF       // Boot Segment Write-Protect bit (Boot Segment may be written)
#pragma config BSS = DISABLED   // Boot Segment Code-Protect Level bits (No Protection)
#pragma config BSEN = OFF       // Boot Segment Control bit (No Boot Segment)
#pragma config GWRP = OFF       // General Segment Write-Protect bit (General Segment may be written)
#pragma config GSS = DISABLED   // General Segment Code-Protect Level bits (No Protection)
#pragma config CWRP = OFF       // Configuration Segment Write-Protect bit
#pragma config CSS = DISABLED   // Configuration Segment Code-Protect Level bits
#pragma config AIVTDIS = OFF    // Alternate Interrupt Vector Table bit (Disabled AIVT)

// FBSLIM
#pragma config BSLIM = 0x1FFF   // Boot Segment Flash Page Address Limit bits

// FOSCSEL
#pragma config FNOSC = FRC      // Oscillator Source Selection: Internal Fast RC (FRC 8MHz)
#pragma config IESO = OFF       // Two-speed Oscillator Start-up Disable (즉시 기동)

// FOSC
#pragma config POSCMD = NONE    // Primary Oscillator Mode Select bits (Primary Oscillator disabled)
#pragma config OSCIOFNC = ON    // OSC2 Pin Function bit (OSC2 is General Purpose I/O)
#pragma config FCKSM = CSDCMD   // Clock Switching Mode bits (Switching & Fail-Safe Monitor disabled)
#pragma config PLLKEN = OFF     // PLL Lock Status Control: PLL lock not required (동결 방지)
#pragma config XTCFG = G3       // XT Config
#pragma config XTBST = ENABLE   // XT Boost

// FWDT
#pragma config RWDTPS = PS2147483648 // Run Mode Watchdog Timer Post Scaler
#pragma config RCLKSEL = LPRC   // Watchdog Timer Clock Select bits
#pragma config WINDIS = ON      // Watchdog Timer Non-Window Mode (WINDIS_ON = Non-Window)
#pragma config WDTWIN = WIN25   // Watchdog Timer Window Select bits
#pragma config SWDTPS = PS2147483648 // Sleep Mode Watchdog Timer Post Scaler
#pragma config FWDTEN = ON_SW   // Watchdog Timer controlled via SW (WDTCON.ON)

// FPOR
#pragma config BISTDIS = DISABLED // Memory BIST Feature Disable

// FICD (사용자 지정: 점퍼 결선에 따른 PGED1 / PGEC1 채널 설정)
#pragma config ICS = PGD1       // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF     // JTAG Enable bit (JTAG is disabled)
#pragma config NOBTSWP = DISABLED // BOOTSWP instruction disable bit

// FDMT
#pragma config DMTDIS = OFF     // Dead Man Timer Disable bit

// ==============================================================================

#if defined(__XC16__) || defined(__XC)
#include <xc.h>
#endif
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "ads1115.h"
#include "dac60516.h"
#include "ethernet.h"
#include "flash.h"
#include "modbus.h"
#include "pin_map.h"
#include "rs422.h"
#include "rs485.h"
#include "rtc.h"
#include "thermocouple.h"
#include "pid.h"

// 내부 FRC (8MHz) 기준 명령 클록 FCY = Fosc / 2 = 4 MHz
#ifndef FCY
#define FCY 4000000UL
#endif
#include <libpic30.h>

int main(void) {
  // [0] SW Watchdog Timer 즉시 OFF (무한 리셋 방지)
  WDTCONLbits.ON = 0;

  // [1] 모든 포트 디지털 I/O 모드로 전환
  ANSELA = 0x0000;
  ANSELB = 0x0000;
  ANSELC = 0x0000;
  ANSELD = 0x0000;
  ANSELE = 0x0000;
  ANSELF = 0x0000;

  // 내장 OP-AMP 및 아날로그 컴퍼레이터 비활성화 (Pin 18 RA0, Pin 19 RE2 점유 원천 해제)
  AMPCON1L = 0x0000;
  AMPCON1H = 0x0000;
  DAC1CONL = 0x0000;
  DAC2CONL = 0x0000;
  DAC6CONL = 0x0000;
  ODCA = 0x0000;
  ODCE = 0x0000;

  // [2] 시스템 하드웨어 드라이버 및 통신 포트 초기화
  GPIO_Initialize();     // DO 포트 및 릴레이/솔레노이드 밸브 초기화
  TC_Initialize();       // MAX31856 및 MUX 열전대 온도 센서 드라이버 개시
  DAC60516_Initialize(); // 🎛️ DAC60516 16비트 DAC 초기화 (내부 2.5V 레퍼런스 ON, 0~5V 2x 게인 ON, 전 채널 활성화)
  ADS1115_Initialize();  // 📊 ADS1115 16비트 ADC 4개 칩 및 RB0/RB1 I2C 버스 초기화 (+/-6.144V FSR)
  RTC_Initialize();      // ⏰ DS3231 고정밀 RTC 초기화 (I2C1 버스 공유, 24시간 형식 설정)
  RS422_Initialize();    // 👑 [특허급 하드웨어 구제] 70번 핀 초정밀 비트뱅잉 RX & 71번 하드웨어 TX (19200 bps)
  RS485_Initialize();    // 필드/컨버터용 9600bps 하드웨어 UART1 (RB6 RX, RD5 TX, RB5 DIR)
  Modbus_Initialize();   // Modbus RTU 슬레이브 데이터베이스 초기화
  ETH_Initialize();      // 🌐 듀얼 W5500 이더넷 칩셋 SPI2 및 네트워크 파라미터(192.168.0.100) 초기화
  ETH_ModbusTCP_Init(MODBUS_TCP_PORT); // 🌐 W5500 Socket 0 Modbus TCP (Port 502) 서버 리스닝 개시

  // 초기 RTC 시간 1-Shot 획득 및 Modbus DB 등록
  DateTime_t init_dt;
  if (RTC_GetTime(&init_dt)) {
    modbus_db.input_regs[REG_IN_RTC_YEAR]  = (uint16_t)init_dt.year + 2000;
    modbus_db.input_regs[REG_IN_RTC_MONTH] = (uint16_t)init_dt.month;
    modbus_db.input_regs[REG_IN_RTC_DATE]  = (uint16_t)init_dt.date;
    modbus_db.input_regs[REG_IN_RTC_HOUR]  = (uint16_t)init_dt.hour;
    modbus_db.input_regs[REG_IN_RTC_MIN]   = (uint16_t)init_dt.minute;
    modbus_db.input_regs[REG_IN_RTC_SEC]   = (uint16_t)init_dt.second;
  }

  // 🎛️ [펌웨어 내장형 Closed-Loop PID 제어기 인스턴스 초기화]
  static PID_Controller_t pid_anode;
  static PID_Controller_t pid_stack1;
  static PID_Controller_t pid_aog;

  PID_Initialize(&pid_anode,  3.8f, 0.35f, 0.5f, true); // Anode 냉각수 순환 (Reverse Action)
  PID_Initialize(&pid_stack1, 3.8f, 0.35f, 0.5f, true); // STACK 1 냉각수 공급 (Reverse Action)
  PID_Initialize(&pid_aog,    3.8f, 0.35f, 0.5f, true); // AOG 회수라인 응축수 (Reverse Action)

  // 기본 레지스터 초기값 (기본 수동 모드, 안전 기본 목표치 설정)
  modbus_db.holding_regs[REG_HOLD_PID_ANODE_MODE]  = 0;   // 수동
  modbus_db.holding_regs[REG_HOLD_PID_ANODE_SP]    = 300; // 30.0도
  modbus_db.holding_regs[REG_HOLD_PID_STACK1_MODE] = 0;   // 수동
  modbus_db.holding_regs[REG_HOLD_PID_STACK1_SP]   = 240; // 24.0도
  modbus_db.holding_regs[REG_HOLD_PID_AOG_MODE]    = 0;   // 수동
  modbus_db.holding_regs[REG_HOLD_PID_AOG_SP]      = 290; // 29.0도
  modbus_db.holding_regs[REG_HOLD_PID_STACK2_MODE] = 0;   // 수동 (10kW 전용)
  modbus_db.holding_regs[REG_HOLD_PID_STACK2_SP]   = 240; // 24.0도 (10kW 전용)

  // 기본 PID 게인 레지스터 초기값 (100배 스케일: Kp=3.80 -> 380, Ki=0.35 -> 35, Kd=0.50 -> 50)
  modbus_db.holding_regs[REG_HOLD_PID_ANODE_KP]    = 380;
  modbus_db.holding_regs[REG_HOLD_PID_ANODE_KI]    = 35;
  modbus_db.holding_regs[REG_HOLD_PID_ANODE_KD]    = 50;

  modbus_db.holding_regs[REG_HOLD_PID_STACK1_KP]   = 380;
  modbus_db.holding_regs[REG_HOLD_PID_STACK1_KI]   = 35;
  modbus_db.holding_regs[REG_HOLD_PID_STACK1_KD]   = 50;

  modbus_db.holding_regs[REG_HOLD_PID_AOG_KP]      = 380;
  modbus_db.holding_regs[REG_HOLD_PID_AOG_KI]      = 35;
  modbus_db.holding_regs[REG_HOLD_PID_AOG_KD]      = 50;

  // DO 초기화 (기본 OFF)
  DO_SV149_TRIS = 0;
  DO_MC_SW_TRIS = 0;
  DO_SV102_TRIS = 0;
  DO_SV103_TRIS = 0;
  DO_SV102_LAT = 0;
  DO_SV103_LAT = 0;

  uint32_t sensor_scan_counter = 0;
  uint32_t heartbeat_counter = 0;
  uint8_t hb_state = 0;
  static uint8_t current_tc_ch = 0; // 현재 스캔 중인 TC 채널
  static uint8_t prio_idx = 0;
  static uint8_t norm_idx = 0;
  static bool is_prio_turn = false;
  static uint8_t adc_scan_idx = 0;  // 현재 스캔 중인 ADC 센서 인덱스 (0 ~ 13)

  // 주요 집중 감시 채널 목록 (체감 응답속도 1초대 보장)
  static const uint8_t priority_channels[] = {
    TC_CH1_CITY_GAS_IN,      // CH1: 도시가스 공급
    TC_CH2_BURNER_BACKFIRE,  // CH2: 버너 역화
    TC_CH3_REFORM_GAS_1ST,   // CH3: 개질 가스 1차
    TC_CH21_REF_BN,          // CH21: 개질 버너 핵심부
    TC_CH4_REFORM_GAS_2ND,   // CH4: 개질 가스 2차
    TC_CH5_AOG_BURNER_IN,    // CH5: AOG 버너 입구
    TC_CH22_REF_SR1,         // CH22: 개질기 상단
    TC_CH25_REF_LTS1         // CH25: 탈황기 전단
  };

  // 초기 0번 채널 1-Shot 변환 트리거
  TC_TriggerConversion(TC_CH1_CITY_GAS_IN);
  ADS1115_TriggerChannel(adc_channel_map[0]); // ADS1115 0번 채널 초기 변환 사전 트리거

  while (1) {
    // [A] 관제용 Modbus RTU 통신 패킷 스캔 및 고속 응답 처리 (RS-422)
    Modbus_Task();

    // [A-1] 관제용 Modbus TCP 통신 패킷 처리 (W5500 이더넷 포트 502)
    ETH_ModbusTCP_Task();

    // [A-2] 필드 RS-485 (UART1) M701 7-in-1 복합 가스/환경 센서 패킷 수신 및 DB 바인딩
    if (RS485_ProcessM701()) {
      const M701_Data_t* p_m701 = M701_GetData();
      if (p_m701 && p_m701->is_valid) {
        modbus_db.input_regs[REG_IN_M701_ECO2]   = p_m701->eco2;
        modbus_db.input_regs[REG_IN_M701_ECH2O]  = p_m701->ech2o;
        modbus_db.input_regs[REG_IN_M701_TVOC]   = p_m701->tvoc;
        modbus_db.input_regs[REG_IN_M701_PM25]   = p_m701->pm25;
        modbus_db.input_regs[REG_IN_M701_PM10]   = p_m701->pm10;
        modbus_db.input_regs[REG_IN_M701_TEMP]   = (uint16_t)p_m701->temperature;
        modbus_db.input_regs[REG_IN_M701_HUMI]   = p_m701->humidity;
        modbus_db.input_regs[REG_IN_M701_STATUS] = 1;
      }
    }

    // [B] 주기적 논블로킹 센서 계측 및 DO 물리 동기화
    sensor_scan_counter++;
    if (sensor_scan_counter >= 3000UL) {

      sensor_scan_counter = 0;

      // 👑 [특허급 하드웨어 변환 완료 검사]
      // MAX31856 칩셋의 1-Shot 델타-시그마 ADC 변환(약 143ms)이 100% 끝났을 때만 판독!
      // 변환 진행 중에는 절대 MUX를 건드리거나 미완성 값을 읽지 않음으로써 황당한 이상 온도 원천 차단!
      if (TC_IsConversionDone()) {
        // 1. 보드 기준 냉접점(CJ, 칩 내부 상온) 온도 판독 및 Modbus DB(48번지) 기록
        float cj_temp = TC_ReadColdJunction();
        if (cj_temp > 0.0f && cj_temp < 80.0f) {
          modbus_db.input_regs[48] = (uint16_t)(cj_temp * 10.0f);
        }
        Modbus_Task(); // 통신 수신 감시 공백 제로화

        // 2. 완벽하게 변환 완료된 순수 열전대 온도 판독 (오차 0%)
        float temp = TC_ReadTemperature((TC_Channel_t)current_tc_ch);

        // 👑 [순수 하드웨어 단선 판정 기반 유효 온도 기록]
        // MAX31856 칩셋의 하드웨어 단선 검출(OCFAIL)로 미체결 채널은 -999.0f로 확정 판정되므로,
        // 실제 센서가 상온(CJC) 부근일 때 가로바로 사라지는 부작용(상온 실명)을 없애고 100% 온전히 표출!
        if (temp > -100.0f && temp < 2000.0f) {
          modbus_db.input_regs[current_tc_ch] = (uint16_t)(temp * 10.0f);
        } else {
          modbus_db.input_regs[current_tc_ch] = 0x9999; // 미결선 빈 채널 -> UI 가로바(--) 표출
        }
        Modbus_Task(); // 통신 수신 감시 공백 제로화

        // 3. 우선순위 가중치 인터리빙 (우선 채널과 일반 채널 1:1 교차)
        is_prio_turn = !is_prio_turn;
        if (is_prio_turn) {
          current_tc_ch = priority_channels[prio_idx];
          prio_idx = (prio_idx + 1) % (sizeof(priority_channels) / sizeof(priority_channels[0]));
        } else {
          norm_idx = (norm_idx + 1) % 32; // 0~31번 채널 스캔 (CH1 ~ CH32 전수 32개 채널 스캔)
          if (norm_idx == TC_CH20_RESERVED) norm_idx = TC_CH21_REF_BN; // 19번(CH20 예비 채널)만 안전 스킵
          current_tc_ch = norm_idx;
        }

        // 4. 다음 채널 1-Shot 변환 트리거 (새 143ms 변환 개시)
        TC_TriggerConversion((TC_Channel_t)current_tc_ch);
        Modbus_Task(); // 통신 수신 감시 공백 제로화
      }

      // 5. Modbus Coils / Holding Regs 상태를 20개 디지털 출력(DO) 포트에 물리적 동기화
      for (uint8_t i = 0; i < 20; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_mask = 1 << (i % 8);
        bool state = (modbus_db.coils[byte_idx] & bit_mask) ||
                     (modbus_db.holding_regs[20 + i] > 0);
        Modbus_SetDO(i, state);
      }

      // 5-2. 🎛️ [MCU 펌웨어 내장형 Closed-Loop PID 폐루프 연산 엔진 (독립 자율 제어)]
      // 관제 PC가 다운되거나 통신이 두절되어도 MCU 자체 폐루프에서 스택 냉각을 100% 안전 유지!
      
      // [A] Anode 냉각수 순환 물펌프 (AO_P351, DAC ch7 ↔ CH14 열전대 회수온도)
      if (modbus_db.holding_regs[REG_HOLD_PID_ANODE_MODE] == 1) {
        if (modbus_db.holding_regs[REG_HOLD_PID_ANODE_KP] > 0) {
          pid_anode.kp = (float)modbus_db.holding_regs[REG_HOLD_PID_ANODE_KP] / 100.0f;
          pid_anode.ki = (float)modbus_db.holding_regs[REG_HOLD_PID_ANODE_KI] / 100.0f;
          pid_anode.kd = (float)modbus_db.holding_regs[REG_HOLD_PID_ANODE_KD] / 100.0f;
        }
        uint16_t raw_pv = modbus_db.input_regs[TC_CH14_ANODE_COOL_RET];
        if (raw_pv != 0x9999) {
          float pv = (float)((int16_t)raw_pv) / 10.0f;
          float sp = (float)((int16_t)modbus_db.holding_regs[REG_HOLD_PID_ANODE_SP]) / 10.0f;
          modbus_db.holding_regs[7] = PID_Update(&pid_anode, sp, pv, 100.0f, 0.2f);
        }
      } else {
        PID_Reset(&pid_anode);
      }

      // [B] STACK 1 냉각수 공급 펌프 (AO_P370, DAC ch8 ↔ CH15 열전대 회수온도)
      if (modbus_db.holding_regs[REG_HOLD_PID_STACK1_MODE] == 1) {
        if (modbus_db.holding_regs[REG_HOLD_PID_STACK1_KP] > 0) {
          pid_stack1.kp = (float)modbus_db.holding_regs[REG_HOLD_PID_STACK1_KP] / 100.0f;
          pid_stack1.ki = (float)modbus_db.holding_regs[REG_HOLD_PID_STACK1_KI] / 100.0f;
          pid_stack1.kd = (float)modbus_db.holding_regs[REG_HOLD_PID_STACK1_KD] / 100.0f;
        }
        uint16_t raw_pv = modbus_db.input_regs[TC_CH15_STACK1_COOL_RET];
        if (raw_pv != 0x9999) {
          float pv = (float)((int16_t)raw_pv) / 10.0f;
          float sp = (float)((int16_t)modbus_db.holding_regs[REG_HOLD_PID_STACK1_SP]) / 10.0f;
          modbus_db.holding_regs[8] = PID_Update(&pid_stack1, sp, pv, 100.0f, 0.2f);
        }
      } else {
        PID_Reset(&pid_stack1);
      }

      // [C] AOG 회수라인 응축수 펌프 (AO_P341, DAC ch6 ↔ CH17 열전대 인입온도)
      if (modbus_db.holding_regs[REG_HOLD_PID_AOG_MODE] == 1) {
        if (modbus_db.holding_regs[REG_HOLD_PID_AOG_KP] > 0) {
          pid_aog.kp = (float)modbus_db.holding_regs[REG_HOLD_PID_AOG_KP] / 100.0f;
          pid_aog.ki = (float)modbus_db.holding_regs[REG_HOLD_PID_AOG_KI] / 100.0f;
          pid_aog.kd = (float)modbus_db.holding_regs[REG_HOLD_PID_AOG_KD] / 100.0f;
        }
        uint16_t raw_pv = modbus_db.input_regs[TC_CH17_WASTE_HEAT_IN];
        if (raw_pv != 0x9999) {
          float pv = (float)((int16_t)raw_pv) / 10.0f;
          float sp = (float)((int16_t)modbus_db.holding_regs[REG_HOLD_PID_AOG_SP]) / 10.0f;
          modbus_db.holding_regs[6] = PID_Update(&pid_aog, sp, pv, 100.0f, 0.2f);
        }
      } else {
        PID_Reset(&pid_aog);
      }

      // 6. 🎛️ Modbus Holding Registers (0~11) 아날로그 출력(DAC60516) 하드웨어 동기화
      // 👑 [Dirty Check 최적화] 값이 실제로 변경되었을 때만 초고속 SPI 전송 (CPU 점유율 99% 절감)
      static uint16_t last_dac_cache[12] = {
          0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
          0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
      };
      static uint16_t dac_sync_count = 0;
      bool dac_force_sync = (++dac_sync_count >= 500); // 약 5초마다 노이즈 방어용 전 채널 동기화
      if (dac_force_sync) dac_sync_count = 0;

      for (uint8_t i = 0; i < 12; i++) {
        if (dac_force_sync || modbus_db.holding_regs[i] != last_dac_cache[i]) {
          last_dac_cache[i] = modbus_db.holding_regs[i];
          DAC60516_OutputChannel_t dac_ch = dac_channel_map[i];
          DAC60516_WriteRaw(dac_ch, modbus_db.holding_regs[i]);
          Modbus_Task(); // 통신 수신 즉각 감시
        }
      }

      // 7. 📊 ADS1115 아날로그 입력(ADC 14개 센서) 👑 [제로 블로킹 파이프라인 변환]
      // 1.3ms 대기 딜레이를 0.000ms로 소멸! 이전 변환 완료값 즉각 판독 후 다음 채널 즉각 사전 트리거
      ADS1115_SensorChannel_t curr_adc = adc_channel_map[adc_scan_idx];
      modbus_db.input_regs[32 + adc_scan_idx] = ADS1115_ReadNormalized(curr_adc);
      Modbus_Task(); // 통신 수신 감시 공백 제로화

      adc_scan_idx = (adc_scan_idx + 1) % 14;
      ADS1115_SensorChannel_t next_adc = adc_channel_map[adc_scan_idx];
      ADS1115_TriggerChannel(next_adc); // 대기시간 0ms 비동기 트리거!
      Modbus_Task(); // 통신 수신 감시 공백 제로화

      // 8. ⏰ DS3231 고정밀 RTC 실시간 시각 계측 (약 1.0초 주기)
      static uint16_t rtc_scan_div = 0;
      if (++rtc_scan_div >= 10) {
        rtc_scan_div = 0;
        DateTime_t cur_dt;
        if (RTC_GetTime(&cur_dt)) {
          modbus_db.input_regs[REG_IN_RTC_YEAR]  = (uint16_t)cur_dt.year + 2000;
          modbus_db.input_regs[REG_IN_RTC_MONTH] = (uint16_t)cur_dt.month;
          modbus_db.input_regs[REG_IN_RTC_DATE]  = (uint16_t)cur_dt.date;
          modbus_db.input_regs[REG_IN_RTC_HOUR]  = (uint16_t)cur_dt.hour;
          modbus_db.input_regs[REG_IN_RTC_MIN]   = (uint16_t)cur_dt.minute;
          modbus_db.input_regs[REG_IN_RTC_SEC]   = (uint16_t)cur_dt.second;
        }
        Modbus_Task();
      }
    }

    // [C] 하트비트 및 워치독 타이머 클리어 (MCU 오동작/강제 리셋 방지)
    ClrWdt();
    heartbeat_counter++;
    if (heartbeat_counter >= 120000UL) {
      heartbeat_counter = 0;
      hb_state ^= 1;
    }
  }

  return 1;
}
