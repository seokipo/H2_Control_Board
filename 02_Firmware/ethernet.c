/**
 * @file ethernet.c
 * @brief Dual W5500 Ethernet Controller & Modbus TCP Server Implementation
 * 
 * @note dsPIC33CK512MP710 MCU의 하드웨어 SPI2 및 PPS 핀 매핑을 활성화하여
 *       공유 SPI 버스 통신을 수행하고, 개별 CS/RESET GPIO선을 이용해 유닛을 제어합니다.
 *       Socket 0을 표준 Modbus TCP (Port 502) 서버로 동작시켜 PC 관제단과 통신합니다.
 */

#include "ethernet.h"
#include "pin_map.h"
#include "modbus.h"
#include "dac60516.h"

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

    // [2] SPI2 주변기기 제어 레지스터 설정 (마스터 모드)
    SPI2CON1L = 0x0000;
    SPI2CON1H = 0x0000;
    
    // 마스터 모드 활성화 (MSTEN = 1)
    SPI2CON1Lbits.MSTEN = 1;
    
    // 클록 극성 및 위상 설정 (Mode 0: CKP = 0, CKE = 1)
    SPI2CON1Lbits.CKP = 0;
    SPI2CON1Lbits.CKE = 1;
    
    // 8비트 모드 설정 (MODE = 0)
    SPI2CON1Lbits.MODE = 0; 
    
    // Baudrate Generator: FCY = 4MHz 기준 SPI2BRGL = 0 -> 2.0 MHz SPI 클록
    SPI2BRGL = 0;

    // SPI2 모드 활성화
    SPI2CON1Lbits.SPIEN = 1;

    // [3] 이더넷 칩셋 하드웨어 일괄 리셋
    ETH_ResetDevice(ETH_UNIT_1);
    ETH_ResetDevice(ETH_UNIT_2);

    // [4] 기본 네트워크 파라미터 (192.168.0.100) 주입 (ETH1)
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
    if (unit == ETH_UNIT_1)
    {
        DO_ETH1_RST_LAT = 0;
        for (volatile uint16_t delay = 0; delay < 10000; delay++);
        DO_ETH1_RST_LAT = 1; // 리셋 해제
    }
    else if (unit == ETH_UNIT_2)
    {
        DO_ETH2_RST_LAT = 0;
        for (volatile uint16_t delay = 0; delay < 10000; delay++);
        DO_ETH2_RST_LAT = 1; // 리셋 해제
    }
    
    // W5500 내부 PLL 락 및 기동 안정화 대기 시간 (약 50ms)
    for (volatile uint32_t delay = 0; delay < 100000; delay++);
}

void ETH_WriteReg(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase, uint8_t data_val)
{
    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 0;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 0;

    SPI2_Exchange8bit((reg_addr >> 8) & 0xFF);
    SPI2_Exchange8bit(reg_addr & 0xFF);
    
    uint8_t ctrl = (control_phase & 0xF8) | 0x04; // Write VDM
    SPI2_Exchange8bit(ctrl);

    SPI2_Exchange8bit(data_val);

    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 1;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 1;
}

uint8_t ETH_ReadReg(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase)
{
    uint8_t rx_data = 0;

    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 0;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 0;

    SPI2_Exchange8bit((reg_addr >> 8) & 0xFF);
    SPI2_Exchange8bit(reg_addr & 0xFF);
    
    uint8_t ctrl = (control_phase & 0xF8) | 0x00; // Read VDM
    SPI2_Exchange8bit(ctrl);

    rx_data = SPI2_Exchange8bit(0xFF);

    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 1;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 1;

    return rx_data;
}

void ETH_WriteBuf(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase, const uint8_t *buf, uint16_t len)
{
    if (!buf || len == 0) return;

    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 0;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 0;

    SPI2_Exchange8bit((reg_addr >> 8) & 0xFF);
    SPI2_Exchange8bit(reg_addr & 0xFF);
    
    uint8_t ctrl = (control_phase & 0xF8) | 0x04; // Write VDM
    SPI2_Exchange8bit(ctrl);

    for (uint16_t i = 0; i < len; i++) {
        SPI2_Exchange8bit(buf[i]);
    }

    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 1;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 1;
}

void ETH_ReadBuf(ETH_Unit_t unit, uint16_t reg_addr, uint8_t control_phase, uint8_t *buf, uint16_t len)
{
    if (!buf || len == 0) return;

    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 0;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 0;

    SPI2_Exchange8bit((reg_addr >> 8) & 0xFF);
    SPI2_Exchange8bit(reg_addr & 0xFF);
    
    uint8_t ctrl = (control_phase & 0xF8) | 0x00; // Read VDM
    SPI2_Exchange8bit(ctrl);

    for (uint16_t i = 0; i < len; i++) {
        buf[i] = SPI2_Exchange8bit(0xFF);
    }

    if (unit == ETH_UNIT_1)      DO_ETH1_CS_LAT = 1;
    else if (unit == ETH_UNIT_2) DO_ETH2_CS_LAT = 1;
}

void ETH_SetNetworkInfo(ETH_Unit_t unit, const ETH_NetInfo_t* net_info)
{
    if (!net_info) return;

    // Gateway IP (0x0001 ~ 0x0004)
    for (uint8_t i = 0; i < 4; i++) {
        ETH_WriteReg(unit, 0x0001 + i, 0x00, net_info->gw[i]);
    }
    // Subnet Mask (0x0005 ~ 0x0008)
    for (uint8_t i = 0; i < 4; i++) {
        ETH_WriteReg(unit, 0x0005 + i, 0x00, net_info->sn[i]);
    }
    // MAC Address (0x0009 ~ 0x000E)
    for (uint8_t i = 0; i < 6; i++) {
        ETH_WriteReg(unit, 0x0009 + i, 0x00, net_info->mac[i]);
    }
    // IP Address (0x000F ~ 0x0012)
    for (uint8_t i = 0; i < 4; i++) {
        ETH_WriteReg(unit, 0x000F + i, 0x00, net_info->ip[i]);
    }
}

void ETH_GetNetworkInfo(ETH_Unit_t unit, ETH_NetInfo_t* net_info)
{
    if (!net_info) return;

    for (uint8_t i = 0; i < 4; i++) {
        net_info->gw[i] = ETH_ReadReg(unit, 0x0001 + i, 0x00);
    }
    for (uint8_t i = 0; i < 4; i++) {
        net_info->sn[i] = ETH_ReadReg(unit, 0x0005 + i, 0x00);
    }
    for (uint8_t i = 0; i < 6; i++) {
        net_info->mac[i] = ETH_ReadReg(unit, 0x0009 + i, 0x00);
    }
    for (uint8_t i = 0; i < 4; i++) {
        net_info->ip[i] = ETH_ReadReg(unit, 0x000F + i, 0x00);
    }
}

/* ==========================================================================
 * 2. Modbus TCP 서버 구현 (W5500 ETH_UNIT_1, Socket 0)
 * ========================================================================== */

static uint16_t s_modbus_tcp_port = MODBUS_TCP_PORT;

// W5500 소켓 0 블록 지정 상수
#define S0_REG_BLOCK  0x08  // Socket 0 Register (BSB = 00001 -> 0x08)
#define S0_TX_BLOCK   0x10  // Socket 0 TX Buffer (BSB = 00010 -> 0x10)
#define S0_RX_BLOCK   0x18  // Socket 0 RX Buffer (BSB = 00011 -> 0x18)

// W5500 Socket 0 레지스터 오프셋
#define Sn_MR         0x0000
#define Sn_CR         0x0001
#define Sn_IR         0x0002
#define Sn_SR         0x0003
#define Sn_PORT0      0x0004
#define Sn_PORT1      0x0005
#define Sn_TX_FSR0    0x0020
#define Sn_TX_FSR1    0x0021
#define Sn_TX_RD0     0x0022
#define Sn_TX_RD1     0x0023
#define Sn_TX_WR0     0x0024
#define Sn_TX_WR1     0x0025
#define Sn_RX_RSR0    0x0026
#define Sn_RX_RSR1    0x0027
#define Sn_RX_RD0     0x0028
#define Sn_RX_RD1     0x0029

// 소켓 상태 상수
#define SOCK_CLOSED      0x00
#define SOCK_INIT        0x13
#define SOCK_LISTEN      0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT  0x1C

// 소켓 명령 상수
#define CMD_OPEN    0x01
#define CMD_LISTEN  0x02
#define CMD_DISCON  0x08
#define CMD_CLOSE   0x10
#define CMD_SEND    0x20
#define CMD_RECV    0x40

void ETH_ModbusTCP_Init(uint16_t port)
{
    s_modbus_tcp_port = port;

    // 소켓 0 강제 종료 및 초기화
    ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_CLOSE);
    while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));

    // 인터럽트 플래그 일괄 클리어
    ETH_WriteReg(ETH_UNIT_1, Sn_IR, S0_REG_BLOCK, 0xFF);
}

// Modbus TCP PDU 처리 및 응답 패킷 생성기
static uint16_t ProcessModbusTCP_PDU(const uint8_t *req, uint16_t req_len, uint8_t *resp)
{
    // 최소 길이: MBAP Header(7) + FC(1) + Addr(2) + Qty/Val(2) = 12 바이트
    if (req_len < 12) return 0;

    // MBAP Header 복사
    resp[0] = req[0]; // Transaction ID High
    resp[1] = req[1]; // Transaction ID Low
    resp[2] = 0x00;   // Protocol ID High (0 = Modbus)
    resp[3] = 0x00;   // Protocol ID Low
    resp[6] = req[6]; // Unit ID (Slave Address)

    uint8_t fc = req[7];
    uint16_t addr = ((uint16_t)req[8] << 8) | req[9];
    uint16_t qty_val = ((uint16_t)req[10] << 8) | req[11];

    uint16_t pdu_len = 0;

    switch (fc)
    {
        case MODBUS_FC_READ_COILS: // 0x01 DO 릴레이 코일 읽기
        {
            if (addr + qty_val > 32) {
                // 예외 응답
                resp[7] = fc | 0x80;
                resp[8] = MODBUS_ERR_ILLEGAL_DATA_ADDR;
                pdu_len = 2;
                break;
            }
            uint8_t byte_count = (uint8_t)((qty_val + 7) / 8);
            resp[7] = fc;
            resp[8] = byte_count;
            pdu_len = 2;

            for (uint8_t b = 0; b < byte_count; b++) {
                uint8_t pack = 0;
                for (uint8_t bit = 0; bit < 8; bit++) {
                    uint16_t idx = addr + b * 8 + bit;
                    if (idx < addr + qty_val && idx < 20) {
                        if (modbus_db.coils[idx / 8] & (1 << (idx % 8))) {
                            pack |= (1 << bit);
                        }
                    }
                }
                resp[7 + pdu_len] = pack;
                pdu_len++;
            }
            break;
        }

        case MODBUS_FC_READ_HOLDING_REGS: // 0x03 Holding Register 읽기
        {
            if (addr + qty_val > 80) {
                resp[7] = fc | 0x80;
                resp[8] = MODBUS_ERR_ILLEGAL_DATA_ADDR;
                pdu_len = 2;
                break;
            }
            uint8_t byte_count = (uint8_t)(qty_val * 2);
            resp[7] = fc;
            resp[8] = byte_count;
            pdu_len = 2;

            for (uint16_t i = 0; i < qty_val; i++) {
                uint16_t val = modbus_db.holding_regs[addr + i];
                resp[7 + pdu_len++] = (uint8_t)((val >> 8) & 0xFF);
                resp[7 + pdu_len++] = (uint8_t)(val & 0xFF);
            }
            break;
        }

        case MODBUS_FC_READ_INPUT_REGS: // 0x04 Input Register 읽기 (TC, ADC 등)
        {
            if (addr + qty_val > 80) {
                resp[7] = fc | 0x80;
                resp[8] = MODBUS_ERR_ILLEGAL_DATA_ADDR;
                pdu_len = 2;
                break;
            }
            uint8_t byte_count = (uint8_t)(qty_val * 2);
            resp[7] = fc;
            resp[8] = byte_count;
            pdu_len = 2;

            for (uint16_t i = 0; i < qty_val; i++) {
                uint16_t val = modbus_db.input_regs[addr + i];
                resp[7 + pdu_len++] = (uint8_t)((val >> 8) & 0xFF);
                resp[7 + pdu_len++] = (uint8_t)(val & 0xFF);
            }
            break;
        }

        case MODBUS_FC_WRITE_SINGLE_COIL: // 0x05 DO 릴레이 단일 제어
        {
            if (addr >= 20) {
                resp[7] = fc | 0x80;
                resp[8] = MODBUS_ERR_ILLEGAL_DATA_ADDR;
                pdu_len = 2;
                break;
            }
            bool state = (qty_val == 0xFF00);
            Modbus_SetDO((uint8_t)addr, state);

            // 요청 PDU 에코백
            resp[7] = fc;
            resp[8] = req[8];
            resp[9] = req[9];
            resp[10] = req[10];
            resp[11] = req[11];
            pdu_len = 5;
            break;
        }

        case MODBUS_FC_WRITE_SINGLE_REG: // 0x06 단일 레지스터 쓰기
        {
            if (addr >= 80) {
                resp[7] = fc | 0x80;
                resp[8] = MODBUS_ERR_ILLEGAL_DATA_ADDR;
                pdu_len = 2;
                break;
            }
            modbus_db.holding_regs[addr] = qty_val;

            // DAC 채널 쓰기인 경우 즉각 아날로그 출력 반영 (0~11번)
            if (addr < 12) {
                DAC60516_WriteVoltage(dac_channel_map[addr], (float)qty_val / 1000.0f);
            }

            // 요청 PDU 에코백
            resp[7] = fc;
            resp[8] = req[8];
            resp[9] = req[9];
            resp[10] = req[10];
            resp[11] = req[11];
            pdu_len = 5;
            break;
        }

        case MODBUS_FC_WRITE_MULTIPLE_REGS: // 0x10 다중 레지스터 쓰기
        {
            uint8_t byte_cnt = req[12];
            if (addr + qty_val > 80 || byte_cnt != qty_val * 2) {
                resp[7] = fc | 0x80;
                resp[8] = MODBUS_ERR_ILLEGAL_DATA_ADDR;
                pdu_len = 2;
                break;
            }
            for (uint16_t i = 0; i < qty_val; i++) {
                uint16_t val = ((uint16_t)req[13 + i * 2] << 8) | req[14 + i * 2];
                modbus_db.holding_regs[addr + i] = val;
                if ((addr + i) < 12) {
                    DAC60516_WriteVoltage(dac_channel_map[addr + i], (float)val / 1000.0f);
                }
            }

            resp[7] = fc;
            resp[8] = req[8];
            resp[9] = req[9];
            resp[10] = req[10];
            resp[11] = req[11];
            pdu_len = 5;
            break;
        }

        default:
            resp[7] = fc | 0x80;
            resp[8] = MODBUS_ERR_ILLEGAL_FUNCTION;
            pdu_len = 2;
            break;

    }

    // MBAP Header의 Length 필드: Unit ID(1) + PDU 길이
    uint16_t mbap_len = 1 + pdu_len;
    resp[4] = (uint8_t)((mbap_len >> 8) & 0xFF);
    resp[5] = (uint8_t)(mbap_len & 0xFF);

    return 6 + mbap_len; // 총 응답 패킷 크기 (MBAP 6 + 1 + PDU)
}

void ETH_ModbusTCP_Task(void)
{
    uint8_t sr = ETH_ReadReg(ETH_UNIT_1, Sn_SR, S0_REG_BLOCK);

    switch (sr)
    {
        case SOCK_CLOSED:
            // 소켓을 TCP 모드로 열기
            ETH_WriteReg(ETH_UNIT_1, Sn_MR, S0_REG_BLOCK, 0x01); // TCP Mode
            ETH_WriteReg(ETH_UNIT_1, Sn_PORT0, S0_REG_BLOCK, (s_modbus_tcp_port >> 8) & 0xFF);
            ETH_WriteReg(ETH_UNIT_1, Sn_PORT1, S0_REG_BLOCK, s_modbus_tcp_port & 0xFF);
            ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_OPEN);
            while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));
            break;

        case SOCK_INIT:
            // Listen 상태로 진입 (PC 연결 대기)
            ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_LISTEN);
            while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));
            break;

        case SOCK_LISTEN:
            // 클라이언트(PC)의 TCP 연결 대기 중 (정상 유휴 상태)
            break;

        case SOCK_ESTABLISHED:
        {
            // 수신 데이터 바이트 수 확인
            uint16_t rx_size = ((uint16_t)ETH_ReadReg(ETH_UNIT_1, Sn_RX_RSR0, S0_REG_BLOCK) << 8) | 
                               ETH_ReadReg(ETH_UNIT_1, Sn_RX_RSR1, S0_REG_BLOCK);

            if (rx_size >= 12)
            {
                if (rx_size > 256) rx_size = 256;

                // RX 포인터 읽기
                uint16_t rx_rd = ((uint16_t)ETH_ReadReg(ETH_UNIT_1, Sn_RX_RD0, S0_REG_BLOCK) << 8) |
                                 ETH_ReadReg(ETH_UNIT_1, Sn_RX_RD1, S0_REG_BLOCK);

                uint8_t rx_buf[256];
                ETH_ReadBuf(ETH_UNIT_1, rx_rd, S0_RX_BLOCK, rx_buf, rx_size);

                // RX 읽기 포인터 갱신 및 RECV 완료 통보
                rx_rd += rx_size;
                ETH_WriteReg(ETH_UNIT_1, Sn_RX_RD0, S0_REG_BLOCK, (rx_rd >> 8) & 0xFF);
                ETH_WriteReg(ETH_UNIT_1, Sn_RX_RD1, S0_REG_BLOCK, rx_rd & 0xFF);
                ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_RECV);
                while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));

                // Modbus TCP PDU 파싱 및 응답 생성
                uint8_t tx_buf[256];
                uint16_t tx_len = ProcessModbusTCP_PDU(rx_buf, rx_size, tx_buf);

                if (tx_len > 0)
                {
                    // TX 포인터 읽기
                    uint16_t tx_wr = ((uint16_t)ETH_ReadReg(ETH_UNIT_1, Sn_TX_WR0, S0_REG_BLOCK) << 8) |
                                     ETH_ReadReg(ETH_UNIT_1, Sn_TX_WR1, S0_REG_BLOCK);

                    ETH_WriteBuf(ETH_UNIT_1, tx_wr, S0_TX_BLOCK, tx_buf, tx_len);

                    tx_wr += tx_len;
                    ETH_WriteReg(ETH_UNIT_1, Sn_TX_WR0, S0_REG_BLOCK, (tx_wr >> 8) & 0xFF);
                    ETH_WriteReg(ETH_UNIT_1, Sn_TX_WR1, S0_REG_BLOCK, tx_wr & 0xFF);

                    // SEND 명령 발행
                    ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_SEND);
                    while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));
                }
            }
            break;
        }

        case SOCK_CLOSE_WAIT:
            // 상대방이 TCP 연결을 닫았을 때
            ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_DISCON);
            while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));
            ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_CLOSE);
            while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));
            break;

        default:
            // 알 수 없는 상태일 경우 소켓 강제 클로즈 후 재초기화
            ETH_WriteReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK, CMD_CLOSE);
            while (ETH_ReadReg(ETH_UNIT_1, Sn_CR, S0_REG_BLOCK));
            break;
    }
}
