/**
 * @file flash.h
 * @brief W25Q256 256Mb SPI Flash Memory Driver Header
 * 
 * @note 본 드라이버는 Mcu.SchDoc 도면의 물리 8, 9, 11, 12번 핀에 결선된 W25Q256 칩셋을
 *       SPI3 하드웨어 컨트롤러를 통해 구동하며 데이터의 영구 저장을 처리합니다.
 */

#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stdbool.h>

// W25Q256 표준 명령어 셋
#define FLASH_CMD_WRITE_ENABLE      0x06
#define FLASH_CMD_WRITE_DISABLE     0x04
#define FLASH_CMD_READ_STATUS1      0x05
#define FLASH_CMD_READ_STATUS2      0x35
#define FLASH_CMD_READ_STATUS3      0x15
#define FLASH_CMD_PAGE_PROGRAM      0x02
#define FLASH_CMD_SECTOR_ERASE      0x20
#define FLASH_CMD_BLOCK_ERASE_64K   0xD8
#define FLASH_CMD_CHIP_ERASE        0xC7
#define FLASH_CMD_READ_DATA         0x03
#define FLASH_CMD_FAST_READ         0x0B
#define FLASH_CMD_JEDEC_ID          0x9F

/* ==========================================================================
 * 1. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief W25Q256 전용 SPI3 채널 및 PPS 매핑 초기화
 */
void FLASH_Initialize(void);

/**
 * @brief 플래시 메모리의 3바이트 JEDEC ID (제조사 및 장치코드) 리드
 * @return uint32_t 리드한 24비트 JEDEC ID 값 (Winbond 제품인 경우 일반적으로 0xEF4019 반환)
 */
uint32_t FLASH_ReadID(void);

/**
 * @brief 플래시의 쓰기 작업 가능 상태(Write Enable) 선언
 */
void FLASH_WriteEnable(void);

/**
 * @brief W25Q256 내부의 상태 레지스터 Busy 비트(WIP) 감시 대기
 */
void FLASH_WaitBusy(void);

/**
 * @brief 지정한 4KB 섹터 영역의 데이터를 리셋(0xFF)으로 일괄 소거
 * @param sector_addr 소거할 섹터의 시작 물리 주소
 */
void FLASH_EraseSector(uint32_t sector_addr);

/**
 * @brief 플래시 내 특정 주소 영역에 데이터 1페이지(최대 256바이트) 연속 쓰기
 * @param addr 시작 물리 주소
 * @param data_buf 전송할 데이터 포인터
 * @param length 쓸 바이트 크기 (최대 256바이트)
 */
void FLASH_WritePage(uint32_t addr, const uint8_t *data_buf, uint16_t length);

/**
 * @brief 플래시 내 특정 주소 영역에서 다바이트 데이터를 연속 읽기
 * @param addr 시작 물리 주소
 * @param rx_buf 수신 데이터를 저장할 메모리 포인터
 * @param length 리드할 바이트 크기
 */
void FLASH_ReadData(uint32_t addr, uint8_t *rx_buf, uint32_t length);

#endif /* FLASH_H */
