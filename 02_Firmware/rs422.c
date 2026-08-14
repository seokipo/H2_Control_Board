/**
 * @file rs422.c
 * @brief RS-422 UART Communication Driver Implementation (Full-Duplex)
 * 
 * @note dsPIC33CK512MP710 MCU의 하드웨어 UART2 및 PPS 설정을 활용하여
 *       노트북/PC 관제 프로그램과의 양방향(Full-Duplex) 통신을 수행합니다.
 */

#include "rs422.h"
#include <xc.h>

// 시스템 주파수 (FOSC/2 = FCY) 가정
#ifndef FCY
#define FCY     80000000UL // 80 MHz 기본 명령 클록 주파수 설정
#endif

void RS422_Initialize(void)
{
    // [1] PPS (Peripheral Pin Select) 설정: UART2 핀 동적 할당
    // dsPIC33CK512MP710의 PPS Unlock 시퀀스 수행
    __builtin_write_RPCON(0x0000); // PPS Lock 해제
    
    // UART2 RXD2 입력 지정: 70번 핀 RE10 (PPS 입력 번호 RP42)
    _U2RXR = 42; 
    
    // UART2 TXD2 출력 지정: 71번 핀 RF13 (PPS 출력 레지스터 RP109R, 기능 코드 2: U2TX)
    _RP109R = 2; 
    
    __builtin_write_RPCON(0x0800); // PPS Lock 설정

    // [2] UART2 제어 레지스터 설정 (9600 bps, 8-N-1 설정)
    U2MODE = 0x0000;    // 8비트 데이터, 무패리티, 1 정지비트, 흐름제어 없음
    U2STA = 0x0000;     // 수신 활성화 준비
    
    // Baudrate Generator  Baudrate = FCY / (16 * U2BRG) - 1
    // 9600 bps = 80,000,000 / (16 * 9600) - 1 = 520.8 -> 521로 반올림 설정
    U2BRG = (FCY / (16UL * RS422_BAUDRATE)) - 1;

    // UART2 주변기기 활성화 및 송수신 인에이블
    U2MODEbits.UARTEN = 1; // UART2 활성화
    U2STAbits.UTXEN = 1;   // 송신 활성화
    U2STAbits.URXEN = 1;   // 수신 활성화
}

void RS422_WriteByte(uint8_t data)
{
    // [1] 송신 버퍼가 비어있을 때까지 대기
    while (U2STAbits.UTXBF);
    
    // [2] 데이터 전송 레지스터에 데이터 주입
    U2TXREG = data;
}

void RS422_WriteBuffer(const uint8_t *buffer, uint16_t length)
{
    if (buffer == 0 || length == 0)
    {
        return;
    }

    // [1] Full-Duplex이므로 방향 제어선 없이 연속 송신 수행
    for (uint16_t i = 0; i < length; i++)
    {
        while (U2STAbits.UTXBF); // 송신 버퍼 차오름 대기
        U2TXREG = buffer[i];
    }
}

bool RS422_IsRxReady(void)
{
    // UART2 수신 데이터 존재 플래그 상태 확인 (URXDA = 1 이면 버퍼에 데이터 있음)
    return (U2STAbits.URXDA == 1);
}

uint8_t RS422_ReadByte(void)
{
    // 수신 오버런 오류 해결 로직 처리 (Overrun 발생 시 수신 불통 방지)
    if (U2STAbits.OERR == 1)
    {
        U2STAbits.OERR = 0; // 에러 해제하여 수신 버퍼 클리어 및 재개
    }
    
    if (U2STAbits.URXDA == 1)
    {
        return U2RXREG;
    }
    
    return 0;
}
