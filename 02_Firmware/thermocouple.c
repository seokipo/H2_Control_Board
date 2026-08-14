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

// SPI 통신 가상 인터페이스 (컴파일 연동용, mcc_generated_files/spi_host/spi1.h 등과 연계 가능)
__attribute__((weak)) uint8_t SPI1_Exchange8bit(uint8_t data);

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
    
    // 쓰기 동작 지칭 (Address의 최고 비트 MSB = 1 설정)
    SPI1_Exchange8bit(0x80 | reg_addr);
    SPI1_Exchange8bit(data_val);
    
    TC_SPI_CS_LAT = 1; // CS 비활성화 (High)
}

static uint8_t MAX31856_ReadRegister(uint8_t reg_addr)
{
    uint8_t rx_data;
    TC_SPI_CS_LAT = 0; // CS 활성화
    
    // 읽기 동작 지칭 (Address의 최고 비트 MSB = 0 설정)
    SPI1_Exchange8bit(0x00 | reg_addr);
    rx_data = SPI1_Exchange8bit(0xFF); // 더미 데이터 송신하여 수신 획득
    
    TC_SPI_CS_LAT = 1; // CS 비활성화
    
    return rx_data;
}

/* ==========================================================================
 * 3. thermocouple.h API 함수 구현부
 * ========================================================================== */

bool TC_Initialize(void)
{
    // CS 핀 출력 및 High 비활성화 초기화
    TC_SPI_CS_LAT = 1;
    TC_SPI_CS_TRIS = 0;
    
    // MUX 어드레스 및 인에이블 출력 핀 초기화
    TC_ADDR0_LAT = 0; TC_ADDR0_TRIS = 0;
    TC_ADDR1_LAT = 0; TC_ADDR1_TRIS = 0;
    TC_ADDR2_LAT = 0; TC_ADDR2_TRIS = 0;
    TC_ADDR3_LAT = 0; TC_ADDR3_TRIS = 0;
    
    TC_EN1_LAT = 0; TC_EN1_TRIS = 0;
    TC_EN2_LAT = 0; TC_EN2_TRIS = 0;
    TC_EN3_LAT = 0; TC_EN3_TRIS = 0;

    // 1. MAX31856 동작 상태 시작 및 자동 변환 활성화 (CR0)
    // CR0 Register: 0x00 번지, 값 0x80 (연속 변환 모드 활성화 및 냉접점 센서 전원 인가)
    MAX31856_WriteRegister(MAX31856_REG_CR0, 0x80);

    return true;
}

bool TC_SelectChannel(TC_Channel_t channel)
{
    if (channel >= TC_MAX_CHANNELS)
    {
        return false;
    }

    const TC_Mapping_t *map = &tc_map[channel];

    // [1] 모든 멀티플렉서 인에이블을 Low(비활성화)로 일체 내림
    TC_EN1_LAT = 0;
    TC_EN2_LAT = 0;
    TC_EN3_LAT = 0;

    // [2] ADG706 멀티플렉서 4비트 주소선(A0~A3) 설정
    TC_ADDR0_LAT = (map->mux_addr & 0x01) ? 1 : 0;
    TC_ADDR1_LAT = (map->mux_addr & 0x02) ? 1 : 0;
    TC_ADDR2_LAT = (map->mux_addr & 0x04) ? 1 : 0;
    TC_ADDR3_LAT = (map->mux_addr & 0x08) ? 1 : 0;

    // MUX 주소 라인 신호 안정을 위해 잠시 대기
    // asm("nop"); asm("nop");

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

    // [4] 온도 타입에 맞추어 MAX31856의 Thermocouple Type 설정 변경 (CR1)
    // CR1 Register: 0x01 번지
    // - T-Type: 비트 [3:0] = 0b0111 (0x07)
    // - K-Type: 비트 [3:0] = 0b0011 (0x03)
    uint8_t cr1_val = (map->tc_type == 0) ? 0x07 : 0x03;
    MAX31856_WriteRegister(MAX31856_REG_CR1, cr1_val);

    return true;
}

float TC_ReadTemperature(TC_Channel_t channel)
{
    // [1] 측정 대상 채널의 MUX 및 칩 설정 선택
    if (!TC_SelectChannel(channel))
    {
        return -999.0f;
    }

    // 아날로그 신호 안정화 및 MAX31856 온도 변환 완료 대기 시간 (약 10ms ~ 100ms 권장)
    // (실무상 타이머 인터럽트를 통해 비동기 스캔하거나, 스케줄러로 처리함)

    // [2] 3바이트 온도 레지스터 값 순차 수신 (0x0C, 0x0D, 0x0E 번지)
    uint8_t th = MAX31856_ReadRegister(MAX31856_REG_LTCBH);
    uint8_t tm = MAX31856_ReadRegister(MAX31856_REG_LTCBM);
    uint8_t tl = MAX31856_ReadRegister(MAX31856_REG_LTCBL);

    // [3] 3바이트 값을 24비트 부호 있는 데이터로 취합
    int32_t raw_temp = ((int32_t)th << 16) | ((int32_t)tm << 8) | tl;

    // 만약 최고 부호 비트가 1이면(음수 온도) 음수 확장 처리
    if (raw_temp & 0x00800000)
    {
        raw_temp |= 0xFF000000;
    }

    // 하위 5비트는 소수점 데이터 정밀도 외의 레지스터 정보이므로 비트 시프트
    raw_temp >>= 5;

    // 1 LSB 단위 온도는 0.0078125도 (즉 1/128도) 이므로 곱하여 Celsius 온도 환산
    float celsius = (float)raw_temp * 0.0078125f;

    return celsius;
}
