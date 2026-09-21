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

// W5500 네트워크 파라미터 구조체
typedef struct {
    uint8_t mac[6];      // MAC 주소 (예: 00:08:DC:11:22:33)
    uint8_t ip[4];       // 로컬 IP 주소 (예: 192.168.0.100)
    uint8_t sn[4];       // 서브넷 마스크 (예: 255.255.255.0)
    uint8_t gw[4];       // 게이트웨이 주소 (예: 192.168.0.1)
} ETH_NetInfo_t;

/* ==========================================================================
 * 1. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief 듀얼 W5500 공유 SPI2 주변기기 및 포트 PPS 설정 초기화 및 기본 IP 주입
 */
void ETH_Initialize(void);

/**
 * @brief 대상 W5500 칩셋의 하드웨어 강제 리셋(Reset) 시퀀스 수행
 * @param unit 리셋할 대상 유닛 (ETH_UNIT_1 또는 ETH_UNIT_2)
 */
void ETH_ResetDevice(ETH_Unit_t unit);

/**
 * @brief W5500 내부 레지스터에 IP/서브넷/게이트웨이/MAC 정보 주입
 * @param unit 대상 유닛
 * @param net_info 주입할 네트워크 정보 구조체 포인터
 */
void ETH_SetNetworkInfo(ETH_Unit_t unit, const ETH_NetInfo_t* net_info);

/**
 * @brief W5500 내부 레지스터로부터 현재 활성화된 네트워크 정보 읽기
 * @param unit 대상 유닛
 * @param net_info 읽어올 네트워크 정보 구조체 포인터
 */
void ETH_GetNetworkInfo(ETH_Unit_t unit, ETH_NetInfo_t* net_info);

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

/**
 * @brief W5500 버퍼 다중 바이트 쓰기
 */
void ETH_WriteBuf(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase, const uint8_t *buf, uint16_t len);

/**
 * @brief W5500 버퍼 다중 바이트 읽기
 */
void ETH_ReadBuf(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase, uint8_t *buf, uint16_t len);

/* ==========================================================================
 * 2. Modbus TCP 서버 API (W5500 Socket 0)
 * ========================================================================== */

#define MODBUS_TCP_PORT 502 // 표준 Modbus TCP 포트

/**
 * @brief W5500 Socket 0을 Modbus TCP 서버 모드로 초기화
 * @param port 바인딩할 TCP 포트 (기본 502)
 */
void ETH_ModbusTCP_Init(uint16_t port);

/**
 * @brief W5500 Socket 0 상태 머신 및 Modbus TCP 패킷 수신/응답 주기적 처리 태스크
 * @note main.c의 메인 루프에서 주기적으로 호출
 */
void ETH_ModbusTCP_Task(void);

#endif /* ETHERNET_H */

