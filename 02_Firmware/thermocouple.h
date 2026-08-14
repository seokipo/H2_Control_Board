/**
 * @file thermocouple.h
 * @brief Multi-channel Thermocouple (TC) Temperature Scanning Driver for H2_Control_Board
 * 
 * @note 본 드라이버는 회로도 Input_Section.SchDoc 내의 MAX31856(IC301) 및 
 *       6개의 ADG706 아날로그 멀티플렉서 스위칭 제어 설정을 기반으로 구현되었습니다.
 */

#ifndef THERMOCOUPLE_H
#define THERMOCOUPLE_H

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 * 1. 31개 열전대 온도 측정 채널 정의 리스트
 * ========================================================================== */
typedef enum {
    /* --- T-Type 온도 센서 채널 (CH1 ~ CH19) --- */
    TC_CH1_CITY_GAS_IN = 0,         // 시티가스 공급온도 (CH1)
    TC_CH2_BURNER_BACKFIRE,         // 개질기 버너 역화 온도 측정 (CH2)
    TC_CH3_REFORM_GAS_1ST,          // 개질 가스 1차 온도 측정 (CH3)
    TC_CH4_REFORM_GAS_2ND,          // 개질 가스 2차 온도 측정 (ANODE 인입 가스) (CH4)
    TC_CH5_AOG_BURNER_IN,           // AOG 버너 입구 온도측정 (CH5)
    TC_CH6_EXHAUST_GAS,             // 배기가스 온도 측정 (CH6)
    TC_CH7_CATHODE_IN,              // Cathode 입구 온도 (CH7)
    TC_CH8_CATHODE_OUT,             // Cathode 출구 온도 (CH8)
    TC_CH9_HOT_WATER_TANK,          // HOT Water Tank 온도 측정 (CH9)
    TC_CH10_COOLANT_RET,            // 시스템 냉각수 회수 온도 (CH10)
    TC_CH11_DI_WATER_TANK,          // DI Water TANK 온도 센서 (CH11)
    TC_CH12_DI_COOLANT_IN,          // DI WATER TANK 냉각수 입구 온도 측정 (CH12)
    TC_CH13_DI_COOLANT_RET,         // DI WATER TANK 냉각수 회수 온도 측정 (CH13)
    TC_CH14_ANODE_COOL_RET,         // ANODE(RG) 냉각수 회수 온도 측정 (CH14)
    TC_CH15_STACK1_COOL_RET,        // STACK 1 냉각수 회수 온도 (CH15)
    TC_CH16_STACK2_COOL_RET,        // STACK 2 냉각수 회수 온도 (CH16)
    TC_CH17_WASTE_HEAT_IN,          // 배열 회수 입구 온도 (CH17)
    TC_CH18_WASTE_HEAT_OUT,         // 배열 회수 출구 온도 (CH18)
    TC_CH19_SYSTEM_INTERNAL,        // 시스템 내부온도 (CH19)

    /* --- K-Type 온도 센서 채널 (CH21 ~ CH32) --- */
    TC_CH21_REF_BN,                 // Ref-BN (CH21)
    TC_CH22_REF_SR1,                // Ref-SR1 (CH22)
    TC_CH23_REF_SR2,                // Ref-SR2 (CH23)
    TC_CH24_REF_SR3,                // Ref-SR3 (CH24)
    TC_CH25_REF_LTS1,               // Ref-LTS1 (CH25)
    TC_CH26_REF_LTS2,               // Ref-LTS2 (CH26)
    TC_CH27_REF_FEED,               // Ref-FEED (CH27)
    TC_CH28_REF_RG,                 // Ref-RG (CH28)
    TC_CH29_REF_EG,                 // Ref-EG (CH29)
    TC_CH30_PROX_01,                // Prox-01 (CH30)
    TC_CH31_PROX_02,                // Prox-02 (CH31)
    TC_CH32_PROX_03,                // Prox-03 (CH32)

    TC_MAX_CHANNELS
} TC_Channel_t;

/* ==========================================================================
 * 2. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief 다채널 온도 스캐너(MAX31856 & ADG706 MUX) 초기화
 * @return true 초기화 성공, false 실패
 */
bool TC_Initialize(void);

/**
 * @brief 특정 열전대 온도 센서 채널을 활성화하기 위해 MUX 스위칭 수행
 * @param channel 선택할 온도 측정 채널 인덱스
 * @return true 스위칭 성공, false 범위 오류
 */
bool TC_SelectChannel(TC_Channel_t channel);

/**
 * @brief 특정 채널의 실제 계측 온도(Celsius)를 취득
 * @param channel 측정할 온도 채널 인덱스
 * @return float Celsius 단위 실수형 온도 값, 오류 시 -999.0f
 */
float TC_ReadTemperature(TC_Channel_t channel);

#endif /* THERMOCOUPLE_H */
