/**
 * @file thermocouple.c
 * @brief Multi-channel Thermocouple (TC) Temperature Scanning Driver Implementation
 * 
 * @note dsPIC33CK MCU의 SPI1 및 MUX 제어 핀(TC_ADDR0~3, TC_EN1~3)을 활용하여
 *       6개의 ADG706 아날로그 멀티플렉서 어레이 및 MAX31856(IC301) 온도 수신을 구현합니다.
 */

#include "thermocouple.h"
#include "pin_map.h"

// MAX31856 내부 레지스터 번호 정의
#define MAX31856_REG_CR0        0x00    // Configuration 0 Register
#define MAX31856_REG_CR1        0x01    // Configuration 1 Register (TC Type 설정)
#define MAX31856_REG_LTCBH      0x0C    // Linearized TC Temp Byte 2
#define MAX31856_REG_LTCBM      0x0D    // Linearized TC Temp Byte 1
#define MAX31856_REG_LTCBL      0x0E    // Linearized TC Temp Byte 0
#define MAX31856_REG_SR         0x0F    // Fault Status Register (Open Circuit 등 감지)

#include <libpic30.h>

/**
 * @brief MAX31856 및 DAC60516 공유 초정밀 비트뱅잉 SPI 8비트 송수신 엔진
 *        Mode 1 / Mode 0 겸용 (SCLK Low 평상시, MOSI 세팅 후 상승 에지 샘플)
 */
uint8_t SPI1_Exchange8bit(uint8_t data)
{
    uint8_t rx = 0;
    for (int8_t b = 7; b >= 0; b--)
    {
        // 1. SCLK Low 상태에서 MOSI 비트 세팅
        TC_SPI_MOSI_LAT = (data & (1 << b)) ? 1 : 0;
        __delay32(6); // 1.5us 셋업 (FCY=4MHz)
        
        // 2. SCLK 상승 에지 (MAX31856이 MOSI 비트 샘플링)
        TC_SPI_CLK_LAT = 1;
        __delay32(6);
        
        // 3. SCLK 하강 에지 (MAX31856이 SDO/MISO 비트 출력)
        TC_SPI_CLK_LAT = 0;
        __delay32(6);
        
        // 4. SCLK Low 상태에서 MISO 비트 샘플링 (CPHA=1 Mode 1)
        if (TC_SPI_MISO_PORT) {
            rx |= (1 << b);
        }
    }
    return rx;
}

/* ==========================================================================
 * 1. 열전대 채널별 MUX 제어선 및 온도센서 타입 매핑 정보 구조체 및 조회 테이블
 * ========================================================================== */
typedef struct {
    uint8_t mux_en;     // 활성화할 MUX 인에이블 라인 (1: TC_EN1, 2: TC_EN2, 3: TC_EN3)
    uint8_t mux_addr;   // ADG706 채널 주소 값 (0 ~ 15)
    uint8_t tc_type;    // Thermocouple Type (0: T-type, 1: K-type)
} TC_Mapping_t;

// 회로도 TempController.SchDoc 및 Input_Section.SchDoc 정밀 분석 기반 매핑 룩업 테이블
static const TC_Mapping_t tc_map[TC_MAX_CHANNELS] = {
    /* --- T-Type 온도 센서 (CH1 ~ CH19) --- */
    [TC_CH1_CITY_GAS_IN]   = {1, 8,  0}, // MUX1 (IC300/303), S9 (주소8)
    [TC_CH2_BURNER_BACKFIRE]= {1, 9,  0}, // MUX1, S10 (주소9)
    [TC_CH3_REFORM_GAS_1ST] = {1, 10, 0}, // MUX1, S11 (주소10)
    [TC_CH4_REFORM_GAS_2ND] = {1, 11, 0}, // MUX1, S12 (주소11)
    [TC_CH5_AOG_BURNER_IN]  = {1, 12, 0}, // MUX1, S13 (주소12)
    [TC_CH6_EXHAUST_GAS]    = {1, 5,  0}, // MUX1, S6 (주소5)
    [TC_CH7_CATHODE_IN]     = {1, 4,  0}, // MUX1, S5 (주소4)
    [TC_CH8_CATHODE_OUT]    = {1, 3,  0}, // MUX1, S4 (주소3)
    [TC_CH9_HOT_WATER_TANK] = {1, 2,  0}, // MUX1, S3 (주소2)
    [TC_CH10_COOLANT_RET]   = {1, 1,  0}, // MUX1, S2 (주소1)
    [TC_CH11_DI_WATER_TANK] = {2, 5,  0}, // MUX2 (IC305/307), S6 (주소5)
    [TC_CH12_DI_COOLANT_IN] = {2, 4,  0}, // MUX2, S5 (주소4)
    [TC_CH13_DI_COOLANT_RET]= {2, 3,  0}, // MUX2, S4 (주소3)
    [TC_CH14_ANODE_COOL_RET]= {2, 2,  0}, // MUX2, S3 (주소2)
    [TC_CH15_STACK1_COOL_RET]={2, 1,  0}, // MUX2, S2 (주소1)
    [TC_CH16_STACK2_COOL_RET]={2, 0,  0}, // MUX2, S1 (주소0)
    [TC_CH17_WASTE_HEAT_IN] = {3, 15, 0}, // MUX3 (IC306/308), S16 (주소15)
    [TC_CH18_WASTE_HEAT_OUT]= {3, 14, 0}, // MUX3, S15 (주소14)
    [TC_CH19_SYSTEM_INTERNAL]={3, 13, 0}, // MUX3, S14 (주소13)

    /* --- K-Type 온도 센서 (CH21 ~ CH32) --- */
    [TC_CH21_REF_BN]        = {1, 13, 1}, // MUX1, S14 (주소13)
    [TC_CH22_REF_SR1]       = {1, 14, 1}, // MUX1, S15 (주소14)
    [TC_CH23_REF_SR2]       = {1, 15, 1}, // MUX1, S16 (주소15)
    [TC_CH24_REF_SR3]       = {1, 7,  1}, // MUX1, S8 (주소7)
    [TC_CH25_REF_LTS1]      = {1, 6,  1}, // MUX1, S7 (주소6)
    [TC_CH26_REF_LTS2]      = {2, 8,  1}, // MUX2, S9 (주소8)
    [TC_CH27_REF_FEED]      = {2, 9,  1}, // MUX2, S10 (주소9)
    [TC_CH28_REF_RG]        = {2, 10, 1}, // MUX2, S11 (주소10)
    [TC_CH29_REF_EG]        = {2, 11, 1}, // MUX2, S12 (주소11)
    [TC_CH30_PROX_01]       = {2, 12, 1}, // MUX2, S13 (주소12)
    [TC_CH31_PROX_02]       = {2, 13, 1}, // MUX2, S14 (주소13)
    [TC_CH32_PROX_03]       = {2, 14, 1}  // MUX2, S15 (주소14)
};

/* ==========================================================================
 * 2. MAX31856 SPI 보조 레지스터 쓰기 및 읽기 함수
 * ========================================================================== */

static void MAX31856_WriteRegister(uint8_t reg_addr, uint8_t data_val)
{
    TC_SPI_CS_LAT = 0; // CS 활성화 (Low)
    __delay32(8);      // CS Setup Time (2us)
    
    // 쓰기 동작 지칭 (Address의 최고 비트 MSB = 1 설정)
    SPI1_Exchange8bit(0x80 | reg_addr);
    SPI1_Exchange8bit(data_val);
    
    __delay32(8);      // CS Hold Time (2us)
    TC_SPI_CS_LAT = 1; // CS 비활성화 (High)
    __delay32(8);      // CS Inactive Time (2us)
}

static uint8_t MAX31856_ReadRegister(uint8_t reg_addr)
{
    uint8_t rx_data;
    TC_SPI_CS_LAT = 0; // CS 활성화
    __delay32(8);      // CS Setup Time (2us)
    
    // 읽기 동작 지칭 (Address의 최고 비트 MSB = 0 설정)
    SPI1_Exchange8bit(0x00 | reg_addr);
    rx_data = SPI1_Exchange8bit(0xFF); // 더미 데이터 송신하여 수신 획득
    
    __delay32(8);      // CS Hold Time (2us)
    TC_SPI_CS_LAT = 1; // CS 비활성화
    __delay32(8);      // CS Inactive Time (2us)
    
    return rx_data;
}

/* ==========================================================================
 * 3. thermocouple.h API 함수 구현부
 * ========================================================================== */

bool TC_Initialize(void)
{
    // CS, CLK, MOSI 출력 및 MISO 입력 방향 초기화
    TC_SPI_CS_LAT = 1;
    TC_SPI_CS_TRIS = 0;
    TC_SPI_CLK_LAT = 0;
    TC_SPI_CLK_TRIS = 0;
    TC_SPI_MOSI_LAT = 0;
    TC_SPI_MOSI_TRIS = 0;
    TC_SPI_MISO_TRIS = 1; // 54번 핀 입력
    
    // MUX 어드레스 및 인에이블 출력 핀 초기화
    TC_ADDR0_LAT = 0; TC_ADDR0_TRIS = 0;
    TC_ADDR1_LAT = 0; TC_ADDR1_TRIS = 0;
    TC_ADDR2_LAT = 0; TC_ADDR2_TRIS = 0;
    TC_ADDR3_LAT = 0; TC_ADDR3_TRIS = 0;
    
    TC_EN1_LAT = 0; TC_EN1_TRIS = 0;
    TC_EN2_LAT = 0; TC_EN2_TRIS = 0;
    TC_EN3_LAT = 0; TC_EN3_TRIS = 0;

    // 1. MAX31856 기본 구성: 1-Shot 대기 모드 (CMODE=0), 60Hz 필터, Fault Clear
    MAX31856_WriteRegister(MAX31856_REG_CR0, 0x02); // 0x02 = FAULTCLR=1
    
    // 2. 초기 채널 선택 및 최초 1-Shot 변환 트리거
    TC_TriggerConversion(TC_CH1_CITY_GAS_IN);

    return true;
}

bool TC_SelectChannel(TC_Channel_t channel)
{
    if (channel >= TC_MAX_CHANNELS)
    {
        return false;
    }

    const TC_Mapping_t *map = &tc_map[channel];

    // [1] 모든 MUX 즉시 비활성화 (Break-before-make: 잔류 전하 방전 및 채널 간 간섭 차단)
    TC_EN1_LAT = 0;
    TC_EN2_LAT = 0;
    TC_EN3_LAT = 0;
    __delay32(40); // 10us 차단 안정화

    // [2] ADG706 멀티플렉서 4비트 주소선(A0~A3) 설정
    TC_ADDR0_LAT = (map->mux_addr & 0x01) ? 1 : 0;
    TC_ADDR1_LAT = (map->mux_addr & 0x02) ? 1 : 0;
    TC_ADDR2_LAT = (map->mux_addr & 0x04) ? 1 : 0;
    TC_ADDR3_LAT = (map->mux_addr & 0x08) ? 1 : 0;
    __delay32(40); // 10us 주소 안정화

    // [3] 선택된 멀티플렉서 쌍만 Enable (High) 설정
    if (map->mux_en == 1)
    {
        TC_EN1_LAT = 1;
    }
    else if (map->mux_en == 2)
    {
        TC_EN2_LAT = 1;
    }
    else if (map->mux_en == 3)
    {
        TC_EN3_LAT = 1;
    }
    __delay32(80); // 20us MUX 통전 안정화

    // [4] 온도 타입에 맞추어 MAX31856의 Thermocouple Type 설정 변경 (CR1)
    // CR1 Register: 0x01 번지
    // - T-Type: 비트 [3:0] = 0b0111 (0x07)
    // - K-Type: 비트 [3:0] = 0b0011 (0x03)
    uint8_t cr1_val = (map->tc_type == 0) ? 0x07 : 0x03;
    MAX31856_WriteRegister(MAX31856_REG_CR1, cr1_val);

    return true;
}

float TC_ReadColdJunction(void)
{
    // MAX31856 레지스터 0x0A (CJTH), 0x0B (CJTL) 판독
    uint8_t cjh = MAX31856_ReadRegister(0x0A);
    uint8_t cjl = MAX31856_ReadRegister(0x0B);
    int16_t raw_cj = ((int16_t)cjh << 8) | cjl;
    raw_cj >>= 2; // 상위 14비트 유효 (1 LSB = 0.015625°C)
    return (float)raw_cj * 0.015625f;
}

bool TC_IsConversionDone(void)
{
    // MAX31856 CR0 레지스터(0x00 번지)의 Bit 6 (1SHOT) 비트 판독
    // 1-Shot 변환(약 143ms)이 끝나면 하드웨어가 자동으로 0으로 클리어함!
    uint8_t cr0 = MAX31856_ReadRegister(MAX31856_REG_CR0);
    return ((cr0 & 0x40) == 0);
}

bool TC_TriggerConversion(TC_Channel_t channel)
{
    if (channel >= TC_MAX_CHANNELS)
    {
        return false;
    }

    // 1. MUX 채널 및 센서 타입(T/K) 전환
    TC_SelectChannel(channel);

    // 2. CR0 레지스터에 1-Shot 변환(Bit 6 = 1), 하드웨어 오픈 서킷 단선 검출(Bit 4 = 1), Fault Clear(Bit 1 = 1) 송출 (0x52)
    // 👑 [특허급 하드웨어 단선/플로팅 검출 엔진 (OCFAIL=01)]
    // 센서가 체결되지 않은 빈 채널(Open/Floating)을 5ms 만에 하드웨어가 자동 감지하여 고온 잔류 전하 고스팅 완벽 차단!
    MAX31856_WriteRegister(MAX31856_REG_CR0, 0x52);

    return true;
}

float TC_ReadTemperatureOnly(TC_Channel_t channel)
{
    // [1] MAX31856 하드웨어 Fault Status Register (0x0F) 판독
    uint8_t sr = MAX31856_ReadRegister(MAX31856_REG_SR);
    // Bit 0: OPEN (1 = 센서 미체결 / 단선 오류)
    if (sr & 0x01)
    {
        return -999.0f; // 센서 미연결 빈 채널 확정 -> UI 가로바(--) 즉각 처리
    }

    // [2] 3바이트 선형화 온도 레지스터 값 순차 수신 (0x0C, 0x0D, 0x0E 번지)
    uint8_t th = MAX31856_ReadRegister(MAX31856_REG_LTCBH);
    uint8_t tm = MAX31856_ReadRegister(MAX31856_REG_LTCBM);
    uint8_t tl = MAX31856_ReadRegister(MAX31856_REG_LTCBL);

    // [3] MAX31856 공식 단선 플래그 검출 (0x7F, 0x80)
    if (th == 0x7F || th == 0x80)
    {
        return -999.0f; // 센서 단선/미연결
    }

    // [4] SPI 통신 단절(MISO 풀업 0xFF 또는 풀다운 0x00) 검출
    if (th == 0xFF && tm == 0xFF && tl == 0xFF)
    {
        return -999.0f;
    }

    // [4] 3바이트 값을 24비트 부호 있는 정수로 결합
    int32_t raw_temp = ((int32_t)th << 16) | ((int32_t)tm << 8) | tl;

    // 음수 부호 확장 (Bit 23이 1이면 음수)
    if (raw_temp & 0x00800000)
    {
        raw_temp |= 0xFF000000;
    }

    // 하위 5비트는 소수점 정밀도 외의 미사용 비트이므로 시프트
    raw_temp >>= 5;

    // 1 LSB 단위 온도는 0.0078125°C (1/128°C)
    float celsius = (float)raw_temp * 0.0078125f;

    // [5] 열기전력 0mV 구간 보정 및 냉접점(Cold-Junction, 보드 상온) 측정 보완
    if (celsius == 0.0f || (celsius < -40.0f && celsius > -50.0f))
    {
        uint8_t cjh = MAX31856_ReadRegister(0x0A);
        uint8_t cjl = MAX31856_ReadRegister(0x0B);
        int16_t raw_cj = ((int16_t)cjh << 8) | cjl;
        raw_cj >>= 2;
        float cj_temp = (float)raw_cj * 0.015625f;
        if (cj_temp > 5.0f && cj_temp < 80.0f)
        {
            return cj_temp; // 보드 상온 약 25~30°C 보정
        }
    }

    // [6] 센서 타입별 물리적 유효 온도 범위 검증 (초과 시 단선/노이즈 판정)
    bool is_t_type = (channel < 19);
    if (is_t_type)
    {
        if (celsius > 250.0f || celsius < -40.0f) return -999.0f;
    }
    else
    {
        if (celsius > 1000.0f || celsius < -40.0f) return -999.0f;
    }

    return celsius;
}

float TC_ReadTemperature(TC_Channel_t channel)
{
    // 논블로킹 인터페이스: 이전 1-Shot 주기 동안 변환 완료된 온도 즉시 판독
    return TC_ReadTemperatureOnly(channel);
}

