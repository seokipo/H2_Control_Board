/**
 * @file rs422.h
 * @brief RS-422 UART Communication Driver (9600bps, Full-Duplex) for H2_Control_Board
 * 
 * @note 본 드라이버는 Mcu.SchDoc 회로도의 MAX3490 트랜시버(IC2) 연동 설계 사양을 기반으로 구현되었습니다.
 *       - MON_422_TX: RF13 (물리 71번 핀) -> UART2 TX (출력 PPS)
 *       - MON_422_RX: RE10 (물리 70번 핀) -> UART2 RX (입력 PPS)
 *       - RS-422은 Full-Duplex이므로 별도의 방향 제어선(DIR)이 필요하지 않습니다.
 */

#ifndef RS422_H
#define RS422_H

#if defined(__XC16__) || defined(__XC)
    #include <xc.h>
#else
    #include <stdint.h>
    #include <stdbool.h>
    // 💡 에디터(VS Code / Antigravity IDE) 구문 검사기 빨간 밑줄 방지용 가상 레지스터
    typedef struct {
        unsigned UARTEN:1; unsigned UTXEN:1; unsigned URXEN:1;
        unsigned UTXBF:1; unsigned URXDA:1;
    } __UART_BITS;
    extern volatile uint16_t U2MODE, U2MODEH, U2STA, U2STAH, U2BRG, U2TXREG, U2RXREG;
    extern volatile __UART_BITS U2MODEbits, U2STAHbits, U2STAbits;
    extern volatile uint16_t _U2RXR, _RP93R;
    #define __builtin_write_RPCON(x) ((void)0)
#endif

#include <stdint.h>
#include <stdbool.h>

// RS-422 기본 보레이트 설정 (19200 bps 2배 고속화)
#define RS422_BAUDRATE  19200

/* ==========================================================================
 * 1. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief RS-422용 UART2 모듈 및 PPS 설정 초기화
 */
void RS422_Initialize(void);

/**
 * @brief RS-422 통신 채널로 1바이트 데이터 송신
 * @param data 송신할 8비트 데이터
 */
void RS422_WriteByte(uint8_t data);

/**
 * @brief RS-422 통신 채널로 다중 데이터 바이트 송신 (관제 프로그램 전송용)
 * @param buffer 송신 데이터 버퍼 포인터
 * @param length 송신할 바이트 크기
 */
void RS422_WriteBuffer(const uint8_t *buffer, uint16_t length);

/**
 * @brief RS-422 통신 버스로부터 수신된 데이터가 있는지 확인
 * @return true 수신 데이터 존재, false 데이터 없음
 */
bool RS422_IsRxReady(void);

/**
 * @brief 수신 버퍼에서 1바이트 데이터 읽기
 * @return uint8_t 읽어낸 수신 데이터, 없으면 0
 */
uint8_t RS422_ReadByte(void);

#endif /* RS422_H */
