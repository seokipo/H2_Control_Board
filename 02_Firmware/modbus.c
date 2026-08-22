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
#include "pin_map.h"

// 전역 Modbus 레지스터 데이터베이스 인스턴스
Modbus_Registers_t modbus_db;

// 통신 수신 제어 변수들
static uint8_t rx_buffer[MODBUS_BUFFER_SIZE];
static uint16_t rx_index = 0;
static bool rx_frame_completed = false;
static uint32_t rx_timeout_counter = 0;

// UI 대시보드 인덱스(0~11)와 MCU DAC60516 채널 간 1:1 매핑 테이블
static const DAC60516_OutputChannel_t dac_channel_map[12] = {
    AO_AB212_STACK_BLOWER,    // 0: STACK 공기 공급 에어블로어 (AO_AB212)
    AO_AB221_BURNER_BLOWER,   // 1: 개질기 연소용 에어블로어 (AO_AB221)
    AO_AB232_PROX_BLOWER,     // 2: Prox 제어용 에어블로어 (AO_AB232)
    AO_MFC111_BNG_FLOW,       // 3: BNG 유량 제어 MFC (AO_MFC111)
    AO_MFC121_PNG_FLOW,       // 4: PNG 유량 제어 MFC (AO_MFC121)
    AO_P108_GAS_PRESS_PUMP,   // 5: 연료가스 공급 가압펌프 (AO_P108)
    AO_P341_AOG_COOL_PUMP,    // 6: AOG 회수라인 응축수 펌프 (AO_P341)
    AO_P351_ANODE_COOL_PUMP,  // 7: Anode 냉각수 순환 물펌프 (AO_P351)
    AO_P370_STACK1_COOL_PUMP, // 8: STACK 1 냉각수 공급 펌프 (AO_P370)
    AO_P375_STACK2_COOL_PUMP, // 9: STACK 2 냉각수 공급 펌프 (AO_P375)
    AO_P380_WATER_PUMP,       // 10: 개질기 물 공급 정량 펌프 (AO_P380)
    AO_SPARE1                 // 11: 예비 아날로그 출력 (AO_SPARE1)
};

void Modbus_SetDO(uint8_t index, bool state)
{
    // 코일 비트 업데이트 (0~19)
    if (index < 32)
    {
        uint8_t byte_idx = index / 8;
        uint8_t bit_mask = 1 << (index % 8);
        if (state)
        {
            modbus_db.coils[byte_idx] |= bit_mask;
        }
        else
        {
            modbus_db.coils[byte_idx] &= ~bit_mask;
        }
    }

    // 홀딩 레지스터(20 + index) 동기화
    if (index < 20)
    {
        modbus_db.holding_regs[20 + index] = state ? 1 : 0;
    }

    // 물리 GPIO LAT 핀 즉각 스위칭
    switch (index)
    {
        case 0:  DO_SV102_LAT = state ? 1 : 0; break; // 가스 인입 SV1
        case 1:  DO_SV103_LAT = state ? 1 : 0; break; // 가스 인입 SV2
        case 2:  DO_SV125_LAT = state ? 1 : 0; break; // 개질기 N2 퍼지 SV
        case 3:  DO_SV145_LAT = state ? 1 : 0; break; // ANODE 인입 SV
        case 4:  DO_SV149_LAT = state ? 1 : 0; break; // 스택 N2 퍼지 SV
        case 5:  DO_SV323_LAT = state ? 1 : 0; break; // DI TANK 드레인 SV
        case 6:  DO_MV124_LAT = state ? 1 : 0; break; // PNG 인입 MV
        case 7:  DO_MV131_LAT = state ? 1 : 0; break; // 개질 가스 인입 MV
        case 8:  DO_MV148_LAT = state ? 1 : 0; break; // 개질가스 바이패스 MV
        case 9:  DO_MV150_LAT = state ? 1 : 0; break; // Anode 출구 MV
        case 10: DO_MV155_LAT = state ? 1 : 0; break; // AOG 전동 MV
        case 11: DO_MV158_LAT = state ? 1 : 0; break; // AOG VENT MV
        case 12: DO_MV300_LAT = state ? 1 : 0; break; // 시설수 입구 MV
        case 13: DO_MV390_LAT = state ? 1 : 0; break; // 배열 회수 입구 MV
        case 14: DO_MC_SW_LAT = state ? 1 : 0; break; // MC 동작 스위치
        case 15: DO_HT193_LAT = state ? 1 : 0; break; // LTS 촉매 히터 1
        case 16: DO_HT194_LAT = state ? 1 : 0; break; // LTS 촉매 히터 2
        case 17: DO_HT195_LAT = state ? 1 : 0; break; // Prox 히터 1
        case 18: DO_HT196_LAT = state ? 1 : 0; break; // Prox 히터 2
        case 19: DO_IGN175_LAT = state ? 1 : 0; break; // 점화 트랜스
        default: break;
    }
}

// Modbus RTU CRC16 고속 연산 표준 다항식 알고리즘 (0xA001)
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

    // MC_SW 기본 ON 보존 (비트 14 = 1)
    modbus_db.coils[1] |= (1 << (14 - 8));
    modbus_db.holding_regs[34] = 1;

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
    // 최소 패킷 크기 검증 (국번 1 + 기능코드 1 + 데이터 4 + CRC 2 = 최소 8바이트)
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
        case MODBUS_FC_READ_COILS: // 0x01 Coils 상태 읽기 (DO 릴레이 20채널)
        {
            if (start_addr + quantity > 32)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            uint8_t byte_count = (uint8_t)((quantity + 7) / 8);
            tx_buf[0] = MODBUS_SLAVE_ADDR;
            tx_buf[1] = func_code;
            tx_buf[2] = byte_count;
            tx_len = 3;

            for (uint8_t b = 0; b < byte_count; b++)
            {
                uint8_t pack = 0;
                for (uint8_t bit = 0; bit < 8; bit++)
                {
                    uint16_t c_idx = start_addr + (b * 8) + bit;
                    if (c_idx < start_addr + quantity && c_idx < 32)
                    {
                        uint8_t src_byte = c_idx / 8;
                        uint8_t src_bit = c_idx % 8;
                        if (modbus_db.coils[src_byte] & (1 << src_bit))
                        {
                            pack |= (1 << bit);
                        }
                    }
                }
                tx_buf[tx_len++] = pack;
            }
            break;
        }

        case MODBUS_FC_READ_DISCRETE_INPUTS: // 0x02 Discrete Inputs 상태 읽기 (DI 16채널)
        {
            if (start_addr + quantity > 16)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            uint8_t byte_count = (uint8_t)((quantity + 7) / 8);
            tx_buf[0] = MODBUS_SLAVE_ADDR;
            tx_buf[1] = func_code;
            tx_buf[2] = byte_count;
            tx_len = 3;

            for (uint8_t b = 0; b < byte_count; b++)
            {
                tx_buf[tx_len++] = modbus_db.discrete_inputs[b];
            }
            break;
        }

        case MODBUS_FC_READ_HOLDING_REGS: // 0x03 Holding Registers 읽기 (DAC 및 제어 설정)
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

        case MODBUS_FC_READ_INPUT_REGS: // 0x04 Input Registers 읽기 (온도 0~30, ADC 32~47, RTC 50~55)
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

        case MODBUS_FC_WRITE_SINGLE_COIL: // 0x05 단일 코일 쓰기 (DO 릴레이 온/오프 제어)
        {
            if (start_addr >= 20)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            uint16_t coil_val = ((uint16_t)rx_buffer[4] << 8) | rx_buffer[5];
            bool coil_on = (coil_val == 0xFF00);

            // 릴레이 물리 핀 및 DB 즉시 스위칭
            Modbus_SetDO((uint8_t)start_addr, coil_on);

            // 에코백 응답 (수신 패킷 그대로 돌려줌)
            for (int i = 0; i < 6; i++)
            {
                tx_buf[i] = rx_buffer[i];
            }
            tx_len = 6;
            break;
        }

        case MODBUS_FC_WRITE_SINGLE_REG: // 0x06 단일 홀딩 레지스터 쓰기 (DAC 및 DO 제어)
        {
            if (start_addr >= 40)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            uint16_t write_val = ((uint16_t)rx_buffer[4] << 8) | rx_buffer[5];
            modbus_db.holding_regs[start_addr] = write_val;

            // [1] DAC60516 아날로그 출력 채널(0 ~ 11번지) 제어
            if (start_addr < 12)
            {
                DAC60516_OutputChannel_t dac_ch = dac_channel_map[start_addr];
                DAC60516_WriteRaw(dac_ch, write_val);
            }
            // [2] DO 디지털 릴레이 출력 채널(20 ~ 39번지) 제어
            else if (start_addr >= 20 && start_addr < 40)
            {
                uint8_t do_idx = (uint8_t)(start_addr - 20);
                Modbus_SetDO(do_idx, write_val > 0);
            }

            // 에코백 응답 조립 (수신 패킷 그대로 돌려줌)
            for (int i = 0; i < 6; i++)
            {
                tx_buf[i] = rx_buffer[i];
            }
            tx_len = 6;
            break;
        }

        case MODBUS_FC_WRITE_MULTIPLE_REGS: // 0x10 복수 홀딩 레지스터 일괄 쓰기
        {
            if (start_addr + quantity > 40)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_ADDR);
                return;
            }

            uint8_t byte_count = rx_buffer[6];
            if (byte_count != quantity * 2)
            {
                SendExceptionResponse(func_code, MODBUS_ERR_ILLEGAL_DATA_VAL);
                return;
            }

            for (uint16_t i = 0; i < quantity; i++)
            {
                uint16_t reg_addr = start_addr + i;
                uint16_t write_val = ((uint16_t)rx_buffer[7 + (i * 2)] << 8) | rx_buffer[8 + (i * 2)];
                modbus_db.holding_regs[reg_addr] = write_val;

                if (reg_addr < 12)
                {
                    DAC60516_OutputChannel_t dac_ch = dac_channel_map[reg_addr];
                    DAC60516_WriteRaw(dac_ch, write_val);
                }
                else if (reg_addr >= 20 && reg_addr < 40)
                {
                    uint8_t do_idx = (uint8_t)(reg_addr - 20);
                    Modbus_SetDO(do_idx, write_val > 0);
                }
            }

            // 정상 응답 (국번, FC, StartAddr_H, StartAddr_L, Qty_H, Qty_L)
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

    // [2] 3.5T 캐릭터 무전송 타임아웃 판정 (보레이트 9600/115200 bps 기준 프레임 경계 감시)
    if (rx_index > 0 && !rx_frame_completed)
    {
        rx_timeout_counter++;
        if (rx_timeout_counter > 50000UL) // 메인 루프 대기 카운트 임계치
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

