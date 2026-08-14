/**
 * @file rs485.c
 * @brief RS-485 UART Communication Driver Implementation (Half-Duplex)
 * 
 * @note dsPIC33CK512MP710 MCU의 하드웨어 UART1 및 PPS 설정, 
 *       그리고 pin_map.h의 DO_485_DIR 방향 전환 핀(RB5)을 활용하여 Half-Duplex 485 통신을 수행합니다.
 */

#include "rs485.h"
#include "pin_map.h"
#include <xc.h>

// 시스템 주파수 (FOSC/2 = FCY) 가정
#ifndef FCY
#define FCY     80000000UL // 80 MHz 기본 명령 클록 주파수 설정
#endif

void RS485_Initialize(void)
{
    // [1] 방향 제어 핀(RB5)을 출력(0)으로 설정하고, 우선 수신 상태(0)로 대기
    DO_485_DIR_LAT = 0;
    DO_485_DIR_TRIS = 0;

    // [2] PPS (Peripheral Pin Select) 설정: UART1 핀 동적 할당
    // dsPIC33CK512MP710의 PPS Unlock 시퀀스 수행
    __builtin_write_RPCON(0x0000); // PPS Lock 해제
    
    // UART1 RXD1 입력 지정: 69번 핀 RB6 (PPS 입력 번호 RP38)
    _U1RXR = 38; 
    
    // UART1 TXD1 출력 지정: 67번 핀 RD5 (PPS 출력 레지스터 RP53R, 기능 코드 1: U1TX)
    _RP53R = 1; 
    
    __builtin_write_RPCON(0x0800); // PPS Lock 설정

    // [3] UART1 제어 레지스터 설정 (9600 bps, 8-N-1 설정)
    U1MODE = 0x0000;    // 8비트 데이터, 무패리티, 1 정지비트, 흐름제어 없음
    U1STA = 0x0000;     // 수신 활성화 준비
    
    // Baudrate Generator  Baudrate = FCY / (16 * U1BRG) - 1
    // 9600 bps = 80,000,000 / (16 * 9600) - 1 = 520.8 -> 521로 반올림 설정
    U1BRG = (FCY / (16UL * RS485_BAUDRATE)) - 1;

    // UART1 주변기기 활성화 및 송수신 인에이블
    U1MODEbits.UARTEN = 1; // UART1 활성화
    U1STAbits.UTXEN = 1;   // 송신 활성화
    U1STAbits.URXEN = 1;   // 수신 활성화
}

void RS485_WriteByte(uint8_t data)
{
    // [1] RS-485 방향을 송신(1)으로 전환
    DO_485_DIR_LAT = 1;
    
    // MUX/Transceiver 안정화 딜레이 (10us 내외)
    // (실무상 딜레이 루프 사용)
    
    // [2] 데이터 레지스터에 쓰기 및 송신 완료 대기
    U1TXREG = data;
    
    // 송신 시프트 레지스터 비움(TRMT)까지 완벽 대기 (바이트 잘림 방지 필수!)
    while (!U1STAbits.TRMT);
    
    // [3] 송신이 끝나면 즉시 수신(0) 상태로 복귀
    DO_485_DIR_LAT = 0;
}

void RS485_WriteBuffer(const uint8_t *buffer, uint16_t length)
{
    if (buffer == 0 || length == 0)
    {
        return;
    }

    // [1] 송신을 위해 방향 제어선을 1(TX)로 설정
    DO_485_DIR_LAT = 1;

    // [2] 연속 바이트 송신 수행
    for (uint16_t i = 0; i < length; i++)
    {
        U1TXREG = buffer[i];
        
        // 송신 버퍼가 꽉 찼는지 확인 후 대기
        while (U1STAbits.UTXBF);
    }
    
    // 마지막 바이트가 완전히 송출(시프트 레지스터 비움)될 때까지 완벽 대기
    while (!U1STAbits.TRMT);
    
    // [3] 수신(0) 상태로 신속 복귀
    DO_485_DIR_LAT = 0;
}

bool RS485_IsRxReady(void)
{
    // UART1 수신 데이터 존재 플래그 상태 확인 (URXDA = 1 이면 버퍼에 데이터 있음)
    return (U1STAbits.URXDA == 1);
}

uint8_t RS485_ReadByte(void)
{
    // 수신 오버런 오류 해결 로직 처리 (Overrun 발생 시 수신 불통 방지)
    if (U1STAbits.OERR == 1)
    {
        U1STAbits.OERR = 0; // 에러 해제하여 수신 버퍼 클리어 및 재개
    }
    
    if (U1STAbits.URXDA == 1)
    {
        return U1RXREG;
    }
    
    return 0;
}
