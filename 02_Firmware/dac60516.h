/**
 * @file dac60516.h
 * @brief DAC60516 16-bit 16-channel DAC Driver & Channel Mapping for H2_Control_Board
 * 
 * @note 본 드라이버는 회로도 Output_Section.SchDoc 내의 DAC60516 칩(IC501)
 *       연동 사양을 기반으로 구현되었습니다. dsPIC33CK MCU의 SPI 모듈과 연동합니다.
 */

#ifndef DAC60516_H
#define DAC60516_H

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 * 1. DAC60516 내부 레지스터 맵 정의 (TI 공식 데이터시트 SLASFK6 표준)
 * ========================================================================== */
#define DAC60516_REG_NOOP       0x00    // 동작 없음 (NOP, W)
#define DAC60516_REG_DEVID      0x01    // 디바이스 ID 레지스터 (0x6516, R)
#define DAC60516_REG_VERSION_ID 0x02    // 버전 ID (R)
#define DAC60516_REG_PWDWN      0x03    // 채널별 전원 관리 레지스터 (R/W, 리셋 0xFFFF)
#define DAC60516_REG_GAIN       0x04    // QUAD 버퍼 게인 설정 레지스터 (R/W, 0=1x 2.5V, 1=2x 5.0V)
#define DAC60516_REG_TRIGGER    0x05    // 소프트웨어 리셋 및 LDAC 트리거 (W)
#define DAC60516_REG_BCAST_DATA 0x06    // 브로드캐스트 데이터 레지스터 (R/W)
#define DAC60516_REG_STATUS     0x07    // 칩 오류 상태 정보 (R)
#define DAC60516_REG_SDO_EN     0x08    // SDO 출력 인에이블 설정 (R/W)
#define DAC60516_REG_GEN_CONFIG 0x09    // 일반 설정: 내부 레퍼런스(Bit 2: REF_PWDWN) 등 (R/W, 리셋 0x0014)
#define DAC60516_REG_SYNC_EN    0x0A    // 채널별 동기화 설정 (R/W, 리셋 0x0000 비동기 즉각 출력)
#define DAC60516_REG_BCAST_EN   0x0B    // 브로드캐스트 인에이블 (R/W)
#define DAC60516_REG_CLEAR      0x0C    // 채널 클리어 (R/W)

// DAC 채널 데이터 버퍼 레지스터 (OUT0 ~ OUT15, Offset 0x10 ~ 0x1F)
#define DAC60516_REG_DAC0       0x10    // OUT0 (AO_AB232 - PrOx 에어블로우)
#define DAC60516_REG_DAC1       0x11    // OUT1 (AO_AB212 - STACK 에어블로어)
#define DAC60516_REG_DAC2       0x12    // OUT2 (AO_P341 - AOG 냉각 물펌프)
#define DAC60516_REG_DAC3       0x13    // OUT3 (AO_P375 - STACK 2 냉각수 공급)
#define DAC60516_REG_DAC4       0x14    // OUT4 (AO_P108 - 가스 가압펌프)
#define DAC60516_REG_DAC5       0x15    // OUT5 (AO_P370 - STACK 1 냉각수 공급)
#define DAC60516_REG_DAC6       0x16    // OUT6 (AO_P380 - 개질수 펌프)
#define DAC60516_REG_DAC7       0x17    // OUT7 (AO_AB221 - 개질기 버너 에어블로어)
#define DAC60516_REG_DAC8       0x18    // OUT8 (AO_MFC111 - BNG 유량 제어)
#define DAC60516_REG_DAC9       0x19    // OUT9 (AO_MFC121 - PNG 유량 제어)
#define DAC60516_REG_DAC10      0x1A    // OUT10 (AO_P351 - Anode 냉각 물펌프)
#define DAC60516_REG_DAC11      0x1B    // OUT11 (AO_SPARE1 - 스페어 아날로그 출력)

/* ==========================================================================
 * 2. 12개 아날로그 출력 채널 매핑 리스트
 * ========================================================================== */
typedef enum {
    AO_AB232_PROX_BLOWER = 0,      // PrOx 에어블로우 (OUT0 - AO_AB232)
    AO_AB212_STACK_BLOWER,         // STACK 에어블로어 (OUT1 - AO_AB212)
    AO_P341_AOG_COOL_PUMP,         // AOG 냉각용 물펌프 (OUT2 - AO_P341)
    AO_P375_STACK2_COOL_PUMP,      // STACK 2 냉각수 공급 (OUT3 - AO_P375)
    AO_P108_GAS_PRESS_PUMP,        // 가스 가압펌프 (OUT4 - AO_P108)
    AO_P370_STACK1_COOL_PUMP,      // STACK 1 냉각수 공급 (OUT5 - AO_P370)
    AO_P380_WATER_PUMP,            // 개질수 펌프 (OUT6 - AO_P380)
    AO_AB221_BURNER_BLOWER,        // 개질기 버너 에어블로어 (OUT7 - AO_AB221)
    AO_MFC111_BNG_FLOW,            // BNG 유량 제어 신호 (OUT8 - AO_MFC111)
    AO_MFC121_PNG_FLOW,            // PNG 유량 제어 신호 (OUT9 - AO_MFC121)
    AO_P351_ANODE_COOL_PUMP,       // Anode(RG) 냉각용 물펌프 (OUT10 - AO_P351)
    AO_SPARE1,                     // 스페어 아날로그 출력 (OUT11 - AO_SPARE1)
    
    AO_MAX_CHANNELS
} DAC60516_OutputChannel_t;

/* ==========================================================================
 * 3. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief DAC60516 칩 통신 초기화 및 출력 인에이블
 * @return true 성공, false 실패
 */
bool DAC60516_Initialize(void);

/**
 * @brief 특정 아날로그 출력 채널에 16비트 디지털 값 출력
 * @param channel 설정할 출력 채널 인덱스
 * @param raw_value 출력할 16비트 디지털 값 (0 ~ 65535)
 * @return true 출력 성공, false 통신 오류
 */
bool DAC60516_WriteRaw(DAC60516_OutputChannel_t channel, uint16_t raw_value);

/**
 * @brief 특정 아날로그 출력 채널에 목표 전압(V) 환산 출력
 * @param channel 설정할 출력 채널 인덱스
 * @param voltage 출력할 목표 전압 (V, 0.0V ~ 5.0V)
 * @return true 출력 성공, false 실패
 */
bool DAC60516_WriteVoltage(DAC60516_OutputChannel_t channel, float voltage);

#endif /* DAC60516_H */
