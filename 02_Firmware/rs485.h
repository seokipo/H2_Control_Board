/**
 * @file rs485.h
 * @brief RS-485 UART Communication Driver (9600bps, Half-Duplex) for H2_Control_Board
 * 
 * @note 본 드라이버는 Mcu.SchDoc 회로도의 MAX3485 트랜시버(IC4) 연동 설계 사양을 기반으로 구현되었습니다.
 *       - 485_TX : RD5 (물리 67번 핀) -> UART1 TX
 *       - 485_RX : RB6 (물리 69번 핀) -> UART1 RX
 *       - 485_DIR: RB5 (물리 68번 핀) -> Half-Duplex 방향 제어선 (1: TX, 0: RX)
 */

#ifndef RS485_H
#define RS485_H

#include <stdint.h>
#include <stdbool.h>

// RS-485 기본 보레이트 설정
#define RS485_BAUDRATE  9600

/* ==========================================================================
 * 1. 드라이버 API 함수 원형
 * ========================================================================== */

/**
 * @brief RS-485용 UART1 모듈 및 방향 제어 GPIO 설정 초기화
 */
void RS485_Initialize(void);

/**
 * @brief RS-485 통신 버스로 1바이트 데이터 송신
 * @param data 송신할 8비트 데이터
 */
void RS485_WriteByte(uint8_t data);

/**
 * @brief RS-485 통신 버스로 다중 데이터 바이트 송신 (문자열 또는 패킷)
 * @param buffer 송신 데이터 버퍼 포인터
 * @param length 송신할 바이트 크기
 */
void RS485_WriteBuffer(const uint8_t *buffer, uint16_t length);

/**
 * @brief RS-485 통신 버스로부터 수신된 데이터가 있는지 확인
 * @return true 수신 데이터 존재, false 데이터 없음
 */
bool RS485_IsRxReady(void);

/**
 * @brief 수신 버퍼에서 1바이트 데이터 읽기
 * @return uint8_t 읽어낸 수신 데이터, 없으면 0
 */
uint8_t RS485_ReadByte(void);

/* ==========================================================================
 * 2. M701 7-in-1 복합 가스/환경 센서 수신 스택
 * ========================================================================== */

/**
 * @brief M701 복합 센서 계측 데이터 구조체
 */
typedef struct {
    uint16_t eco2;        /**< eCO2 농도 (ppm, 400~5000) */
    uint16_t ech2o;       /**< eCH2O 포름알데히드 농도 (ug/m3, 0~2000) */
    uint16_t tvoc;        /**< TVOC 농도 (ug/m3, 0~5000) */
    uint16_t pm25;        /**< PM2.5 극초미세먼지 (ug/m3, 0~999) */
    uint16_t pm10;        /**< PM10 미세먼지 (ug/m3, 0~1000) */
    int16_t  temperature; /**< 온도 (0.1도 단위 x10 스케일, 영하 지원) */
    uint16_t humidity;    /**< 습도 (0.1% 단위 x10 스케일) */
    bool     is_valid;    /**< 최근 유효 패킷 수신 성공 여부 */
    uint32_t rx_count;    /**< 누적 수신 패킷 카운트 */
} M701_Data_t;

/**
 * @brief RS-485 UART1 수신 버퍼를 폴링하여 M701 17바이트 패킷을 상태 머신으로 파싱
 * @return true 새로운 유효 패킷 수신 완료 및 데이터 갱신됨, false 미완료 또는 패킷 없음
 */
bool RS485_ProcessM701(void);

/**
 * @brief 최신 M701 센서 계측 데이터 구조체 포인터 반환
 * @return const M701_Data_t* 최신 계측 데이터 포인터
 */
const M701_Data_t* M701_GetData(void);

#endif /* RS485_H */

