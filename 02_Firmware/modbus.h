/**
 * @file modbus.h
 * @brief Modbus RTU Slave Protocol Stack for H2_Control_Board
 * 
 * @note 본 헤더는 관제 PC(RS-422) 및 내부 장치(RS-485) 연동용 Modbus RTU 슬레이브 스택의 
 *       레지스터 매핑 및 프로토콜 분석 함수를 선언합니다.
 */

#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>
#include <stdbool.h>

// Modbus RTU 슬레이브 기본 국번 (Slave Address)
#define MODBUS_SLAVE_ADDR       0x01

// Modbus RTU 최대 패킷 버퍼 크기
#define MODBUS_BUFFER_SIZE      256

/* ==========================================================================
 * 1. Modbus RTU 표준 펑션 코드 정의
 * ========================================================================== */
#define MODBUS_FC_READ_COILS            0x01 // Coils 상태 읽기
#define MODBUS_FC_READ_DISCRETE_INPUTS  0x02 // Discrete Inputs 상태 읽기
#define MODBUS_FC_READ_HOLDING_REGS     0x03 // Holding Registers 값 읽기
#define MODBUS_FC_READ_INPUT_REGS       0x04 // Input Registers 값 읽기
#define MODBUS_FC_WRITE_SINGLE_COIL     0x05 // Single Coil 값 쓰기
#define MODBUS_FC_WRITE_SINGLE_REG      0x06 // Single Register 값 쓰기
#define MODBUS_FC_WRITE_MULTIPLE_REGS   0x10 // Multiple Registers 값 쓰기

/* ==========================================================================
 * 2. Modbus 예외 코드 정의
 * ========================================================================== */
#define MODBUS_ERR_ILLEGAL_FUNCTION     0x01
#define MODBUS_ERR_ILLEGAL_DATA_ADDR    0x02
#define MODBUS_ERR_ILLEGAL_DATA_VAL     0x03
#define MODBUS_ERR_SLAVE_DEVICE_FAILURE 0x04

/* ==========================================================================
 * 3. Modbus RTU 슬레이브 레지스터 맵 및 전역 통신 버퍼 구조체
 * ========================================================================== */
typedef struct {
    // 16비트 아날로그 입력 레지스터 (3XXXX 영역, Read-Only)
    // 0~31: thermocouple 다채널 측정 온도 (CH1~CH32 총 32개 채널)
    // 32~47: ads1115 ADC 아날로그 입력 값 16개
    // 48: MAX31856 보드 냉접점(CJC) 상온
    // 50~55: DS3231 RTC 날짜/시간
    // 56~62: M701 7-in-1 복합 가스 및 환경 센서 계측치
    uint16_t input_regs[80];

    // 16비트 아날로그 홀딩 레지스터 (4XXXX 영역, Read/Write)
    // 0~11: dac60516 12개 출력 채널 전압 설정량 (0 ~ 65535 스케일)
    // 12~19: 온도 및 압력 루프 제어 설정값들 (Mode, SP)
    // 20~39: DO 디지털 릴레이/솔밸브 출력 설정 (index 0~19)
    // 40~46: DS3231 RTC 시간 동기화 설정 (Year, Month, Date, Hour, Min, Sec, Trigger)
    // 50~61: Closed-Loop PID 동적 게인 주입 (Kp, Ki, Kd, 값/100.0 스케일)
    uint16_t holding_regs[80];

    // 1비트 디지털 코일 레지스터 (0XXXX 영역, Read/Write)
    // 0~31: pin_map.h의 히터 및 MV/SV 디지털 출력 상태값
    uint8_t coils[4]; // 32개 비트 상태

    // 1비트 디지털 입력 레지스터 (1XXXX 영역, Read-Only)
    // 0~15: pin_map.h의 히터 경고 및 버너 화염/경보 입력값
    uint8_t discrete_inputs[2]; // 16개 비트 상태
} Modbus_Registers_t;

/* ==========================================================================
 * 4. Closed-Loop PID 제어 Holding Register 매핑 상수 (40013 ~ 40020)
 * ========================================================================== */
#define REG_HOLD_PID_ANODE_MODE     12  // Anode 냉각수 순환 펌프(AO_P351, DAC ch7) 제어 모드 (0: 수동, 1: AUTO)
#define REG_HOLD_PID_ANODE_SP       13  // Anode 냉각수 목표 온도 SP (0.1도 단위 부호화 정수, 30.0도 -> 300)
#define REG_HOLD_PID_STACK1_MODE    14  // STACK 1 냉각수 공급 펌프(AO_P370, DAC ch8) 제어 모드 (0: 수동, 1: AUTO)
#define REG_HOLD_PID_STACK1_SP      15  // STACK 1 냉각수 목표 온도 SP (0.1도 단위 부호화 정수, 24.0도 -> 240)
#define REG_HOLD_PID_AOG_MODE       16  // AOG 응축수 펌프(AO_P341, DAC ch6) 제어 모드 (0: 수동, 1: AUTO)
#define REG_HOLD_PID_AOG_SP         17  // AOG 응축수 목표 온도 SP (0.1도 단위 부호화 정수, 29.0도 -> 290)
#define REG_HOLD_PID_STACK2_MODE    18  // STACK 2 냉각수 공급 펌프(AO_P375, DAC ch9) 제어 모드 (10kW 전용)
#define REG_HOLD_PID_STACK2_SP      19  // STACK 2 냉각수 목표 온도 SP (10kW 전용)

/* ==========================================================================
 * 5. Closed-Loop PID 실시간 게인 Holding Register 매핑 상수 (40051 ~ 40062)
 *    값 스케일: 100배 정수 (예: Kp=3.80 -> 380, Ki=0.35 -> 35, Kd=0.50 -> 50)
 * ========================================================================== */
#define REG_HOLD_PID_ANODE_KP       50  // Anode 냉각수 Kp (Holding 40051)
#define REG_HOLD_PID_ANODE_KI       51  // Anode 냉각수 Ki (Holding 40052)
#define REG_HOLD_PID_ANODE_KD       52  // Anode 냉각수 Kd (Holding 40053)

#define REG_HOLD_PID_STACK1_KP      53  // STACK 1 냉각수 Kp (Holding 40054)
#define REG_HOLD_PID_STACK1_KI      54  // STACK 1 냉각수 Ki (Holding 40055)
#define REG_HOLD_PID_STACK1_KD      55  // STACK 1 냉각수 Kd (Holding 40056)

#define REG_HOLD_PID_STACK2_KP      56  // STACK 2 냉각수 Kp (Holding 40057)
#define REG_HOLD_PID_STACK2_KI      57  // STACK 2 냉각수 Ki (Holding 40058)
#define REG_HOLD_PID_STACK2_KD      58  // STACK 2 냉각수 Kd (Holding 40059)

#define REG_HOLD_PID_AOG_KP         59  // AOG 응축수 Kp (Holding 40060)
#define REG_HOLD_PID_AOG_KI         60  // AOG 응축수 Ki (Holding 40061)
#define REG_HOLD_PID_AOG_KD         61  // AOG 응축수 Kd (Holding 40062)

/* ==========================================================================
 * 5. DS3231 RTC Input / Holding Register 매핑 상수
 * ========================================================================== */
// Input Registers (Read-Only: 30051 ~ 30056)
#define REG_IN_RTC_YEAR         50  // 년도 (예: 2026)
#define REG_IN_RTC_MONTH        51  // 월 (1~12)
#define REG_IN_RTC_DATE         52  // 일 (1~31)
#define REG_IN_RTC_HOUR         53  // 시 (0~23)
#define REG_IN_RTC_MIN          54  // 분 (0~59)
#define REG_IN_RTC_SEC          55  // 초 (0~59)

// Holding Registers (Read/Write: 40041 ~ 40047)
#define REG_HOLD_RTC_YEAR       40  // 설정 년도 (2026 또는 26)
#define REG_HOLD_RTC_MONTH      41  // 설정 월 (1~12)
#define REG_HOLD_RTC_DATE       42  // 설정 일 (1~31)
#define REG_HOLD_RTC_HOUR       43  // 설정 시 (0~23)
#define REG_HOLD_RTC_MIN        44  // 설정 분 (0~59)
#define REG_HOLD_RTC_SEC        45  // 설정 초 (0~59)
#define REG_HOLD_RTC_TRIGGER    46  // 1 기입 시 RTC_SetTime() 즉각 수행 후 0으로 자동 복귀

/* ==========================================================================
 * 5. M701 복합 가스/환경 센서 Input Register 매핑 상수 (30057 ~ 30064)
 * ========================================================================== */
#define REG_IN_M701_ECO2        56  // eCO2 농도 (ppm, 400~5000)
#define REG_IN_M701_ECH2O       57  // eCH2O 포름알데히드 (ug/m3)
#define REG_IN_M701_TVOC        58  // TVOC 농도 (ug/m3)
#define REG_IN_M701_PM25        59  // PM2.5 초미세먼지 (ug/m3)
#define REG_IN_M701_PM10        60  // PM10 미세먼지 (ug/m3)
#define REG_IN_M701_TEMP        61  // 온도 (0.1도 단위 부호화 정수, 25.4도 -> 254)
#define REG_IN_M701_HUMI        62  // 습도 (0.1% 단위, 50.2% -> 502)
#define REG_IN_M701_STATUS      63  // 센서 통신 상태 플래그 (1: 정상 수신)

#include "dac60516.h"
#include "rtc.h"

extern Modbus_Registers_t modbus_db;

extern const DAC60516_OutputChannel_t dac_channel_map[12];

/* ==========================================================================
 * 4. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief Modbus 레지스터 데이터베이스 변수 초기화
 */
void Modbus_Initialize(void);

/**
 * @brief 수신된 UART 바이트를 Modbus 프레임 버퍼에 누적 처리하는 함수
 * @param rx_byte UART 수신 1바이트
 */
void Modbus_ProcessRxByte(uint8_t rx_byte);

/**
 * @brief Modbus 3.5T 프레임 타임아웃 경과 처리 및 수신 완료 프레임 분석 루프
 * @note 메인 루프 또는 주기적 타이머 인터럽트 내에서 주기적으로 호출되어야 합니다.
 */
void Modbus_Task(void);

/**
 * @brief Modbus RTU 표준 패킷의 16비트 CRC 계산
 * @param buffer 계산 대상 데이터 버퍼
 * @param length 데이터 길이
 * @return uint16_t 계산된 16비트 CRC 값
 */
uint16_t Modbus_CRC16(const uint8_t *buffer, uint16_t length);

/**
 * @brief 특정 DO 디지털 출력 채널 물리 핀(LAT) 즉각 제어
 * @param index DO 인덱스 (0 ~ 19)
 * @param state true(ON), false(OFF)
 */
void Modbus_SetDO(uint8_t index, bool state);

#endif /* MODBUS_H */
