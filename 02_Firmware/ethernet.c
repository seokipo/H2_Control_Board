/**
 * @file ethernet.c
 * @brief Dual W5500 Ethernet Controller Driver Implementation
 * 
 * @note dsPIC33CK512MP710 MCU의 하드웨어 SPI2 및 PPS 핀 매핑을 활성화하여
 *       공유 SPI 버스 통신을 수행하고, 개별 CS/RESET GPIO선을 이용해 유닛을 제어합니다.
 */

#include "ethernet.h"
#include "pin_map.h"

// SPI2 통신용 보조 전송 함수 (타임아웃 보호 탑재)
static uint8_t SPI2_Exchange8bit(uint8_t data_val)
{
    uint16_t timeout = 5000;
    // SPI2 송신 버퍼가 비어있을 때까지 대기
    while (SPI2STATLbits.SPITBF && --timeout);
    
    // 데이터 주입
    SPI2BUFL = data_val;
    
    // 수신 완료될 때까지 대기 (SPIRBF = 1)
    timeout = 5000;
    while (!SPI2STATLbits.SPIRBF && --timeout);
    
    return SPI2BUFL;
}

void ETH_Initialize(void)
{
    // [1] PPS (Peripheral Pin Select) 설정: SPI2 공유 버스 핀 동적 할당
    __builtin_write_RPCON(0x0000); // PPS Lock 해제
    
    // SPI2 Data Input (SDI2) 입력 지정: 4번 핀 RE1 (PPS 번호 RP81)
    _SDI2R = 81; 
    
    // SPI2 Data Output (SDO2) 출력 지정: 5번 핀 RF0 (PPS 출력 레지스터 RP96R, 기능 코드 8: SDO2)
    _RP96R = 8; 
    
    // SPI2 Clock (SCK2) 출력 지정: 6번 핀 RC12 (PPS 출력 레지스터 RP60R, 기능 코드 9: SCK2)
    _RP60R = 9; 
    
    __builtin_write_RPCON(0x0800); // PPS Lock 설정

    // [2] SPI2 주변기기 제어 레지스터 설정 (마스터 모드, 10 MHz 클록)
    SPI2CON1L = 0x0000;
    SPI2CON1H = 0x0000;
    
    // 마스터 모드 활성화 (MSTEN = 1)
    SPI2CON1Lbits.MSTEN = 1;
    
    // 클록 극성 및 위상 설정 (Mode 0: CKP = 0, CKE = 1)
    SPI2CON1Lbits.CKP = 0;
    SPI2CON1Lbits.CKE = 1;
    
    // 8비트 모드 설정 (MODE = 0)
    SPI2CON1Lbits.MODE = 0; 
    
    // Baudrate Generator  Baudrate = FCY / (2 * (SPI2BRGL + 1))
    // 10 MHz = 80,000,000 / (2 * (3 + 1)) -> SPI2BRGL = 3 설정
    SPI2BRGL = 3;

    // SPI2 모드 활성화
    SPI2CON1Lbits.SPIEN = 1;

    // [3] 이더넷 칩셋 하드웨어 일괄 리셋
    ETH_ResetDevice(ETH_UNIT_1);
    ETH_ResetDevice(ETH_UNIT_2);

    // [4] 기본 공장출하 네트워크 파라미터 (192.168.0.100) 주입
    ETH_NetInfo_t default_net = {
        .mac = {0x00, 0x08, 0xDC, 0x55, 0x00, 0x01},
        .ip  = {192, 168, 0, 100},
        .sn  = {255, 255, 255, 0},
        .gw  = {192, 168, 0, 1}
    };
    ETH_SetNetworkInfo(ETH_UNIT_1, &default_net);

    // 2호기 (ETH2) 보조 채널은 101번 IP로 기본 할당
    ETH_NetInfo_t default_net2 = {
        .mac = {0x00, 0x08, 0xDC, 0x55, 0x00, 0x02},
        .ip  = {192, 168, 0, 101},
        .sn  = {255, 255, 255, 0},
        .gw  = {192, 168, 0, 1}
    };
    ETH_SetNetworkInfo(ETH_UNIT_2, &default_net2);
}

void ETH_ResetDevice(ETH_Unit_t unit)
{
    // Active-Low 리셋선을 잠시 Low로 내려 강제 리셋 가동
    if (unit == ETH_UNIT_1)
    {
        DO_ETH1_RST_LAT = 0;
        // 하드웨어 리셋 딜레이 루프 (수 ms)
        for (volatile uint16_t delay = 0; delay < 10000; delay++);
        DO_ETH1_RST_LAT = 1; // 리셋 해제
    }
    else if (unit == ETH_UNIT_2)
    {
        DO_ETH2_RST_LAT = 0;
        for (volatile uint16_t delay = 0; delay < 10000; delay++);
        DO_ETH2_RST_LAT = 1; // 리셋 해제
    }
    
    // W5500 내부 PLL 락 및 기동 안정화 대기 시간 (약 50ms 권장)
    for (volatile uint32_t delay = 0; delay < 200000; delay++);
}

void ETH_WriteReg(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase, uint8_t data_val)
{
    // [1] 대상 장치 CS선 활성화 (Active-Low)
    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 0;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 0;

    // [2] W5500 표준 3바이트 주소/제어 헤더 송신
    // - Byte 1: 주소 High (offset 8)
    // - Byte 2: 주소 Low
    // - Byte 3: 제어 위상 (Control Phase: 블록 지정 및 R/W 구분 비트)
    //   (W5500 쓰기 시 제어 페이즈의 2비트는 0b01(Write)로 설정되어야 함)
    SPI2_Exchange8bit((reg_addr >> 8) & 0xFF);
    SPI2_Exchange8bit(reg_addr & 0xFF);
    
    // 쓰기 모드 지칭 마스크 (bit 2 = 1, bit 1:0 = 00 Variable Data Length)
    uint8_t ctrl = (control_phase & 0xF8) | 0x04; 
    SPI2_Exchange8bit(ctrl);

    // [3] 실제 데이터 주입 송신
    SPI2_Exchange8bit(data_val);

    // [4] 대상 장치 CS선 비활성화 (High)
    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 1;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 1;
}

uint8_t ETH_ReadReg(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase)
{
    uint8_t rx_data = 0;

    // [1] 대상 장치 CS선 활성화
    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 0;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 0;

    // [2] 3바이트 주소/제어 헤더 송신 (읽기 모드)
    SPI2_Exchange8bit((reg_addr >> 8) & 0xFF);
    SPI2_Exchange8bit(reg_addr & 0xFF);
    
    // 읽기 모드 지칭 마스크 (bit 2 = 0, bit 1:0 = 00)
    uint8_t ctrl = (control_phase & 0xF8) | 0x00; 
    SPI2_Exchange8bit(ctrl);

    // [3] 더미 데이터를 전송하여 레지스터 데이터 읽기
    rx_data = SPI2_Exchange8bit(0xFF);

    // [4] 대상 장치 CS선 비활성화
    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 1;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 1;

    return rx_data;
}

void ETH_SetNetworkInfo(ETH_Unit_t unit, const ETH_NetInfo_t* net_info)
{
    if (!net_info) return;

    // [1] Gateway IP 주소 설정 (GAR: 0x0001 ~ 0x0004)
    for (uint8_t i = 0; i < 4; i++) {
        ETH_WriteReg(unit, 0x0001 + i, 0x00, net_info->gw[i]);
    }

    // [2] Subnet Mask 설정 (SUBR: 0x0005 ~ 0x0008)
    for (uint8_t i = 0; i < 4; i++) {
        ETH_WriteReg(unit, 0x0005 + i, 0x00, net_info->sn[i]);
    }

    // [3] Source Hardware MAC 주소 설정 (SHAR: 0x0009 ~ 0x000E)
    for (uint8_t i = 0; i < 6; i++) {
        ETH_WriteReg(unit, 0x0009 + i, 0x00, net_info->mac[i]);
    }

    // [4] Source IP 주소 설정 (SIPR: 0x000F ~ 0x0012)
    for (uint8_t i = 0; i < 4; i++) {
        ETH_WriteReg(unit, 0x000F + i, 0x00, net_info->ip[i]);
    }
}

void ETH_GetNetworkInfo(ETH_Unit_t unit, ETH_NetInfo_t* net_info)
{
    if (!net_info) return;

    // [1] Gateway IP 읽기
    for (uint8_t i = 0; i < 4; i++) {
        net_info->gw[i] = ETH_ReadReg(unit, 0x0001 + i, 0x00);
    }

    // [2] Subnet Mask 읽기
    for (uint8_t i = 0; i < 4; i++) {
        net_info->sn[i] = ETH_ReadReg(unit, 0x0005 + i, 0x00);
    }

    // [3] MAC 주소 읽기
    for (uint8_t i = 0; i < 6; i++) {
        net_info->mac[i] = ETH_ReadReg(unit, 0x0009 + i, 0x00);
    }

    // [4] IP 주소 읽기
    for (uint8_t i = 0; i < 4; i++) {
        net_info->ip[i] = ETH_ReadReg(unit, 0x000F + i, 0x00);
    }
}
