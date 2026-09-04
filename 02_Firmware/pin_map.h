/**
 * @file pin_map.h
 * @brief H2_Control_Board dsPIC33CK512MP710 GPIO Pin Mapping & Initialization
 *
 * @note 본 헤더 파일은 회로도 Mcu.SchDoc 분석을 기반으로 I/O 네트 및 물리 핀
 * 정보를 매핑한 파일입니다.
 *       - DO_ : 디지털 출력 (TRIS = 0)
 *       - DI_ : 디지털 입력 (TRIS = 1)
 */

#ifndef PIN_MAP_H
#define PIN_MAP_H

#if defined(__XC16__) || defined(__XC)
#include <xc.h>
#else
// 💡 에디터(VS Code / Antigravity IDE) 구문 검사기(IntelliSense) 빨간 밑줄
// 방지용 dsPIC33CK 가상 레지스터 맵
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  unsigned LAT0 : 1;
  unsigned LAT1 : 1;
  unsigned LAT2 : 1;
  unsigned LAT3 : 1;
  unsigned LAT4 : 1;
  unsigned LAT5 : 1;
  unsigned LAT6 : 1;
  unsigned LAT7 : 1;
  unsigned LAT8 : 1;
  unsigned LAT9 : 1;
  unsigned LAT10 : 1;
  unsigned LAT11 : 1;
  unsigned LAT12 : 1;
  unsigned LAT13 : 1;
  unsigned LAT14 : 1;
  unsigned LAT15 : 1;
} __SFR_BITS;

typedef struct {
  unsigned TRIS0 : 1;
  unsigned TRIS1 : 1;
  unsigned TRIS2 : 1;
  unsigned TRIS3 : 1;
  unsigned TRIS4 : 1;
  unsigned TRIS5 : 1;
  unsigned TRIS6 : 1;
  unsigned TRIS7 : 1;
  unsigned TRIS8 : 1;
  unsigned TRIS9 : 1;
  unsigned TRIS10 : 1;
  unsigned TRIS11 : 1;
  unsigned TRIS12 : 1;
  unsigned TRIS13 : 1;
  unsigned TRIS14 : 1;
  unsigned TRIS15 : 1;
} __TRIS_BITS;

typedef struct {
  unsigned RA0 : 1;
  unsigned RA1 : 1;
  unsigned RA2 : 1;
  unsigned RA3 : 1;
  unsigned RA4 : 1;
  unsigned RA5 : 1;
  unsigned RA6 : 1;
  unsigned RA7 : 1;
  unsigned RA8 : 1;
  unsigned RA9 : 1;
  unsigned RA10 : 1;
  unsigned RA11 : 1;
  unsigned RA12 : 1;
  unsigned RA13 : 1;
  unsigned RA14 : 1;
  unsigned RA15 : 1;
  unsigned LAT0 : 1;
  unsigned LAT1 : 1;
  unsigned LAT2 : 1;
  unsigned LAT3 : 1;
  unsigned LAT4 : 1;
  unsigned LAT5 : 1;
  unsigned LAT6 : 1;
  unsigned LAT7 : 1;
  unsigned LAT8 : 1;
  unsigned LAT9 : 1;
  unsigned LAT10 : 1;
  unsigned LAT11 : 1;
  unsigned LAT12 : 1;
  unsigned LAT13 : 1;
  unsigned LAT14 : 1;
  unsigned LAT15 : 1;
  unsigned TRISA0 : 1;
  unsigned TRISA1 : 1;
  unsigned TRISA2 : 1;
  unsigned TRISA3 : 1;
  unsigned TRISA4 : 1;
  unsigned TRISA5 : 1;
  unsigned TRISA6 : 1;
  unsigned TRISA7 : 1;
  unsigned TRISA8 : 1;
  unsigned TRISA9 : 1;
  unsigned TRISA10 : 1;
  unsigned TRISA11 : 1;
  unsigned TRISA12 : 1;
  unsigned TRISA13 : 1;
  unsigned TRISA14 : 1;
  unsigned TRISA15 : 1;
  unsigned LATA0 : 1;
  unsigned LATA1 : 1;
  unsigned LATA2 : 1;
  unsigned LATA3 : 1;
  unsigned LATA4 : 1;
  unsigned LATA5 : 1;
  unsigned LATA6 : 1;
  unsigned LATA7 : 1;
  unsigned LATA8 : 1;
  unsigned LATA9 : 1;
  unsigned LATA10 : 1;
  unsigned LATA11 : 1;
  unsigned LATA12 : 1;
  unsigned LATA13 : 1;
  unsigned LATA14 : 1;
  unsigned LATA15 : 1;
} __PORTA_BITS;

typedef struct {
  unsigned RB0 : 1;
  unsigned RB1 : 1;
  unsigned RB2 : 1;
  unsigned RB3 : 1;
  unsigned RB4 : 1;
  unsigned RB5 : 1;
  unsigned RB6 : 1;
  unsigned RB7 : 1;
  unsigned RB8 : 1;
  unsigned RB9 : 1;
  unsigned RB10 : 1;
  unsigned RB11 : 1;
  unsigned RB12 : 1;
  unsigned RB13 : 1;
  unsigned RB14 : 1;
  unsigned RB15 : 1;
  unsigned TRISB0 : 1;
  unsigned TRISB1 : 1;
  unsigned TRISB2 : 1;
  unsigned TRISB3 : 1;
  unsigned TRISB4 : 1;
  unsigned TRISB5 : 1;
  unsigned TRISB6 : 1;
  unsigned TRISB7 : 1;
  unsigned TRISB8 : 1;
  unsigned TRISB9 : 1;
  unsigned TRISB10 : 1;
  unsigned TRISB11 : 1;
  unsigned TRISB12 : 1;
  unsigned TRISB13 : 1;
  unsigned TRISB14 : 1;
  unsigned TRISB15 : 1;
  unsigned LATB0 : 1;
  unsigned LATB1 : 1;
  unsigned LATB2 : 1;
  unsigned LATB3 : 1;
  unsigned LATB4 : 1;
  unsigned LATB5 : 1;
  unsigned LATB6 : 1;
  unsigned LATB7 : 1;
  unsigned LATB8 : 1;
  unsigned LATB9 : 1;
  unsigned LATB10 : 1;
  unsigned LATB11 : 1;
  unsigned LATB12 : 1;
  unsigned LATB13 : 1;
  unsigned LATB14 : 1;
  unsigned LATB15 : 1;
} __PORTB_BITS;

typedef struct {
  unsigned RC0 : 1;
  unsigned RC1 : 1;
  unsigned RC2 : 1;
  unsigned RC3 : 1;
  unsigned RC4 : 1;
  unsigned RC5 : 1;
  unsigned RC6 : 1;
  unsigned RC7 : 1;
  unsigned RC8 : 1;
  unsigned RC9 : 1;
  unsigned RC10 : 1;
  unsigned RC10_unused : 1;
  unsigned RC11 : 1;
  unsigned RC12 : 1;
  unsigned RC13 : 1;
  unsigned RC14 : 1;
  unsigned RC15 : 1;
  unsigned TRISC0 : 1;
  unsigned TRISC1 : 1;
  unsigned TRISC2 : 1;
  unsigned TRISC3 : 1;
  unsigned TRISC4 : 1;
  unsigned TRISC5 : 1;
  unsigned TRISC6 : 1;
  unsigned TRISC7 : 1;
  unsigned TRISC8 : 1;
  unsigned TRISC9 : 1;
  unsigned TRISC10 : 1;
  unsigned TRISC11 : 1;
  unsigned TRISC12 : 1;
  unsigned TRISC13 : 1;
  unsigned TRISC14 : 1;
  unsigned TRISC15 : 1;
  unsigned LATC0 : 1;
  unsigned LATC1 : 1;
  unsigned LATC2 : 1;
  unsigned LATC3 : 1;
  unsigned LATC4 : 1;
  unsigned LATC5 : 1;
  unsigned LATC6 : 1;
  unsigned LATC7 : 1;
  unsigned LATC8 : 1;
  unsigned LATC9 : 1;
  unsigned LATC10 : 1;
  unsigned LATC11 : 1;
  unsigned LATC12 : 1;
  unsigned LATC13 : 1;
  unsigned LATC14 : 1;
  unsigned LATC15 : 1;
} __PORTC_BITS;

typedef struct {
  unsigned RD0 : 1;
  unsigned RD1 : 1;
  unsigned RD2 : 1;
  unsigned RD3 : 1;
  unsigned RD4 : 1;
  unsigned RD5 : 1;
  unsigned RD6 : 1;
  unsigned RD7 : 1;
  unsigned RD8 : 1;
  unsigned RD9 : 1;
  unsigned RD10 : 1;
  unsigned RD11 : 1;
  unsigned RD12 : 1;
  unsigned RD13 : 1;
  unsigned RD14 : 1;
  unsigned RD15 : 1;
  unsigned TRISD0 : 1;
  unsigned TRISD1 : 1;
  unsigned TRISD2 : 1;
  unsigned TRISD3 : 1;
  unsigned TRISD4 : 1;
  unsigned TRISD5 : 1;
  unsigned TRISD6 : 1;
  unsigned TRISD7 : 1;
  unsigned TRISD8 : 1;
  unsigned TRISD9 : 1;
  unsigned TRISD10 : 1;
  unsigned TRISD11 : 1;
  unsigned TRISD12 : 1;
  unsigned TRISD13 : 1;
  unsigned TRISD14 : 1;
  unsigned TRISD15 : 1;
  unsigned LATD0 : 1;
  unsigned LATD1 : 1;
  unsigned LATD2 : 1;
  unsigned LATD3 : 1;
  unsigned LATD4 : 1;
  unsigned LATD5 : 1;
  unsigned LATD6 : 1;
  unsigned LATD7 : 1;
  unsigned LATD8 : 1;
  unsigned LATD9 : 1;
  unsigned LATD10 : 1;
  unsigned LATD11 : 1;
  unsigned LATD12 : 1;
  unsigned LATD13 : 1;
  unsigned LATD14 : 1;
  unsigned LATD15 : 1;
} __PORTD_BITS;

typedef struct {
  unsigned RE0 : 1;
  unsigned RE1 : 1;
  unsigned RE2 : 1;
  unsigned RE3 : 1;
  unsigned RE4 : 1;
  unsigned RE5 : 1;
  unsigned RE6 : 1;
  unsigned RE7 : 1;
  unsigned RE8 : 1;
  unsigned RE9 : 1;
  unsigned RE10 : 1;
  unsigned RE11 : 1;
  unsigned RE12 : 1;
  unsigned RE13 : 1;
  unsigned RE14 : 1;
  unsigned RE15 : 1;
  unsigned TRISE0 : 1;
  unsigned TRISE1 : 1;
  unsigned TRISE2 : 1;
  unsigned TRISE3 : 1;
  unsigned TRISE4 : 1;
  unsigned TRISE5 : 1;
  unsigned TRISE6 : 1;
  unsigned TRISE7 : 1;
  unsigned TRISE8 : 1;
  unsigned TRISE9 : 1;
  unsigned TRISE10 : 1;
  unsigned TRISE11 : 1;
  unsigned TRISE12 : 1;
  unsigned TRISE13 : 1;
  unsigned TRISE14 : 1;
  unsigned TRISE15 : 1;
  unsigned LATE0 : 1;
  unsigned LATE1 : 1;
  unsigned LATE2 : 1;
  unsigned LATE3 : 1;
  unsigned LATE4 : 1;
  unsigned LATE5 : 1;
  unsigned LATE6 : 1;
  unsigned LATE7 : 1;
  unsigned LATE8 : 1;
  unsigned LATE9 : 1;
  unsigned LATE10 : 1;
  unsigned LATE11 : 1;
  unsigned LATE12 : 1;
  unsigned LATE13 : 1;
  unsigned LATE14 : 1;
  unsigned LATE15 : 1;
} __PORTE_BITS;

typedef struct {
  unsigned RF0 : 1;
  unsigned RF1 : 1;
  unsigned RF2 : 1;
  unsigned RF3 : 1;
  unsigned RF4 : 1;
  unsigned RF5 : 1;
  unsigned RF6 : 1;
  unsigned RF7 : 1;
  unsigned RF8 : 1;
  unsigned RF9 : 1;
  unsigned RF10 : 1;
  unsigned RF11 : 1;
  unsigned RF12 : 1;
  unsigned RF13 : 1;
  unsigned RF14 : 1;
  unsigned RF15 : 1;
  unsigned TRISF0 : 1;
  unsigned TRISF1 : 1;
  unsigned TRISF2 : 1;
  unsigned TRISF3 : 1;
  unsigned TRISF4 : 1;
  unsigned TRISF5 : 1;
  unsigned TRISF6 : 1;
  unsigned TRISF7 : 1;
  unsigned TRISF8 : 1;
  unsigned TRISF9 : 1;
  unsigned TRISF10 : 1;
  unsigned TRISF11 : 1;
  unsigned TRISF12 : 1;
  unsigned TRISF13 : 1;
  unsigned TRISF14 : 1;
  unsigned TRISF15 : 1;
  unsigned LATF0 : 1;
  unsigned LATF1 : 1;
  unsigned LATF2 : 1;
  unsigned LATF3 : 1;
  unsigned LATF4 : 1;
  unsigned LATF5 : 1;
  unsigned LATF6 : 1;
  unsigned LATF7 : 1;
  unsigned LATF8 : 1;
  unsigned LATF9 : 1;
  unsigned LATF10 : 1;
  unsigned LATF11 : 1;
  unsigned LATF12 : 1;
  unsigned LATF13 : 1;
  unsigned LATF14 : 1;
  unsigned LATF15 : 1;
} __PORTF_BITS;

extern volatile __PORTA_BITS PORTAbits, TRISAbits, LATAbits;
extern volatile __PORTB_BITS PORTBbits, TRISBbits, LATBbits;
extern volatile __PORTC_BITS PORTCbits, TRISCbits, LATCbits;
extern volatile __PORTD_BITS PORTDbits, TRISDbits, LATDbits;
extern volatile __PORTE_BITS PORTEbits, TRISEbits, LATEbits;
extern volatile __PORTF_BITS PORTFbits, TRISFbits, LATFbits;

// SPI 및 PPS 하드웨어 주변기기 목업
typedef struct {
  unsigned SPIRBF : 1;
  unsigned SPITBF : 1;
  unsigned SPITUR : 1;
  unsigned SRMT : 1;
  unsigned SPIROV : 1;
  unsigned dummy : 11;
} __SPI_STAT_BITS;

typedef struct {
  unsigned MODE : 2;
  unsigned CKE : 1;
  unsigned CKP : 1;
  unsigned MSTEN : 1;
  unsigned SPIEN : 1;
  unsigned dummy : 10;
} __SPI_CON1L_BITS;

extern volatile __SPI_STAT_BITS SPI2STATLbits;
extern volatile __SPI_CON1L_BITS SPI2CON1Lbits;
extern volatile uint16_t SPI2CON1L, SPI2CON1H, SPI2BUFL, SPI2BRGL;
extern volatile uint16_t _SDI2RXR, _RP96R, _RP108R;

#ifndef __builtin_write_RPCON
#define __builtin_write_RPCON(x)
#endif

#ifndef ClrWdt
#define ClrWdt()
#endif
#endif

/* ==========================================================================
 * 1. 디지털 출력 (Digital Outputs - LAT & TRIS)
 * ========================================================================== */

// 76번 핀: DO_MC_SW (RF15) -> 메인 전원 스위치 제어
#define DO_MC_SW_LAT LATFbits.LATF15
#define DO_MC_SW_TRIS TRISFbits.TRISF15

// 77번 핀: DO_HT193 (RB9) -> LTS 촉매 히터 1 제어
#define DO_HT193_LAT LATBbits.LATB9
#define DO_HT193_TRIS TRISBbits.TRISB9

// 79번 핀: DO_HT194 / DO_HT_SPEAR1 (RA5) -> LTS 촉매 히터 2 제어
#define DO_HT194_LAT LATAbits.LATA5
#define DO_HT194_TRIS TRISAbits.TRISA5
#define DO_HT_SPEAR1_LAT LATAbits.LATA5
#define DO_HT_SPEAR1_TRIS TRISAbits.TRISA5

// 81번 핀: DO_HT195 / DO_TRI (RE13) -> Prox 히터 1 제어
#define DO_HT195_LAT LATEbits.LATE13
#define DO_HT195_TRIS TRISEbits.TRISE13
#define DO_TRI_LAT LATEbits.LATE13
#define DO_TRI_TRIS TRISEbits.TRISE13

// 83번 핀: DO_HT196 (RC10) -> Prox 히터 2 제어
#define DO_HT196_LAT LATCbits.LATC10
#define DO_HT196_TRIS TRISCbits.TRISC10

// 91번 핀: DO_MV131 (RD0) -> 개질 가스 인입 밸브 제어
#define DO_MV131_LAT LATDbits.LATD0
#define DO_MV131_TRIS TRISDbits.TRISD0

// 92번 핀: DO_MV_SPARE (RD1) -> 스페어 전동 밸브 제어
#define DO_MV_SPARE_LAT LATDbits.LATD1
#define DO_MV_SPARE_TRIS TRISDbits.TRISD1

// 68번 핀: DO_485_DIR (RB5) -> RS-485 송수신 방향 제어 (1: TX, 0: RX)
#define DO_485_DIR_LAT LATBbits.LATB5
#define DO_485_DIR_TRIS TRISBbits.TRISB5

// 7번 핀: DO_ETH1_CS (RC13) -> W5500 1호기 Chip Select
#define DO_ETH1_CS_LAT LATCbits.LATC13
#define DO_ETH1_CS_TRIS TRISCbits.TRISC13

// 2번 핀: DO_ETH1_RST (RE0) -> W5500 1호기 Reset (Active-Low)
#define DO_ETH1_RST_LAT LATEbits.LATE0
#define DO_ETH1_RST_TRIS TRISEbits.TRISE0

// 3번 핀: DI_ETH1_INT (RB15) -> W5500 1호기 Interrupt (Active-Low)
#define DI_ETH1_INT_PORT PORTBbits.RB15
#define DI_ETH1_INT_TRIS TRISBbits.TRISB15

// 1번 핀: DO_ETH2_CS (RB14) -> W5500 2호기 Chip Select
#define DO_ETH2_CS_LAT LATBbits.LATB14
#define DO_ETH2_CS_TRIS TRISBbits.TRISB14

// 27번 핀: DO_ETH2_RST (RA3) -> W5500 2호기 Reset (Active-Low)
#define DO_ETH2_RST_LAT LATAbits.LATA3
#define DO_ETH2_RST_TRIS TRISAbits.TRISA3

// 26번 핀: DI_ETH2_INT (RF5) -> W5500 2호기 Interrupt (Active-Low)
#define DI_ETH2_INT_PORT PORTFbits.RF5
#define DI_ETH2_INT_TRIS TRISFbits.TRISF5

// 12번 핀: DO_FLASH_CS (RF1) -> W25Q256 Flash Chip Select
#define DO_FLASH_CS_LAT LATFbits.LATF1
#define DO_FLASH_CS_TRIS TRISFbits.TRISF1

// 72번 핀: DI_RTC_INT (RB7) -> DS3231 RTC Interrupt (Active-Low)
#define DI_RTC_INT_PORT PORTBbits.RB7
#define DI_RTC_INT_TRIS TRISBbits.TRISB7

// 94번 핀: DO_MV124 (RE14) -> PNG 인입 밸브 제어
#define DO_MV124_LAT LATEbits.LATE14
#define DO_MV124_TRIS TRISEbits.TRISE14

// 95번 핀: DO_MV158 (RA6) -> AOG VENT 전동 밸브 제어
#define DO_MV158_LAT LATAbits.LATA6
#define DO_MV158_TRIS TRISAbits.TRISA6

// 96번 핀: DO_MV150 (RA7) -> Anode 출구 전동 밸브 제어
#define DO_MV150_LAT LATAbits.LATA7
#define DO_MV150_TRIS TRISAbits.TRISA7

// 97번 핀: DO_MV148 (RB12) -> 개질가스 Bypass 전동 밸브 제어
#define DO_MV148_LAT LATBbits.LATB12
#define DO_MV148_TRIS TRISBbits.TRISB12

// 98번 핀: DO_MV390 (RE15) -> 배열 회수 입구 전동 볼 밸브 제어
#define DO_MV390_LAT LATEbits.LATE15
#define DO_MV390_TRIS TRISEbits.TRISE15

// 15번 핀: DO_MV300 (RD14) -> 시설수 입구 전동 볼밸브 제어
#define DO_MV300_LAT LATDbits.LATD14
#define DO_MV300_TRIS TRISDbits.TRISD14

// 16번 핀: DO_MV155 (RD13) -> AOG 전동 밸브 제어
#define DO_MV155_LAT LATDbits.LATD13
#define DO_MV155_TRIS TRISDbits.TRISD13

// 17번 핀: DO_SV_SPARE (RC0) -> 스페어 솔레노이드 밸브 제어
#define DO_SV_SPARE_LAT LATCbits.LATC0
#define DO_SV_SPARE_TRIS TRISCbits.TRISC0

// 18번 핀: DO_SV102 (RA0) -> 가스 인입 솔레노이드 밸브 1 제어
#define DO_SV102_LAT LATAbits.LATA0
#define DO_SV102_TRIS TRISAbits.TRISA0

// 19번 핀: DO_SV103 (RE2) -> 가스 인입 솔레노이드 밸브 2 제어
#define DO_SV103_LAT LATEbits.LATE2
#define DO_SV103_TRIS TRISEbits.TRISE2

// 20번 핀: DO_SV145 (RF2) -> ANODE 인입 솔레노이드 밸브 제어
#define DO_SV145_LAT LATFbits.LATF2
#define DO_SV145_TRIS TRISFbits.TRISF2

// 21번 핀: DO_SV323 (RA1) -> DI Water TANK 드레인 솔레노이드 밸브 제어
#define DO_SV323_LAT LATAbits.LATA1
#define DO_SV323_TRIS TRISAbits.TRISA1

// 22번 핀: DO_SV125 (RE3) -> 개질기 질소퍼지 솔레노이드 밸브 제어
#define DO_SV125_LAT LATEbits.LATE3
#define DO_SV125_TRIS TRISEbits.TRISE3

// 23번 핀: DO_SV149 (RF3) -> 스택 질소퍼지 솔레노이드 밸브 제어
#define DO_SV149_LAT LATFbits.LATF3
#define DO_SV149_TRIS TRISFbits.TRISF3

// 25번 핀: DO_FAN504 (RF4) -> 가스 환기팬1 제어
#define DO_FAN504_LAT LATFbits.LATF4
#define DO_FAN504_TRIS TRISFbits.TRISF4

// 62번 핀: DO_LCD500 (RD8) -> 7인치 LCD 전원/신호 제어
#define DO_LCD500_LAT LATDbits.LATD8
#define DO_LCD500_TRIS TRISDbits.TRISD8

// 73번 핀: DO_IGN175 (RE11) -> 점화 트랜스 제어
#define DO_IGN175_LAT LATEbits.LATE11
#define DO_IGN175_TRIS TRISEbits.TRISE11

// 열전대(TC) 채널 선택 어드레스 (출력 설정)
#define TC_ADDR0_LAT LATFbits.LATF5
#define TC_ADDR0_TRIS TRISFbits.TRISF5
#define TC_ADDR1_LAT LATAbits.LATA3
#define TC_ADDR1_TRIS TRISAbits.TRISA3
#define TC_ADDR2_LAT LATEbits.LATE4
#define TC_ADDR2_TRIS TRISEbits.TRISE4
#define TC_ADDR3_LAT LATFbits.LATF6
#define TC_ADDR3_TRIS TRISFbits.TRISF6

// 열전대(TC) 멀티플렉서 인에이블 (출력 설정)
// 32번 핀: TC_EN1 (RF7) -> MUX1 인에이블 (CH1~CH10, CH21~CH25)
#define TC_EN1_LAT LATFbits.LATF7
#define TC_EN1_TRIS TRISFbits.TRISF7

// 31번 핀: TC_EN2 (RE5) -> MUX2 인에이블 (CH11~CH16, CH26~CH32)
#define TC_EN2_LAT LATEbits.LATE5
#define TC_EN2_TRIS TRISEbits.TRISE5

// 35번 핀: TC_EN3 (RC3) -> MUX3 인에이블 (CH17~CH20)
#define TC_EN3_LAT LATCbits.LATC3
#define TC_EN3_TRIS TRISCbits.TRISC3

// 57번 핀: TC_SPI_CS (RB4) -> MAX31856 SPI CS 제어
#define TC_SPI_CS_LAT LATBbits.LATB4
#define TC_SPI_CS_TRIS TRISBbits.TRISB4

// 56번 핀: TC_SPI_CLK (RF11) -> MAX31856/DAC SPI Clock
#define TC_SPI_CLK_LAT LATFbits.LATF11
#define TC_SPI_CLK_TRIS TRISFbits.TRISF11

// 53번 핀: TC_SPI_MOSI (RF10) -> MAX31856/DAC SPI Data Out (MCU -> Device)
#define TC_SPI_MOSI_LAT LATFbits.LATF10
#define TC_SPI_MOSI_TRIS TRISFbits.TRISF10

// 54번 핀: TC_SPI_MISO (RB3) -> MAX31856 SPI Data In (Device -> MCU)
#define TC_SPI_MISO_PORT PORTBbits.RB3
#define TC_SPI_MISO_TRIS TRISBbits.TRISB3

// 33번 핀: DAC_CS (RB8) -> DAC60516 SPI CS 제어
#define DAC_CS_LAT LATBbits.LATB8
#define DAC_CS_TRIS TRISBbits.TRISB8

/* ==========================================================================
 * 2. 디지털 입력 (Digital Inputs - PORT & TRIS)
 * ========================================================================== */

// 77번 물리적 라벨: DI_HT193 (RE12) -> LTS 촉매 히터 1 피드백
#define DI_HT193_PORT PORTEbits.RE12
#define DI_HT193_TRIS TRISEbits.TRISE12

// 78번 핀: DI_HT193_WD (RE12) -> LTS 촉매 히터 1 단선 경보
#define DI_HT193_WD_PORT PORTEbits.RE12
#define DI_HT193_WD_TRIS TRISEbits.TRISE12

// 80번 핀: DI_HT194_WD (RC4) -> LTS 촉매 히터 2 단선 경보
#define DI_HT194_WD_PORT PORTCbits.RC4
#define DI_HT194_WD_TRIS TRISCbits.TRISC4

// 82번 핀: DI_HT195_WD (RC5) -> Prox 히터 1 단선 경보
#define DI_HT195_WD_PORT PORTCbits.RC5
#define DI_HT195_WD_TRIS TRISCbits.TRISC5

// 84번 핀: DI_HT196_WD (RC11) -> Prox 히터 2 단선 경보
#define DI_HT196_WD_PORT PORTCbits.RC11
#define DI_HT196_WD_TRIS TRISCbits.TRISC11

// 89번 물리적 라벨: DI_FD176 (RD2) -> 버너 화염감지기 신호
#define DI_FD176_PORT PORTDbits.RD2
#define DI_FD176_TRIS TRISDbits.TRISD2

// 24번 핀: DI_FAN504 (RA2) -> 가스 환기팬1 상태 피드백
#define DI_FAN504_PORT PORTAbits.RA2
#define DI_FAN504_TRIS TRISAbits.TRISA2

// 59번 핀: DI_GD501 (RC9) -> 메탄 누출 감지
#define DI_GD501_PORT PORTCbits.RC9
#define DI_GD501_TRIS TRISCbits.TRISC9

// 60번 핀: DI_GD502 (RF12) -> 수소 누출 감지
#define DI_GD502_PORT PORTFbits.RF12
#define DI_GD502_TRIS TRISFbits.TRISF12

// 74번 핀: POWER_FAIL_DET (RF14) -> 메인 정전 감지 입력
#define POWER_FAIL_DET_PORT PORTFbits.RF14
#define POWER_FAIL_DET_TRIS TRISFbits.TRISF14

/* ==========================================================================
 * 3. 입출력 포트 방향 초기화 함수
 * ========================================================================== */
static inline void GPIO_Initialize(void) {
  // [0] dsPIC33CK 포트(A~E) 아날로그(ANSEL) 해제 및 순수 디지털 I/O 모드 전환
  // (기본값 1인 경우 UART RX 및 디지털 입력 버퍼가 차단되므로 반드시 0으로
  // 클리어)
  ANSELA = 0x0000;
  ANSELA = 0x0000;
  ANSELB = 0x0000;
  ANSELC = 0x0000;
  ANSELD = 0x0000;
  ANSELE = 0x0000;
  ANSELF = 0x0000;

  // [1] 모든 디지털 출력 방향(TRIS = 0) 설정 및 초기 출력값(Low) 클리어
  DO_MC_SW_LAT = 0;
  DO_MC_SW_TRIS = 0;

  DO_HT193_LAT = 0;
  DO_HT193_TRIS = 0;

  DO_HT194_LAT = 0;
  DO_HT194_TRIS = 0;

  DO_HT195_LAT = 0;
  DO_HT195_TRIS = 0;

  DO_HT196_LAT = 0;
  DO_HT196_TRIS = 0;

  DO_HT_SPEAR1_LAT = 0;
  DO_HT_SPEAR1_TRIS = 0;

  DO_TRI_LAT = 0;
  DO_TRI_TRIS = 0;

  DO_MV131_LAT = 0;
  DO_MV131_TRIS = 0;

  DO_MV_SPARE_LAT = 0;
  DO_MV_SPARE_TRIS = 0;

  DO_MV124_LAT = 0;
  DO_MV124_TRIS = 0;

  DO_MV158_LAT = 0;
  DO_MV158_TRIS = 0;

  DO_MV150_LAT = 0;
  DO_MV150_TRIS = 0;

  DO_MV148_LAT = 0;
  DO_MV148_TRIS = 0;

  DO_MV390_LAT = 0;
  DO_MV390_TRIS = 0;

  DO_MV300_LAT = 0;
  DO_MV300_TRIS = 0;

  DO_MV155_LAT = 0;
  DO_MV155_TRIS = 0;

  DO_SV_SPARE_LAT = 0;
  DO_SV_SPARE_TRIS = 0;

  DO_SV102_LAT = 0;
  DO_SV102_TRIS = 0;

  DO_SV103_LAT = 0;
  DO_SV103_TRIS = 0;

  DO_SV145_LAT = 0;
  DO_SV145_TRIS = 0;

  DO_SV323_LAT = 0;
  DO_SV323_TRIS = 0;

  DO_SV125_LAT = 0;
  DO_SV125_TRIS = 0;

  DO_SV149_LAT = 0;
  DO_SV149_TRIS = 0;

  DO_FAN504_LAT = 0;
  DO_FAN504_TRIS = 0;

  DO_LCD500_LAT = 0;
  DO_LCD500_TRIS = 0;

  DO_IGN175_LAT = 0;
  DO_IGN175_TRIS = 0;

  TC_ADDR0_LAT = 0;
  TC_ADDR0_TRIS = 0;
  TC_ADDR1_LAT = 0;
  TC_ADDR1_TRIS = 0;
  TC_ADDR2_LAT = 0;
  TC_ADDR2_TRIS = 0;
  TC_ADDR3_LAT = 0;
  TC_ADDR3_TRIS = 0;

  TC_EN1_LAT = 0;
  TC_EN1_TRIS = 0;
  TC_EN2_LAT = 0;
  TC_EN2_TRIS = 0;

  TC_EN3_LAT = 0;
  TC_EN3_TRIS = 0;

  // MAX31856 SPI 핀 디지털 모드 및 입출력 설정
  ANSELBbits.ANSELB3 = 0; // RB3 디지털 모드
  ANSELBbits.ANSELB4 = 0; // RB4 디지털 모드
  TC_SPI_MISO_TRIS = 1;   // MISO 입력
  TC_SPI_MOSI_LAT = 0;
  TC_SPI_MOSI_TRIS = 0;   // MOSI 출력
  TC_SPI_CLK_LAT = 0;
  TC_SPI_CLK_TRIS = 0;    // CLK 출력
  TC_SPI_CS_LAT = 1;      // CS 핀 High (비활성화)
  TC_SPI_CS_TRIS = 0;

  DAC_CS_LAT = 1; // DAC60516 CS 핀 High (비활성화)
  DAC_CS_TRIS = 0;

  DO_485_DIR_LAT = 0; // 수신 상태로 대기
  DO_485_DIR_TRIS = 0;

  // 듀얼 W5500 이더넷 CS 및 Reset 출력/비활성화 초기화
  DO_ETH1_CS_LAT = 1;
  DO_ETH1_CS_TRIS = 0;
  DO_ETH2_CS_LAT = 1;
  DO_ETH2_CS_TRIS = 0;

  // 리셋은 Active-Low 이므로 1(High)로 설정하여 동작 상태 유지
  DO_ETH1_RST_LAT = 1;
  DO_ETH1_RST_TRIS = 0;
  DO_ETH2_RST_LAT = 1;
  DO_ETH2_RST_TRIS = 0;

  // 인터럽트 핀 입력 설정
  DI_ETH1_INT_TRIS = 1;
  DI_ETH2_INT_TRIS = 1;

  // 외부 W25Q256 SPI Flash Chip Select 초기화 (Active-Low 이므로 High 대기)
  DO_FLASH_CS_LAT = 1;
  DO_FLASH_CS_TRIS = 0;

  // DS3231 RTC 알람 인터럽트 포트 입력 방향 설정
  DI_RTC_INT_TRIS = 1;

  // [2] 모든 디지털 입력 방향(TRIS = 1) 설정
  DI_HT193_WD_TRIS = 1;
  DI_HT194_WD_TRIS = 1;
  DI_HT195_WD_TRIS = 1;
  DI_HT196_WD_TRIS = 1;
  DI_FD176_TRIS = 1;
  DI_FAN504_TRIS = 1;
  DI_GD501_TRIS = 1;
  DI_GD502_TRIS = 1;
  POWER_FAIL_DET_TRIS = 1;

  // [3] 모든 DO(릴레이/솔레노이드 밸브) 출력 방향(TRIS = 0) 최종 확증
  DO_MC_SW_TRIS = 0;
  DO_HT193_TRIS = 0;
  DO_HT194_TRIS = 0;
  DO_HT195_TRIS = 0;
  DO_HT196_TRIS = 0;
  DO_SV102_TRIS = 0;
  DO_SV103_TRIS = 0;
  DO_SV145_TRIS = 0;
  DO_SV323_TRIS = 0;
  DO_SV125_TRIS = 0;
  DO_SV149_TRIS = 0;
  DO_MV124_TRIS = 0;
  DO_MV131_TRIS = 0;
  DO_MV148_TRIS = 0;
  DO_MV150_TRIS = 0;
  DO_MV155_TRIS = 0;
  DO_MV158_TRIS = 0;
  DO_MV300_TRIS = 0;
  DO_MV390_TRIS = 0;
  DO_IGN175_TRIS = 0;
}

#endif /* PIN_MAP_H */
