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

#if defined(__XC16__) || defined(__XC)
#include <xc.h>
#endif
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

// 80 MHz FCY 설정용
#ifndef FCY
#define FCY 80000000UL
#endif

// 센서 스캐닝 및 갱신 주기 제어용 카운터 (메인 루프 분주)
static uint32_t sensor_scan_counter = 0;

int main(void) {
  // [1] 시스템 최초 부팅 후 모든 드라이버 모듈 일괄 초기화
  GPIO_Initialize(); // 디지털 및 MUX 방향 제어 포트 초기화

  // I2C, SPI 통신 기반 외부 칩셋들 디바이스 초기 구동
  TC_Initialize();       // MAX31856 및 MUX 스캐너 시작
  ADS1115_Initialize();  // ADS1115 16채널 ADC 초기화
  DAC60516_Initialize(); // DAC60516 12채널 아날로그 출력 초기화
  ETH_Initialize(); // 듀얼 W5500 이더넷 컨트롤러 시작 (SPI2 및 CS/RESET 초기화)
  FLASH_Initialize(); // W25Q256 SPI 플래시 메모리 시작 (SPI3 및 CS 초기화)
  RTC_Initialize();   // DS3231 고정밀 RTC 시작 (I2C1 및 인터럽트 핀 초기화)

  // 시리얼 UART 채널 및 Modbus 프로토콜 슬레이브 데이터베이스 초기화
  RS485_Initialize();  // 현장 센서 연동용 RS-485 구동 (9600 bps)
  RS422_Initialize();  // 관제 노트북 PC 연동용 RS-422 구동 (9600 bps)
  Modbus_Initialize(); // 슬레이브 레지스터 및 송수신 버퍼 클리어

  // Watchdog Timer 클리어 및 동작 대기
  ClrWdt();

  while (1) {
    // [2] 관제용 Modbus RTU 통신 패킷 스캔 및 레지스터 갱신 수행
    // RS-422 Full-Duplex 수신 상태를 상시 파싱하여 실시간 명령을 처리하고,
    // 노트북에서 레지스터 0~11번지를 수정 시 DAC60516을 즉각 하드웨어
    // 동기화합니다.
    Modbus_Task();

    // [3] 주기적 아날로그 센서 및 열전대 온도 스캔 연동 (약 200ms 주기로
    // 스케줄링)
    sensor_scan_counter++;
    if (sensor_scan_counter >= 30000UL) // 메인 루프 기준 시간 지연 분주
    {
      sensor_scan_counter = 0;

      // A. 31개 열전대 온도 계측 및 Modbus DB 바인딩 (input_regs 0 ~ 30)
      // 소수점 1자리 정밀도를 보존하기 위해 온도를 10배 곱한 정수로 레지스터에
      // 기록합니다. 예: 25.4도는 254로 전송되어 관제 프로그램에서 10.0으로
      // 나누어 소수 표현.
      for (uint8_t ch = 0; ch < TC_MAX_CHANNELS; ch++) {
        float temp = TC_ReadTemperature((TC_Channel_t)ch);
        if (temp > -200.0f && temp < 2000.0f) // 비정상 에러 온도 범위 예외 처리
        {
          modbus_db.input_regs[ch] = (uint16_t)(temp * 10.0f);
        } else {
          modbus_db.input_regs[ch] = 0x9999; // 센서 단선 또는 에러 코드 표시
        }
      }

      // B. 16채널 ADS1115 ADC 전압 계측 및 Modbus DB 바인딩 (input_regs 32 ~
      // 47) 4개의 ADS1115 칩셋(0x48~0x4B)을 순차 리드하여 16비트 원시 ADC
      // 코드를 입력 레지스터에 대입합니다.
      for (uint8_t ch = 0; ch < 16; ch++) {
        uint16_t adc_raw = ADS1115_ReadChannel((ADS1115_SensorChannel_t)ch);
        modbus_db.input_regs[32 + ch] = adc_raw;
      }

      // B-2. DS3231 RTC 실시간 시각 획득 및 Modbus DB 바인딩 (input_regs 50 ~
      // 55)
      DateTime_t current_time;
      if (RTC_GetTime(&current_time)) {
        modbus_db.input_regs[50] = current_time.year;
        modbus_db.input_regs[51] = current_time.month;
        modbus_db.input_regs[52] = current_time.date;
        modbus_db.input_regs[53] = current_time.hour;
        modbus_db.input_regs[54] = current_time.minute;
        modbus_db.input_regs[55] = current_time.second;
      }

      // C. 디지털 입력 포트(DI) 비트 상태를 Modbus Discrete Inputs DB에
      // 업데이트 0xFFFF 리셋 후 pin_map.h 입력 상태 비트 팩킹
      uint16_t di_pack = 0;
      if (DI_HT193_WD_PORT)
        di_pack |= (1 << 0);
      if (DI_HT194_WD_PORT)
        di_pack |= (1 << 1);
      if (DI_HT195_WD_PORT)
        di_pack |= (1 << 2);
      if (DI_HT196_WD_PORT)
        di_pack |= (1 << 3);
      if (DI_FD176_PORT)
        di_pack |= (1 << 4);
      if (DI_FAN504_PORT)
        di_pack |= (1 << 5);
      if (DI_GD501_PORT)
        di_pack |= (1 << 6);
      if (DI_GD502_PORT)
        di_pack |= (1 << 7);
      if (POWER_FAIL_DET_PORT == 0)
        di_pack |= (1 << 8); // 정전 감지 시 비트 셋

      modbus_db.discrete_inputs[0] = (uint8_t)(di_pack & 0xFF);
      modbus_db.discrete_inputs[1] = (uint8_t)((di_pack >> 8) & 0xFF);

      // D. Modbus Coils / Holding Regs 상태를 20개 디지털 출력(DO) 포트에
      // 물리적 동기화 노트북 관제단에서 코일/홀딩 레지스터 상태를 쓰면 MCU의
      // 실제 핀 릴레이 20채널을 스위칭합니다.
      for (uint8_t i = 0; i < 20; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_mask = 1 << (i % 8);
        bool state = (modbus_db.coils[byte_idx] & bit_mask) ||
                     (modbus_db.holding_regs[20 + i] > 0);
        Modbus_SetDO(i, state);
      }
    }

    // [4] Watchdog Timer 리셋하여 오동작으로 인한 강제 MCU 리셋 방지
    ClrWdt();
  }

  return 1;
}
