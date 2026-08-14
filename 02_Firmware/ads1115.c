/**
 * @file ads1115.c
 * @brief ADS1115 16-bit ADC Driver Implementation for H2_Control_Board
 * 
 * @note dsPIC33CK의 I2C1 모듈(Melody 호환 인터페이스)을 활용하여
 *       실제 ADS1115 하드웨어 레지스터와 통신 및 데이터 획득을 수행합니다.
 */

#include "ads1115.h"

// MCC Melody I2C1 드라이버 헤더 가상 인클루드 (컴파일 연동용)
// #include "mcc_generated_files/i2c_host/i2c1.h"

/* ==========================================================================
 * 내부 보조 함수: I2C 가상 쓰기/읽기 인터페이스 정의
 * (실제 MCC Melody I2C 라이브러리가 제너레이트되면 이에 맞게 매핑됩니다.)
 * ========================================================================== */

// 가상 I2C 드라이버 API 매핑 (MCC Melody I2C1 라이브러리 사용 가정)
__attribute__((weak)) bool I2C1_Write(uint16_t address, uint8_t *data, size_t length)
{
    // 사용자 하드웨어 I2C1 Write 함수 매핑 공간
    return true; 
}

__attribute__((weak)) bool I2C1_Read(uint16_t address, uint8_t *data, size_t length)
{
    // 사용자 하드웨어 I2C1 Read 함수 매핑 공간
    return true;
}

/**
 * @brief 센서 채널에 따른 ADS1115 디바이스 주소 및 내부 AIN 채널 매핑 조회
 */
static bool GetChannelMapping(ADS1115_SensorChannel_t channel, uint8_t *dev_addr, uint16_t *mux_config)
{
    if (channel >= AI_MAX_CHANNELS)
    {
        return false;
    }

    // 1. 디바이스 주소 매핑
    if (channel <= AI_PT146_ANODE_PRESS)
    {
        *dev_addr = ADS1115_ADDR_GND;
    }
    else if (channel <= AI_MFC121_PNG_FLOW)
    {
        *dev_addr = ADS1115_ADDR_VDD;
    }
    else if (channel <= AI_PT109_GAS_BUF_PRESS)
    {
        *dev_addr = ADS1115_ADDR_SDA;
    }
    else
    {
        *dev_addr = ADS1115_ADDR_SCL;
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
    bool status = true;
    uint8_t addr_list[4] = {
        ADS1115_ADDR_GND,
        ADS1115_ADDR_VDD,
        ADS1115_ADDR_SDA,
        ADS1115_ADDR_SCL
    };

    // 4개의 ADS1115 칩에 기본 초기화 명령(Config Register 설정) 전송
    for (int i = 0; i < 4; i++)
    {
        uint8_t tx_data[3];
        tx_data[0] = ADS1115_REG_CONFIG; // 포인터 주소 설정
        
        // 싱글샷 변환 모드, +/-4.096V PGA 레인지, 128SPS, 비교기 비활성화 기본값 설정
        uint16_t config_val = ADS1115_CFG_MODE_SINGLE | ADS1115_CFG_PGA_4_096V | 
                              ADS1115_CFG_DR_128SPS | ADS1115_CFG_COMP_QUE_DISABLE;
        
        tx_data[1] = (uint8_t)((config_val >> 8) & 0xFF);
        tx_data[2] = (uint8_t)(config_val & 0xFF);

        // I2C 버스를 통해 Config Write 실행
        if (!I2C1_Write(addr_list[i], tx_data, 3))
        {
            status = false; // 하나라도 통신 에러가 나면 false 반환
        }
    }

    return status;
}

bool ADS1115_ReadRaw(ADS1115_SensorChannel_t channel, int16_t *value)
{
    uint8_t dev_addr = 0;
    uint16_t mux_config = 0;

    if (!GetChannelMapping(channel, &dev_addr, &mux_config))
    {
        return false;
    }

    // 1. Config Register에 계측할 MUX 채널을 주입하고 싱글샷 변환 시작(OS = 1) 명령 전송
    uint16_t config_val = ADS1115_CFG_OS_SINGLE | mux_config | ADS1115_CFG_MODE_SINGLE | 
                          ADS1115_CFG_PGA_4_096V | ADS1115_CFG_DR_860SPS | ADS1115_CFG_COMP_QUE_DISABLE;
    
    uint8_t tx_buf[3];
    tx_buf[0] = ADS1115_REG_CONFIG;
    tx_buf[1] = (uint8_t)((config_val >> 8) & 0xFF);
    tx_buf[2] = (uint8_t)(config_val & 0xFF);

    if (!I2C1_Write(dev_addr, tx_buf, 3))
    {
        return false;
    }

    // 2. ADC 변환 대기 (860SPS 기준 변환 완료까지 최소 1.2ms 대기 필요)
    // dsPIC33CK MCU Delay 함수 등을 활용한 하드웨어 딜레이 수행 공간
    for (volatile uint16_t delay = 0; delay < 1500; delay++); 

    // 3. Pointer Register를 Conversion Register로 전환
    uint8_t reg_conversion = ADS1115_REG_CONVERSION;
    if (!I2C1_Write(dev_addr, &reg_conversion, 1))
    {
        return false;
    }

    // 4. 변환 완료된 16비트 데이터 수신
    uint8_t rx_buf[2];
    if (!I2C1_Read(dev_addr, rx_buf, 2))
    {
        return false;
    }

    // 2바이트 수신 데이터 병합 (MSB -> LSB)
    *value = (int16_t)(((uint16_t)rx_buf[0] << 8) | rx_buf[1]);
    return true;
}

bool ADS1115_ReadVoltage(ADS1115_SensorChannel_t channel, float *voltage)
{
    int16_t raw_val = 0;
    if (!ADS1115_ReadRaw(channel, &raw_val))
    {
        return false;
    }

    // PGA 설정이 +/-4.096V이므로, LSB 당 전압 강하는 4.096V / 32768 = 0.125mV = 0.000125V
    // (ADS1115는 16비트 양/음방향 센싱용 부호 있는 정수 포맷을 가집니다.)
    *voltage = (float)raw_val * 0.000125f;
    return true;
}
