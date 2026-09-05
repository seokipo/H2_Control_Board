#include "dac60516.h"
#include "pin_map.h"

#ifndef FCY
#define FCY 4000000UL
#endif
#include <libpic30.h>

/* ==========================================================================
 * 내부 비트뱅잉 SPI 통신 엔진
 * DAC60516은 CS Low 상태에서 SCLK의 하강 에지(Falling Edge)에 SDI 데이터를 샘플링함.
 * ========================================================================== */

/**
 * @brief DAC60516 전용 8비트 데이터 송신 (SCLK 하강 에지 샘플링)
 */
static void DAC60516_SendByte(uint8_t data)
{
    for (int8_t b = 7; b >= 0; b--)
    {
        // 1. SCLK High 상태에서 SDI(MOSI) 비트 세팅
        TC_SPI_CLK_LAT = 1;
        TC_SPI_MOSI_LAT = (data & (1 << b)) ? 1 : 0;
        __delay32(4); // SDI 셋업 시간 보장 (5ns min, 실제 1.0us)

        // 2. SCLK 하강 에지 (DAC60516이 SDI 비트 샘플링)
        TC_SPI_CLK_LAT = 0;
        __delay32(4); // SDI 홀드 시간 보장 (8ns min, 실제 1.0us)
    }
}

/**
 * @brief DAC60516 내부 레지스터에 24비트 데이터 전송 (8비트 주소 + 16비트 값)
 * @param reg_addr 레지스터 번지 (Offset 0x00 ~ 0x1F, Bit 7은 Write=0)
 * @param reg_val 16비트 레지스터 데이터 값
 */
static bool DAC60516_SPI_Write(uint8_t reg_addr, uint16_t reg_val)
{
    // [1] 공유 SPI 버스 상의 열전대(MAX31856) CS 비활성화(High) 보장
    TC_SPI_CS_LAT = 1;

    // [2] DAC CS 활성화 (Active-Low)
    DAC_CS_LAT = 0;
    __delay32(4); // tCSS 셋업 시간 (15ns min, 실제 1.0us)

    // [3] 24비트 패킷 연속 전송:
    // Byte 1: [RW=0 (Write), A[6:0]=reg_addr]
    DAC60516_SendByte(reg_addr & 0x7F);
    // Byte 2: 데이터 상위 바이트 (MSB)
    DAC60516_SendByte((uint8_t)((reg_val >> 8) & 0xFF));
    // Byte 3: 데이터 하위 바이트 (LSB)
    DAC60516_SendByte((uint8_t)(reg_val & 0xFF));

    __delay32(4); // tCSH 홀드 시간 (15ns min, 실제 1.0us)

    // [4] DAC CS 비활성화 (High) -> DAC 내부 래치 확정
    DAC_CS_LAT = 1;
    __delay32(8); // tCSHIGH 유휴 시간 (15ns min, 실제 2.0us)

    return true;
}

/**
 * @brief 센서 채널에 따른 DAC60516 내부 출력 레지스터 번호 매핑 조회
 */
static uint8_t GetRegisterAddress(DAC60516_OutputChannel_t channel)
{
    switch (channel)
    {
        case AO_AB232_PROX_BLOWER:    return DAC60516_REG_DAC0;  // 0x10
        case AO_AB212_STACK_BLOWER:   return DAC60516_REG_DAC1;  // 0x11
        case AO_P341_AOG_COOL_PUMP:   return DAC60516_REG_DAC2;  // 0x12
        case AO_P375_STACK2_COOL_PUMP:return DAC60516_REG_DAC3;  // 0x13
        case AO_P108_GAS_PRESS_PUMP:  return DAC60516_REG_DAC4;  // 0x14
        case AO_P370_STACK1_COOL_PUMP:return DAC60516_REG_DAC5;  // 0x15
        case AO_P380_WATER_PUMP:      return DAC60516_REG_DAC6;  // 0x16
        case AO_AB221_BURNER_BLOWER:  return DAC60516_REG_DAC7;  // 0x17
        case AO_MFC111_BNG_FLOW:      return DAC60516_REG_DAC8;  // 0x18
        case AO_MFC121_PNG_FLOW:      return DAC60516_REG_DAC9;  // 0x19
        case AO_P351_ANODE_COOL_PUMP: return DAC60516_REG_DAC10; // 0x1A
        case AO_SPARE1:               return DAC60516_REG_DAC11; // 0x1B
        default:                      return DAC60516_REG_NOOP;
    }
}

bool DAC60516_Initialize(void)
{
    // [1] CS 및 SPI 공용 핀 방향 설정 및 초기화
    DAC_CS_LAT = 1;
    DAC_CS_TRIS = 0;
    TC_SPI_CS_LAT = 1;
    TC_SPI_CS_TRIS = 0;
    TC_SPI_CLK_TRIS = 0;
    TC_SPI_MOSI_TRIS = 0;
    TC_SPI_CLK_LAT = 0;
    TC_SPI_MOSI_LAT = 0;

    __delay32(40000); // 부팅 후 10ms 하드웨어 안정화 대기

    // [2] 소프트웨어 리셋 (TRIGGER 레지스터 0x05에 SOFT_RST = 0x000A)
    DAC60516_SPI_Write(DAC60516_REG_TRIGGER, 0x000A);
    __delay32(40000); // 리셋 완료 및 준비 대기 (tAMCRDY = 10ms)

    // [3] GEN_CONFIG 레지스터 (0x09): 내부 2.5V 고정밀 레퍼런스 활성화
    // Bit 2: REF_PWDWN = 0 (내부 전압 기준원 가동), Bit 4: ODE = 1 (기본 오픈드레인)
    DAC60516_SPI_Write(DAC60516_REG_GEN_CONFIG, 0x0000);
    __delay32(40000); // 10ms 내부 레퍼런스 PLL/충전 안정화 대기

    // [4] DAC_GAIN 레지스터 (0x04): 모든 QUAD 채널 출력 게인 2x 설정 (0V ~ 5.0V 출력 범위)
    // Bit 0 (OUT0~3), Bit 1 (OUT4~7), Bit 2 (OUT8~11), Bit 3 (OUT12~15) = 1 -> 0x000F
    DAC60516_SPI_Write(DAC60516_REG_GAIN, 0x000F);

    // [5] PWDWN 레지스터 (0x03): 16개 모든 DAC 채널 파워다운 해제 (0 = 정상 활성화)
    // 리셋 시 0xFFFF(모두 전원 꺼짐)이므로 0x0000을 써서 전 채널 파워 온!
    DAC60516_SPI_Write(DAC60516_REG_PWDWN, 0x0000);

    // [6] SYNC_EN 레지스터 (0x0A): 전 채널 비동기(Asynchronous) 즉시 출력 모드 (0x0000)
    DAC60516_SPI_Write(DAC60516_REG_SYNC_EN, 0x0000);

    // [7] 초기 상태: 12개 모든 아날로그 출력 포트 0V(0x0000) 안전 초기화
    for (uint8_t ch = 0; ch < AO_MAX_CHANNELS; ch++)
    {
        DAC60516_WriteRaw((DAC60516_OutputChannel_t)ch, 0x0000);
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

    // DAC60516(12비트 DAC) 데이터 포맷: Bit [15:4]에 12비트 데이터 배치, Bit [3:0]은 Reserved 0h
    // 16비트 Modbus raw_value(0 ~ 65535)의 상위 12비트 정밀도를 100% 온전히 보존
    uint16_t send_val = raw_value & 0xFFF0;

    return DAC60516_SPI_Write(reg_addr, send_val);
}

bool DAC60516_WriteVoltage(DAC60516_OutputChannel_t channel, float voltage)
{
    if (voltage < 0.0f)  voltage = 0.0f;
    if (voltage > 5.0f)  voltage = 5.0f; // 최대 작동 사양 5V 제한

    // 16비트 해상도 (0 ~ 65535) 매핑: Vout = (raw / 65535) * 5.0V
    uint32_t raw_cal = (uint32_t)((voltage / 5.0f) * 65535.0f);
    if (raw_cal > 65535) raw_cal = 65535;

    return DAC60516_WriteRaw(channel, (uint16_t)raw_cal);
}
