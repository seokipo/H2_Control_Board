/**
 * @file modbus.c
 * @brief Modbus RTU Slave Protocol Stack Implementation
 * 
 * @note dsPIC33CK MCU의 RS-422(UART2) 송수신 포트를 활용하여 관제 노트북과 통신하며,
 *       수신된 국번 및 CRC16 무결성을 검증하고 펑션 코드에 따른 레지스터 읽기/쓰기를 처리합니다.
 */

#include "modbus.h"
#include "rs422.h"
#include "dac60516.h"

// 전역 Modbus 레지스터 데이터베이스 인스턴스
Modbus_Registers_t modbus_db;

// 통신 수신 제어 변수들
static uint8_t rx_buffer[MODBUS_BUFFER_SIZE];
static uint16_t rx_index = 0;
static bool rx_frame_completed = false;
static uint32_t rx_timeout_counter = 0;

// Modbus RTU CRC16 룩업 테이블 없이 고속 연산하는 표준 다항식 알고리즘
uint16_t Modbus_CRC16(const uint8_t *buffer, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= buffer[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void Modbus_Initialize(void)
{
    // 데이터베이스 메모리 초기화
    for (int i = 0; i < 80; i++) modbus_db.input_regs[i] = 0;
    for (int i = 0; i < 40; i++) modbus_db.holding_regs[i] = 0;
    for (int i = 0; i < 4; i++) modbus_db.coils[i] = 0;
    for (int i = 0; i < 2; i++) modbus_db.discrete_inputs[i] = 0;

    rx_index = 0;
    rx_frame_completed = false;
    rx_timeout_counter = 0;
}

void Modbus_ProcessRxByte(uint8_t rx_byte)
{
    if (rx_frame_completed)
    {
        return; // 현재 프레임 분석 처리 중에는 추가 수신 생략
    }

    if (rx_index < MODBUS_BUFFER_SIZE)
    {
        rx_buffer[rx_index++] = rx_byte;
    }
    
    // 수신 시마다 문자 간 타임아웃 카운터 초기화 (3.5T 무전송 시간 감시 목적)
    rx_timeout_counter = 0; 
}

// 에러 예외 응답 프레임 생성 헬퍼 함수
static void SendExceptionResponse(uint8_t func_code, uint8_t exception_code)
{
    uint8_t tx_buf[5];
    tx_buf[0] = MODBUS_SLAVE_ADDR;
    tx_buf[1] = func_code | 0x80; // 에러 표시 비트 추가 (MSB=1)
    tx_buf[2] = exception_code;
    
    uint16_t crc = Modbus_CRC16(tx_buf, 3);
    tx_buf[3] = (uint8_t)(crc & 0xFF);         // CRC Low
    tx_buf[4] = (uint8_t)((crc >> 8) & 0xFF);  // CRC High

    // RS-422 Full-Duplex 포트를 통해 관제 프로그램으로 송출
    RS422_WriteBuffer(tx_buf, 5);
}

// 정상 응답 패킷 분석 및 분기 제어 함수
static void Modbus_ProcessFrame(void)
{
    // 최소 패킷 크기 검증 (국번 1 + 기능코드 1 + 데이터시작/길이/수식 4 + CRC 2 = 최소 8바이트)
    if (rx_index < 8)
    {
        return;
    }

    // [1] 수신된 국번(Slave ID) 검증
    if (rx_buffer[0] != MODBUS_SLAVE_ADDR)
    {
        return; // 나를 호출한 패킷이 아니면 무시
    }

    // [2] CRC16 무결성 검증
    uint16_t rx_crc = (uint16_t)rx_buffer[rx_index - 2] | ((uint16_t)rx_buffer[rx_index - 1] << 8);
    uint16_t calc_crc = Modbus_CRC16(rx_buffer, rx_index - 2);
    if (rx_crc != calc_crc)
    {
        return; // CRC 에러인 경우 응답 없이 버림
    }

    uint8_t func_code = rx_buffer[1];
    uint16_t start_addr = ((uint16_t)rx_buffer[2] << 8) | rx_buffer[3];
    uint16_t quantity = ((uint16_t)rx_buffer[4] << 8) | rx_buffer[5];

    uint8_t tx_buf[MODBUS_BUFFER_SIZE];
    uint16_t tx_len = 0;

    switch (func_code)
    {
        case MODBUS_FC_READ_HOLDING_REGS: // 0x03 번지 읽기
        {
            if (start_addr + quantity > 40)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            tx_buf[0] = MODBUS_SLAVE_ADDR;
            tx_buf[1] = func_code;
            tx_buf[2] = (uint8_t)(quantity * 2); // 바이트 개수
            
            tx_len = 3;
            for (uint16_t i = 0; i < quantity; i++)
            {
                uint16_t reg_val = modbus_db.holding_regs[start_addr + i];
                tx_buf[tx_len++] = (uint8_t)((reg_val >> 8) & 0xFF);
                tx_buf[tx_len++] = (uint8_t)(reg_val & 0xFF);
            }
            break;
        }

        case MODBUS_FC_READ_INPUT_REGS: // 0x04 번지 읽기 (아날로그 온도 및 ADC 계측값)
        {
            if (start_addr + quantity > 80)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            tx_buf[0] = MODBUS_SLAVE_ADDR;
            tx_buf[1] = func_code;
            tx_buf[2] = (uint8_t)(quantity * 2);
            
            tx_len = 3;
            for (uint16_t i = 0; i < quantity; i++)
            {
                uint16_t reg_val = modbus_db.input_regs[start_addr + i];
                tx_buf[tx_len++] = (uint8_t)((reg_val >> 8) & 0xFF);
                tx_buf[tx_len++] = (uint8_t)(reg_val & 0xFF);
            }
            break;
        }

        case MODBUS_FC_WRITE_SINGLE_REG: // 0x06 단일 홀딩 레지스터 쓰기 (설정 변경 연동)
        {
            if (start_addr >= 40)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            uint16_t write_val = ((uint16_t)rx_buffer[4] << 8) | rx_buffer[5];
            modbus_db.holding_regs[start_addr] = write_val;

            // 💡 실무 연동 특화: 만약 관제단에서 DAC60516 설정 채널(0 ~ 11번지)을 변경한 경우
            // 즉시 해당 DAC 채널 출력을 실시간 갱신 처리!
            if (start_addr < 12)
            {
                DAC60516_WriteChannel(start_addr, write_val);
            }

            // 에코백 응답 조립 (수신 패킷 그대로 돌려줌)
            for (int i = 0; i < 6; i++)
            {
                tx_buf[i] = rx_buffer[i];
            }
            tx_len = 6;
            break;
        }

        default:
            // 지원하지 않는 기능 코드인 경우 예외 프레임 송출
            SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_FUNCTION);
            return;
    }

    // CRC 추가 및 패킷 송출
    uint16_t tx_crc = Modbus_CRC16(tx_buf, tx_len);
    tx_buf[tx_len++] = (uint8_t)(tx_crc & 0xFF);
    tx_buf[tx_len++] = (uint8_t)((tx_crc >> 8) & 0xFF);

    RS422_WriteBuffer(tx_buf, tx_len);
}

void Modbus_Task(void)
{
    // [1] 시리얼 포트(RS-422)로부터 수신 데이터가 들어오고 있는지 상시 긁어옴
    while (RS422_IsRxReady())
    {
        uint8_t rx_data = RS422_ReadByte();
        Modbus_ProcessRxByte(rx_data);
    }

    // [2] 3.5T 캐릭터 무전송 타임아웃 판정 (보레이트 9600 bps 기준 약 4ms 수준의 간격)
    // 메인 루프 속도에 맞추어 카운터를 올리며 감시
    if (rx_index > 0 && !rx_frame_completed)
    {
        rx_timeout_counter++;
        if (rx_timeout_counter > 50000UL) // 메인 루프 대기 카운트 임계치 (대략 수 ms 수준)
        {
            rx_frame_completed = true;
        }
    }

    // [3] 수신 완료된 프레임이 있으면 분석 연산 처리하고 버퍼 리셋
    if (rx_frame_completed)
    {
        Modbus_ProcessFrame();
        
        // 프레임 버퍼 리셋하여 다음 패킷 수신 준비
        rx_index = 0;
        rx_frame_completed = false;
        rx_timeout_counter = 0;
    }
}
