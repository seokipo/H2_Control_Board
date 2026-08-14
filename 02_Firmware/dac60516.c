/**
 * @file dac60516.c
 * @brief DAC60516 16-bit 16-channel DAC Driver Implementation for H2_Control_Board
 * 
 * @note dsPIC33CK의 SPI1 모듈과 pin_map.h의 DAC_CS 제어선(RB8)을 활용하여
 *       실제 DAC60516 칩과 24비트 SPI 통신 및 아날로그 출력을 수행합니다.
 */

#include "dac60516.h"
#include "pin_map.h"

// MCC Melody SPI1 드라이버 헤더 가상 인클루드 (컴파일 연동용)
// #include "mcc_generated_files/spi_host/spi1.h"

/* ==========================================================================
 * 내부 보조 함수: SPI 가상 데이터 교환 인터페이스 정의
 * (실제 MCC Melody SPI 라이브러리가 제너레이트되면 이에 맞게 매핑됩니다.)
 * ========================================================================== */

// 가상 SPI1 8비트 데이터 송수신 함수 (MCC Melody SPI1 라이브러리 사용 가정)
__attribute__((weak)) uint8_t SPI1_Exchange8bit(uint8_t data)
{
    // 사용자 하드웨어 SPI1 Write/Read 1바이트 교환 매핑 공간
    return 0;
}

/**
 * @brief DAC60516 내부 레지스터에 24비트 데이터 전송 (8비트 주소 + 16비트 값)
 */
static bool DAC60516_SPI_Write(uint8_t reg_addr, uint16_t reg_val)
{
    // CS 핀을 활성화 (Low)
    DAC_CS_LAT = 0;
    
    // 24비트 데이터 송신 (8비트 주소 -> 8비트 데이터 MSB -> 8비트 데이터 LSB)
    SPI1_Exchange8bit(reg_addr);
    SPI1_Exchange8bit((uint8_t)((reg_val >> 8) & 0xFF));
    SPI1_Exchange8bit((uint8_t)(reg_val & 0xFF));
    
    // CS 핀을 비활성화 (High)
    DAC_CS_LAT = 1;
    
    return true;
}

/**
 * @brief 센서 채널에 따른 DAC60516 내부 출력 레지스터 번호 매핑 조회
 */
static uint8_t GetRegisterAddress(DAC60516_OutputChannel_t channel)
{
    switch (channel)
    {
        case AO_AB232_PROX_BLOWER:    return DAC60516_REG_DAC0;
        case AO_AB212_STACK_BLOWER:   return DAC60516_REG_DAC1;
        case AO_P341_AOG_COOL_PUMP:   return DAC60516_REG_DAC2;
        case AO_P375_STACK2_COOL_PUMP:return DAC60516_REG_DAC3;
        case AO_P108_GAS_PRESS_PUMP:  return DAC60516_REG_DAC4;
        case AO_P370_STACK1_COOL_PUMP:return DAC60516_REG_DAC5;
        case AO_P380_WATER_PUMP:      return DAC60516_REG_DAC6;
        case AO_AB221_BURNER_BLOWER:  return DAC60516_REG_DAC7;
        case AO_MFC111_BNG_FLOW:      return DAC60516_REG_DAC8;
        case AO_MFC121_PNG_FLOW:      return DAC60516_REG_DAC9;
        case AO_P351_ANODE_COOL_PUMP: return DAC60516_REG_DAC10;
        case AO_SPARE1:               return DAC60516_REG_DAC11;
        default:                      return DAC60516_REG_NOOP;
    }
}

bool DAC60516_Initialize(void)
{
    // CS 핀의 방향을 출력(0)으로 설정하고 하이(1)로 초기화
    DAC_CS_LAT = 1;
    DAC_CS_TRIS = 0;

    // 1. DAC CONFIG 레지스터 초기화: 내부 레퍼런스 활성화(REF_PWDN = 0)
    // CONFIG Register: 0x03 번지, 값 0x0000 (기본 내부 레퍼런스 및 정상 전원 인가)
    if (!DAC60516_SPI_Write(DAC60516_REG_CONFIG, 0x0000))
    {
        return false;
    }

    // 2. 이득(GAIN) 레지스터 초기화: 모든 채널 버퍼 게인 = 2x 설정 (출력 범위 0V ~ 5V 확보)
    // GAIN Register: 0x04 번지, 기본 0x0000 설정 시 모든 채널 게인 2x 적용 (레퍼런스 2.5V * 2 = 5V)
    if (!DAC60516_SPI_Write(DAC60516_REG_GAIN, 0x0000))
    {
        return false;
    }

    return true;
}

bool DAC60516_WriteRaw(DAC60516_OutputChannel_t channel, uint16_t raw_value)
{
    uint8_t reg_addr = GetRegisterAddress(channel);
    if (reg_addr == DAC60516_REG_NOOP)
    {
        return false;
    }

    return DAC60516_SPI_Write(reg_addr, raw_value);
}

bool DAC60516_WriteVoltage(DAC60516_OutputChannel_t channel, float voltage)
{
    if (voltage < 0.0f)  voltage = 0.0f;
    if (voltage > 5.0f)  voltage = 5.0f; // 최대 작동 사양 5V 제한

    // 16비트 해상도 (0 ~ 65535) 매핑: Vout = (raw / 65536) * 5.0V
    // raw = (voltage / 5.0V) * 65536
    uint32_t raw_cal = (uint32_t)((voltage / 5.0f) * 65535.0f);
    if (raw_cal > 65535) raw_cal = 65535;

    return DAC60516_WriteRaw(channel, (uint16_t)raw_cal);
}
