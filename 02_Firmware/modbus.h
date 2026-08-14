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
    // 0~30: thermocouple 다채널 측정 온도 (T/K타입 31개 채널)
    // 32~47: ads1115 ADC 아날로그 입력 값 16개
    uint16_t input_regs[80];

    // 16비트 아날로그 홀딩 레지스터 (4XXXX 영역, Read/Write)
    // 0~11: dac60516 12개 출력 채널 전압 설정량 (0 ~ 65535 스케일)
    // 12~19: 온도 및 압력 루프 제어 설정값들
    uint16_t holding_regs[40];

    // 1비트 디지털 코일 레지스터 (0XXXX 영역, Read/Write)
    // 0~31: pin_map.h의 히터 및 MV/SV 디지털 출력 상태값
    uint8_t coils[4]; // 32개 비트 상태

    // 1비트 디지털 입력 레지스터 (1XXXX 영역, Read-Only)
    // 0~15: pin_map.h의 히터 경고 및 버너 화염/경보 입력값
    uint8_t discrete_inputs[2]; // 16개 비트 상태
} Modbus_Registers_t;

extern Modbus_Registers_t modbus_db;

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

#endif /* MODBUS_H */
