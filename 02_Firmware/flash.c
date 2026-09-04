/**
 * @file flash.c
 * @brief W25Q256 SPI Flash Memory Driver Implementation
 * 
 * @note dsPIC33CK MCU의 SPI3 및 PPS 동적 매핑을 통해 외부 플래시 메모리 칩과 통신하며,
 *       섹터 소거, 페이지 프로그래밍 및 고속 리드 트랜잭션을 가동합니다.
 */

#include "flash.h"
#include "pin_map.h"
#include <xc.h>

// SPI3 통신용 1바이트 교환 헬퍼 함수 (타임아웃 보호 탑재)
static uint8_t SPI3_Exchange8bit(uint8_t data_val)
{
    uint16_t timeout = 5000;
    while (SPI3STATLbits.SPITBF && --timeout);
    SPI3BUFL = data_val;
    timeout = 5000;
    while (!SPI3STATLbits.SPIRBF && --timeout);
    return SPI3BUFL;
}

void FLASH_Initialize(void)
{
    // [1] PPS (Peripheral Pin Select) 설정: SPI3 핀 동적 할당
    __builtin_write_RPCON(0x0000); // PPS Lock 해제
    
    // SPI3 Data Input (SDI3) 입력: 11번 핀 RD15 (PPS 번호 RP79)
    _SDI3R = 79; 
    
    // SPI3 Data Output (SDO3) 출력: 9번 핀 RC15 (PPS 출력 레지스터 RP63R, 기능 코드 11: SDO3)
    _RP63R = 11; 
    
    // SPI3 Clock (SCK3) 출력: 8번 핀 RC14 (PPS 출력 레지스터 RP62R, 기능 코드 12: SCK3)
    _RP62R = 12; 
    
    __builtin_write_RPCON(0x0800); // PPS Lock 설정

    // [2] SPI3 주변기기 동작 레지스터 설정 (마스터 모드, 10 MHz)
    SPI3CON1L = 0x0000;
    SPI3CON1H = 0x0000;
    
    SPI3CON1Lbits.MSTEN = 1;      // 마스터 모드
    SPI3CON1Lbits.CKP = 0;        // Mode 0: CKP = 0, CKE = 1
    SPI3CON1Lbits.CKE = 1;
    SPI3CON1Lbits.MODE = 0;       // 8비트 통신
    
    SPI3BRGL = 3;                 // Baudrate 10 MHz (80MHz FCY / 8)
    
    SPI3CON1Lbits.SPIEN = 1;      // SPI3 채널 온

    // CS 핀 비활성화 대기
    DO_FLASH_CS_LAT = 1;
}

uint32_t FLASH_ReadID(void)
{
    uint32_t jedec_id = 0;

    DO_FLASH_CS_LAT = 0; // CS On
    
    SPI3_Exchange8bit(FLASH_CMD_JEDEC_ID); // JEDEC ID 리드 커맨드 (0x9F)
    
    uint8_t mfg_id = SPI3_Exchange8bit(0xFF);    // Manufacturer ID (Winbond = 0xEF)
    uint8_t mem_type = SPI3_Exchange8bit(0xFF);  // Memory Type (0x40)
    uint8_t capacity = SPI3_Exchange8bit(0xFF);  // Capacity (0x19)
    
    DO_FLASH_CS_LAT = 1; // CS Off

    jedec_id = ((uint32_t)mfg_id << 16) | ((uint32_t)mem_type << 8) | capacity;
    return jedec_id;
}

void FLASH_WriteEnable(void)
{
    DO_FLASH_CS_LAT = 0;
    SPI3_Exchange8bit(FLASH_CMD_WRITE_ENABLE); // Write Enable (0x06)
    DO_FLASH_CS_LAT = 1;
}

void FLASH_WaitBusy(void)
{
    uint8_t status = 0;
    
    do {
        DO_FLASH_CS_LAT = 0;
        SPI3_Exchange8bit(FLASH_CMD_READ_STATUS1); // Status Register 1 읽기 (0x05)
        status = SPI3_Exchange8bit(0xFF);
        DO_FLASH_CS_LAT = 1;
    } while (status & 0x01); // bit 0 (WIP: Write In Progress) 이 0이 될 때까지 대기
}

void FLASH_EraseSector(uint32_t sector_addr)
{
    FLASH_WriteEnable(); // 쓰기 방지 잠금 해제
    FLASH_WaitBusy();

    DO_FLASH_CS_LAT = 0;
    SPI3_Exchange8bit(FLASH_CMD_SECTOR_ERASE); // 4KB Sector Erase (0x20)
    
    // 24비트(3바이트) 시작 주소 전달
    SPI3_Exchange8bit((sector_addr >> 16) & 0xFF);
    SPI3_Exchange8bit((sector_addr >> 8) & 0xFF);
    SPI3_Exchange8bit(sector_addr & 0xFF);
    DO_FLASH_CS_LAT = 1;

    FLASH_WaitBusy(); // 소거 완료될 때까지 대기 (일반적으로 수십 ms 소요)
}

void FLASH_WritePage(uint32_t addr, const uint8_t *data_buf, uint16_t length)
{
    if (length > 256) length = 256; // 1페이지 전송 임계값 제한

    FLASH_WriteEnable();
    FLASH_WaitBusy();

    DO_FLASH_CS_LAT = 0;
    SPI3_Exchange8bit(FLASH_CMD_PAGE_PROGRAM); // Page Program (0x02)
    
    SPI3_Exchange8bit((addr >> 16) & 0xFF);
    SPI3_Exchange8bit((addr >> 8) & 0xFF);
    SPI3_Exchange8bit(addr & 0xFF);

    for (uint16_t i = 0; i < length; i++)
    {
        SPI3_Exchange8bit(data_buf[i]);
    }
    DO_FLASH_CS_LAT = 1;

    FLASH_WaitBusy(); // 페이지 프로그래밍 기입 완료 대기
}

void FLASH_ReadData(uint32_t addr, uint8_t *rx_buf, uint32_t length)
{
    FLASH_WaitBusy();

    DO_FLASH_CS_LAT = 0;
    SPI3_Exchange8bit(FLASH_CMD_READ_DATA); // Read Data (0x03)
    
    SPI3_Exchange8bit((addr >> 16) & 0xFF);
    SPI3_Exchange8bit((addr >> 8) & 0xFF);
    SPI3_Exchange8bit(addr & 0xFF);

    for (uint32_t i = 0; i < length; i++)
    {
        rx_buf[i] = SPI3_Exchange8bit(0xFF);
    }
    DO_FLASH_CS_LAT = 1;
}
