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
#include <stdint.h>
#include <stdbool.h>

// 시스템 주파수 (FOSC/2 = FCY) 내부 FRC 8MHz 기준 (FCY = 4MHz)
#ifndef FCY
#define FCY     4000000UL
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
    
    // UART1 TXD1 출력 지정: 67번 핀 RD5 (PPS 출력 레지스터 RP69R, 기능 코드 1: U1TX)
    _RP69R = 1; 
    
    __builtin_write_RPCON(0x0800); // PPS Lock 설정

    // [3] UART1 제어 레지스터 설정 (9600 bps, 8-N-1 설정)
    U1MODE = 0x0000;    // 8비트 데이터, 무패리티, 1 정지비트, 흐름제어 없음
    U1MODEH = 0x0000;
    U1STA = 0x0000;     // 상태 클리어
    U1STAH = 0x0000;
    
    // Baudrate Generator  Baudrate = FCY / (16 * U1BRG) - 1
    U1BRG = (FCY / (16UL * RS485_BAUDRATE)) - 1;

    // UART1 주변기기 활성화 및 송수신 인에이블 (dsPIC33CK 구조)
    U1MODEbits.UARTEN = 1; // UART1 활성화
    U1MODEbits.UTXEN = 1;  // 송신 활성화
    U1MODEbits.URXEN = 1;  // 수신 활성화
}

void RS485_WriteByte(uint8_t data)
{
    // [1] RS-485 방향을 송신(1)으로 전환
    DO_485_DIR_LAT = 1;
    
    // [2] 송신 버퍼 빌 때까지 대기 후 데이터 쓰기
    while (U1STAHbits.UTXBF);
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
        while (U1STAHbits.UTXBF);
        U1TXREG = buffer[i];
    }
    
    // 마지막 바이트가 완전히 송출(시프트 레지스터 비움)될 때까지 완벽 대기
    while (!U1STAbits.TRMT);
    
    // [3] 수신(0) 상태로 신속 복귀
    DO_485_DIR_LAT = 0;
}

bool RS485_IsRxReady(void)
{
    // UART1 수신 데이터 존재 플래그 상태 확인 (URXBE == 0 이면 버퍼에 데이터 있음)
    return (U1STAHbits.URXBE == 0);
}

uint8_t RS485_ReadByte(void)
{
    // 수신 오버런 오류 해결 로직 처리 (Overrun 발생 시 수신 불통 방지)
    if (U1STAbits.OERR == 1)
    {
        U1STAbits.OERR = 0; // 에러 해제하여 수신 버퍼 클리어 및 재개
    }
    
    if (U1STAHbits.URXBE == 0)
    {
        return (uint8_t)U1RXREG;
    }
    
    return 0;
}

/* ==========================================================================
 * 2. M701 7-in-1 복합 가스/환경 센서 파서 구현
 * ========================================================================== */

// 최신 계측 데이터 보관 정적 인스턴스
static M701_Data_t s_m701_data = {0};

// M701 패킷 파서 내부 상태
typedef enum {
    M701_STATE_WAIT_HEADER1 = 0,
    M701_STATE_WAIT_HEADER2,
    M701_STATE_COLLECT_DATA,
    M701_STATE_WAIT_CHECKSUM
} M701_ParserState_t;

static M701_ParserState_t s_parser_state = M701_STATE_WAIT_HEADER1;
static uint8_t s_rx_buf[17];
static uint8_t s_rx_index = 0;

bool RS485_ProcessM701(void)
{
    bool new_packet_received = false;

    // 수신 버퍼에 데이터가 있는 동안 연속 처리
    while (RS485_IsRxReady())
    {
        uint8_t byte = RS485_ReadByte();

        switch (s_parser_state)
        {
            case M701_STATE_WAIT_HEADER1:
                if (byte == 0x3C)
                {
                    s_rx_buf[0] = byte;
                    s_parser_state = M701_STATE_WAIT_HEADER2;
                }
                break;

            case M701_STATE_WAIT_HEADER2:
                if (byte == 0x02)
                {
                    s_rx_buf[1] = byte;
                    s_rx_index = 2; // B3(인덱스 2)부터 수집 시작
                    s_parser_state = M701_STATE_COLLECT_DATA;
                }
                else if (byte == 0x3C)
                {
                    // 헤더 바이트 재인입 처리
                    s_rx_buf[0] = byte;
                    s_parser_state = M701_STATE_WAIT_HEADER2;
                }
                else
                {
                    s_parser_state = M701_STATE_WAIT_HEADER1;
                }
                break;

            case M701_STATE_COLLECT_DATA:
                s_rx_buf[s_rx_index++] = byte;
                if (s_rx_index == 16) // B16까지 수집 완료
                {
                    s_parser_state = M701_STATE_WAIT_CHECKSUM;
                }
                break;

            case M701_STATE_WAIT_CHECKSUM:
            {
                s_rx_buf[16] = byte; // B17 Check Code

                // [1] 체크코드 무결성 검증 (B1 ~ B16 바이트 합의 하위 8비트)
                uint8_t calc_chk = 0;
                for (uint8_t i = 0; i < 16; i++)
                {
                    calc_chk += s_rx_buf[i];
                }

                if (calc_chk == byte)
                {
                    // [2] 7대 복합 센서 데이터 파싱
                    // B3, B4 : eCO2
                    s_m701_data.eco2  = ((uint16_t)s_rx_buf[2] << 8) | s_rx_buf[3];
                    // B5, B6 : eCH2O
                    s_m701_data.ech2o = ((uint16_t)s_rx_buf[4] << 8) | s_rx_buf[5];
                    // B7, B8 : TVOC
                    s_m701_data.tvoc  = ((uint16_t)s_rx_buf[6] << 8) | s_rx_buf[7];
                    // B9, B10 : PM2.5
                    s_m701_data.pm25  = ((uint16_t)s_rx_buf[8] << 8) | s_rx_buf[9];
                    // B11, B12 : PM10
                    s_m701_data.pm10  = ((uint16_t)s_rx_buf[10] << 8) | s_rx_buf[11];

                    // B13, B14 : 온도 (B13 bit7이 1이면 영하)
                    uint8_t temp_int_byte = s_rx_buf[12];
                    uint8_t temp_dec_byte = s_rx_buf[13];
                    bool is_negative = (temp_int_byte & 0x80) != 0;
                    int16_t temp_int = (temp_int_byte & 0x7F);
                    int16_t temp_calc = (temp_int * 10) + (temp_dec_byte % 10);
                    if (is_negative)
                    {
                        temp_calc = -temp_calc;
                    }
                    s_m701_data.temperature = temp_calc;

                    // B15, B16 : 습도 (0.1% 단위 스케일)
                    s_m701_data.humidity = ((uint16_t)s_rx_buf[14] * 10) + (s_rx_buf[15] % 10);

                    s_m701_data.is_valid = true;
                    s_m701_data.rx_count++;
                    new_packet_received = true;
                }

                // 다음 패킷 수신 대기 상태로 리셋
                s_parser_state = M701_STATE_WAIT_HEADER1;
                s_rx_index = 0;
                break;
            }

            default:
                s_parser_state = M701_STATE_WAIT_HEADER1;
                s_rx_index = 0;
                break;
        }
    }

    return new_packet_received;
}

const M701_Data_t* M701_GetData(void)
{
    return &s_m701_data;
}
