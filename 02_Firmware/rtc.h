/**
 * @file rtc.h
 * @brief DS3231 High-Precision RTC Driver Header
 * 
 * @note 본 드라이버는 온보드 I2C1 채널을 공유하여 DS3231SN+ RTC와 통신하며,
 *       실시간 시간 설정 및 획득 기능을 제공합니다.
 */

#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stdbool.h>

// DS3231 7비트 I2C 슬레이브 주소
#define RTC_I2C_ADDR            0x68

// DS3231 내부 레지스터 맵
#define RTC_REG_SECONDS         0x00
#define RTC_REG_MINUTES         0x01
#define RTC_REG_HOURS           0x02
#define RTC_REG_DAY             0x03
#define RTC_REG_DATE            0x04
#define RTC_REG_MONTH           0x05
#define RTC_REG_YEAR            0x06

// 시간 데이터를 관리하기 위한 구조체 정의
typedef struct {
    uint8_t year;       // 년 (00 ~ 99)
    uint8_t month;      // 월 (01 ~ 12)
    uint8_t date;       // 일 (01 ~ 31)
    uint8_t day;        // 요일 (1: 일, 2: 월, ... 7: 토)
    uint8_t hour;       // 시 (00 ~ 23, 24시간 형식 지원)
    uint8_t minute;     // 분 (00 ~ 59)
    uint8_t second;     // 초 (00 ~ 59)
} DateTime_t;

/* ==========================================================================
 * 1. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief DS3231 RTC 초기화 (초기 통신 확인 및 24시간 형식 강제 지정)
 * @return bool 초기화 성공 여부 (I2C 통신 확인 결과)
 */
bool RTC_Initialize(void);

/**
 * @brief RTC로부터 현재 실시간 시각 획득
 * @param dt 수신한 시간 정보를 기록할 구조체 포인터
 * @return bool 시간 읽기 성공 여부
 */
bool RTC_GetTime(DateTime_t *dt);

/**
 * @brief RTC에 새로운 시간 정보 기입 (시간 동기화용)
 * @param dt 새로 설정할 시간 정보를 담은 구조체 포인터
 * @return bool 시간 설정 성공 여부
 */
bool RTC_SetTime(const DateTime_t *dt);

#endif /* RTC_H */
