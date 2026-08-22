# Graph Report - H2_Control_Board  (2026-08-20)

## Corpus Check
- 26 files · ~1,861,969 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 103 nodes · 160 edges · 9 communities detected
- Extraction: 79% EXTRACTED · 21% INFERRED · 0% AMBIGUOUS · INFERRED: 33 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 0|Community 0]]
- [[_COMMUNITY_Community 1|Community 1]]
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 4|Community 4]]
- [[_COMMUNITY_Community 5|Community 5]]
- [[_COMMUNITY_Community 6|Community 6]]
- [[_COMMUNITY_Community 7|Community 7]]
- [[_COMMUNITY_Community 8|Community 8]]

## God Nodes (most connected - your core abstractions)
1. `main()` - 16 edges
2. `SequenceEngine` - 13 edges
3. `handler()` - 9 edges
4. `SPI3_Exchange8bit()` - 7 edges
5. `Modbus_ProcessFrame()` - 7 edges
6. `I2C1_Write()` - 6 edges
7. `ADS1115_ReadRaw()` - 6 edges
8. `Modbus_Task()` - 6 edges
9. `DAC60516_WriteRaw()` - 5 edges
10. `FLASH_WaitBusy()` - 5 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `RS485_Initialize()`  [INFERRED]
  02_Firmware\main.c → 02_Firmware\rs485.c
- `main()` --calls--> `RS422_Initialize()`  [INFERRED]
  02_Firmware\main.c → 02_Firmware\rs422.c
- `main()` --calls--> `ADS1115_Initialize()`  [INFERRED]
  02_Firmware\main.c → 02_Firmware\ads1115.c
- `main()` --calls--> `ADS1115_ReadChannel()`  [INFERRED]
  02_Firmware\main.c → 02_Firmware\ads1115.c
- `SPI1_Exchange8bit()` --calls--> `MAX31856_WriteRegister()`  [INFERRED]
  02_Firmware\dac60516.c → 02_Firmware\thermocouple.c

## Communities

### Community 0 - "Community 0"
Cohesion: 0.24
Nodes (4): 센서 및 DI/ADC 데이터 갱신 및 인터록 감시, 수소 연료전지 자동 제어 시퀀스 비동기 상태 머신 엔진 (Sequence Engine)     JSON 레시피를 로드하여 스텝별 순차 자동 제어, SequenceEngine, handler()

### Community 1 - "Community 1"
Cohesion: 0.27
Nodes (12): ADS1115_Initialize(), ADS1115_ReadChannel(), ADS1115_ReadRaw(), ADS1115_ReadVoltage(), GetChannelMapping(), I2C1_Read(), I2C1_Write(), BCD2DEC() (+4 more)

### Community 2 - "Community 2"
Cohesion: 0.27
Nodes (10): Modbus_CRC16(), Modbus_ProcessFrame(), Modbus_ProcessRxByte(), Modbus_SetDO(), Modbus_Task(), SendExceptionResponse(), RS422_Initialize(), RS422_IsRxReady() (+2 more)

### Community 3 - "Community 3"
Cohesion: 0.24
Nodes (9): FLASH_Initialize(), main(), Modbus_Initialize(), GPIO_Initialize(), MAX31856_ReadRegister(), MAX31856_WriteRegister(), TC_Initialize(), TC_ReadTemperature() (+1 more)

### Community 4 - "Community 4"
Cohesion: 0.64
Nodes (7): FLASH_EraseSector(), FLASH_ReadData(), FLASH_ReadID(), FLASH_WaitBusy(), FLASH_WriteEnable(), FLASH_WritePage(), SPI3_Exchange8bit()

### Community 5 - "Community 5"
Cohesion: 0.52
Nodes (6): DAC60516_Initialize(), DAC60516_SPI_Write(), DAC60516_WriteRaw(), DAC60516_WriteVoltage(), GetRegisterAddress(), SPI1_Exchange8bit()

### Community 6 - "Community 6"
Cohesion: 0.38
Nodes (5): build_modbus_frame(), modbus_crc16(), 표준 Modbus RTU CRC-16 계산 (다항식 0xA001), 표준 Modbus RTU 요청 바이트 프레임 생성 (CRC16 포함), serial_reader()

### Community 7 - "Community 7"
Cohesion: 0.53
Nodes (5): ETH_Initialize(), ETH_ReadReg(), ETH_ResetDevice(), ETH_WriteReg(), SPI2_Exchange8bit()

### Community 8 - "Community 8"
Cohesion: 0.33
Nodes (1): RS485_Initialize()

## Knowledge Gaps
- **4 isolated node(s):** `수소 연료전지 자동 제어 시퀀스 비동기 상태 머신 엔진 (Sequence Engine)     JSON 레시피를 로드하여 스텝별 순차 자동 제어`, `센서 및 DI/ADC 데이터 갱신 및 인터록 감시`, `표준 Modbus RTU CRC-16 계산 (다항식 0xA001)`, `표준 Modbus RTU 요청 바이트 프레임 생성 (CRC16 포함)`
  These have ≤1 connection - possible missing edges or undocumented components.
- **Thin community `Community 8`** (6 nodes): `rs485.c`, `RS485_Initialize()`, `RS485_IsRxReady()`, `RS485_ReadByte()`, `RS485_WriteBuffer()`, `RS485_WriteByte()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `main()` connect `Community 3` to `Community 1`, `Community 2`, `Community 5`, `Community 7`, `Community 8`?**
  _High betweenness centrality (0.333) - this node is a cross-community bridge._
- **Why does `FLASH_Initialize()` connect `Community 3` to `Community 4`?**
  _High betweenness centrality (0.089) - this node is a cross-community bridge._
- **Why does `ETH_Initialize()` connect `Community 7` to `Community 3`?**
  _High betweenness centrality (0.058) - this node is a cross-community bridge._
- **Are the 15 inferred relationships involving `main()` (e.g. with `GPIO_Initialize()` and `TC_Initialize()`) actually correct?**
  _`main()` has 15 INFERRED edges - model-reasoned connections that need verification._
- **Are the 6 inferred relationships involving `handler()` (e.g. with `.set_broadcast_callback()` and `.start_recipe()`) actually correct?**
  _`handler()` has 6 INFERRED edges - model-reasoned connections that need verification._
- **Are the 2 inferred relationships involving `Modbus_ProcessFrame()` (e.g. with `DAC60516_WriteRaw()` and `RS422_WriteBuffer()`) actually correct?**
  _`Modbus_ProcessFrame()` has 2 INFERRED edges - model-reasoned connections that need verification._
- **What connects `수소 연료전지 자동 제어 시퀀스 비동기 상태 머신 엔진 (Sequence Engine)     JSON 레시피를 로드하여 스텝별 순차 자동 제어`, `센서 및 DI/ADC 데이터 갱신 및 인터록 감시`, `표준 Modbus RTU CRC-16 계산 (다항식 0xA001)` to the rest of the system?**
  _4 weakly-connected nodes found - possible documentation gaps or missing edges._