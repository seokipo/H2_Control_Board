/**
 * @file ethernet.h
 * @brief Dual W5500 Ethernet Controller Driver for H2_Control_Board
 * 
 * @note 본 드라이버는 Mcu.SchDoc 및 Ethernet.SchDoc 회로 설계를 기반으로 하며,
 *       공유 SPI 버스와 개별 CS/RESET 제어선을 이용해 듀얼 W5500 칩셋을 핸들링합니다.
 */

#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdint.h>
#include <stdbool.h>

// W5500 칩 구분 식별자
typedef enum {
    ETH_UNIT_1 = 1, // W5500 1호기 (ETH1)
    ETH_UNIT_2 = 2  // W5500 2호기 (ETH2)
} ETH_Unit_t;

/* ==========================================================================
 * 1. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief 듀얼 W5500 공유 SPI2 주변기기 및 포트 PPS 설정 초기화
 */
void ETH_Initialize(void);

/**
 * @brief 대상 W5500 칩셋의 하드웨어 강제 리셋(Reset) 시퀀스 수행
 * @param unit 리셋할 대상 유닛 (ETH_UNIT_1 또는 ETH_UNIT_2)
 */
void ETH_ResetDevice(ETH_Unit_t unit);

/**
 * @brief W5500 내부 레지스터 1바이트 쓰기
 * @param unit 대상 유닛
 * @param reg_addr 16비트 레지스터 주소
 * @param control_phase 제어 위상 (블록 선택자 등)
 * @param data_val 쓸 데이터 값
 */
void ETH_WriteReg(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase, uint8_t data_val);

/**
 * @brief W5500 내부 레지스터 1바이트 읽기
 * @param unit 대상 유닛
 * @param reg_addr 16비트 레지스터 주소
 * @param control_phase 제어 위상
 * @return uint8_t 읽어낸 레지스터 값
 */
uint8_t ETH_ReadReg(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase);

#endif /* ETHERNET_H */
