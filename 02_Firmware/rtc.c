/**
 * @file rtc.c
 * @brief DS3231 High-Precision RTC Driver Implementation
 * 
 * @note 본 드라이버는 ads1115.c 에 구축된 I2C1 송수신 함수들을 외부 연동하여 작동하며,
 *       DS3231 레지스터의 BCD 형식 데이터를 Dec 10진수로 변환하여 핸들링합니다.
 */

#include "rtc.h"
#include <xc.h>

// ads1115.c 에 탑재되어 있는 하부 I2C1 통신 API 외부 참조 연동
extern bool I2C1_Write(uint8_t dev_addr, const uint8_t *data, uint16_t length);
extern bool I2C1_Read(uint8_t dev_addr, uint8_t *data, uint16_t length);

// BCD (Binary Coded Decimal) <-> DEC (Decimal) 고속 변환 인라인 변환기
static inline uint8_t BCD2DEC(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

static inline uint8_t DEC2BCD(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

bool RTC_Initialize(void)
{
    uint8_t reg_addr = RTC_REG_SECONDS;
    uint8_t sec_data = 0;
    
    // [1] DS3231 리얼타임 클록 장치 I2C1 통신 테스트
    if (!I2C1_Write(RTC_I2C_ADDR, &reg_addr, 1))
    {
        return false; // I2C 통신 실패
    }
    
    if (!I2C1_Read(RTC_I2C_ADDR, &sec_data, 1))
    {
        return false;
    }
    
    // [2] 24시간 형식 지정 상태 확인 및 복원
    // Hours 레지스터(0x02)의 bit 6 가 0 이어야 24시간제 작동 모드입니다.
    reg_addr = RTC_REG_HOURS;
    uint8_t hours_val = 0;
    
    if (I2C1_Write(RTC_I2C_ADDR, &reg_addr, 1) && I2C1_Read(RTC_I2C_ADDR, &hours_val, 1))
    {
        if (hours_val & 0x40) // 12시간제로 되어 있을 경우 (bit 6 = 1)
        {
            // 24시간제로 복원 제어 (bit 6 = 0 클리어)
            uint8_t tx_buf[2];
            tx_buf[0] = RTC_REG_HOURS;
            tx_buf[1] = hours_val & 0xBF; // 12/24 비트 해제
            I2C1_Write(RTC_I2C_ADDR, tx_buf, 2);
        }
    }

    return true;
}

bool RTC_GetTime(DateTime_t *dt)
{
    if (dt == 0) return false;

    uint8_t reg_addr = RTC_REG_SECONDS;
    uint8_t rx_buf[7];

    // [1] 레지스터 주소 포인터를 0x00(Seconds)으로 지칭 후 7바이트 연속 읽기
    if (!I2C1_Write(RTC_I2C_ADDR, &reg_addr, 1))
    {
        return false;
    }

    if (!I2C1_Read(RTC_I2C_ADDR, rx_buf, 7))
    {
        return false;
    }

    // [2] 수신된 BCD 포맷 데이터를 10진수로 언팩킹하여 저장
    dt->second = BCD2DEC(rx_buf[0] & 0x7F);
    dt->minute = BCD2DEC(rx_buf[1] & 0x7F);
    
    // Hours 24시간제 파싱 (bit 5: 20 hour, bit 4: 10 hour)
    dt->hour   = BCD2DEC(rx_buf[2] & 0x3F);
    
    dt->day    = rx_buf[3] & 0x07; // 요일 (1~7)
    dt->date   = BCD2DEC(rx_buf[4] & 0x3F);
    
    // Month 레지스터의 MSB(bit 7)는 Century 비트이므로 마스킹 제거
    dt->month  = BCD2DEC(rx_buf[5] & 0x1F);
    dt->year   = BCD2DEC(rx_buf[6]);

    return true;
}

bool RTC_SetTime(const DateTime_t *dt)
{
    if (dt == 0) return false;

    uint8_t tx_buf[8];

    // [1] 기입할 7바이트 시간 레지스터 데이터를 BCD 포맷으로 패킹
    tx_buf[0] = RTC_REG_SECONDS; // 레지스터 쓰기 시작 번지
    tx_buf[1] = DEC2BCD(dt->second);
    tx_buf[2] = DEC2BCD(dt->minute);
    
    // 24시간제 활성화 기입 (bit 6 = 0 강제 클리어 상태로 BCD 기입)
    tx_buf[3] = DEC2BCD(dt->hour) & 0x3F;
    
    tx_buf[4] = dt->day & 0x07;
    tx_buf[5] = DEC2BCD(dt->date);
    tx_buf[6] = DEC2BCD(dt->month);
    tx_buf[7] = DEC2BCD(dt->year);

    // [2] I2C 버스를 통해 RTC 메모리 일괄 쓰기 수행
    return I2C1_Write(RTC_I2C_ADDR, tx_buf, 8);
}
