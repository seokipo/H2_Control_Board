/**
 * @file rs485.h
 * @brief RS-485 UART Communication Driver (9600bps, Half-Duplex) for H2_Control_Board
 * 
 * @note 본 드라이버는 Mcu.SchDoc 회로도의 MAX3485 트랜시버(IC4) 연동 설계 사양을 기반으로 구현되었습니다.
 *       - 485_TX : RD5 (물리 67번 핀) -> UART1 TX
 *       - 485_RX : RB6 (물리 69번 핀) -> UART1 RX
 *       - 485_DIR: RB5 (물리 68번 핀) -> Half-Duplex 방향 제어선 (1: TX, 0: RX)
 */

#ifndef RS485_H
#define RS485_H

#include <stdint.h>
#include <stdbool.h>

// RS-485 기본 보레이트 설정
#define RS485_BAUDRATE  9600

/* ==========================================================================
 * 1. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief RS-485용 UART1 모듈 및 방향 제어 GPIO 설정 초기화
 */
void RS485_Initialize(void);

/**
 * @brief RS-485 통신 버스로 1바이트 데이터 송신
 * @param data 송신할 8비트 데이터
 */
void RS485_WriteByte(uint8_t data);

/**
 * @brief RS-485 통신 버스로 다중 데이터 바이트 송신 (문자열 또는 패킷)
 * @param buffer 송신 데이터 버퍼 포인터
 * @param length 송신할 바이트 크기
 */
void RS485_WriteBuffer(const uint8_t *buffer, uint16_t length);

/**
 * @brief RS-485 통신 버스로부터 수신된 데이터가 있는지 확인
 * @return true 수신 데이터 존재, false 데이터 없음
 */
bool RS485_IsRxReady(void);

/**
 * @brief 수신 버퍼에서 1바이트 데이터 읽기
 * @return uint8_t 읽어낸 수신 데이터, 없으면 0
 */
uint8_t RS485_ReadByte(void);

#endif /* RS485_H */
