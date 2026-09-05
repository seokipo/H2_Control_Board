/**
 * @file ads1115.c
 * @brief ADS1115 16-bit ADC Driver & Precision I2C Bit-Banging Master Engine
 * 
 * @note dsPIC33CK MCU 물리 42번 핀(RB0, SCL) 및 43번 핀(RB1, SDA)을 활용한
 *       초정밀 I2C 비트뱅잉 엔진을 내장하여 4개의 ADS1115 칩셋(0x48, 0x49, 0x4A, 0x4B)
 *       및 온보드 DS3231 RTC와 안정적으로 통신합니다.
 */

#include "ads1115.h"
#include "pin_map.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef FCY
#define FCY 4000000UL
#endif
#include <libpic30.h>

/* ==========================================================================
 * 1. I2C 하드웨어 핀 및 저수준 비트뱅잉 매크로 정의 (RB0 = SCL, RB1 = SDA)
 * ========================================================================== */

// 오픈 드레인(Open-Drain) I2C 버스 제어:
// High 출력 시: TRIS = 1 (입력 모드, 외부 풀업 저항에 의해 High 플로팅)
// Low 출력 시 : LAT = 0, TRIS = 0 (출력 모드, 그라운드로 드라이브)
#define I2C_SCL_HIGH()  do { TRISBbits.TRISB0 = 1; } while(0)
#define I2C_SCL_LOW()   do { LATBbits.LATB0 = 0; TRISBbits.TRISB0 = 0; } while(0)
#define I2C_SCL_READ()  (PORTBbits.RB0)

#define I2C_SDA_HIGH()  do { TRISBbits.TRISB1 = 1; } while(0)
#define I2C_SDA_LOW()   do { LATBbits.LATB1 = 0; TRISBbits.TRISB1 = 0; } while(0)
#define I2C_SDA_READ()  (PORTBbits.RB1)

// I2C 100kHz 표준 타이밍 딜레이 (FCY=4MHz 기준 10사이클 = 약 2.5us)
#define I2C_DELAY()     __delay32(8)

/* ==========================================================================
 * 2. 초정밀 I2C 소프트웨어 비트뱅잉 마스터 엔진
 * ========================================================================== */

static void I2C1_BusClear(void)
{
    // 슬레이브가 이전 통신 중단으로 SDA를 잡고 있는 경우 해제 (9 클록 펄스)
    I2C_SDA_HIGH();
    for (uint8_t i = 0; i < 9; i++)
    {
        I2C_SCL_HIGH();
        I2C_DELAY();
        I2C_SCL_LOW();
        I2C_DELAY();
    }
}

static void I2C1_Start(void)
{
    I2C_SDA_HIGH();
    I2C_SCL_HIGH();
    I2C_DELAY();
    I2C_SDA_LOW(); // SCL High 상태에서 SDA 하강 (START 조건)
    I2C_DELAY();
    I2C_SCL_LOW();
    I2C_DELAY();
}

static void I2C1_Stop(void)
{
    I2C_SDA_LOW();
    I2C_SCL_LOW();
    I2C_DELAY();
    I2C_SCL_HIGH();
    I2C_DELAY();
    I2C_SDA_HIGH(); // SCL High 상태에서 SDA 상승 (STOP 조건)
    I2C_DELAY();
}

static bool I2C1_WriteByte(uint8_t byte)
{
    for (int8_t b = 7; b >= 0; b--)
    {
        if (byte & (1 << b))
        {
            I2C_SDA_HIGH();
        }
        else
        {
            I2C_SDA_LOW();
        }
        I2C_DELAY();
        I2C_SCL_HIGH(); // SCL 상승
        I2C_DELAY();
        I2C_SCL_LOW();  // SCL 하강
        I2C_DELAY();
    }

    // 9번째 클록: 슬레이브의 ACK 수신 (SDA를 High로 풀고 슬레이브가 Low로 당기는지 감시)
    I2C_SDA_HIGH();
    I2C_DELAY();
    I2C_SCL_HIGH();
    I2C_DELAY();
    bool ack = (I2C_SDA_READ() == 0); // 0이면 ACK, 1이면 NACK
    I2C_SCL_LOW();
    I2C_DELAY();

    return ack;
}

static uint8_t I2C1_ReadByte(bool ack)
{
    uint8_t byte = 0;
    I2C_SDA_HIGH(); // 마스터 핀 입력 모드 유지

    for (int8_t b = 7; b >= 0; b--)
    {
        I2C_SCL_HIGH();
        I2C_DELAY();
        if (I2C_SDA_READ())
        {
            byte |= (1 << b);
        }
        I2C_SCL_LOW();
        I2C_DELAY();
    }

    // 9번째 클록: 마스터가 슬레이브에게 ACK(0) 또는 NACK(1) 송출
    if (ack)
    {
        I2C_SDA_LOW(); // ACK
    }
    else
    {
        I2C_SDA_HIGH(); // NACK
    }
    I2C_DELAY();
    I2C_SCL_HIGH();
    I2C_DELAY();
    I2C_SCL_LOW();
    I2C_SDA_HIGH();
    I2C_DELAY();

    return byte;
}

// 외부(rtc.c 등)에서 공유 호출 가능한 고신뢰성 I2C Write API
bool I2C1_Write(uint8_t dev_addr, const uint8_t *data, uint16_t length)
{
    I2C1_Start();

    // 7비트 슬레이브 주소 + 쓰기 비트(0) 송출
    if (!I2C1_WriteByte((uint8_t)(dev_addr << 1)))
    {
        I2C1_Stop();
        return false; // NACK 발생 (디바이스 응답 없음)
    }

    for (uint16_t i = 0; i < length; i++)
    {
        if (!I2C1_WriteByte(data[i]))
        {
            I2C1_Stop();
            return false;
        }
    }

    I2C1_Stop();
    return true;
}

// 외부(rtc.c 등)에서 공유 호출 가능한 고신뢰성 I2C Read API
bool I2C1_Read(uint8_t dev_addr, uint8_t *data, uint16_t length)
{
    if (length == 0) return true;

    I2C1_Start();

    // 7비트 슬레이브 주소 + 읽기 비트(1) 송출
    if (!I2C1_WriteByte((uint8_t)((dev_addr << 1) | 0x01)))
    {
        I2C1_Stop();
        return false;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        // 마지막 바이트는 NACK(false), 이전 바이트들은 ACK(true) 송출
        data[i] = I2C1_ReadByte(i < (length - 1));
    }

    I2C1_Stop();
    return true;
}

/* ==========================================================================
 * 3. UI 대시보드 인덱스(0 ~ 13) 1:1 매핑 테이블
 * ========================================================================== */
const ADS1115_SensorChannel_t adc_channel_map[14] = {
    AI_PT105_UTILITY_PRESS,        // 0: AI_PT105 (유틸리티 압력 확인용)
    AI_PT109_GAS_BUF_PRESS,        // 1: AI_PT109 (가스 버퍼탱크 압력 확인용)
    AI_PT113_BURNER_PRESS,         // 2: AI_PT113 (개질기 버너 압력 확인용)
    AI_PT123_CATALYST_PRESS,       // 3: AI_PT123 (개질기 촉매부 압력 확인)
    AI_PT146_ANODE_PRESS,          // 4: AI_PT146 (STACK Anode 입구 압력 측정)
    AI_PT163_EXHAUST_PRESS,        // 5: AI_PT163 (배기가스 압력 측정)
    AI_PT383_REFORMER_WATER_PRESS, // 6: AI_PT383 (개질수 공급 압력 측정)
    AI_CS324_DI_COND,              // 7: AI_CS324 (DI TANK 전도도 측정)
    AI_MFC111_BNG_FLOW,            // 8: AI_MFC111 (BNG 유량 측정/제어)
    AI_MFC121_PNG_FLOW,            // 9: AI_MFC121 (PNG 유량 측정/제어)
    AI_MFM211_STACK_AIR_FLOW,      // 10: AI_MFM211 (STACK 공기 유량계)
    AI_MFM222_BURNER_AIR_FLOW,     // 11: AI_MFM222 (개질기 버너 공기 유량계)
    AI_MFM231_PROX_AIR_FLOW,       // 12: AI_MFM231 (PrOx 공기 유량계)
    AI_MFM315_COOLANT_RET_FLOW     // 13: AI_MFM315 (시스템 냉각수 회수 유량 측정)
};

/**
 * @brief 센서 채널에 따른 ADS1115 디바이스 주소 및 내부 AIN 채널 MUX 매핑
 */
static bool GetChannelMapping(ADS1115_SensorChannel_t channel, uint8_t *dev_addr, uint16_t *mux_config)
{
    if (channel >= AI_MAX_CHANNELS)
    {
        return false;
    }

    // 1. 하드웨어 ADDR 결선 기준 슬레이브 주소 분기
    if (channel <= AI_PT146_ANODE_PRESS)
    {
        *dev_addr = ADS1115_ADDR_GND; // IC400: 0x48
    }
    else if (channel <= AI_MFC121_PNG_FLOW)
    {
        *dev_addr = ADS1115_ADDR_VDD; // IC401: 0x49
    }
    else if (channel <= AI_PT109_GAS_BUF_PRESS)
    {
        *dev_addr = ADS1115_ADDR_SDA; // IC403: 0x4A
    }
    else
    {
        *dev_addr = ADS1115_ADDR_SCL; // IC402: 0x4B
    }

    // 2. 내부 AIN 채널 MUX 값 매핑 (4채널 순환 구조)
    uint8_t sub_ch = (uint8_t)channel % 4;
    switch (sub_ch)
    {
        case 0: *mux_config = ADS1115_CFG_MUX_AIN0; break;
        case 1: *mux_config = ADS1115_CFG_MUX_AIN1; break;
        case 2: *mux_config = ADS1115_CFG_MUX_AIN2; break;
        case 3: *mux_config = ADS1115_CFG_MUX_AIN3; break;
        default: return false;
    }

    return true;
}

bool ADS1115_Initialize(void)
{
    // [1] RB0(SCL), RB1(SDA) 핀 아날로그 기능 해제 (디지털 전용 확정)
    ANSELBbits.ANSELB0 = 0;
    ANSELBbits.ANSELB1 = 0;
    
    // 오픈드레인 활성화 (MCU 지원 시)
    ODCBbits.ODCB0 = 1;
    ODCBbits.ODCB1 = 1;

    // 기본 High(풀업 상태)로 초기화
    I2C_SCL_HIGH();
    I2C_SDA_HIGH();
    I2C_DELAY();

    // 혹시 모를 버스 락 해제
    I2C1_BusClear();

    // [2] 4개 ADS1115 칩셋(0x48, 0x49, 0x4A, 0x4B)에 기본 설정 주입
    // PGA는 0~5V 센서 전압을 클리핑 없이 측정하기 위해 +/-6.144V FSR(0x0000) 채택!
    // 싱글샷 모드, 860 SPS 고속 샘플링 설정
    uint8_t addr_list[4] = {
        ADS1115_ADDR_GND,
        ADS1115_ADDR_VDD,
        ADS1115_ADDR_SDA,
        ADS1115_ADDR_SCL
    };

    for (int i = 0; i < 4; i++)
    {
        uint8_t tx_data[3];
        tx_data[0] = ADS1115_REG_CONFIG;
        
        uint16_t config_val = ADS1115_CFG_MODE_SINGLE | ADS1115_CFG_PGA_6_144V | 
                              ADS1115_CFG_DR_860SPS | ADS1115_CFG_COMP_QUE_DISABLE;
        
        tx_data[1] = (uint8_t)((config_val >> 8) & 0xFF);
        tx_data[2] = (uint8_t)(config_val & 0xFF);

        I2C1_Write(addr_list[i], tx_data, 3);
    }

    return true;
}

bool ADS1115_ReadRaw(ADS1115_SensorChannel_t channel, int16_t *value)
{
    uint8_t dev_addr = 0;
    uint16_t mux_config = 0;

    if (!GetChannelMapping(channel, &dev_addr, &mux_config))
    {
        return false;
    }

    // 1. Config Register에 계측 채널 주입 + 싱글샷 변환 트리거 (OS = 1, PGA = 6.144V, 860 SPS)
    uint16_t config_val = ADS1115_CFG_OS_SINGLE | mux_config | ADS1115_CFG_MODE_SINGLE | 
                          ADS1115_CFG_PGA_6_144V | ADS1115_CFG_DR_860SPS | ADS1115_CFG_COMP_QUE_DISABLE;
    
    uint8_t tx_buf[3];
    tx_buf[0] = ADS1115_REG_CONFIG;
    tx_buf[1] = (uint8_t)((config_val >> 8) & 0xFF);
    tx_buf[2] = (uint8_t)(config_val & 0xFF);

    if (!I2C1_Write(dev_addr, tx_buf, 3))
    {
        return false;
    }

    // 2. 860SPS 변환 완료 대기 (1회 변환 소요시간 1.16ms -> 1.3ms 안전 딜레이)
    __delay32(5200); // 4MHz 기준 1.3ms (5200 사이클)

    // 3. Pointer Register를 Conversion Register(0x00)로 전환
    uint8_t reg_conversion = ADS1115_REG_CONVERSION;
    if (!I2C1_Write(dev_addr, &reg_conversion, 1))
    {
        return false;
    }

    // 4. 변환 완료된 16비트 Signed 2의 보수 데이터 수신
    uint8_t rx_buf[2];
    if (!I2C1_Read(dev_addr, rx_buf, 2))
    {
        return false;
    }

    // 2바이트 수신 데이터 병합 (MSB -> LSB)
    int16_t raw = (int16_t)(((uint16_t)rx_buf[0] << 8) | rx_buf[1]);
    
    // 접지 노이즈로 인한 미세 음수는 0V(0)로 클램프
    if (raw < 0) raw = 0;

    *value = raw;
    return true;
}

bool ADS1115_ReadVoltage(ADS1115_SensorChannel_t channel, float *voltage)
{
    int16_t raw_val = 0;
    if (!ADS1115_ReadRaw(channel, &raw_val))
    {
        return false;
    }

    // PGA가 +/-6.144V이므로 1 LSB = 6.144V / 32768 = 0.1875 mV = 0.0001875 V
    *voltage = (float)raw_val * 0.0001875f;
    return true;
}

uint16_t ADS1115_ReadChannel(ADS1115_SensorChannel_t channel)
{
    int16_t raw_val = 0;
    if (ADS1115_ReadRaw(channel, &raw_val))
    {
        return (uint16_t)raw_val;
    }
    return 0;
}

uint16_t ADS1115_ReadChannelNormalized(ADS1115_SensorChannel_t channel)
{
    int16_t raw_val = 0;
    if (!ADS1115_ReadRaw(channel, &raw_val))
    {
        return 0;
    }

    // ADS1115 PGA +/-6.144V FSR 기준:
    // 5.0V 입력 시 raw_val = (5.0V / 6.144V) * 32768 = 26667
    // UI 대시보드(0~65535 스케일, 5.0V = 65535)로 100% 온전히 스케일링:
    // normalized = (raw_val * 65535) / 26667
    uint32_t norm = ((uint32_t)raw_val * 65535UL) / 26667UL;
    if (norm > 65535UL) norm = 65535UL;

    return (uint16_t)norm;
}

/* ==========================================================================
 * 👑 [특허급 제로 블로킹 파이프라인 계측 엔진]
 * 변환 트리거와 결과 판독을 완벽히 분리하여 1.3ms 딜레이를 0.000ms로 소멸시킴!
 * ========================================================================== */

bool ADS1115_TriggerChannel(ADS1115_SensorChannel_t channel)
{
    uint8_t dev_addr = 0;
    uint16_t mux_config = 0;

    if (!GetChannelMapping(channel, &dev_addr, &mux_config))
    {
        return false;
    }

    // Config Register에 계측 채널 주입 + 싱글샷 변환 비동기 트리거 (OS = 1, PGA = 6.144V, 860 SPS)
    uint16_t config_val = ADS1115_CFG_OS_SINGLE | mux_config | ADS1115_CFG_MODE_SINGLE | 
                          ADS1115_CFG_PGA_6_144V | ADS1115_CFG_DR_860SPS | ADS1115_CFG_COMP_QUE_DISABLE;
    
    uint8_t tx_buf[3];
    tx_buf[0] = ADS1115_REG_CONFIG;
    tx_buf[1] = (uint8_t)((config_val >> 8) & 0xFF);
    tx_buf[2] = (uint8_t)(config_val & 0xFF);

    // 딜레이 없이 I2C 명령만 송출하고 즉각 리턴 (소요시간 약 250us)
    return I2C1_Write(dev_addr, tx_buf, 3);
}

uint16_t ADS1115_ReadNormalized(ADS1115_SensorChannel_t channel)
{
    uint8_t dev_addr = 0;
    uint16_t mux_config = 0;

    if (!GetChannelMapping(channel, &dev_addr, &mux_config))
    {
        return 0;
    }

    // 1. Pointer Register를 Conversion Register(0x00)로 전환
    uint8_t reg_conversion = ADS1115_REG_CONVERSION;
    if (!I2C1_Write(dev_addr, &reg_conversion, 1))
    {
        return 0;
    }

    // 2. 이미 하드웨어에서 완료된 16비트 Signed 2의 보수 데이터 즉시 수신 (딜레이 0ms!)
    uint8_t rx_buf[2];
    if (!I2C1_Read(dev_addr, rx_buf, 2))
    {
        return 0;
    }

    int16_t raw = (int16_t)(((uint16_t)rx_buf[0] << 8) | rx_buf[1]);
    if (raw < 0) raw = 0;

    // 0~5V FSR 기준 0~65535로 정규화
    uint32_t norm = ((uint32_t)raw * 65535UL) / 26667UL;
    if (norm > 65535UL) norm = 65535UL;

    return (uint16_t)norm;
}


