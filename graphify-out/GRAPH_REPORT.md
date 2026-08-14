# Graph Report - .  (2026-08-12)

## Corpus Check
- Large corpus: 221 files · ~1,819,591 words. Semantic extraction will be expensive (many Claude tokens). Consider running on a subfolder, or use --no-semantic to run AST-only.

## Summary
- 98 nodes · 170 edges · 8 communities detected
- Extraction: 87% EXTRACTED · 13% INFERRED · 0% AMBIGUOUS · INFERRED: 22 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Modbus Protocol & RS-422|Modbus Protocol & RS-422]]
- [[_COMMUNITY_I2C Sensor Interface|I2C Sensor Interface]]
- [[_COMMUNITY_Main Loop & Thermocouple Control|Main Loop & Thermocouple Control]]
- [[_COMMUNITY_SPI Flash Memory Driver|SPI Flash Memory Driver]]
- [[_COMMUNITY_DAC60516 Analog Output|DAC60516 Analog Output]]
- [[_COMMUNITY_W5500 Ethernet Controller|W5500 Ethernet Controller]]
- [[_COMMUNITY_RS-485 Serial Communication|RS-485 Serial Communication]]
- [[_COMMUNITY_Python Serial Bridge Server|Python Serial Bridge Server]]

## God Nodes (most connected - your core abstractions)
1. `main()` - 13 edges
2. `SPI3_Exchange8bit()` - 8 edges
3. `I2C1_Write()` - 7 edges
4. `Modbus_Task()` - 7 edges
5. `ADS1115_ReadRaw()` - 6 edges
6. `FLASH_WaitBusy()` - 6 edges
7. `Modbus_ProcessFrame()` - 6 edges
8. `RTC_GetTime()` - 6 edges
9. `I2C1_Read()` - 5 edges
10. `SPI1_Exchange8bit()` - 5 edges

## Surprising Connections (you probably didn't know these)
- `MAX31856_WriteRegister()` --calls--> `SPI1_Exchange8bit()`  [INFERRED]
  D:\Work\H2_Control_Board\02_Firmware\thermocouple.c → D:\Work\H2_Control_Board\02_Firmware\dac60516.c
- `MAX31856_ReadRegister()` --calls--> `SPI1_Exchange8bit()`  [INFERRED]
  D:\Work\H2_Control_Board\02_Firmware\thermocouple.c → D:\Work\H2_Control_Board\02_Firmware\dac60516.c
- `main()` --calls--> `ETH_Initialize()`  [INFERRED]
  D:\Work\H2_Control_Board\02_Firmware\main.c → D:\Work\H2_Control_Board\02_Firmware\ethernet.c
- `main()` --calls--> `FLASH_Initialize()`  [INFERRED]
  D:\Work\H2_Control_Board\02_Firmware\main.c → D:\Work\H2_Control_Board\02_Firmware\flash.c
- `main()` --calls--> `RTC_Initialize()`  [INFERRED]
  D:\Work\H2_Control_Board\02_Firmware\main.c → D:\Work\H2_Control_Board\02_Firmware\rtc.c

## Communities

### Community 0 - "Modbus Protocol & RS-422"
Cohesion: 0.3
Nodes (11): Modbus_CRC16(), Modbus_Initialize(), Modbus_ProcessFrame(), Modbus_ProcessRxByte(), Modbus_Task(), SendExceptionResponse(), RS422_Initialize(), RS422_IsRxReady() (+3 more)

### Community 1 - "I2C Sensor Interface"
Cohesion: 0.32
Nodes (11): ADS1115_Initialize(), ADS1115_ReadRaw(), ADS1115_ReadVoltage(), GetChannelMapping(), I2C1_Read(), I2C1_Write(), BCD2DEC(), DEC2BCD() (+3 more)

### Community 2 - "Main Loop & Thermocouple Control"
Cohesion: 0.27
Nodes (7): main(), GPIO_Initialize(), MAX31856_ReadRegister(), MAX31856_WriteRegister(), TC_Initialize(), TC_ReadTemperature(), TC_SelectChannel()

### Community 3 - "SPI Flash Memory Driver"
Cohesion: 0.6
Nodes (8): FLASH_EraseSector(), FLASH_Initialize(), FLASH_ReadData(), FLASH_ReadID(), FLASH_WaitBusy(), FLASH_WriteEnable(), FLASH_WritePage(), SPI3_Exchange8bit()

### Community 4 - "DAC60516 Analog Output"
Cohesion: 0.61
Nodes (6): DAC60516_Initialize(), DAC60516_SPI_Write(), DAC60516_WriteRaw(), DAC60516_WriteVoltage(), GetRegisterAddress(), SPI1_Exchange8bit()

### Community 5 - "W5500 Ethernet Controller"
Cohesion: 0.62
Nodes (5): ETH_Initialize(), ETH_ReadReg(), ETH_ResetDevice(), ETH_WriteReg(), SPI2_Exchange8bit()

### Community 6 - "RS-485 Serial Communication"
Cohesion: 0.48
Nodes (5): RS485_Initialize(), RS485_IsRxReady(), RS485_ReadByte(), RS485_WriteBuffer(), RS485_WriteByte()

### Community 7 - "Python Serial Bridge Server"
Cohesion: 0.7
Nodes (3): handler(), main(), serial_reader()

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `main()` connect `Main Loop & Thermocouple Control` to `Modbus Protocol & RS-422`, `I2C Sensor Interface`, `SPI Flash Memory Driver`, `W5500 Ethernet Controller`, `RS-485 Serial Communication`?**
  _High betweenness centrality (0.495) - this node is a cross-community bridge._
- **Why does `FLASH_Initialize()` connect `SPI Flash Memory Driver` to `Main Loop & Thermocouple Control`?**
  _High betweenness centrality (0.126) - this node is a cross-community bridge._
- **Why does `SPI1_Exchange8bit()` connect `DAC60516 Analog Output` to `Main Loop & Thermocouple Control`?**
  _High betweenness centrality (0.101) - this node is a cross-community bridge._
- **Are the 11 inferred relationships involving `main()` (e.g. with `GPIO_Initialize()` and `TC_Initialize()`) actually correct?**
  _`main()` has 11 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `I2C1_Write()` (e.g. with `RTC_Initialize()` and `RTC_GetTime()`) actually correct?**
  _`I2C1_Write()` has 3 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `Modbus_Task()` (e.g. with `main()` and `RS422_IsRxReady()`) actually correct?**
  _`Modbus_Task()` has 3 INFERRED edges - model-reasoned connections that need verification._