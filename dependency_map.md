# 🗺️ H2_Control_Board 시스템 의존성 지도 (dependency_map.md)

이 문서는 6kW 및 10kW BOP(Balance of Plant) 수소 제어 시스템의 **dsPIC33CK MCU 펌웨어**, **파이썬 비동기 브릿지 서버**, **FHD SCADA 관제 UI** 간의 상호 의존성, 공유 버스, 통신 무결성 및 출력 제어 흐름을 한눈에 보여주는 최신 아키텍처 지도입니다.

> 💡 **시각화 대시보드 안내**: 본 의존성 지도를 브라우저에서 인터랙티브하게 탐색할 수 있는 **[dependency_map.html](file:///d:/Work/H2_Control_Board/dependency_map.html)** 시각화 뷰어가 제공됩니다.

---

## 🏗️ 1. 전체 3계층 시스템 아키텍처 및 데이터 흐름

```mermaid
graph TD
    subgraph Layer3 [03_Control_UI : 프론트엔드 관제 레이어]
        UI["🖥️ index.html / sequence_manager.html<br>(FHD SCADA 대시보드 & 시퀀스 엔진)"]
        CRC_JS["⚙️ calculateModbusCRC()<br>(표준 다항식 0xA001 검사합)"]
        UI --- CRC_JS
    end

    subgraph Layer2 [Gateway Bridge : 중계 서버 레이어]
        PY["🐍 serial_bridge.py<br>(비동기 WebSocket & Modbus RTU 게이트웨이)"]
        AUTO_POLL["⏱️ Auto-Polling Task<br>(400ms 주기 0x04/0x03 질의)"]
        PY --- AUTO_POLL
    end

    subgraph Layer1 [02_Firmware : dsPIC33CK MCU 펌웨어 레이어]
        MB["📡 modbus.c/.h<br>(Modbus RTU 슬레이브 스택)"]
        MAIN["⚡ main.c<br>(메인 루프 & 200ms 스케줄러)"]
        PIN["📍 pin_map.h<br>(GPIO & SFR 매핑/초기화)"]
        
        UART2["🔌 rs422.c/.h<br>(UART2 Full-Duplex 115200bps)"]
        UART1["🔌 rs485.c/.h<br>(UART1 Half-Duplex 필드 센서)"]
        
        TC["🌡️ thermocouple.c/.h<br>(MAX31856 + ADG706 6개 MUX)"]
        ADC["📊 ads1115.c/.h<br>(ADS1115 4개 16ch ADC)"]
        DAC["🎛️ dac60516.c/.h<br>(DAC60516 12ch 16비트 DAC)"]
        RTC["⏰ rtc.c/.h<br>(DS3231 고정밀 RTC)"]
        ETH["🌐 ethernet.c/.h<br>(듀얼 W5500 이더넷)"]
        FLASH["💾 flash.c/.h<br>(W25Q256 SPI 플래시)"]
        
        MAIN --> MB
        MAIN --> TC & ADC & DAC & RTC & ETH & FLASH & UART1 & UART2
        MB --> UART2
        MB --> DAC
        MB --> PIN
    end

    subgraph Layer0 [Hardware : 물리 하드웨어 및 액추에이터]
        DO_LOADS["⚡ 20채널 DO 디지털 부하<br>(솔레노이드 6종, 모터밸브 8종, 히터 4종, 점화 1종, MC_SW)"]
        AO_DEVICES["🎛️ 11채널 아날로그 출력<br>(에어블로어 3종, 펌프 6종, MFC 2종 0~5V)"]
        SENSORS["🌡️ 물리 센서군<br>(열전대 31ch, 압력/유량 ADC 16ch, DI 알람 16ch)"]
    end

    UI <-->|WebSocket ws://localhost:8888| PY
    PY <-->|이원화 #1: RS-422 Modbus RTU + CRC16| UART2
    PY <-->|이원화 #2: Ethernet LAN (Modbus TCP 502)| ETH
    ETH <-->|Modbus TCP Frame| MB
    PIN --> DO_LOADS
    DAC --> AO_DEVICES
    SENSORS --> TC & ADC & RTC & PIN
```

---

## 📂 2. 모듈별 파일 의존성 및 통신 인터페이스 상세

### 🔌 2.1. 메인 제어 및 하드웨어 매핑 계통
*   **[main.c](file:///d:/Work/H2_Control_Board/02_Firmware/main.c)**: 
    *   **의존성**: `pin_map.h`, `ads1115.h`, `dac60516.h`, `thermocouple.h`, `modbus.h`, `rs422.h`, `rs485.h`, `rtc.h`, `ethernet.h`, `flash.h`.
    *   **역할**: 200ms 주기로 전체 센서 계측 및 Modbus DB 바인딩을 수행하고, 20채널 DO 릴레이 하드웨어 동기화 및 WDT 리셋을 총괄합니다.
*   **[pin_map.h](file:///d:/Work/H2_Control_Board/02_Firmware/pin_map.h)**:
    *   **역할**: dsPIC33CK512MP710 MCU의 모든 GPIO/SFR 비트필드 정의 및 `GPIO_Initialize()` 포트 초기화 제공.

### 📡 2.2. 통신 및 프로토콜 제어 계통
*   **[modbus.c](file:///d:/Work/H2_Control_Board/02_Firmware/modbus.c) / [modbus.h](file:///d:/Work/H2_Control_Board/02_Firmware/modbus.h)**:
    *   **의존성**: `rs422.h`, `dac60516.h`, `ads1115.h`, `thermocouple.h`, `pin_map.h`.
    *   **지원 펑션 코드**:
        *   `0x01`: Read Coils (DO 0~19)
        *   `0x02`: Read Discrete Inputs (DI 0~15)
        *   `0x03`: Read Holding Registers (DAC 0~11, DO 20~39)
        *   `0x04`: Read Input Registers (TC 온도 0~30, ADC 32~47, RTC 50~55)
        *   `0x05`: Write Single Coil (DO 직접 스위칭)
        *   `0x06`: Write Single Register (DAC 전압 및 DO 릴레이 제어)
        *   `0x10`: Write Multiple Registers (다중 DAC/DO 일괄 제어)
    *   **핵심 함수**: `Modbus_SetDO(uint8_t index, bool state)`를 통해 물리 `LAT` 레지스터를 즉시 스위칭.
*   **[rs422.c](file:///d:/Work/H2_Control_Board/02_Firmware/rs422.c) / [rs422.h](file:///d:/Work/H2_Control_Board/02_Firmware/rs422.h)**: 관제 PC 연동용 Full-Duplex UART2 드라이버.
*   **[rs485.c](file:///d:/Work/H2_Control_Board/02_Firmware/rs485.c) / [rs485.h](file:///d:/Work/H2_Control_Board/02_Firmware/rs485.h)**: 필드 인버터 및 순시 유량계 연동용 Half-Duplex UART1 드라이버.

### 🌡️ 2.3. 센서 계측 및 드라이빙 출력 계통 (공유 버스)
*   **[thermocouple.c](file:///d:/Work/H2_Control_Board/02_Firmware/thermocouple.c) & [dac60516.c](file:///d:/Work/H2_Control_Board/02_Firmware/dac60516.c)**:
    *   **공유 버스**: **`SPI1`** 버스 공유.
    *   **간섭 방지**: `TC_SPI_CS`와 `DAC_CS` 핀을 상호 배타적으로 제어하여 버스 충돌 방지.
*   **[ads1115.c](file:///d:/Work/H2_Control_Board/02_Firmware/ads1115.c) & [rtc.c](file:///d:/Work/H2_Control_Board/02_Firmware/rtc.c)**:
    *   **공유 버스**: **`I2C1`** 버스 공유.
    *   **주소 맵**: ADS1115 (0x48~0x4B), DS3231 (0x68).
    *   **신규 API**: `uint16_t ADS1115_ReadChannel(ADS1115_SensorChannel_t channel)` 직접 바인딩 지원.
*   **[ethernet.c](file:///d:/Work/H2_Control_Board/02_Firmware/ethernet.c) / [ethernet.h](file:///d:/Work/H2_Control_Board/02_Firmware/ethernet.h)**:
    *   **공유 버스**: **`SPI2`** 버스 기반 듀얼 W5500 하드웨어 TCP/IP 이더넷 컨트롤러 드라이버.
    *   **역할**: RS-422 시리얼 통신과 함께 **관제 PC(UI/Gateway)와 초고속 LAN 직접 연결을 담당하는 이원화(Redundant) 통신 채널**입니다. Modbus TCP (Port 502) 및 TCP Socket 통신을 통해 펌웨어 내부의 `modbus.c`와 데이터를 양방향 교환합니다.
*   **[flash.c](file:///d:/Work/H2_Control_Board/02_Firmware/flash.c) / [flash.h](file:///d:/Work/H2_Control_Board/02_Firmware/flash.h)**: **`SPI3`** 버스 기반 W25Q256 256Mbit 외장 플래시 메모리 제어.

---

## 🖥️ 3. 관제 UI 및 브릿지 게이트웨이 연동

*   **[index.html](file:///d:/Work/H2_Control_Board/03_Control_UI/index.html)**:
    *   `calculateModbusCRC(bytes)`: 표준 다항식 `0xA001` 기반의 실시간 16비트 검사합 계산기.
    *   `buildModbusRTUFrame()`: 국번, FC, 번지, 데이터에 CRC-16을 부착한 프레임 빌더.
    *   `processSerialPacket()`: MCU의 에코백 수신 응답(0x06, 0x05, 0x04, 0x03, 0x01) 파싱 및 `[HW ACK]` 동기화.
*   **[serial_bridge.py](file:///d:/Work/H2_Control_Board/03_Control_UI/serial_bridge.py)**:
    *   `asyncio` 백그라운드 자동 폴링(Auto-Polling): 400ms 주기로 Input/Holding Regs 요청 송출.
    *   `RESET_ALL_OUTPUTS`: 메인 전원(`DO_MC_SW`) 보존 상태에서 19개 부하 릴레이 및 11개 DAC(0V) 하드웨어 안전 일괄 리셋.

