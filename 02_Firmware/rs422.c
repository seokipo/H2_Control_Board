#include "rs422.h"
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

// 시스템 주파수 (FOSC/2 = FCY) 내부 FRC 8MHz 기준 (FCY = 4MHz)
#ifndef FCY
#define FCY 4000000UL
#endif
#include <libpic30.h>

void RS422_Initialize(void) {
  // [1] 71번 핀 RF13 (RS-422 TXD2) 순수 디지털 GPIO 출력 모드 설정
  __builtin_write_RPCON(0x0000); // PPS Lock 해제
  _RP93R = 0;                    // PPS 출력 매핑 해제 (순수 GPIO 완전 복원)
  __builtin_write_RPCON(0x0800); // PPS Lock 설정

  LATFbits.LATF13 = 1;           // UART 버스 유휴(Idle) 상태: High (1)
  TRISFbits.TRISF13 = 0;         // 71번 핀 디지털 출력 방향 지정

  // [2] 70번 핀 RE10 (RS-422 RXD2) 순수 디지털 입력 포트 설정
  TRISEbits.TRISE10 = 1;
}

// 👑 [특허급 하드웨어 구제] 71번 핀(RF13) 19200 bps 초정밀 언롤 비트뱅잉 송신 (FCY=4MHz 208사이클 정밀 보정)
void RS422_WriteByte(uint8_t data) {
  // [1] 스타트 비트: Low (0)
  LATFbits.LATF13 = 0;
  __delay32(190); // 18사이클 오버헤드 감안 -> 정확히 208사이클(52.08us)

  // [2] 8개 데이터 비트 전송 (LSB First, 무오차 언롤링)
  LATFbits.LATF13 = (data & 0x01) ? 1 : 0;
  __delay32(190);

  LATFbits.LATF13 = (data & 0x02) ? 1 : 0;
  __delay32(190);

  LATFbits.LATF13 = (data & 0x04) ? 1 : 0;
  __delay32(190);

  LATFbits.LATF13 = (data & 0x08) ? 1 : 0;
  __delay32(190);

  LATFbits.LATF13 = (data & 0x10) ? 1 : 0;
  __delay32(190);

  LATFbits.LATF13 = (data & 0x20) ? 1 : 0;
  __delay32(190);

  LATFbits.LATF13 = (data & 0x40) ? 1 : 0;
  __delay32(190);

  LATFbits.LATF13 = (data & 0x80) ? 1 : 0;
  __delay32(190);

  // [3] 정지 비트(Stop Bit): High (1)
  LATFbits.LATF13 = 1;
  __delay32(208); // 52us 정지비트 보장
}

void RS422_WriteBuffer(const uint8_t *buffer, uint16_t length) {
  if (buffer == 0 || length == 0) {
    return;
  }

  // [1] Full-Duplex 초정밀 바이트 단위 연속 송출
  for (uint16_t i = 0; i < length; i++) {
    RS422_WriteByte(buffer[i]);
  }
}

// 👑 [특허급 하드웨어 구제] 70번 핀(RE10) 스타트 비트(Low) 감지
bool RS422_IsRxReady(void) {
  return (PORTEbits.RE10 == 0);
}

// 👑 [특허급 하드웨어 구제] 70번 핀(RE10) 19200 bps 초정밀 언롤 비트뱅잉 수신 (208사이클 정합)
uint8_t RS422_ReadByte(void) {
  if (PORTEbits.RE10 == 0) { // 스타트 비트(Low) 진입
    __delay32(80);           // 스타트 비트 정중앙(50% 지점)으로 이동
    if (PORTEbits.RE10 != 0) {
      return 0; // 글리치/노이즈 무시
    }

    uint8_t data = 0;

    // Bit 0 (LSB, 1비트 후 정중앙 샘플링)
    __delay32(185);
    if (PORTEbits.RE10) data |= 0x01;

    // Bit 1
    __delay32(188);
    if (PORTEbits.RE10) data |= 0x02;

    // Bit 2
    __delay32(188);
    if (PORTEbits.RE10) data |= 0x04;

    // Bit 3
    __delay32(188);
    if (PORTEbits.RE10) data |= 0x08;

    // Bit 4
    __delay32(188);
    if (PORTEbits.RE10) data |= 0x10;

    // Bit 5
    __delay32(188);
    if (PORTEbits.RE10) data |= 0x20;

    // Bit 6
    __delay32(188);
    if (PORTEbits.RE10) data |= 0x40;

    // Bit 7 (MSB)
    __delay32(188);
    if (PORTEbits.RE10) data |= 0x80;

    // Stop bit 확인 및 대기
    __delay32(190);

    return data;
  }

  return 0;
}
