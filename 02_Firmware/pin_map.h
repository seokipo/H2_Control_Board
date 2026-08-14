/**
 * @file pin_map.h
 * @brief H2_Control_Board dsPIC33CK512MP710 GPIO Pin Mapping & Initialization
 * 
 * @note 본 헤더 파일은 회로도 Mcu.SchDoc 분석을 기반으로 I/O 네트 및 물리 핀 정보를 매핑한 파일입니다.
 *       - DO_ : 디지털 출력 (TRIS = 0)
 *       - DI_ : 디지털 입력 (TRIS = 1)
 */

#ifndef PIN_MAP_H
#define PIN_MAP_H

#include <xc.h>

/* ==========================================================================
 * 1. 디지털 출력 (Digital Outputs - LAT & TRIS)
 * ========================================================================== */

// 76번 핀: DO_MC_SW (RF15) -> 메인 전원 스위치 제어
#define DO_MC_SW_LAT             LATFbits.LATF15
#define DO_MC_SW_TRIS            TRISFbits.TRISF15

// 77번 핀: DO_HT193 (RB9) -> LTS 촉매 히터 1 제어
#define DO_HT193_LAT             LATBbits.LATB9
#define DO_HT193_TRIS            TRISBbits.TRISB9

// 86번 핀: DO_HT196 (RD3) -> Prox 히터 2 제어
#define DO_HT196_LAT             LATDbits.LATD3
#define DO_HT196_TRIS            TRISDbits.TRISD3

// 85번 핀: DO_HT_SPEAR1 (RD4) -> 배열 회수 입구 전동 볼 밸브 여유 히터 제어
#define DO_HT_SPEAR1_LAT         LATDbits.LATD4
#define DO_HT_SPEAR1_TRIS        TRISDbits.TRISD4

// 88번 핀: DO_TRI (RD2) -> 트라이액 제어
#define DO_TRI_LAT               LATDbits.LATD2
#define DO_TRI_TRIS              TRISDbits.TRISD2

// 91번 핀: DO_MV131 (RD0) -> 개질 가스 인입 밸브 제어
#define DO_MV131_LAT             LATDbits.LATD0
#define DO_MV131_TRIS            TRISDbits.TRISD0

// 92번 핀: DO_MV_SPARE (RD1) -> 스페어 전동 밸브 제어
#define DO_MV_SPARE_LAT          LATDbits.LATD1
#define DO_MV_SPARE_TRIS         TRISDbits.TRISD1

// 68번 핀: DO_485_DIR (RB5) -> RS-485 송수신 방향 제어 (1: TX, 0: RX)
#define DO_485_DIR_LAT           LATBbits.LATB5
#define DO_485_DIR_TRIS          TRISBbits.TRISB5

// 7번 핀: DO_ETH1_CS (RC13) -> W5500 1호기 Chip Select
#define DO_ETH1_CS_LAT           LATCbits.LATC13
#define DO_ETH1_CS_TRIS          TRISCbits.TRISC13

// 2번 핀: DO_ETH1_RST (RE0) -> W5500 1호기 Reset (Active-Low)
#define DO_ETH1_RST_LAT          LATEbits.LATE0
#define DO_ETH1_RST_TRIS         TRISEbits.TRISE0

// 3번 핀: DI_ETH1_INT (RB15) -> W5500 1호기 Interrupt (Active-Low)
#define DI_ETH1_INT_PORT         PORTBbits.RB15
#define DI_ETH1_INT_TRIS         TRISBbits.TRISB15

// 1번 핀: DO_ETH2_CS (RB14) -> W5500 2호기 Chip Select
#define DO_ETH2_CS_LAT           LATBbits.LATB14
#define DO_ETH2_CS_TRIS          TRISBbits.TRISB14

// 27번 핀: DO_ETH2_RST (RA3) -> W5500 2호기 Reset (Active-Low)
#define DO_ETH2_RST_LAT          LATAbits.LATA3
#define DO_ETH2_RST_TRIS         TRISAbits.TRISA3

// 26번 핀: DI_ETH2_INT (RF5) -> W5500 2호기 Interrupt (Active-Low)
#define DI_ETH2_INT_PORT         PORTFbits.RF5
#define DI_ETH2_INT_TRIS         TRISFbits.TRISF5

// 12번 핀: DO_FLASH_CS (RF1) -> W25Q256 Flash Chip Select
#define DO_FLASH_CS_LAT          LATFbits.LATF1
#define DO_FLASH_CS_TRIS         TRISFbits.TRISF1

// 72번 핀: DI_RTC_INT (RB7) -> DS3231 RTC Interrupt (Active-Low)
#define DI_RTC_INT_PORT          PORTBbits.RB7
#define DI_RTC_INT_TRIS         TRISBbits.TRISB7

// 94번 핀: DO_MV124 (RA6) -> PNG 인입 밸브 제어
#define DO_MV124_LAT             LATAbits.LATA6
#define DO_MV124_TRIS            TRISAbits.TRISA6

// 95번 핀: DO_MV158 (RA7) -> AOG VENT 전동 밸브 제어
#define DO_MV158_LAT             LATAbits.LATA7
#define DO_MV158_TRIS            TRISAbits.TRISA7

// 96번 핀: DO_MV150 (RB12) -> Anode 출구 전동 밸브 제어
#define DO_MV150_LAT             LATBbits.LATB12
#define DO_MV150_TRIS            TRISBbits.TRISB12

// 97번 핀: DO_MV148 (RE14) -> 개질가스 Bypass 전동 밸브 제어
#define DO_MV148_LAT             LATEbits.LATE14
#define DO_MV148_TRIS            TRISEbits.TRISE14

// 98번 핀: DO_MV390 (RE15) -> 배열 회수 입구 전동 볼 밸브 제어
#define DO_MV390_LAT             LATEbits.LATE15
#define DO_MV390_TRIS            TRISEbits.TRISE15

// 15번 핀: DO_MV300 (RD14) -> 시설수 입구 전동 볼밸브 제어
#define DO_MV300_LAT             LATDbits.LATD14
#define DO_MV300_TRIS            TRISDbits.TRISD14

// 16번 핀: DO_MV155 (RD13) -> AOG 전동 밸브 제어
#define DO_MV155_LAT             LATDbits.LATD13
#define DO_MV155_TRIS            TRISDbits.TRISD13

// 17번 핀: DO_SV_SPARE (RC0) -> 스페어 솔레노이드 밸브 제어
#define DO_SV_SPARE_LAT          LATCbits.LATC0
#define DO_SV_SPARE_TRIS         TRISCbits.TRISC0

// 18번 핀: DO_SV102 (RA0) -> 가스 인입 솔레노이드 밸브 1 제어
#define DO_SV102_LAT             LATAbits.LATA0
#define DO_SV102_TRIS            TRISAbits.TRISA0

// 19번 핀: DO_SV103 (RE2) -> 가스 인입 솔레노이드 밸브 2 제어
#define DO_SV103_LAT             LATEbits.LATE2
#define DO_SV103_TRIS            TRISEbits.TRISE2

// 20번 핀: DO_SV145 (RF2) -> ANODE 인입 솔레노이드 밸브 제어
#define DO_SV145_LAT             LATFbits.LATF2
#define DO_SV145_TRIS            TRISFbits.TRISF2

// 21번 핀: DO_SV323 (RA1) -> DI Water TANK 드레인 솔레노이드 밸브 제어
#define DO_SV323_LAT             LATAbits.LATA1
#define DO_SV323_TRIS            TRISAbits.TRISA1

// 22번 핀: DO_SV125 (RE3) -> 개질기 질소퍼지 솔레노이드 밸브 제어
#define DO_SV125_LAT             LATEbits.LATE3
#define DO_SV125_TRIS            TRISEbits.TRISE3

// 23번 핀: DO_SV149 (RF3) -> 스택 질소퍼지 솔레노이드 밸브 제어
#define DO_SV149_LAT             LATFbits.LATF3
#define DO_SV149_TRIS            TRISFbits.TRISF3

// 25번 핀: DO_FAN504 (RF4) -> 가스 환기팬1 제어
#define DO_FAN504_LAT            LATFbits.LATF4
#define DO_FAN504_TRIS           TRISFbits.TRISF4

// 62번 핀: DO_LCD500 (RD8) -> 7인치 LCD 전원/신호 제어
#define DO_LCD500_LAT            LATDbits.LATD8
#define DO_LCD500_TRIS           TRISDbits.TRISD8

// 73번 핀: DO_IGN175 (RE11) -> 점화 트랜스 제어
#define DO_IGN175_LAT            LATEbits.LATE11
#define DO_IGN175_TRIS           TRISEbits.TRISE11

// 열전대(TC) 채널 선택 어드레스 (출력 설정)
#define TC_ADDR0_LAT             LATFbits.LATF5
#define TC_ADDR0_TRIS            TRISFbits.TRISF5
#define TC_ADDR1_LAT             LATAbits.LATA3
#define TC_ADDR1_TRIS            TRISAbits.TRISA3
#define TC_ADDR2_LAT             LATEbits.LATE4
#define TC_ADDR2_TRIS            TRISEbits.TRISE4
#define TC_ADDR3_LAT             LATFbits.LATF6
#define TC_ADDR3_TRIS            TRISFbits.TRISF6

// 열전대(TC) 멀티플렉서 인에이블 (출력 설정)
#define TC_EN1_LAT               LATEbits.LATE5
#define TC_EN1_TRIS              TRISEbits.TRISE5
#define TC_EN2_LAT               LATAbits.LATA4
#define TC_EN2_TRIS              TRISAbits.TRISA4
#define TC_EN3_LAT               LATBbits.LATB12 // 도면상 34번 핀 매핑
#define TC_EN3_TRIS              TRISBbits.TRISB12 

// 35번 핀: TC_SPI_CS (RC3) -> MAX31856 SPI CS 제어
#define TC_SPI_CS_LAT            LATCbits.LATC3
#define TC_SPI_CS_TRIS           TRISCbits.TRISC3 


/* ==========================================================================
 * 2. 디지털 입력 (Digital Inputs - PORT & TRIS)
 * ========================================================================== */

// 77번 물리적 라벨: DI_HT193 (RE12) -> LTS 촉매 히터 1 피드백
#define DI_HT193_PORT            PORTEbits.RE12
#define DI_HT193_TRIS           TRISEbits.TRISE12

// 78번 물리적 라벨: DI_HT193_WD (RA5) -> LTS 촉매 히터 1 단선 경보
#define DI_HT193_WD_PORT         PORTAbits.RA5
#define DI_HT193_WD_TRIS        TRISAbits.TRISA5

// 79번 물리적 라벨: DI_HT194 (RC4) -> LTS 촉매 히터 2 피드백
#define DI_HT194_PORT            PORTCbits.RC4
#define DI_HT194_TRIS           TRISCbits.TRISC4

// 80번 물리적 라벨: DI_HT194_WD (RE13) -> LTS 촉매 히터 2 단선 경보
#define DI_HT194_WD_PORT         PORTEbits.RE13
#define DI_HT194_WD_TRIS        TRISEbits.TRISE13

// 81번 물리적 라벨: DI_HT195 (RC5) -> Prox 히터 1 피드백
#define DI_HT195_PORT            PORTCbits.RC5
#define DI_HT195_TRIS           TRISCbits.TRISC5

// 82번 물리적 라벨: DI_HT195_WD (RC10) -> Prox 히터 1 단선 경보
#define DI_HT195_WD_PORT         PORTCbits.RC10
#define DI_HT195_WD_TRIS        TRISCbits.TRISC10

// 83번 물리적 라벨: DI_HT196 (RC11) -> Prox 히터 2 피드백
#define DI_HT196_PORT            PORTCbits.RC11
#define DI_HT196_TRIS           TRISCbits.TRISC11

// 84번 물리적 라벨: DI_HT196_WD (RD4) -> Prox 히터 2 단선 경보
#define DI_HT196_WD_PORT         PORTDbits.RD4
#define DI_HT196_WD_TRIS        TRISDbits.TRISD4

// 89번 물리적 라벨: DI_FD176 (RD2) -> 버너 화염감지기 신호
#define DI_FD176_PORT            PORTDbits.RD2
#define DI_FD176_TRIS           TRISDbits.TRISD2

// 24번 핀: DI_FAN504 (RA2) -> 가스 환기팬1 상태 피드백
#define DI_FAN504_PORT           PORTAbits.RA2
#define DI_FAN504_TRIS          TRISAbits.TRISA2

// 59번 핀: DI_GD501 (RC9) -> 메탄 누출 감지
#define DI_GD501_PORT            PORTCbits.RC9
#define DI_GD501_TRIS           TRISCbits.TRISC9

// 60번 핀: DI_GD502 (RF12) -> 수소 누출 감지
#define DI_GD502_PORT            PORTFbits.RF12
#define DI_GD502_TRIS           TRISFbits.TRISF12

// 74번 핀: POWER_FAIL_DET (RF14) -> 메인 정전 감지 입력
#define POWER_FAIL_DET_PORT      PORTFbits.RF14
#define POWER_FAIL_DET_TRIS     TRISFbits.TRISF14


/* ==========================================================================
 * 3. 입출력 포트 방향 초기화 함수
 * ========================================================================== */
static inline void GPIO_Initialize(void)
{
    // [1] 모든 디지털 출력 방향(TRIS = 0) 설정 및 초기 출력값(Low) 클리어
    DO_MC_SW_LAT = 0;
    DO_MC_SW_TRIS = 0;
    
    DO_HT193_LAT = 0;
    DO_HT193_TRIS = 0;
    
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
    
    TC_SPI_CS_LAT = 1; // CS 핀은 High로 초기 설정 (비활성화)
    TC_SPI_CS_TRIS = 0;

    DO_485_DIR_LAT = 0; // 수신 상태로 대기
    DO_485_DIR_TRIS = 0;

    // 듀얼 W5500 이더넷 CS 및 Reset 출력/비활성화 초기화
    DO_ETH1_CS_LAT = 1; DO_ETH1_CS_TRIS = 0;
    DO_ETH2_CS_LAT = 1; DO_ETH2_CS_TRIS = 0;
    
    // 리셋은 Active-Low 이므로 1(High)로 설정하여 동작 상태 유지
    DO_ETH1_RST_LAT = 1; DO_ETH1_RST_TRIS = 0;
    DO_ETH2_RST_LAT = 1; DO_ETH2_RST_TRIS = 0;

    // 인터럽트 핀 입력 설정
    DI_ETH1_INT_TRIS = 1;
    DI_ETH2_INT_TRIS = 1;

    // 외부 W25Q256 SPI Flash Chip Select 초기화 (Active-Low 이므로 High 대기)
    DO_FLASH_CS_LAT = 1;
    DO_FLASH_CS_TRIS = 0;

    // DS3231 RTC 알람 인터럽트 포트 입력 방향 설정
    DI_RTC_INT_TRIS = 1;

    // [2] 모든 디지털 입력 방향(TRIS = 1) 설정
    DI_HT193_TRIS = 1;
    DI_HT193_WD_TRIS = 1;
    DI_HT194_TRIS = 1;
    DI_HT194_WD_TRIS = 1;
    DI_HT195_TRIS = 1;
    DI_HT195_WD_TRIS = 1;
    DI_HT196_TRIS = 1;
    DI_HT196_WD_TRIS = 1;
    DI_FD176_TRIS = 1;
    DI_FAN504_TRIS = 1;
    DI_GD501_TRIS = 1;
    DI_GD502_TRIS = 1;
    POWER_FAIL_DET_TRIS = 1;
}

#endif /* PIN_MAP_H */
