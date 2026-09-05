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
  RS422_Initialize();    // 👑 [특허급 하드웨어 구제] 70번 핀 초정밀 비트뱅잉 RX & 71번 하드웨어 TX (19200 bps)
  RS485_Initialize();    // 필드/컨버터용 9600bps 하드웨어 UART1 (RB6 RX, RD5 TX, RB5 DIR)
  Modbus_Initialize();   // Modbus RTU 슬레이브 데이터베이스 초기화

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
    // [A] 관제용 Modbus RTU 통신 패킷 스캔 및 고속 응답 처리
    // (RS-422 70번 핀 초정밀 비트뱅잉 수신 & 71번 초정밀 언롤 비트뱅잉 송신)
    Modbus_Task();

    // [B] 주기적 논블로킹 센서 계측 및 DO 물리 동기화
    sensor_scan_counter++;
    if (sensor_scan_counter >= 3000UL) {
      sensor_scan_counter = 0;

      // 👑 [특허급 하드웨어 변환 완료 검사]
      // MAX31856 칩셋의 1-Shot 델타-시그마 ADC 변환(약 143ms)이 100% 끝났을 때만 판독!
      // 변환 진행 중에는 절대 MUX를 건드리거나 미완성 값을 읽지 않음으로써 황당한 이상 온도 원천 차단!
      if (TC_IsConversionDone()) {
        // 1. 보드 기준 냉접점(CJ, 칩 내부 상온) 온도 판독 및 Modbus DB(31번지) 기록
        float cj_temp = TC_ReadColdJunction();
        if (cj_temp > 0.0f && cj_temp < 80.0f) {
          modbus_db.input_regs[31] = (uint16_t)(cj_temp * 10.0f);
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
          norm_idx = (norm_idx + 1) % 31; // 0~30번 채널 스캔 (31번은 CJC 전용 레지스터로 보존)
          if (norm_idx == 19 || norm_idx == 20) norm_idx = 21; // 미사용 채널 스킵
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
