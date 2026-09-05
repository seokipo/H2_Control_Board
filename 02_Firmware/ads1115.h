/**
 * @file ads1115.h
 * @brief ADS1115 16-bit ADC Driver & Channel Mapping for H2_Control_Board
 * 
 * @note 본 드라이버는 회로도 Input_Section.SchDoc 내의 ADS1115 4개 칩(IC400~IC403)
 *       연동 사양을 기반으로 구현되었습니다. dsPIC33CK MCU의 I2C 모듈과 연동합니다.
 */

#ifndef ADS1115_H
#define ADS1115_H

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 * 1. ADS1115 I2C 7비트 슬레이브 주소 정의 (ADDR 핀 결선 사양)
 * ========================================================================== */
#define ADS1115_ADDR_GND        0x48    // IC400 (ADDR -> GND)
#define ADS1115_ADDR_VDD        0x49    // IC401 (ADDR -> VDD)
#define ADS1115_ADDR_SDA        0x4A    // IC403 (ADDR -> SDA)
#define ADS1115_ADDR_SCL        0x4B    // IC402 (ADDR -> SCL)

/* ==========================================================================
 * 2. ADS1115 내부 레지스터 포인터 정의
 * ========================================================================== */
#define ADS1115_REG_CONVERSION  0x00    // 변환 결과 데이터 레지스터 (R)
#define ADS1115_REG_CONFIG      0x01    // 칩 설정 레지스터 (R/W)
#define ADS1115_REG_LO_THRESH   0x02    // 비교기 하한치 레지스터 (R/W)
#define ADS1115_REG_HI_THRESH   0x03    // 비교기 상한치 레지스터 (R/W)

/* ==========================================================================
 * 3. Config Register 설정 마스크 정의
 * ========================================================================== */
#define ADS1115_CFG_OS_SINGLE   0x8000  // 싱글 컨버전 시작

// 입력 MUX 설정 (단단 입력 AINx vs GND)
#define ADS1115_CFG_MUX_AIN0    0x4000  // AIN0 vs GND
#define ADS1115_CFG_MUX_AIN1    0x5000  // AIN1 vs GND
#define ADS1115_CFG_MUX_AIN2    0x6000  // AIN2 vs GND
#define ADS1115_CFG_MUX_AIN3    0x7000  // AIN3 vs GND

// 증폭기 이득(PGA) 설정
#define ADS1115_CFG_PGA_6_144V  0x0000  // +/-6.144V 레인지
#define ADS1115_CFG_PGA_4_096V  0x0200  // +/-4.096V 레인지 (기본)
#define ADS1115_CFG_PGA_2_048V  0x0400  // +/-2.048V 레인지

// 모드 설정
#define ADS1115_CFG_MODE_SINGLE 0x0100  // 싱글 샷 모드 (초절전/기본)
#define ADS1115_CFG_MODE_CONT   0x0000  // 연속 변환 모드

// 데이터 속도(SPS) 설정
#define ADS1115_CFG_DR_128SPS   0x0080  // 128 SPS (기본)
#define ADS1115_CFG_DR_860SPS   0x00E0  // 860 SPS (고속)

#define ADS1115_CFG_COMP_QUE_DISABLE 0x0003 // 비교기 비활성화 (기본)

/* ==========================================================================
 * 4. 16개 아날로그 입력 센서 채널 정의 리스트
 * ========================================================================== */
typedef enum {
    /* --- IC400 (Address: 0x48) 계통 --- */
    AI_PT383_REFORMER_WATER_PRESS = 0, // 개질수 공급 압력 측정 (AIN0 - AI_PT383)
    AI_PT163_EXHAUST_PRESS,            // 배기가스 압력 측정 (AIN1 - AI_PT163)
    AI_PT105_UTILITY_PRESS,            // 유틸리티 압력 확인용 (AIN2 - AI_PT105)
    AI_PT146_ANODE_PRESS,              // STACK Anode 입구 압력 측정 (AIN3 - AI_PT146)

    /* --- IC401 (Address: 0x49) 계통 --- */
    AI_MFM211_STACK_AIR_FLOW,          // STACK 공기 유량계 (AIN0 - AI_MFM211)
    AI_MFM222_BURNER_AIR_FLOW,         // 개질기 버너 공기 유량계 (AIN1 - AI_MFM222)
    AI_MFC111_BNG_FLOW,                // BNG 유량 측정/제어 (AIN2 - AI_MFC111)
    AI_MFC121_PNG_FLOW,                // PNG 유량 측정/제어 (AIN3 - AI_MFC121)

    /* --- IC403 (Address: 0x4A) 계통 --- */
    AI_PT113_BURNER_PRESS,             // 개질기 버너 압력 확인용 (AIN0 - AI_PT113)
    AI_PT123_CATALYST_PRESS,           // 개질기 촉매부 압력 확인 (AIN1 - AI_PT123)
    AI_CS324_DI_COND,                  // DI TANK 전도도 측정 (AIN2 - AI_CS324)
    AI_PT109_GAS_BUF_PRESS,            // 가스 버퍼탱크 압력 확인용 (AIN3 - AI_PT109)

    /* --- IC402 (Address: 0x4B) 계통 --- */
    AI_TC_AMB_I,                       // 대기 온도 피드백 (AIN0 - TC_AMB_I)
    AI_SPARE1,                         // 아날로그 센서 spare1 (AIN1 - AI_spare1)
    AI_MFM315_COOLANT_RET_FLOW,        // 시스템 냉각수 회수 유량 측정 (AIN2 - AI_MFM315)
    AI_MFM231_PROX_AIR_FLOW,           // PrOx 공기 유량계 (AIN3 - AI_MFM231)
    
    AI_MAX_CHANNELS
} ADS1115_SensorChannel_t;

/* ==========================================================================
 * 5. 드라이버 API 함수 원형
 * ========================================================================== */

// UI 대시보드 인덱스(0 ~ 13)와 매칭되는 센서 채널 순서 테이블 (14개 유효 센서)
extern const ADS1115_SensorChannel_t adc_channel_map[14];

/**
 * @brief ADS1115 4개 칩의 통신 상태 검증 및 I2C 버스 초기화
 * @return true 초기화 성공, false 실패
 */
bool ADS1115_Initialize(void);

/**
 * @brief 특정 센서 채널의 아날로그 원시 값(Raw Data) 계측
 * @param channel 계측할 센서 채널 인덱스
 * @param[out] value 읽어온 16비트 ADC 원시 값 결과
 * @return true 계측 성공, false 통신 오류로 인한 실패
 */
bool ADS1115_ReadRaw(ADS1115_SensorChannel_t channel, int16_t *value);

/**
 * @brief 특정 센서 채널의 계측 전압(Voltage) 환산
 * @param channel 계측할 센서 채널 인덱스
 * @param[out] voltage 환산된 전압 결과값 (V)
 * @return true 성공, false 실패
 */
bool ADS1115_ReadVoltage(ADS1115_SensorChannel_t channel, float *voltage);

/**
 * @brief 특정 센서 채널의 16비트 ADC 원시 값 직접 반환
 * @param channel 계측할 센서 채널 인덱스
 * @return uint16_t 16비트 ADC 원시 값
 */
uint16_t ADS1115_ReadChannel(ADS1115_SensorChannel_t channel);

/**
 * @brief 특정 센서 채널의 전압을 0V~5V FSR 기준 0~65535로 정규화하여 반환 (UI 직결용)
 * @param channel 계측할 센서 채널 인덱스
 * @return uint16_t 0 ~ 65535 정규화된 ADC 코드 (5.0V = 65535)
 */
uint16_t ADS1115_ReadChannelNormalized(ADS1115_SensorChannel_t channel);

/**
 * @brief 👑 [제로 블로킹 파이프라인] 센서 채널 변환 비동기 트리거 (딜레이 0ms)
 * @param channel 트리거할 센서 채널
 * @return true 성공, false I2C 오류
 */
bool ADS1115_TriggerChannel(ADS1115_SensorChannel_t channel);

/**
 * @brief 👑 [제로 블로킹 파이프라인] 이전 완료된 변환 결과 즉시 판독 및 0~65535 정규화 반환 (딜레이 0ms)
 * @param channel 판독할 센서 채널
 * @return uint16_t 정규화된 0~65535 값
 */
uint16_t ADS1115_ReadNormalized(ADS1115_SensorChannel_t channel);

#endif /* ADS1115_H */

