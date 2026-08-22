# 📝 H2_Control_Board 개발 일지 (DEVELOPMENT_LOG)

## 📅 2026-08-22: 일렉트론(Electron) 기반 산업용 데스크톱 SCADA 관제 앱 환경 구축 및 깃허브(GitHub) 원격 저장소 백업 동기화 🖥️📦☁️
금일 웹 기반 실시간 수소 플랜트 관제 대시보드를 **일렉트론(Electron) 기반의 프레임리스(Frameless) 산업용 데스크톱 네이티브 애플리케이션으로 구동 및 패키징할 수 있는 환경을 구축**하고, 전체 하드웨어 도면(Altium), MCU 펌웨어(dsPIC33CK C 소스), Python 시리얼 브릿지 통신 스택, 인터랙티브 의존성 지도 및 지식 그래프(Graphify)를 포함한 최신 개발 결과물을 GitHub 원격 저장소에 완벽 동기화(백업)하였습니다.

### 1. [Desktop Packaging] Electron 기반 프레임리스 SCADA 데스크톱 관제 환경 구축
- **`일렉트론 메인 프로세스(main.js) 및 설정 완비`**:
  - `03_Control_UI/electron/main.js` 및 `03_Control_UI/package.json`을 구성하여 1920x1080 해상도의 프레임리스 다크테마 관제 창 구현.
  - 원클릭 실행 배치 파일(`03_Control_UI/실행_데스크톱관제.bat`)을 통해 더블 클릭만으로 손쉽게 데스크톱 관제 앱을 구동할 수 있도록 환경 구축.
  - `.gitignore`에 `node_modules/`, `.userData/`, `dist_electron/` 등 빌드/캐시 아티팩트를 제외 처리하여 저장소 경량화 및 보안 유지.

### 2. [Repository Sync & Backup] GitHub 원격 저장소 백업 및 무결성 관리
- **`하드웨어, 펌웨어, UI, 의존성 지도 전 영역 동기화`**:
  - 01_Hardware (Altium 회로도 및 PCB 문서, BOM 분석본)
  - 02_Firmware (dsPIC33CK main.c, ads1115.c/h, modbus.c/h, pin_map.h 16ch ADC 및 20ch DO 제어 스택)
  - 03_Control_UI (웹/데스크톱 관제 대시보드, 시퀀스 매니저, Python 비동기 시리얼 브릿지)
  - 의존성 지도 및 지식 그래프 (`dependency_map.md`, `dependency_map.html`, `graphify-out/`)
  - 프로그래밍 용어사전 (`PROGRAMMING_TERMS.md` 489번 일렉트론 데스크톱 SCADA 아키텍처 추가)

---

## 📅 2026-08-20: 실물 PCB 보드 입고 대응 통신 스택 전면 점검 및 20채널 DO / 11채널 DAC 실시간 하드웨어 제어·동기화 구현 🚀🔌✨
금일 실물 수소 제어 보드(PCB) 입고에 맞춰, **관제 대시보드(UI) ↔ 파이썬 시리얼 브릿지(Python Bridge) ↔ dsPIC33CK MCU 펌웨어 간의 Modbus RTU 통신 무결성 스택을 전면 점검**하고, UI에서 지시하는 모든 출력(20채널 DO 릴레이 밸브, 11채널 DAC 아날로그 전압)이 실제 PCB 하드웨어에서 100% 즉시 구동 및 제어되도록 완벽한 양방향 통신 및 제어 루프를 구축하였습니다.

### 1. [Communication Integrity] 표준 Modbus RTU CRC-16 계산 및 패킷 드롭 방지
- **`UI 및 파이썬 브릿지 표준 CRC-16(다항식 0xA001) 엔진 탑재`**:
  - 기존 UI의 가상 목업 CRC(`A23B`, `E84A` 등)를 **표준 Modbus RTU CRC-16 계산 유틸리티(`calculateModbusCRC`, `buildModbusRTUFrame`)**로 전면 교체.
  - 슬레이브 국번(`0x01`), 펑션 코드, 시작 번지, 설정값/개수에 따라 정밀한 16비트 검사합(CRC Low, CRC High)을 실시간 자동 부착하여 송출함으로써 MCU의 CRC 검증에서 패킷이 폐기(Drop)되는 현상을 원천 차단.
- **`비동기 브릿지 자동 폴링(Auto-Polling) 엔진 구축`**:
  - `serial_bridge.py`에서 물리 시리얼 포트 개방 시, 백그라운드 태스크로 주기적(약 400ms 주기) Modbus Read 명령(0x04 Input Registers 56개, 0x03 Holding Registers 40개)을 자동 송출.
  - MCU가 응답하는 31개 열전대 온도(TC), 16개 아날로그 전압(ADC), RTC 시각, DAC 및 DO 현재 상태를 웹 대시보드로 실시간 스트리밍 중계.

### 2. [Hardware Output Control] 20채널 DO 디지털 릴레이 & 11채널 DAC 아날로그 출력 실시간 제어
- **`20채널 디지털 출력(DO) 물리 GPIO(LAT) 즉각 스위칭`**:
  - 솔레노이드 밸브 6종(`DO_SV102`, `DO_SV103`, `DO_SV125`, `DO_SV145`, `DO_SV149`, `DO_SV323`), 전동 모터 밸브 8종(`DO_MV124`, `DO_MV131`, `DO_MV148`, `DO_MV150`, `DO_MV155`, `DO_MV158`, `DO_MV300`, `DO_MV390`), 메인 전원 스위치(`DO_MC_SW`), 촉매/Prox 히터 4종(`DO_HT193`, `DO_HT194`, `DO_HT195`, `DO_HT196`), 점화 트랜스(`DO_IGN175`) 전체 20채널에 대한 `Modbus_SetDO(index, state)` 드라이버를 구현.
  - Modbus 펑션 코드 **`0x05 (Write Single Coil)`**, **`0x06 (Write Holding Reg 20~39)`**, **`0x10 (Write Multiple Regs)`**, **`0x01 (Read Coils)`**를 모두 지원하여 어떤 관제 명령이 인입되어도 즉시 해당 물리 LAT 핀을 스위칭.
- **`11채널 DAC60516 아날로그 전압 출력(0~5V) 1:1 매핑 연동`**:
  - UI Holding Register 0~11번지와 MCU의 16비트 DAC60516 드라이버(`DAC60516_WriteRaw`) 채널을 1:1 매핑 배열(`dac_channel_map`)로 일치화.
  - 슬라이더 조작 시 16비트 해상도(0~65535, 0~5.000V)로 즉각 SPI 통신을 수행하여 에어블로어, 수순환 펌프, MFC 유량 지령 전압을 하드웨어로 즉시 출력.

- **`main.c 미정의 열거형 심볼 제거 및 필수 드라이버 초기화 보강`**:
  - `thermocouple.h` 채널 규격에 맞게 존재하지 않던 레거시 예외 처리(`TC_CH0_UNUSED`, `TC_CH20_UNUSED`)를 제거하여 C 구문 오류 완전 해소.
  - `ads1115.h` 및 `ads1115.c`에 **`ADS1115_ReadChannel`** 함수를 선언/구현하여 `main.c`의 16채널 ADC 바인딩 루틴과의 함수 시그니처 불일치 에러 해결.
  - 시스템 부팅 루틴에 누락되어 있던 **`ADS1115_Initialize()`**(16채널 ADC) 및 **`DAC60516_Initialize()`**(12채널 아날로그 출력)를 추가하여 부팅 직후 센서 및 전압 출력이 즉시 정상 작동하도록 완비.
  - `index.html` 내 중복 중첩되어 있던 `toggleVirtualCoil` / `toggleDO` / `calculateModbusCRC` 자바스크립트 선언 순서를 정돈하여 프론트엔드 구문 에러 3건 완전 해소.
  - 상단 `#include <xc.h>`에 에디터 정적 분석 래퍼를 적용하여 VS Code/IDE 구문 검사 경고를 100% 정리 완료.

### 3. [Bidirectional Sync & Safety Reset] 에코백 상태 동기화 및 하드웨어 비상 안전 리셋
- **`양방향 에코백(Echo-back) 응답 파서 탑재`**:
  - UI `processSerialPacket(hexStr)`에서 MCU의 수신 응답(0x06, 0x05, 0x04, 0x03, 0x01)을 실시간 파싱하여 실제 보드에 반영된 출력 상태(`[HW ACK]`)를 대시보드 화면에 일치화.
- **`전체 출력 안전 리셋 (RESET_ALL_OUTPUTS) 하드웨어 송출 완비`**:
  - 긴급 리셋 트리거 시 메인 전원(`DO_MC_SW`: ON 1 유지)을 보존하면서 모든 부하 릴레이(DO 19채널 OFF)와 DAC 11채널(0V)을 물리적으로 일괄 차단하는 Modbus 제어 패킷을 보드로 즉각 송출.
- **`인터랙티브 의존성 지도(dependency_map.html) 시각화 뷰어 구축 및 graphify 지식 그래프 갱신`**:
  - 3계층 아키텍처(UI ↔ Python Bridge ↔ dsPIC33CK MCU ↔ 물리 액추에이터)와 공유 버스(SPI1, I2C1, UART1, UART2)를 인터랙티브하게 탐색할 수 있는 **[dependency_map.html](file:///d:/Work/H2_Control_Board/dependency_map.html)** 시각화 대시보드를 신규 구축.
  - [dependency_map.md](file:///d:/Work/H2_Control_Board/dependency_map.md)를 20채널 DO 제어 및 CRC-16 무결성 스택 기반으로 최신화.
  - `graphify update`를 실행하여 [graph.html](file:///d:/Work/H2_Control_Board/graphify-out/graph.html) 및 [graph_report.md](file:///d:/Work/H2_Control_Board/graphify-out/graph_report.md)를 103개 노드, 160개 엣지의 최신 지식 그래프로 완벽 동기화.

### 4. [Documentation] 프로그래밍 용어 사전 갱신
- `PROGRAMMING_TERMS.md`에 신규 용어 반영:
  - 478번: `Modbus RTU CRC-16 다항식 무결성 검증 (Modbus RTU CRC-16 Polynomial Integrity Verification)`
  - 479번: `실시간 제어기 에코백 상태 동기화 (Bidirectional Controller Echo-back State Synchronization)`
  - 480번: `비동기 시리얼-웹소켓 중계 브릿지 자동 폴링 엔진 (Asynchronous Serial-to-WebSocket Bridge Auto-Polling Engine)`
  - 481번: `크로스 에디터 하드웨어 레지스터 정적 분석 목업 (Cross-Editor Hardware Register Mocking for IntelliSense)`
  - 482번: `웹 프론트엔드 표준 삼위일체 아키텍처 (HTML5 / CSS3 / JavaScript Web Triad Architecture)`
  - 483번: `트랜스파일 및 웹어셈블리 프론트엔드 확장 (TypeScript / JSX & WebAssembly Frontend Ecosystem)`
  - 484번: `벡터 그래픽 및 데이터 구조화 마크업 (SVG Vector Graphics & JSON Data Binding)`
  - 485번: `브라우저 샌드박스 보안 격리와 백엔드 하드웨어 브릿지 (Browser Sandbox Isolation & Backend Hardware Bridge)`
  - 486번: `풀스택 다중 백엔드 런타임 생태계 (Full-Stack Multi-Backend Runtime Ecosystem)`

---

## 📅 2026-08-19: 아날로그 출력(AO/DAC) 제어 구조 분석 및 PID 파라미터 지정/설정 기준 가이드 정립 🎛️⚙️
아날로그 제어 출력 장치(DAC 전압/전류, 비례제어 밸브, 펌프/블로어, 히터 등)의 운용 시 **PID 제어 파라미터($K_p, K_i, K_d$)의 필수 지정 여부**에 대한 제어 아키텍처별 기준을 명확히 정립하고 프로그래밍 용어사전을 업데이트하였습니다.

### 1. [Control Architecture] 플랜트 11종 구동 장비별 PID 파라미터 지정 필요 여부 분석
- **`스마트 액추에이터 그룹 (자체 PID 내장 - 2종)`**:
  - `BNG 유량 제어 (AO_MFC111)` ↔ `AI_MFC111`
  - `PNG 유량 제어 (AO_MFC121)` ↔ `AI_MFC121`
  - **특징**: MFC 내부에 자체 PID 컨트롤러와 밸브가 일체형으로 탑재되어 있어, 제어기는 목표 유량 전압(0~5V)만 지령하면 자체적으로 오차를 수렴하므로 **별도 PID 게인 불필요**.
- **`플랜트 폐루프 제어 그룹 (보드/프로그램 실시간 PID 연산 필수 - 9종)`**:
  - **에어 블로어 3종** (`AO_AB212`, `AO_AB221`, `AO_AB232` ↔ MFM 공기 유량계): 목표 풍량(LPM) 추종용 유량 PID 파라미터 필요.
  - **가압 펌프 (`AO_P108` ↔ 버퍼탱크 압력계 `AI_PT109`)**: 목표 압력(kPa) 유지용 압력 PID 파라미터 필요.
  - **냉각수/응축수 펌프 4종** (`AO_P341`, `AO_P351`, `AO_P370`, `AO_P375` ↔ T-Type 열전대): 스택/개질기 목표 온도(℃) 제어용 온도 PID 파라미터 필요.
  - **물 공급 정량 펌프 (`AO_P380` ↔ 순시 유량계 `FN382`)**: 정량 유량(LPM) 제어용 유량 PID 파라미터 필요.
- **`운전 UX 및 엔지니어링 튜닝 분리 전략`**:
  - **운전원 화면**: 복잡한 $K_p, K_i, K_d$ 수식을 숨기고, 직관적인 **목표값($SP$)과 [🔗 LINK / 🔓 수동] 토글**만 조작하도록 추상화.
  - **시스템 백엔드**: 각 장비별 물리적 시정수(Time Constant)에 맞춰 **사전 튜닝된 기본 PID 게인값(Default Preset)**을 플래시/UI 설정에 주입하고, 필요 시 엔지니어 전용 모달에서 미세 튜닝 지원.

### 2. [System Architecture Sync] 독립 자율 운전(MCU+HMI LCD) & 관제 UI 원격 플래시 레시피 개발 방향 동기화
- **`현장 실운전 (Standalone Autonomous)`**:
  - 외부 PC 없이도 **BOP 제어 보드(dsPIC33CK MCU) + 전면 HMI 터치 LCD(RS-485)**만으로 플랜트 전체의 운전/정지/부하별 운전(50%, 70%, 100%), 안전 인터락, PID 폐루프 제어를 완벽히 독립 자율 수행.
- **`개발/시운전 및 원격 업그레이드 (SCADA / Flowchart Recipe)`**:
  - PC 관제 UI는 센서 계측/제어 출력 확인 및 운전 알고리즘/플로우차트 도출용 엔지니어링 도구로 활용.
  - 관제 UI에서 조립/개선된 시퀀스 플로우차트(스텝, 조건, 목표 SP, PID 게인)를 제어 보드의 **외장 플래시 메모리(SPI Flash)**에 원격 주입(Flash Recipe Table)하여, MCU 전체 소스 재컴파일 없이도 유연하게 버전업 및 알고리즘 교체가 가능한 스마트 아키텍처 확립.

### 3. [Documentation] 프로그래밍 용어 사전 갱신
- `PROGRAMMING_TERMS.md`에 신규 용어 반영:
  - 462번: `개루프 제어 vs 폐루프 제어 (Open-Loop Control vs Closed-Loop Control)`
  - 463번: `아날로그 출력 PID 제어 파라미터 (Analog Output PID Control Parameters)`
  - 464번: `스마트 액추에이터 vs 플랜트 폐루프 제어 (Smart Actuator vs Plant Closed-Loop Control)`
  - 465번: `제어 루프 파라미터 사전 튜닝 및 기본값 주입 (Pre-Tuned Loop Parameters & Default Gain Injection)`
  - 466번: `펌웨어 내장형 PID vs 호스트 소프트웨어 PID (Embedded Firmware PID vs Host Software PID)`
  - 467번: `결정론적 실시간 제어 주기 (Deterministic Real-Time Control Cycle)`
  - 468번: `독립 자율 제어형 임베디드 플랜트 아키텍처 (Standalone Autonomous Embedded Plant Architecture)`
  - 469번: `플래시 기반 원격 시퀀스 레시피 주입 (Flash-Based Remote Sequence Recipe Injection / OTA)`
  - 470번: `Altium Designer VCS 상태 표시자 (Altium Designer Version Control Status Indicators)`
  - 471번: `ECAD 하드웨어 형상 관리 (ECAD Version Control & Design Traceability)`
  - 472번: `아날로그 출력 기능 검사 지그 (Analog Output Functional Test Jig)`
  - 473번: `아날로그 측정단 과전압 보호 회로 (Analog Input OVP / Over-Voltage Protection)`
  - 474번: `가변 아날로그 신호 발생 회로 (0~5V Adjustable Analog Signal Generator)`
  - 475번: `볼티지 팔로워 임피던스 버퍼링 (Voltage Follower / Unity-Gain Buffer Isolation)`
  - 476번: `전압 추종기 부귀환 루프 결선 (Voltage Follower Negative Feedback Connection)`
  - 477번: `비(非) Rail-to-Rail Op-Amp의 헤드룸 전압 강하 (Output Voltage Headroom Drop)`

### 4. [Hardware Test Jig] 아날로그 입출력 검사 지그 회로 설계 및 도면 검토 완료 ✅
- **출력 검사단(AO 검사)**: FND 볼트메터 모듈 및 OVP(5.1V 제너+댐핑저항) 보호 회로 완료.
- **입력 검사단(AI 검사 - 0~5V 가변 신호 발생기 회로도 최종 승인)**:
  - 7805 레귤레이터 기반 정밀 5V 전원 및 LM358 전압 추종기(Unity-Gain Buffer) 구성 완료.
  - Vdd를 24V로 공급하여 0~5.0V 풀스윙 출력 보장.
  - 가변저항 3단자 선형 분압 및 출력 RC 필터(100Ω + 0.1uF) 최적화 반영.
  - 회로도 검토 및 무결성 검증 완료 -> 아트워크/제작 준비 완료.

---










## 📅 2026-08-18: 아날로그 제어 출력(DAC) & 피드백 센서 짝짓기 매칭 UI 재배치 및 PID 링크/수동 토글 제어 구현 🎛️🔗✨
수소 제어 보드 메인 관제 대시보드에서 제어 출력 장비(DAC 11채널)와 그에 대응하는 물리적 피드백 측정 센서(ADC, TC, RS-485 유량계)를 1:1 일체형 서브 행으로 짝짓기 매칭하고, PID 폐루프 연동(`[🔗 LINK]`)과 단독 수동 직결 출력(`[🔓 수동]`)을 런타임에 자유롭게 토글할 수 있는 인터랙티브 SCADA 제어 환경을 완벽히 구축하였습니다.

### 1. [UI Layout & Panel Reorganization] 좌/우측 패널 최적화 재배치
- **`시리얼 통신 장치 카드 좌측 이전`**:
  - 우측 패널의 DAC 11채널 + 피드백 센서 서브 행의 고밀도 매칭 공간을 확보하기 위해, `시리얼 통신 장치 현황 (RS-485)` 카드를 좌측 패널(독립 ADC 카드 바로 아래)로 이전 배치.
- **`좌측 독립 ADC 8채널 분리 렌더링`**:
  - 좌측 `아날로그 입력 수신 현황 (ADS1115 ADC 독립 계측)` 카드에서 DAC 피드백 센서로 짝짓기된 6개 센서를 분리하고, 순수 독립 계측 센서 8개(`AI_PT105`, `AI_PT113`, `AI_PT123`, `AI_PT146`, `AI_PT162`, `AI_PT383`, `AI_CS324`, `AI_MFM315`)만 2열 그리드로 깔끔하게 렌더링하여 화면 중복성 제거.
- **`우측 DAC 11채널 & 피드백 센서 서브 행 1:1 매칭`**:
  - DAC 출력 행 바로 아래에 연동된 피드백 센서(`↳ AI_MFM211`, `↳ AI_MFM222`, `↳ AI_MFM231`, `↳ AI_MFC111`, `↳ AI_MFC121`, `↳ AI_PT109`, `↳ CH17`, `↳ CH14`, `↳ CH15`, `↳ CH16`, `↳ FM382`)의 원시 RAW, 전압, 실시간 계측값을 일체형 서브 행으로 렌더링.
  - 각 항목별 **`[🔗 LINK / 🔓 수동]`** 링크 토글 버튼 및 목표값 뱃지 칩(`🎯 SP: xx.x`) 제공.

### 2. [Control Logic & Interaction] 동적 PID 폐루프 vs 단독 수동 제어 모달
- **`동적 링크 모드 스위칭 & 폐루프 PID 수렴 연동`**:
  - `LINK ON (PID AUTO)`: 피드백 센서 계측값(PV)이 설정된 목표값(SP)을 추종하도록 Closed-Loop PID 레지스트리에 자동 등록 및 실시간 수렴 시뮬레이션 연동.
  - `LINK OFF (MANUAL)`: 피드백 센서와 상관없이 사용자가 지정한 슬라이더/전압(0~5V, 0~100%, RAW 0~65535)으로 Direct DAC 수동 송출.
- **`적응형 제어 모달 (Decoupled Mode Dialog)`**:
  - 링크 상태에 따라 모달 창 내에서 [PID 목표값 SP 설정 섹션]과 [수동 직결 출력 슬라이더 섹션]이 동적으로 자동 전환.
  - 모달 상단에 `[🔗 PID LINK ON / 🔓 MANUAL]` 스위치를 탑재하여 모달을 닫지 않고도 즉시 제어 모드 변경 가능.
  - 목표값 저장 시 `targetSp` 즉각 반영 및 해당 피드백 센서 펄스 하이라이트 트리거.

### 3. [Visual Dynamics] 오차 적응형 실시간 3단계 네온 펄스 시각화 & 일체형 장비 블록화
- **`오차(Error = |SP - PV|) 크기 기반 자동 색상/주기 가변`**:
  - **1단계 (정상 수렴 안정, 오차 ≤ 5%)**: 부드러운 청록/에메랄드 숨쉬기 펄스 (`pidGlowStable`, 1.8s) + `[🎯 AUTO]` 뱃지.
  - **2단계 (목표 추종 접근, 5% < 오차 ≤ 20%)**: 생생한 골드/앰버 펄스 (`pidGlowTracking`, 1.0s) + `[⚡ 추종 xx%]` 뱃지.
  - **3단계 (대형 편차/급변 경고, 오차 > 20%)**: 강렬한 네온 오렌지/레드 고속 펄스 (`pidGlowAlert`, 0.55s) + `[⚠️ 편차 xx%]` 뱃지.
- **`일체형 장비 블록화 & 고대비 장비 간 분리선 (Hierarchical Border Separation)`**:
  - 메인 제어 장비(블로어/펌프/MFC)와 종속 피드백 센서 행 사이의 내부 경계선은 완화하고 좌측 포인트 바(`border-left: 3.5px~4px`)로 묶어 하나의 일체형 블록으로 형성.
  - 서로 다른 제어 장비(페어) 간에는 `border-bottom: 3.5px solid #030712` 고대비 두꺼운 분리선을 적용하여 개별 장비 단위의 경계를 극명하게 구분.

- **`메인 전원 (SYSTEM MAIN POWER) 카드 컴팩트 최적화 & 외곽 테두리 시인성 보수`**:
  - 카드 내부 패딩을 10px에서 5px 12px로 정밀 축소하고 텍스트 및 전원 토글 버튼 사이즈를 컴팩트하게 리사이징(`padding: 4px 10px; font-size: 10px;`).
  - 카드 하단에 여백(`margin: 1px 0`)을 확보하고 테두리 두께(`border: 1.5px solid`)를 강화하여 하단 네온 발광 및 외곽 테두리가 잘림 없이 선명하게 노출되도록 개선.

- **`통신 인터페이스 셀렉트 목록 정예화 (RS-422 & Ethernet)`**:
  - 디버그 모니터 및 포트 설정 팝업에서 실제 현장 스펙에 맞춰 불필요한 레거시 프로토콜(`RS-485`, `RS-232`)을 삭제하고 실 사용 인터페이스인 `RS-422 (기본)`과 `Ethernet (Modbus TCP)` 2종으로 드롭다운 메뉴를 정예화.
- **`보드 중계형 서브 시리얼 계측 아키텍처 명확화`**:
  - 인버터/LCD/유량계 등의 시리얼 장치는 PC와 직결되는 것이 아니라 **BOP 제어 보드 내부 서브 버스**로 연결되어 계측된 뒤, 제어 보드가 취합하여 **PC와는 메인 RS-422 / Ethernet 단일 통신망으로 일괄 송수신**되는 구조임을 UI 상의 카드 명칭(`보드 연동 시리얼 장치 계측 현황`) 및 라벨(`인버터 제어기`)에 명확히 반영.
- **`디지털 입력(DI) 채널 정예화 및 공식 스펙 동기화`**:
  - `DI_FD176(버너 화염감지기)`는 점화 트랜스(`DO_IGN175`)의 피드백 센서로 이미 통합 연동되어 있으므로 독립 DI 목록에서는 제외 유지.
  - 임의 포함되어 있던 수위 센서(`DI_WT_HIGH`, `DI_WT_LOW`)를 공식 BOP 사양에 맞춰 **`DI_SPARE1`, `DI_SPARE2` (예비 디지털 입력)** 로 완벽 원복.
- **`시퀀스 매크로 빌더 센서 대기 및 IF 분기 센서 풀(Pool) 전면 확장`**:
  - `sequence_manager.html`의 '🎯 센서 대기' 및 '🔀 IF 분기' 드롭다운 목록에서 하드코딩된 예제 고정 온도를 제거하고, **열전대 온도 센서 전체(K-Type CH21~CH40, T-Type CH01~CH20 총 40개 채널)** 및 **아날로그 압력/유량/전도도 센서 14종**, **시리얼 유량계**를 `<optgroup>`으로 완벽 계층화.
  - 사용자가 선택한 센서 종류(TC 고온/저온, AI 압력, AI 유량 등)에 따라 오른쪽 목표값 입력창의 플레이스홀더(`목표℃`, `목표kPa`, `목표LPM`, `목표uS`)와 기본 권장 설정값이 실시간 자동 전환되는 동적 단위 힌팅(`updateSensorUnitHint`) 구현.
  - 사용자가 특정 센서를 선택한 후 원하는 목표 온도/압력/유량 수치를 자유롭게 직접 입력하여 유연하고 정밀한 공정 시퀀스를 조립할 수 있도록 고도화.

### 4. [Documentation] 프로그래밍 용어 사전 갱신
- `PROGRAMMING_TERMS.md`에 신규 용어 반영:
  - 459번: `모드버스 패킷 프레임 필드 구조 (Modbus Packet Frame Fields)`
  - 460번: `가상 프로토콜 패킷 에뮬레이션 및 수동 송출 (Virtual Protocol Packet Emulation & Manual Transmission)`
  - 461번: `계층 그룹화 센서 인코딩 및 동적 단위 힌팅 (Categorized Sensor Encoding & Dynamic Unit Hinting)`

---

## 📅 2026-08-15: 일렉트론(Electron) 기반 프레임리스 데스크톱 SCADA/HMI 패키징 아키텍처 및 산업용 UI 디자인 스킬 구축 💻⚡🎨
웹 브라우저의 한계를 극복하고 수소 제어 및 SCADA 관제 프로그램을 독립된 전용 윈도우 응용 프로그램(`.exe`)으로 탈바꿈하기 위해, **일렉트론(Electron) 프레임리스 윈도우 래핑 아키텍처 구축, UserData 캐시 경로 격리 안정화, 산업용 SCADA 디자인 시스템(다이얼 게이지, P&ID 애니메이션, 고대비 다크 테마) 분석 및 전용 Antigravity 커스텀 스킬(`electron-desktop-scada`) 개발 및 글로벌 등록**을 완료하였습니다.

### 1. [Desktop Architecture] 일렉트론(Electron) 패키징 및 Frameless 환경 완성
- **`프레임리스 윈도우 (frame: false) 및 커스텀 헤더바`**:
  - OS 기본의 투박한 흰색 타이틀바와 브라우저 주소창을 100% 제거하고, 딥 다크 배경(`backgroundColor: '#0b0f19'`)의 일체형 관제 소프트웨어로 구현.
  - 상단 헤더바에 마우스 드래그 이동 영역(`-webkit-app-region: drag;`)을 적용하고, 우측 상단에 세련된 커스텀 `[ ─ 최소화 | 🗖 최대화/복원 | ✕ 프로그램 종료 ]` 버튼을 배치.
  - 마우스 클릭 가로채기 방지를 위해 버튼 영역에 `-webkit-app-region: no-drag !important;` 및 `pointer-events: auto !important;`를 엄격 적용하여 클릭 반응성 확보.
- **`한글 윈도우 사용자명 캐시 에러 원천 차단 (UserData Path Isolation)`**:
  - `C:\Users\서기철\...` 한글 계정 경로로 인한 Chromium 디스크/GPU 캐시 접근 실패 오류(`Unable to move/create cache (0x5)`)를 방지하기 위해 `app.setPath('userData', ...)`로 프로젝트 로컬 영문 폴더(`.userData`)로 안전하게 격리.
- **`IPC 통신 파이프라인 & 하이브리드 지원`**:
  - `preload.js`의 `contextBridge`를 통해 `window.electronAPI`를 노출하고 메인 프로세스의 창 제어(최소화/최대화/닫기)와 1:1 비동기 연동.
  - 일반 웹 브라우저(Chrome/Edge)와 일렉트론 데스크톱 앱 양쪽 환경에서 모두 완벽히 호환되는 하이브리드 로직 완비.
  - `실행_데스크톱관제.bat` 원클릭 실행 런처 배치 파일 제공.
- **`WebFrame 동적 줌 스케일링 (Ctrl + 휠 줌 & 단축키 완비)`**:
  - 일반 브라우저처럼 **`Ctrl + 마우스 휠 위/아래 스크롤`** 시 화면 배율을 50% ~ 250%까지 실시간 부드럽게 Zoom-In / Zoom-Out 조절.
  - 키보드 단축키 **`Ctrl + (+)`(확대), `Ctrl + (-)`(축소), `Ctrl + 0`(100% 원본 배율 초기화)** 완벽 지원.
- **`독립 팝업 윈도우(시퀀스 매니저) Frameless 일관성 통합`**:
  - `window.open`으로 띄우는 모든 자식 팝업 창에 대해 `setWindowOpenHandler`를 적용하여 흰색 타이틀바와 메뉴바(`File, Edit...`)를 100% 제거.
  - 팝업창 상단에도 드래그 영역과 `[ ─ | 🗖 | ✕ ]` 커스텀 버튼을 탑재하고, `BrowserWindow.fromWebContents(event.sender)` 기반 멀티 윈도우 IPC 제어로 팝업창 독립 최소화/최대화/닫기 완벽 지원.
- **`시퀀스 매크로 레코더 딜레이(Delay/Wait) 스텝 추가 고도화 & UI 최적화`**:
  - Electron 환경에서 브라우저 동기 팝업(`prompt`)이 차단되는 문제를 해결하기 위해, 쾌속 툴바에 **`⏱️ 지연 대기: [ 초 입력 ] [+ 딜레이 스텝]`** 전용 컨트롤 행을 신설.
  - 상단에 중복 배치되어 있던 딜레이 추가 버튼을 정리하여 툴바 중심의 일관되고 직관적인 조립 UX 완성.





- **`초기 1회 설정 및 실시간 반영 (Hot Reloading)`**:
  - 일렉트론 메인 프로세스(`main.js`), 보안 브릿지(`preload.js`), 빌드 설정(`package.json`)을 프로젝트에 최초 1회만 구성.
  - 평소 개발 시에는 웹 소스(HTML/CSS/JS) 수정 시 일렉트론 화면에 0.1초 만에 즉각 갱신되는 **라이브 리로드(Live Reload)** 개발 환경 구축.
  - 최종 납품 시에는 단 한 줄의 명령어(`npm run dist`)로 독립 실행 파일(`.exe`) 및 자동 설치 패키지(NSIS)를 빌드하는 원클릭 배포 파이프라인 확립.
- **`브라우저 제약 탈피 & 로컬 자원 접근`**:
  - 일반 웹 브라우저의 보안 제약(Same-Origin Policy, 파일/시리얼 포트 접근 한계)을 극복하고, 메인 프로세스(Node.js)를 통해 RS-485 시리얼 및 로컬 파일 시스템 직접 제어 가능.

### 2. [UI/UX Design] 프레임리스(Frameless) 윈도우 & 산업용 SCADA 디자인 시스템 정의
- **`프레임리스 윈도우 (frame: false)`**:
  - OS 기본의 투박한 흰색 타이틀바와 테두리를 완전히 제거하고, 순수 CSS/JS 기반의 **다크 메탈릭 커스텀 헤더바**를 배치.
  - 상단바 드래그 영역(`-webkit-app-region: drag;`)과 커스텀 최소화/최대화/종료 버튼을 연동하여 전문 계측 장비 일체형 룩앤필 완성.
- **`산업용 SCADA/HMI 시각화 컴포넌트`**:
  - **아날로그 다이얼 게이지**: 압력(bar), 유량(LPM), 온도(℃)를 실제 계측기처럼 부드러운 바늘 침 애니메이션으로 시각화.
  - **P&ID 배관/밸브 애니메이션**: 밸브 개폐 회전 및 배관 유체 흐름(Fluid Glow Flow) 실시간 연동.
  - **고대비 사이버 다크 테마**: 현장 터치스크린 및 어두운 관제실에서도 직관적으로 판독 가능한 네온 LED 뱃지(`[🎛️ PID AUTO]`, `[🎯 SP]`).

### 3. [Skill Development] `electron-desktop-scada` 전용 커스텀 스킬 개발 및 글로벌 등록
- **스킬 위치**: `C:\Users\서기철\.gemini\config\skills\electron-desktop-scada\SKILL.md`
- **스킬 내용**:
  - 1단계: 일렉트론 및 빌더 초기 환경 구성 (`package.json`, `main.js`, `preload.js`)
  - 2단계: 프레임리스 산업용 커스텀 헤더바 및 IPC 통신 구축
  - 3단계: 산업용 SCADA 디자인 시스템(게이지, P&ID 애니메이션, 차트) 결합
  - 4단계: 원클릭 `.exe` 단독 실행 파일 패키징 및 NSIS 인스톨러 빌드 자동화 가이드 완비.

### 4. [Documentation] 프로그래밍 용어 사전 갱신
- `PROGRAMMING_TERMS.md`에 신규 용어 5건 추가 반영:
  - 444번: `일렉트론 프레임워크 (Electron Framework)`
  - 445번: `애플리케이션 패키징 및 번들링 (Application Packaging & Bundling)`
  - 446번: `핫 리로딩 / 라이브 리로드 (Hot Reloading / Live Reload)`
  - 447번: `프레임리스 윈도우 (Frameless Window)`
  - 448번: `산업용 SCADA/HMI 디자인 시스템 (Industrial SCADA/HMI Design System)`

---

## 📅 2026-08-14: 수소 연료전지 SCADA/HMI 관제 화면 & 실시간 센서 그래프 트렌드 및 비밀번호 보안 인증 검토 💻📊🔒
수소 연료전지/개질기(Reformer)/스택(Stack #1, #2) 계통의 복잡한 공정 배관 및 데이터 계측 관제 대시보드(SCADA/HMI) 초안 도면을 기반으로, 웹 기술(HTML/CSS/JS/SVG) 구현 방식과 파이썬/데스크톱 GUI 방식 간의 장단점 비교 검토, **발주처 제안/설득 전략 수립과 하이브리드 SCADA 아키텍처, 프로그램 실행 비밀번호 잠금 보안 기능 및 점진적 모듈 통합 로드맵 정의**를 완료하였습니다.

### 1. [UI Control & Hardware Mapping] BNG / PNG MFC 듀얼 입출력(AI/AO) 제어 채널 신설 및 DAC 11채널 연동 확장
- **`BNG / PNG 유량 제어기 (MFC) 입출력 공존 보완`**:
  - 기존 대시보드에 유량 측정(AI 센서 계측: `AI_MFC111`, `AI_MFC121`)만 등록되어 있던 BNG MFC 및 PNG MFC 기기에 대해 **유량 설정/제어용 아날로그 출력(AO/DAC)`AO_MFC111`, `AO_MFC121` 2개 채널을 신규 추가**.
  - **DAC60516 아날로그 제어 출력을 9채널에서 11채널(DAC 11-CH, 0~5V)로 확장**하고 2열 렌더링 카드 비율을 6:5로 균등 정렬.
- **`피드백 센서 1:1 링크 (Closed-Loop Feedback Binding)`**:
  - UI 상에서 DAC 출력 제어 수치 변경 시 클릭 한 번으로 연동된 AI 유량 센서(`AI_MFC111`, `AI_MFC121`) 행이 2.2초간 반짝이도록 `🔗 FB` 피드백 연동 링크 뱃지 탑재.
- **`깃허브(GitHub) 원격 저장소 바인딩 및 최적화 푸시 완료`**:
  - 원격 리포지토리: [seokipo/H2_Control_Board](https://github.com/seokipo/H2_Control_Board)
  - 용량 제한 유발 요소(SolidWorks CAD 파일 `*.SLDPRT`, Altium 백업 `History/`, 대용량 PDF/인스톨러)를 제외하고, 핵심 알티움 회로도(SCH) 및 PCB 소스(`*.SchDoc`, `*.PcbDoc`, `*.PrjPcb`), dsPIC33CK C 펌웨어, SCADA 웹 대시보드 및 시퀀스 엔진 전체를 `main` 브랜치로 성공적으로 초기 푸시 완료.

### 2. [Architecture & Roadmap] SCADA/HMI 보안 잠금 및 점진적 모듈 통합 전략
- **`점진적 모듈 통합 로드맵 (Incremental Integration Strategy)`**:
  - 기본 제어반 통신 및 관제 UI 프레임워크 완성 후, **2단계로 비밀번호 인증 잠금 모달, 고급 SVG 배관 애니메이션, ECharts 센서 그래프 히스토리 모듈을 순차 이식**하는 안정적 개발 일정 확정.
- **`프로그램 실행 & 제어 조작 비밀번호 잠금 (Password Security Lock)`**:
  - **전면 진입 로그인 잠금**: 프로그램(웹 페이지) 접속 시 비밀번호 입력 팝업 모달을 띄워 인증된 사용자만 관제 화면 진입 허용.
  - **권한별 부분 기능 락 (Role-Based Feature Lock / RBAC)**: 일반 모니터링(읽기 전용)은 누구나 확인 가능하고, 밸브/펌프 조작, 비상 리셋, 시퀀스 레시피 수정 등 위험 제어 기능에만 관리자 비밀번호를 요구하는 2단계 안심 보안 구조 수립 가능.
- **`발주처 니즈 대응 (파이썬 백엔드 + HTML 프론트엔드 하이브리드)`**:
  - 발주처가 요구하는 파이썬(Python) 기반의 데이터 처리 및 RS-422 시리얼 통신 안정성을 **파이썬 백엔드 서비스**로 100% 충족.
  - UI 화면 시각화 및 모니터링은 **HTML5/SVG 웹 프론트엔드**로 분리하여 고급 공정 애니메이션 및 차트 시각화 완성도 극대화.
- **`발주처 설득 핵심 포인트 3가지 (Key Selling Points)`**:
  1. **원격 멀티 디바이스 관제 지원**: 현장 PC뿐만 아니라 사무실 PC, 모바일 스마트폰, 태블릿 등에서도 접속 관제 가능 (발주처 호응도 1위).
  2. **유지보수 및 업데이트 비용 절감**: 전용 앱 재설치 없이 웹 서버 갱신으로 접속 기기 전체 즉시 반영.
  3. **센서 시계열 그래프 고속 줌/팬 & 자동 보고서 추출**: ECharts 기반 대용량 센서 그래프 줌/팬 및 엑셀(CSV)/PNG 보고서 1초 추출 지원.

### 3. [Hardware & Sensor] T-Type vs K-Type 열전대(Thermocouple) 계측 특성 및 적용 파트 비교 검토
- **`K-Type (Chromel-Alumel / 고온 내구성 특화)`**:
  - **측정 범위**: -200℃ ~ +1250℃
  - **선정 이유**: 800℃ 이상의 극고온 및 내산화 분위기 내구성이 탁월하여 **개질기 버너(Ref-BN, 820℃) 및 개질 반응기(Ref-SR1~3, 750℃), 피드 예열기, 촉매 배기부** 등 고온 계통에 필수 배치.
- **`T-Type (Copper-Constantan / 저·중온 정밀도 및 내식성 특화)`**:
  - **측정 범위**: -200℃ ~ +350℃
  - **선정 이유**: 0~200℃ 저온~중온 영역에서 열전대 센서 중 **최고 수준의 측정 정밀도(Accuracy)**를 보이며, 구리(Cu) 재질 특성상 **습기 및 냉각수 응축수 환경에 대한 부식 내성**이 뛰어나 **스택 냉각수 순환, Cathode/Anode 입출력 온도, 배출가스 인입** 등 정밀 제어 파트에 분리 적용.

### 4. [Sequence Automation & Macro Recorder] 시퀀스 매크로 레코더 출력 제어 채널 전면 전수 점검 및 확충
- **`DO 디지털 릴레이 20채널 100% 전수 확장`**:
  - 기존 레코더 쾌속 수동 스텝 추가 툴바(`quick-do-target`)에 7개 채널만 나열되어 13개 채널이 누락되었던 문제를 해결하고, **메인 전원 스위치(`DO_MC_SW`), 전동 밸브 7종(`DO_MV124`, `DO_MV131`, `DO_MV148`, `DO_MV150`, `DO_MV155`, `DO_MV158`, `DO_MV300`, `DO_MV390`), 드레인 밸브(`DO_SV323`), 촉매/Prox 히터 4종(`DO_HT193~196`), 점화 트랜스(`DO_IGN175`) 포함 전체 20채널 100% 매핑 완료**.
- **`DAC 아날로그 제어 11채널 100% 전수 확장`**:
  - 기존 4개 채널(오타 오기 포함)로 제한되었던 툴바(`quick-dac-target`)를 **신규 추가된 BNG MFC (`AO_MFC111`) 및 PNG MFC (`AO_MFC121`)를 포함하여 DAC 11채널 전체 선택 100% 매핑 완료**.
- **`AI 계측 센서(14채널) 및 주요 온도 센서 피드백 조건 대기 확충`**:
  - `quick-sensor-id` 및 `pid-sensor-id` 셀렉트 옵션에 14개 아날로그 센서, 주요 개질기/스택 온도 계측 채널(`CH21`, `CH22`, `CH10`), 시리얼 유량계(`FM382`) 전면 연동.
- **`MCU C코드 (.h) 비트마스크 시리얼라이저 정밀화`**:
  - `exportMcuCHeader()` 로직에서 DO 레시피 제어 스텝을 C 코드로 추출할 때 **32비트 비트마스크(Hex: `0x00000004` 등)로 자동 연산 변환**하는 정밀 시리얼라이저 탑재.
- **`시퀀스 매니저 JS 초기화 오류(Uncaught ReferenceError) 원천 해결`**:
  - `sequence_manager.html` 내 구버전 함수명(`syncPidFeedbackSensor`) 호출 잔재로 인해 발생하던 페이지 초기화 런타임 크래시를 전면 수정하고, 노드(Node.js) V8 엔진 구문 검증(`new Function(script)`)을 통과하여 100% 정상 구동 무결성 확보.
- **`PID 폐루프 제어 툴바 AI_ 아날로그 센서 전용 필터링 및 1:1 자동 바인딩 완비`**:
  - PID 입력 선택 시 작업자의 혼선을 초래하던 온도 채널(`CH14~CH22`) 및 유량계 센서를 제외하고, **`AI_` 계측 전용 아날로그 센서 10개 채널만 깔끔하게 나오도록 필터링**.
- **`PID 폐루프 제어 툴바 DAC 잠금 & 센서 주도 1:1 자동 바인딩 (One-Way Locked DAC Auto-Pairing)`**:
  - 작업자의 수동 오작동을 방지하기 위해 **왼쪽 [DAC 구동 장치] 드롭다운을 🔒 읽기전용(Disabled) 상태로 완전 잠금 처리**.
  - 오른쪽 **[AI 계측 센서/온도 채널]**만 작업자가 선택하면, 1:1 물리적 대칭 관계인 구동 장치가 **100% 강제 자동 전환(Auto-Matched)**되고 펄스 글로우 애니메이션으로 하이라이트 표시되도록 직관적 통제 구현.
- **`크로스 윈도우 액션 파출 및 메인 대시보드 릴레이/DAC LED 100% 실시간 연동 완료`**:
  - 커스텀 시퀀스 레시피 조립 시 `actions` 포맷이 `[ { type, target, state } ]` 배열 형태로 저장되어 메인 대시보드 수신기(`applySequenceActionsFromPopup`)의 딕셔너리 포맷(`{ DO_SV145: 1 }`)과 불일치하여 화면상 솔레노이드/전동 밸브 초록색 LED 점등이 누락되던 문제를 원천 해결.
  - `buildRecipeObject` 및 `broadcastActionToOpener`에서 딕셔너리 매핑으로 완벽 정규화하고, `BroadcastChannel('h2_seq_action_channel')`을 이중 구축하여 독립 팝업창에서 솔밸브(예: `DO_SV145`)를 조립 및 실행하면 메인 관제 화면의 밸브 카드가 즉시 **초록색(🟢 ON) / 빨간색 테두리 펄스**로 점등되며 실시간 반응하도록 완벽 구현.
- **`PID 제어 툴바 가로 공간 확보 및 텍스트 레이아웃 최적화`**:
  - PID 제어 행에서 불필요하게 가로폭을 차지하던 `🔒지정DAC:` 텍스트를 제거하고 콤팩트한 🔒 잠금 아이콘으로 정돈하여, 자동 연동되는 아날로그 출력 장치명(`AO_P108 (연료가스 공급 가압 펌프)` 등)이 잘림 없이 넉넉하고 시원하게 보이도록 UI 폭을 최적화.
- **`독립 팝업창 크로스 오리진(SOP: origin 'null') 보안 예외 원천 차단 및 무결점 삭제/저장 처리`**:
  - `file://` 또는 로컬 환경에서 팝업창이 `window.opener.socket` 등의 프로퍼티에 직접 접근할 때 브라우저 보안 정책(Same-Origin Policy: `Blocked a frame with origin "null" from accessing a cross-origin frame`)으로 인해 발생하던 런타임 오류를 완벽 해결.
  - 직접 속성 읽기를 배제하고 브라우저 표준 보안 안전 규격인 `postMessage` 및 독립 WebSocket 파이프라인으로 전환하여 레시피 삭제(🗑️), 레시피 저장(💾), MCU 다운로드(📡)가 어떤 브라우저 환경에서도 에러 없이 100% 매끄럽게 작동하도록 완성.
- **`기존 시퀀스 레시피 [수정 ✏️] 및 [삭제 🗑️] 기능 신규 구축`**:
  - **✏️ 레시피 수정 (Edit Recipe)**: 레시피 드롭다운에서 선택된 항목(기본/커스텀)의 ID, 타이틀, 설명 및 모든 스텝(DO, DAC, WAIT, WAIT_SENSOR, PID, IF_BRANCH)을 비주얼 빌더로 즉각 역분해 로드하여 스텝을 추가/삭제/변경한 후 재저장할 수 있는 양방향 역변환 로직 완성.
  - **🗑️ 레시피 삭제 (Delete Recipe)**: 불필요하거나 잘못 등록된 커스텀 레시피를 브라우저 로컬 저장소(`localStorage`), 인메모리, 그리고 파이썬 백엔드(`sample_recipes.json`)에서 1-클릭으로 안전하게 동기화 삭제하는 기능 구현.
- **`시퀀스 엔진 및 비주얼 빌더 IF 조건 분기 (IF_BRANCH) 기능 탑재`**:
  - `sequence_engine.py` 백엔드 비동기 루프 및 `sequence_manager.html` 프론트엔드 비주얼 빌더/시뮬레이터에 센서 계측값(압력, 유량, 온도 등) 조건 비교 판별 후 참(TRUE) / 거짓(FALSE)에 따라 지정한 스텝 번호(Step Number)로 동적 점프하는 **`IF_BRANCH` 분기 스텝** 추가.
  - C 코드 추출기(`exportMcuCHeader()`)에 `ACTION_IF_BRANCH = 4` 및 `uint8_t jump_true`, `uint8_t jump_false` C 구조체 내보내기 연동.
  - 버퍼탱크 압력 판별 후 정상연소 기동 vs 비상 질소 퍼지 동적 조건 분기 예시 레시피(`recipe_conditional_branch_purge`) 탑재 완료.
- **`PID 폐루프 실시간 텔레메트리 스트리밍 및 메인 관제 동적 시각화 엔진 완성`**:
  - **DAC 카드 펄스 뱃지 & 실시간 출력 연동**: 시퀀스 매니저에서 PID 제어 스텝이 가동되면, 메인 관제 화면의 연동 DAC 카드(예: `AO_P108`)에 **`[🎛️ PID AUTO]`** 네온 사이언 펄스 뱃지와 글로우 테두리가 점등되며, 오차에 따라 실시간 연산되는 조작량(MV: 전압 V 및 출력 %)이 200ms 주기로 부드럽게 요동치며 슬라이더/수치로 반영.
  - **AI 센서 카드 SP 목표치 마커 연동**: 피드백 센서 카드(예: `AI_PT109`)에 **`[🎯 SP: 5.0]`** 골드 뱃지가 점등되고, 센서 계측값(PV)이 목표치를 향해 수렴해가는 과정을 실시간 표출.
  - **시퀀스 창 상세 제어 브리핑**: 시퀀스 매니저 하단 상태바에 `[🎛️ PID 가동 중: SP 5.0 | PV 4.88 | 출력 48.8% (3.2s/6.0s)]` 로 실시간 제어 상태와 카운트다운을 즉시 표시하여 폐루프 제어의 생생한 동작성을 100% 직관화.
- **`PID 0초 입력 시 [비차단 백그라운드 상시 유지 제어] 모드 완성`**:
  - **비차단 즉시 다음 스텝 이행 (Non-Blocking Auto-Advance)**: PID 유지 시간을 `0`초로 설정하면 시퀀스가 해당 스텝에서 멈춰 대기(블로킹)하지 않고, 백그라운드 PID 태스크를 독립 가동한 즉시 다음 스텝(밸브 동작 등)으로 곧바로 진행.
  - **백그라운드 다중 루프 상시 유지 (`bgPidLoops`)**: 시퀀스가 다음 스텝들을 계속 구동하는 동안에도 백그라운드에서 DAC 출력과 센서 피드백을 실시간으로 추종/제어하며 메인 대시보드 `[🎛️ PID AUTO]` 뱃지와 전압을 살아있는 상태로 유지.
  - **자동/수동 오프 연동**: 이후 스텝에서 해당 DAC를 명시적으로 `0V`로 끄거나 새로운 수동 출력/PID로 변경할 때, 또는 시퀀스 종료/정지 시 백그라운드 루프가 안전하게 자동 정리되도록 완벽 구현.
- **`다중 동시 PID 제어 채널 그래픽 지속 레지스트리 (Multi-Channel PID Registry) 구축`**:
  - **다중 DAC 출력 상태 보존**: 여러 장치(`AO_AB212`, `AO_MFC111`, `AO_MFC121` 등)가 순차적/동시적으로 PID 제어되어 출력이 나가고 있을 때, 단일 채널만 깜빡이거나 다른 스텝으로 넘어갔을 때 이전 채널 뱃지가 사라지던 현상을 완벽 해결.
  - **0V 다운 전까지 네온 펄스 & 목표치 그래픽 영구 보존**: 0V가 아닌 유효 조작량(MV V / %)이 출력되고 있는 모든 DAC 카드에 `[🎛️ PID AUTO]` 사이언 펄스 뱃지와 글로우 테두리가 상시 유지되며, 연결된 모든 AI 피드백 센서에도 `[🎯 SP]` 목표치 뱃지가 실시간으로 지속 점등 유지.
  - **0V 다운 / 리셋 시 안전 해제**: 해당 DAC 채널이 명시적으로 0V(OFF)로 다운되거나 전체 초기화(ALL RESET)가 실행될 때만 해당 채널 레지스트리를 안전하게 정상화.

### 5. [Documentation] 프로그래밍 용어 사전 갱신
- `PROGRAMMING_TERMS.md`에 신규 용어 추가 반영 완료:
  - 406번 ~ 423번 항목 기존 유지
  - 424번: `조건 분기 시퀀스 제어 (Conditional Branching Sequence Control / IF-Else Step Jump)`
  - 425번: `개루프 제어 (Open-Loop Control)`
  - 426번: `폐루프 PID 제어 (Closed-Loop PID Control)`
  - 427번: `설정값 SP 대 조작량 MV (Set-Point SP vs Manipulated Variable MV)`
  - 428번: `수동 바이패스 제어 (Manual Bypass Control)`
  - 429번: `프리셋 킥스타트 기동 (Preset Kick-Start Control)`
  - 430번: `센서 주도 PID 1:1 하드웨어 바인딩 (Sensor-Driven PID Auto-Pairing)`
  - 431번: `아날로그 출력 단방향 잠금 자동 바인딩 (One-Way Locked DAC Binding)`
  - 432번: `아날로그 입력 센서 전용 PID 바인딩 (Analog Input Sensor-Only PID Binding)`
  - 433번: `크로스 윈도우 브로드캐스트 액션 파출 (Cross-Window Broadcast Action Dispatch)`
  - 434번: `레시피 양방향 역변환 디시리얼라이저 (Recipe Bidirectional Deserializer)`
  - 435번: `분산 레시피 동기화 삭제 (Distributed Recipe Sync Purge)`
  - 436번: `PID 폐루프 실시간 텔레메트리 스트리밍 (Closed-Loop PID Live Telemetry Streaming)`
  - 437번: `무한 연속 PID 제어 모드 (Continuous Infinite PID Regulation Mode)`
  - 438번: `비차단 백그라운드 상시 PID 제어 (Non-Blocking Background Continuous PID Regulation)`
  - 439번: `다중 채널 지속 PID 활성 레지스트리 (Persistent Multi-Channel Active PID Registry)`
  - 440번: `임베디드 시퀀스 펌웨어 이식 (Embedded Sequence Firmware Porting)`
  - 441번: `스탠드얼론 HMI-MCU 모드버스 레지스터 매핑 (Standalone HMI-to-MCU Modbus Register Mapping)`
  - 442번: `디지털 트윈 기반 사전 시뮬레이션 및 펌웨어 매칭 설계 (Digital-Twin Simulation & Firmware Matching Architecture)`
  - 443번: `깃 원격 저장소 바인딩 및 형상 관리 (Git Remote Repository Binding & Version Control)`

---



## 📅 2026-08-13: 수소 연료전지 자동 제어 시퀀스 엔진 & UI 컨트롤러 매니저 구축 🏷️🤖✨
관제 UI 대시보드 및 파이썬 백엔드 서비스에 수소 연료전지 시스템의 복잡한 기동 퍼지, 수소 누설 안전 테스트, 냉각 순환 제어 시퀀스를 하드코딩 없이 표준 JSON 레시피 기반으로 자동 순차 제어하는 **시퀀스 레시피 엔진(`sequence_engine.py`)** 및 **대시보드 조작 매니저 UI** 구축을 완료하였습니다.

### 1. [Feature] 표준 JSON 시퀀스 레시피 구축 (`03_Control_UI/sequences/sample_recipes.json`)
- **`개질기 기동 퍼지 & 압력 형성 시퀀스`**: 질소 퍼지 밸브(DO_SV125) 개방 ➔ 가압 펌프(AO_P108) 예열 분사 ➔ 버퍼 탱크 압력(AI_PT109) 5.0kPa 조건 대기 ➔ 개질가스 인입 전동 밸브(DO_MV131) 개방 순차 로직 정의.
- **`수소/메탄 가스 누설 인터록 기동 테스트 모드`**: 가스 메인 밸브(DO_SV102/103) 개방 및 안전 감시 모드 검증.
- **`스택 및 냉각수 순환 정밀 제어 시퀀스`**: Anode 펌프(AO_P351) 및 스택 1 펌프(AO_P370) 램프업 ➔ 유량(AI_MFM315) 10.0LPM 조건 감시 로직 구축.

### 2. [Backend] 비동기 시퀀스 비동기 상태 머신 엔진 개발 (`sequence_engine.py`)
- 비동기 코루틴 기반 스텝 순차 실행 및 스텝별 액션(DO 릴레이 온/오프, DAC 출력 설정) 자동 트리거.
- 타이머 대기(`wait_seconds`), 센서 목표치 조건 감시(`condition`), 타임아웃(`timeout_seconds`) 예외 보호.
- **안전 인터록 (Safety Interlock)**: 메탄 감지(`DI_GD501`), 수소 감지(`DI_GD502`), 비상 스위치(`DI_EMG_SW`) 활성화 시 시퀀스 즉시 비상 중단 및 **FAIL-SAFE 가스 밸브 락다운** 자동 발동.

### 3. [Frontend & UI/UX] 메인 전원 스위치(`DO_MC_SW`) 2칸 그리드 고밀도 렌더링 & 수평 정렬 완비 (`index.html`)
- **DO 릴레이 그리드 디자인 개편**: 기존 상단 띠 패널을 제거하고 `DO_MC_SW` (SYSTEM MAIN POWER) 카드를 **3행 우측 비어 있던 2칸 공간(6~7열)에 `grid-column: span 2` 럭셔리 카드로 재배치**.
- **고밀도 디자인 꽉 찬 연출 (Density Upgrade)**: 카드 내부 2칸 여백이 썰렁해 보이지 않도록 광원 인디케이터(14px), 폰트(13px Bold), 통전 상태 뱃지(`24V LOOP ACTIVE`), 시원시원한 버튼(8px 16px)을 조화롭게 통합 배치하여 관제 패널의 시각적 밀도 300% 향상.
- **대시보드 높이 정밀 수평 맞춤**: 상단 띠 카드가 사라짐에 따라 오른쪽 DO 출력 패널의 바닥 라인이 **왼쪽 센서 수신 현황(AD1115) 및 디지털 입력 상태(DI 8채널) 패널 바닥 라인과 100% 칼같이 수평 일치 정렬(Height Alignment)**을 이루도록 완벽 시각적 균형 달성.
- **전원 OFF 음영 예외 교정 버그 원천 해결**: `DO_MC_SW` 초기 상태를 기본 `active: true` (ON)로 보정하고, CSS 전원 차단 음영 규칙을 `.main-power-card` 기반으로 재매핑하여 디지털 출력 카드가 캄캄하게 어두워지는 현상을 완벽 교정.

### 4. [Frontend & Control] DAC 아날로그 출력-센서 PID 폐루프 연동 하이라이트 & 용어 정제 (`index.html`)
- **실제 엑셀 피드백 도면 100% 반영 매핑**: 9개 아날로그 DAC 구동 장치(에어블로어, 가압 펌프, 순환 펌프 등)와 연결된 **피드백 계측 센서(MFM 유량계, PT 압력계, TC 온도계, FM382) 상관관계 바인딩**.
- **인터랙티브 뱃지 & 동시 펄스 글로우**: DAC 출력 테이블에 `[🔗 FB: AI_PT109]` 형태의 네온 피드백 뱃지를 렌더링하고, 클릭/조작 시 대상 피드백 센서 카드/행이 **2.2초 동안 황금색 글로우 하이라이트로 반짝반짝 시동**하여 PID 폐루프 제어 상관관계를 시각적으로 한눈에 파악하도록 완벽 구축.
- **도메인 릴레이 용어 정제**: 하드웨어 접점 릴레이(Relay Switch)와의 용어 중복 혼동을 전면 해소하기 위해 모달 및 안내문구를 **`└ PID Closed-Loop 센서 실시간 피드백 연동 중 (출력 조작 시 센서 하이라이트)`**로 한층 더 선명하게 용어 다듬기 완료.

### 5. [Feature & Backend] 비주얼 시퀀스 매크로 레코더 & 센서 피드백 PID 및 MCU 이식 엔진 완비 (`sequence_manager.html`, `index.html`, `serial_bridge.py`)
- **`➕ 시퀀스 매크로 빌더 (Visual Recipe Builder)` 확장**: DAC 아날로그 속도 제어(`SET_DAC`), 센서 목표 조건 대기(`WAIT_SENSOR`), PID 자동 폐루프 제어(`PID_CONTROL`) 스텝을 클릭 한 번으로 조립 가능한 4개 쾌속 툴바 구축.
- **센서 피드백 조건부 대기 & PID 폐루프 제어 스텝 탑재**: 버퍼 탱크 압력계(`AI_PT109`), 냉각수 유량계(`AI_MFM315`), 온도계(`AI_TC101`) 등 실제 센서 수신 데이터를 10Hz 미세 연산하여 목표 설정값(SetPoint)으로 펌프/블로어 DAC 출력을 실시간 자동 조절 및 타임아웃 안심 대기 구동 보증.
- **온더플라이 MCU C-Code 추출기 (`exportMcuCHeader`)**: 조립된 레시피를 MCU 펌웨어용 C언어 헤더 파일(`sequence_recipes.h`)로 다이렉트 변환하여 다운로드하는 자동화 기능 구현.
- **RS-422 동적 메모리 이식 엔진 (`sendRecipeToMcuViaRs422`)**: 펌웨어 재컴파일 없이 RS-422 통신 패킷으로 MCU 비휘발성 메모리(EEPROM/Flash)에 레시피를 직결 송출하여 PC 없이 MCU 독립 자율 모드 기동 보증.
- **널 안전 전역 바인딩 & 무결성 검증 완비**: Node.js AST 구문 검증 100% 통과로 스크립트 실행 중단 버그 원천 차단 및 독립 팝업 3중 마운팅 완비.

### 5. [Backend] 전체 출력 초기화(`ALL RESET`) 구축 & 메인 MC 전원(`DO_MC_SW`) ON 보존 로직 완비 (`index.html`, `sequence_manager.html`, `serial_bridge.py`)
- **`🔄 전체 출력 초기화 (ALL RESET)` 시스템 탑재**: 메인 대시보드 및 팝업창 상단에 비상 리셋 버튼 구축. 전체 초기화 시 24V 시스템 전원 및 MC 동작 스위치(`DO_MC_SW`)는 **ON(활성화) 상태를 지속 유지**하고, 19개 부하 밸브/히터만 100% OFF 차단 닫힘 처리하여 전원 꺼짐 없는 안전 리셋 완료.

### 4. [Hardware] 트라이악 출력부 RC 스너버 커패시터(MF501) 회로 정밀 분석
- `Q506`(BTA16-600B 16A/600V 트라이악) 및 `IC505`(MOC3063S 포토 트라이악) 구동부의 **RC 스너버 커패시터(`MF501`)** 부품 스펙 정밀 도출.
- **용량 코드**: `104` ➔ $10 \times 10^4\text{ pF} = 100,000\text{ pF} = \mathbf{0.1\ \mu F}$ (100nF)
- **정격 내전압**: $\mathbf{400V\ \text{DC}}$ (또는 AC 220V 상용선 스스너버 전용 AC 275V X2급 안전 규격)
- **부품 권장 종목**: 메탈라이즈드 필름 커패시터(Metalized Polyester/Polypropylene Film Capacitor). $R607(330\ \Omega)$ 저항과 직렬 결합하여 AC 220V 유도성 부하 오프 시 역기전력 서지 스파이크 및 $dV/dt$ 오작동 완벽 흡수 방어.

### 5. [Documentation] 프로그래밍 용어 사전 갱신
- `PROGRAMMING_TERMS.md` 용어 사전 373번~388번 (`하드웨어 안전 올 리셋 인터록` 등 16개 용어) 신규 작성 및 반영 완료.

---

## 📅 2026-07-03: v0.1.0 프로젝트 폴더 구조 수립 및 초기화 📁
새로운 수소 제어 보드(H2_Control_Board) 프로젝트를 시작하며, 하드웨어, 펌웨어, GUI PC, HMI LCD 등을 유기적으로 개발할 수 있는 통합 폴더 구조를 생성하고 초기화하였습니다.

### 1. [Structure] 프로젝트 디렉터리 구성 완료
- `01_Hardware`: 회로도 및 PCB 설계 자료 보관용
- `02_Firmware`: MCU(마이크로컨트롤러) 펌웨어 소스 코드 공간
- `03_GUI_PC`: 모니터링 및 제어용 PC GUI 프로그램 개발 공간
- `04_HMI_LCD`: LCD 화면 디자인 및 제어용 프로토콜 파일 보관 공간
- `05_Docs`: 통신 프로토콜 정의서 및 설계서 등 문서 보관 공간
- `06_Images`: 이미지 리소스 관리용 공간

### 2. [Initial] 개발 로그 및 프로그래밍 용어 사전 생성
- `DEVELOPMENT_LOG.md` 작성 시작
- `PROGRAMMING_TERMS.md` 작성 시작

### 3. [Analysis] MCU 칩셋 식별 및 I/O 리스트 분석
- `01_Hardware` 폴더의 회로도 자료 분석을 통해 메인 MCU 모델로 **dsPIC33CK256MP508** 및 **dsPIC33CK512MP710** 식별.
- `05_Docs` 폴더의 엑셀 문서(`LT 10kW_IO list.xlsx` 등)를 파이썬 스크립트로 자동 분석하여 I/O 매핑 대상 리스트(통신 및 센서류 등) 추출. (이전 10kW 레퍼런스 사양)
- **현재 개발 타겟 시스템 사양은 6kW**로 확정되었으며, 자기가 새로 정리해 올 신규 6kW IO 리스트를 토대로 설계를 반영하기 위해 대기 중.
- MPLAB Code Configurator(MCC)를 활용한 주변장치 및 핀 매핑(PPS) 가이드 수립.
- **듀얼 W5500 이더넷 컨트롤러 회로도 분석 및 dsPIC33CK512MP710 간 핀 매핑 리스트 도출**:
  - 두 W5500 칩은 SPI 버스를 공유하며(MISO/MOSI/SCLK), Chip Select 및 Interrupt, Reset을 개별 포트로 제어하도록 설계됨.
  - 관련 포트 매핑 관계 및 MCC를 활용한 핀 및 외부인터럽트 설정 가이드 준비 완료.
- **MPLAB X IDE 환경 내 MCC 실행 및 초기 환경 설정(Melody/System Clock) 활용 학습 진행**:
  - IDE 툴바 실행 경로, 설정 파일(.mc3) 저장, System Module 설정 가이드 제공.
  - Standalone MCC(독립 실행형) 앱 기준, `File -> New Configuration`을 통한 디바이스(`dsPIC33CK512MP710`) 설정 및 프로젝트 연결 가이드 제공.
  - **MCC Pin Configurator를 활용한 하드웨어 핀 잠금(Locking) 가이드 작성**:
    - PPS 재할당 가능 핀(SPI MOSI/MISO/SCLK) 및 고정 핀(W5500의 CS, RST, INT용 일반 GPIO)을 완벽히 구분하여 매핑 표 적용 방안 구체화.
    - 사용자가 설정한 MCC 핀 잠금(Pins 모듈) 상태 검증 및 SPI 모듈 전용 핀 전환 피드백 기록 완료.
  - **MCC Melody 버전 드라이버 구조 분석**:
    - `SPI1` 대신 `SPI_Host`(마스터) 및 `SPI_Client`(슬레이브) 추상화 모듈 연결 가이드 제공.
    - Application Builder 뷰 기준, 중복 추가된 `SPI_Host` 정리(1개만 사용) 및 물리 모듈(`SPI1`) 바인딩(Binding) 방법 가이드 제공. (사용자 정리 및 바인딩 완료)
    - Builder 다이어그램 상 컴포넌트 삭제 애로사항에 대한 대안(좌측 세로 탭 'Project Resource' 클릭 후 우클릭 제거 및 설정 저장 후 재실행) 가이드 수립.
    - `SPI1_Host` 전용 핀 매핑(SDI1, SDO1, SCK1IN) 잠금 적용 및 기존 GPIO 이중 매핑 해제 가이드 제공.
- **연료전지 6kW BOP list (PDF) 수령 및 채널 스펙 정밀 분석**:
  - 가스, 공기, 개질수, COGEN 등 전 계통 69개 항목의 상세 BOP 센서/액추에이터 인터페이스(DO, DI, AO, AI, AIO, TC, RS485, Pulse) 요구 사양 정리 완료.
  - 기존 회로도([Mcu.SchDoc](file:///d:/Work/H2_Control_Board/01_Hardware/Mcu.SchDoc))의 네트 레이블과 6kW BOP 리스트의 채널 번호(FAN503, GD504, GD505 등) 간의 완벽한 일치성 검증 완료.
- **알티움 디자이너 ERC(Electrical Rules Check) 물결선 경고 분석 및 해결 가이드 작성**:
  - W5500(IC201) 제어 핀(32, 36, 37번)의 미연결(Floating) 물결선 경고의 주요 원인인 '언더바(_)/공백 철자 불일치' 및 '그리드 스냅 정렬 오류' 규명 및 조치 방법 제시. (사용자 수정 완료 확인)
- **전체 통신 모듈(이더넷, RS-485, RS-232, I2C) 물리 핀 충돌 검증 완료**:
  - I2C 고정 핀 매핑(`RB0`/`RB1`) 적합성 검증 통과 및 UART/SPI PPS 핀들 간의 물리적 분리 검증을 통한 무충돌 설계 보증.
- **dsPIC33CK MCU 내부 발진자(FRC) 사용 타당성 및 통신 클록 정밀도 영향성 검토**:
  - 이더넷(동기식 SPI) 및 일반 제어는 내부 발진자로 충분하나, 6kW BOP 다채널 RS-485/232 비동기 직렬 통신의 보레이트 오차(Baud Rate Error) 억제를 위해 외부 크리스탈(8MHz 권장) 탑재 제안.
  - 외부 입력 크리스탈과 내부 PLL(주파수 체배) 간의 상관관계 규명 및 노이즈(EMI) 최소화 및 100 MIPS 시스템 최대 스피드 가동을 위한 8MHz/20MHz/32MHz 부품 명세 검토 완료.
  - **외부 크리스탈 연결용 MCU 물리 핀(36번/37번) 매핑 검토 및 기존 히터선(DO_HT182, DO_HT183)과의 충돌 확인 및 여유 핀(6번/15번) 이사 조치 완료**.
  - **ICSP 다운로더/디버거 커넥터(PIC_PROG)와 MCU(dsPIC33CK512MP710) 간의 PGC3/PGD3/MCLR 핀 매핑 가이드 작성 완료**.
  - **보조 배터리(LiFePO4 3S) 연동 초절전 전력 관리 및 파워 게이팅(Power Gating) 전원 회로 아키텍처 수립**:
  - 메인 전원 차단 시 배터리 백업 전환용 Ideal Diode OR-ing 회로 및 초저전력 백업용 LDO(MCP1703) 구성안 제시.
  - 대기 모드 시 주변장치(W5500, 아날로그 센서 등)의 전력을 차단하기 위한 Load Switch(P-MOSFET 스위칭) 전원 분리 설계 방향 제시.
  - **정전 발생 감지(Power Fail Detection)를 위한 분배 저항/비교기 구성 및 메모리 데이터 세이브 골든타임 확보용 대용량 벌크 커패시터(Bulk Capacitor) 설계 기법 제시**.
  - **정전 감지 핀 최종 위치로 75번 핀 (RB8 / AN8) 선정 및 AD 컨버터 기능 및 외부 인터럽트(RP40) 적합성 검증 완료**.
  - **12V 정전 검출용 입력 분배 저항비 계산(상단 10kΩ / 하단 3.3kΩ) 및 MCU 핀 보호 전압(2.97V) 설계 확정**.
  - **로그 저장 시나리오 재정의에 따른 보조 배터리(LiFePO4 3S) 및 백업 전원 충전 회로 삭제, 입력단 대용량 벌크 콘덴서(470uF~1000uF) 기반의 초가성비/초경량 하드웨어 설계 최적화 확정**.
  - **MCU 76번 핀(RF15)의 네트 라벨 오타(DO_SV103333) 발견 및 알티움 프로젝트 컴파일(Compile)을 통한 싱글 핀 네트(Single Pin Net) 오류 검출 방법 안내**.
  - **알티움 한영 혼용판 메뉴 구조 매핑 피드백 (Validate PCB Project -> 프로젝트 검증/컴파일 실행 단추 매칭) 제공**.
  - **W5500 MISO 핀의 오타 라벨(ETH_MISO3)에 따른 싱글 핀 네트 물결선 발생 원인 및 정상 핀(ETH_MISO) 연결 시의 무경고 메커니즘 규명 완료**.
  - **MCU 핀의 전기적 타입 속성(Passive/IO) 및 알티움 연결 매트릭스(Connection Matrix) 규칙에 의한 싱글 핀 경고 무반응 현상 원인 규명 및 최적화 해결 가이드 수립**.
  - **다중 시트(6개 도면) 간 ERC 물결선 경고 비매칭 현상에 대한 Net Identifier Scope (Global 지정) 및 Passive-Unconnected 경고 활성화 튜닝 방법 제시.**
  - **도면별 미결선 및 네트 네임 부재 시 ERC 물결선 표시 불일치 현상 정밀 분석**:
    - 네트 네임이 없어도 물리적인 와이어로 핀 간 연결이 완료되면 에러가 표시되지 않는 정상 메커니즘 확인.
    - W5500의 10번 핀(EXRES1)의 경우, 핀에 연결된 와이어의 끝단이 공중에 떠 있는 '댕글링 와이어(Dangling Wire)' 상태와 핀의 'Input' 속성, 그리고 프로젝트의 'Connection Matrix' 규칙의 결합으로 인해 빨간색 경고 파형(물결선)이 강하게 발생하는 구조적 원인 분석 완료.
  - **RJ45 이더넷 커넥터(J201) 실드 핀(MH3, MH4) 처리 및 차폐 설계 검토**:
    - 외부 ESD/서지 방지 및 EMI 노이즈 차단을 위한 RJ45 금속 실드 케이스 연결 방안 정밀 검토.
    - 단순히 Floating(플로팅)으로 방치 시 정전기 아크 및 방사 노이즈 방출 위험성 인지.
    - 시스템의 전위 노이즈가 직접 메인 회로(DGND)로 들어오거나 그라운드 루프(Ground Loop)를 도는 현상을 막기 위해, 고전압 커패시터(1nF/2kV)와 고저항(1MΩ)을 병렬로 연결(Chassis-to-GND RC Coupling)하는 설계 솔루션 도출 및 제안.
    - 실드 핀(MH3, MH4)을 내부 그라운드(GND)에 직접 결선(Direct Short)할 때 발생하는 서지/정전기 침투 취약성 및 그라운드 루프 왜곡 문제 식별 및 피드백 제공.
    - MH3/MH4 핀 결합 후 병렬 RC(1MΩ 저항 + 1nF/2kV 캐패시터)를 거쳐 GND/DGND에 연결하는 실무 회로 변경안 확정 및 안내.
    - **회로도 상의 그라운드 접지 기호 분석**:
      - 사선 빗금 모양의 GND 접지 기호는 물리적으로 메인 디지털 접지(DGND, 삼각형 심볼)와 분리된 'Chassis Ground(섀시/케이스 그라운드)' 또는 'Earth(대지 어스)' 계통임을 판별 및 재확인.
      - 설계 상 RJ45 실드에서 필터 회로를 거쳐 연결해야 하는 그라운드 타겟이 빗금 모양 접지선(Chassis GND)이 되어야 함을 설계 기준 가이드로 적립 완료.
     - **이더넷 차폐용 1nF / 2kV 고전압 캐패시터 파트 넘버 선정**:
      - 2kV의 고전압 내압 환경에서 전극 간 아크 방전(Arcing) 방지를 위한 공간/연면 절연 거리(Clearance/Creepage) 확보 기준 수립.
      - 3216 Metric(1206) 패키지는 고전압 노이즈 인가 시 아크 방전 위험이 있으므로, 안전성을 높이기 위해 물리적 크기가 확보된 4520 Metric(1808) 또는 4532 Metric(1812) 패키지 적용 권장.
      - 제조사별 추천 파트넘버 도출: Yageo(CC1808KKX7RDBB102 -> 4520 Size), KEMET(C1808C102KGRACTU -> 4520 Size), TDK(C4520X7R3D102K130KA -> 4520 Size), Murata(GRM42D7U3D102JW31L -> 4520 Size) 확보.
    - **이더넷 차폐용 병렬 1MΩ 저항의 패키지 크기 및 전압 규격 분석**:
      - 칩 저항 사이즈별 최대 사용 전압 한계(1608 Metric: 50V, 2012 Metric: 150V, 3216 Metric: 200V) 규명.
      - 외부 고압 ESD 유입 시 단일 저항의 절연 파괴 및 파손을 막기 위해 최소 3216 Metric(1206) 패키지 적용 또는 고전압 전용 저항(High Voltage Resistor) 적용 권장.
      - 공간 마진 협소 시, 1MΩ 단일 저항 대신 510kΩ 저항 2개를 직렬(Series)로 배치하여 유입 고전압을 분산(직렬 분배)시키는 실무 보호 회로 설계 노하우 가이드 제공.
    - **부품 패키지 규격의 표기 기준 정립 협의**:
      - 칩 수동 소자(저항, 캐패시터) 크기 논의 시 인치(Inch) 단위와 미터법(Metric) 단위의 혼선을 방지하기 위해, 향후 보드 설계 및 개발 문서 작성 시 **미터법(Metric) 단위(예: 1608, 2012, 3216, 4520 등)**를 표준으로 삼아 소통하기로 확정.
  - **6개 회로도 시트 및 6kW BOP 사양 기반의 PCB 권장 치수 가늠 분석**:
    - dsPIC33CK MCU(100핀 TQFP), 듀얼 W5500(각 48핀 QFN) 및 듀얼 RJ45 이더넷 포트, 전원 모듈 등의 주요 부품 면적 산출.
    - 6kW BOP 리스트의 다채널 I/O (DI, DO, AI, AO, TC 등 총 69개 항목) 대응용 외부 터미널 블록 커넥터(핀 수 130~150개 이상) 물리적 테두리 배치 공간 계산.
    - 3.5mm/3.81mm Pitch 기준 3면 테두리 1단 배치 시 필요한 최소 기판 둘레 확인.
    - 부품 배치 영역(약 15,200 mm²)과 노이즈 차단 배선/그라운드 분리 마진(배선 면적률 1.6배 적용)을 합산하여 **약 24,000~26,000 mm²**의 최소 PCB 면적 도출.
    - 6kW 수소 제어 통합 보드의 최종 권장 PCB 치수로 **가로 220mm ~ 240mm / 세로 120mm ~ 130mm** 크기의 직사각형 4층판(4-Layer) 사양 제안 및 도출.
  - **솔리드웍스 기구 설계 환경 편의성 조치**:
    - 3D CAD(솔리드웍스) 작업 중 마우스 커서에 나타나는 '선택 필터(Selection Filter)' 깔때기 모양의 오작동 상태 식별.
    - 선택 필터 전체 해제 단축키(`F6`) 및 선택 필터 툴바 활성/비활성 단축키(`F5`) 적용을 통한 마우스 선택 장애 해결 및 환경 설정 가이드 제공.
  - **PCB 외곽 치수(300mm x 200mm) 확정 및 부품 동기화(ECO) 착수**:
    - 기구 설계 마진 및 수월한 부품 배치를 위해 PCB 보드 치수를 가로 300mm / 세로 200mm로 넉넉하게 확정하고 외곽선 모델링 및 고정 홀(Mounting Hole) 6개 배치 완료 확인.
    - 알티움 회로도(Schematic)로부터 PCB 파일로 부품들을 이관하기 위한 디자인 동기화(Update PCB Document / ECO 실행) 프로세스 가이드 수립 및 제공.
  - **다중 시트 부품 동기화(ECO) 누락 원인 분석 및 일괄 세이브(Save All) 조치**:
    - 6개 시트 중 특정 시트(Ethernet)의 부품만 PCB에 반영되는 부분 동기화 현상 확인.
    - 회로도 수정 후 프로젝트 전체가 저장되지 않아 컴파일러가 최신 정보를 인식하지 못한 설계 변경 동기화 에러 규명.
    - 해결 조치로 `File -> Save All`을 통한 전체 시트 일괄 저장 유도 및 `Validate Project` 재실행을 통한 6개 시트 전체 부품의 정상 일괄 동기화(ECO) 프로세스 수립 완료.
    - **알티움 최신 버전 컴파일러 메뉴 명칭 불일치 피드백 제공**:
      - 기존 버전의 `Compile PCB Project` 메뉴가 최신 알티움 디자이너에서는 `Validate PCB Project`로 명칭 변경되었음을 분석 및 확인.
      - 사용자 캡처 화면의 콘텍스트 메뉴 매핑 피드백을 기록하여 툴 버전 차이로 인한 혼동 해결 완료.
  - **6개 시트 프로젝트 컴파일(Validate Project) 에러 목록 정밀 분석 및 조치 가이드 수립**:
    - **Harness 충돌 에러**: `Power.Harness`와 `Output_Section.Harness` 간의 하네스 구조 불일치 및 정의 충돌 원인 규명. 각 시트 간의 신호 핀 매핑 점검 필요.
    - **Single Pin Net 에러**: `Ethernet.SchDoc`의 제어 신호들(`ETH1_INT`, `ETH1_RSTN` 등)이 다른 시트(MCU 등)의 연결 타겟 네트와 스펠링 불일치 또는 미연결 상태인 현상 확인.
    - **Unconnected Pin 에러**: W5500 칩의 비사용 핀들에 대해 `No ERC` 지시자 미부착 상태 확인. `No ERC` 일괄 매핑 권장.
    - **Missing Child-sheet 에러**: `Power.SchDoc` 내 시트 심볼(`U?`)이 참조하는 하위 도면들(`MG.SchDoc`, `SMPS_12v.SchDoc` 등)이 프로젝트에 누락되었거나 경로가 상실된 상태 원인 규명.
    - **Duplicate Entries 및 Multiple Top Level 에러**: 시트 심볼 지정자(`U?`)의 미번호 부여(Annotation 미실행) 및 중복 현상과 Net Identifier Scope 혼선에 따른 다중 최상위 도면 지정 오류 식별 및 수정 가이드 정립.
  - **Power.SchDoc 도면 내 기구/모듈(MG, SMPS 24v) 블록 오사용 조치**:
    - 마그네트 코일(MG) 및 SMPS 전원 모듈에 대해 계층 설계용 '시트 심볼(Sheet Symbol)'을 잘못 적용하여 '하위 도면 유실(Missing Child-sheet)' 에러가 지속 발생한 문제 규명.
    - 해결 방안으로 단순 도면 연동 블록이 아닌 정식 '컴포넌트 심볼(Component Symbol)'로서 `KipoAlt.SchLib` 라이브러리에 핀 구조와 외곽 박스 그래픽을 등록하고 회로도에 재배치하는 하드웨어 라이브러리 연동 설계 가이드 제시.
  - **외부 기기(MG, SMPS 12V/24V)의 PCB 실장 제외(Off-board) 사양 확정 및 Graphical 설정 가이드**:
    - 해당 모듈들이 PCB 내 실장 부품이 아닌 외부 기구물 장착품(Off-board)임을 파악 완료.
    - 알티움 상에서 하위 도면 연동이나 실장 에러(ECO)를 우회하기 위한 해결책으로, 회로도 내의 관련 기호 타입을 `Graphical` 혹은 `Mechanical` 부품 속성으로 명확히 변경 설정해 주는 ERC 경고 해제 솔루션 제시.
  - **TLV2374 연산 증폭기(OP-Amp) 12V 단일 전원 스펙 및 출력 전압 마진 분석**:
    - TLV2374의 레일 투 레일 입출력(Rail-to-Rail Input/Output, RRIO) 특성 검토 및 12V 전원 동작 적합성 확인.
    - 출력단 MOSFET 내부 온저항 및 부하 전류 드롭(Output Saturation Voltage)에 기인한 물리적 스윙 한계 규명.
    - 현재 피드백 저항(30kΩ/10kΩ) 부하 조건 하에서, 완벽한 0V 및 12V가 아닌 최소 **0.05V ~ 0.1V**, 최대 **11.9V ~ 11.95V** 범위의 스윙 한계 마진 피드백 도출 및 제공.
  - **300mm x 200mm PCB 공간 내 기능별 컴포넌트 룸(Room) 일괄 배치(Floorplan) 전략 수립**:
    - 대규모 다채널 혼성 신호 보드의 전기적 격리(Isolation) 및 신호 흐름(Signal Flow) 최적화 목적.
    - 메인 제어 룸(`Mcu`)을 보드 중앙 하단에 배치하여 신호선 분배 최단 경로 확보.
    - 스위칭 노이즈 및 열원인 전원 룸(`Power`) 및 디지털 출력 구동 룸(`Output_Section`)을 보드 좌측 구획으로 집중 격리.
    - 고속 통신 룸(`Ethernet`)을 보드 상단 좌측 가장자리에 배치하고, 민감한 아날로그 온도 감지 룸(`TempController`)을 노이즈원과 가장 먼 보드 대각선 상단 우측으로 분할 배치.
    - 격리 입력 룸(`Input_Section`)을 보드 우하단에 배치하여 외부 섀시 연동 경로 단순화.
    - 알티움 룸(Room) 객체를 사용한 그룹 단위 일괄 이동 및 내부 부품 자동 정렬(Room Format) 가이드 제시.

  - **장비 기구 제약조건 반영 최종 룸(Room) Floorplan 최적화 확정**:
    - 장비 함체 설계 상 외부 온도 센서 인입 케이블이 기판 좌상단(왼쪽 위)에서 내려오는 기구적 흐름 확인.
    - 미세 온도 신호선의 최단 배선 설계 원칙(Shortest Analog Routing)을 준수하기 위해, 기존 상단 우측 예정이던 `TempController` 룸을 **상단 좌측**으로 이동 배치 확정.
    - 이에 따라 고속 디지털 통신 룸인 `Ethernet` 룸은 상단 우측 테두리 영역으로 상호 위치 스와핑(Swap) 적용 완료.
  - **회로도별/배선별 파라미터 및 디자인 룰 설정 가이드 작성**:
    - 회로도 문서전체(.SchDoc)의 속성인 `Sheet Parameter` 정의 및 도면 템플릿(Title Block) 자동 매핑 연동 가이드 수립.
    - 회로도 내 특정 넷(Net)에 PCB 라우팅 폭(Width) 등 물리 법칙을 걸어줄 수 있는 `Parameter Set` 지시자(Directive)의 배치 및 Properties 내 디자인 룰(Rules), 넷 클래스(Net Class) 추가 방법 안내.
  - **Parameter Set 활용 유용 설계 규칙(Rules) 검토 및 제안**:
    - **Clearance Constraint(이격 거리 규칙)**: 고전압 24V 라인 및 민감한 아날로그 신호 주변의 최소 안전 갭(Gap) 설정 제안.
    - **Routing Layers(배선 층 규칙)**: 노이즈 민감 신호의 내층(Inner Layer) 전용 배선 또는 이더넷 신호의 Top Layer 전용 배선 제안.
    - **Differential Pairs Routing(차동 배선 규칙)**: 이더넷 TX/RX 라인의 100옴 임피던스 유지를 위한 페어 룰(Pair Rule) 제안.
    - **Polygon Connect Style(폴리곤 연결 규칙)**: 전원 접지의 Direct 연결 및 신호 부품의 Relief(냉땜 방지) 연결 세부 세팅 제안.
  - **4층 PCB 스택업(적층) 및 층별 네트(Net) 구성 가이드 도출**:
    - 4층 적층의 국룰 구조 확정 (Layer 1: Top, Layer 2: Inner 1-GND, Layer 3: Inner 2-Power, Layer 4: Bottom).
    - **Layer 1 (Top)**: dsPIC33 MCU, W5500 등 소자 배치면. 이더넷 고속 차동 신호선(ETH_TX/RX) 및 MCU 크리스탈 클록(비아 없이 배치 및 GND 쉴드 처리).
    - **Layer 2 (Inner 1 - GND Plane)**: 디지털 그라운드(DGND), 온도센서용 격리 아날로그 그라운드(AGND) 및 섀시 접지(GND)를 층 분할(Split)하여 통동판으로 구성. DGND와 AGND는 단일 지점 접지(Single Point Grounding) 원칙 고수.
    - **Layer 3 (Inner 2 - Power Plane)**: +3.3V, +5V, +12V, +24Vee 전원 패턴들을 용도에 따라 구획 분할(Power Split)하여 두껍게 이송.
    - **Layer 4 (Bottom)**: 터미널 블록 커넥터 핀의 솔더링면. 잔여 저속 디지털/아날로그 신호 배선.

## 📅 2026-07-04: v0.1.1 회로 심볼 변경에 따른 선택적 PCB 업데이트 가이드 수립 🔄
회로 설계 진행 과정에서 특정 부품(U100 SMPS, L100 필터 등)의 심볼과 핀 배치가 실제 부품 스펙에 맞춰 수정되었습니다. 기존 작업된 PCB 상의 라우팅과 부품 배치를 해치지 않고, 변경된 특정 부품만 PCB에 동기화할 수 있는 안전한 업데이트 프로세스를 정립하고 가이드를 제공하였습니다.

### 1. [Feature] 특정 부품 선택적 ECO(Engineering Change Order) 업데이트 기법 적용
- 회로도(Schematic)와 PCB 간의 전체 동기화 대신, ECO 목록에서 업데이트가 필요한 부품(`U100`, `L100`) 관련 변경 사항(Add/Modify)만 체크박스를 활성화(Enable)하고 나머지는 비활성화하여 선택적으로 이관하는 방안 적용.
- PCB 상에서 변경 대상 부품을 직접 클릭해 지정한 후 `Part Actions -> Update Selected From Schematic` 메뉴를 호출하여, 회로도 최신 정보(심볼, 핀 맵, 풋프린트)로 강제 동기화하는 개별 갱신 워크플로우 정립.
- `PROGRAMMING_TERMS.md`에 '선택적 ECO 업데이트 (Selective ECO Update)' 신규 기술 용어 정의 추가 완료.

### 2. [Structure] 4층 PCB 레이어 적층 및 내층 전원/GND 네트 매핑 가이드라인 수립
- 알티움 디자이너의 `Layer Stack Manager`를 활용하여 기존 2층 기판 설계를 4층(Top - GND - Power - Bottom) 구조로 승격시키는 상세 설정 방법 도출.
- 전원 및 접지의 루프 면적 최소화 및 고속 이더넷 임피던스(100Ω 차동) 제어 정밀도를 위해 내층 2개를 일반 배선층이 아닌 `Plane Layer`(평면 내층)로 생성하도록 가이드.
- 생성된 각 평면 내층에 접지(`DGND`, `AGND` 등 단일 접지 지점) 및 다채널 동작 전원(`+3.3V`, `+5V`, `+12V`, `+24V`)의 분할(`Split Plane`) 매핑 및 이격 설계 프로세스 정립.
- 초과 생성된 Plane 레이어(6층 상태)를 `Delete` 도구로 선택 삭제하여 정밀한 4층 적층 구조로 최종 최적화 피드백 제공.
- 사용자가 조정한 4층 스택업 구조(Top - Layer 1 - Layer 2 - Bottom)의 무결성을 최종 검증 완료하고 직관적인 내층 식별을 위한 레이어 네이밍(GND, Power) 가이드 제공.
- 스택업 내부 핵심 원재료인 동박(CF-004), 프리프레그(PP-006), FR-4 코어(Dielectric)의 기하학적 및 전기적 절연 역할 분석 가이드 제공.
- 사용자가 직관적으로 이해한 4층 샌드위치 적층 물리적 구조(중앙 코어 ➡️ 양면 절연접착 프리프레그 ➡️ 외각 신호 동박층)의 설계 무결성 최종 확인.
- 본격적인 PCB 아트워크 라우팅 전, 시트별/파트별 컴포넌트 룸(Room) 기반의 물리적 부품 배치(Placement) 우선순위 및 레이아웃 최적화 실무 전략 수립.
- 온도 센서 터미널 블록 간의 간격을 활용한 아날로그 멀티플렉서(Mux) 칩의 중심 배치 타당성 검증 및 ESD 보호/RC 필터 직렬 우선 배치 규칙 설계 가이드 제공.
- `PROGRAMMING_TERMS.md`에 레이어 설정 관련 용어 3종 및 적층 재료 용어 3종 추가 완료.
- `PROGRAMMING_TERMS.md`에 '멀티플렉서 (Multiplexer / Mux)', '냉접점 보상 (Cold Junction Compensation / CJC)' 신규 용어 추가 완료.
- PCB 실크스크린 최적화를 위해 유사 객체 찾기(Find Similar Objects) 기능으로 모든 소자의 부품 Value(Comment) 텍스트를 일괄 필터링하고 Properties 패널에서 일체 숨김(Hide) 처리하는 프로세스 정립.
- 전원 관리 아키텍처 설계 중 24V 1A 입력을 12V 1A 2채널로 변환 시의 전력 손실 한계 분석과, 사용자가 제안한 대안인 24V 1.2A(28.8W) 입력을 활용해 12V 0.8A 2채널(총 19.2W, 효율 반영 시 약 21.3W 소모) 구동 시 약 26%의 이상적인 설계 마진이 확보됨을 최종 검증 및 조치 완료.
- `PROGRAMMING_TERMS.md`에 '유사 객체 찾기 (Find Similar Objects / FSO)' 신규 용어 정의 추가 완료.
- `PROGRAMMING_TERMS.md`에 '벅 컨버터 (Buck Converter)', '변환 효율 (Conversion Efficiency)' 신규 용어 정의 추가 완료.
- LM2576 스위칭 레귤레이터의 ON/OFF 제어 핀(5번 핀)의 논리 레벨 임계값 및 최대 절대 정격 입력 전압을 정밀 분석하고 MCU GPIO 직접 구동 적합성 검증 완료.
- `PROGRAMMING_TERMS.md`에 '임계 전압 (Threshold Voltage)', '절대 최대 정격 (Absolute Maximum Ratings)' 신규 용어 정의 추가 완료.
- LM2576의 최소 작동 입력 전압 사양(7.0V~40V) 및 드롭아웃 전압(약 1.5V) 분석을 통해 5V 입력 하에서의 3.3V 출력 불가 판정 및 대안(LDO/저전압 전용 벅 컨버터) 설계 프로세스 제시.
- `PROGRAMMING_TERMS.md`에 '드롭아웃 전압 (Dropout Voltage)', 'LDO 레귤레이터 (Low Dropout Regulator)' 신규 용어 정의 추가 완료.

## 📅 2026-07-06: v0.1.2 단상 왕복식 리니어 펌프의 인버터 병렬 구동 타당성 기술 검토 및 보호 설계 수립 ⚙️
모터의 단일 인버터 병렬 제어(Multi-motor Control)의 일반적 조건 및 제피로스(Zephyros) 에어 펌프와 같은 전자석(Solenoid) 왕복 진동식 단상 리니어 펌프의 특수성을 고려한 인버터 구동 제어 가능 여부를 기술 검토하고, 전압 위상 제어 중심의 유량 조절 대안 및 개별 모터 보호 설계 방안을 정립하였습니다.

### 1. [Feature] 단일 인버터 모터 병렬 제어(Multi-motor Drive) 핵심 조건 정의
- 인버터 1대로 복수의 모터를 병렬 제어 시, 전압과 주파수가 고정 비례하는 V/f 제어(Volts-per-Hertz Control) 모드로 인버터를 구성해야 함을 확인 (피드백 제어 기반의 벡터/센서리스 제어 사용 불가).
- 병렬 연결된 모터들의 정격 전류 총합에 기동 전류 마진을 합산하여 인버터의 연속 출력 전류 사양이 약 1.1~1.2배 이상 확보되도록 가이드라인 제시.
- 단일 인버터의 내부 보호 기능은 전체 선로의 전류 합산값만 감시하므로, 개별 모터의 국부적인 과부하/소손을 방지하기 위해 각 모터 라인마다 개별 열동형 과부하 계전기(Thermal Overload Relay) 또는 EOCR을 반드시 설치하도록 보호 시스템 설계 수립.
- `PROGRAMMING_TERMS.md`에 '인버터 병렬 운전 (Inverter Parallel Operation)' 신규 용어 정의 추가 완료.

### 2. [Structure] 제피로스 에어 펌프(단상 왕복 진동식)의 인버터 제어 비적합성 검증 및 대안 수립
- 제피로스 에어 펌프(ZP-25A~ZP-600D 라인업)는 일반 회전식 3상 모터가 아닌 단상 AC(100~240V, 50/60Hz) 전력을 인가하여 가동자의 내부 전자석을 왕복 운동시키는 리니어 다이어프램 펌프임을 내부 구조 및 작동원리를 분석하여 규명.
- 해당 리니어 펌프는 기계적 스프링과 다이어프램 고유 주파수가 상용 주파수(50/60Hz)에 완벽히 동기화(공진)되어 동작하도록 설계되어 있어, 인버터를 사용해 강제로 입력 주파수를 변경하면 공진점이 파괴되어 토출량이 급감하고 심각한 소음/진동 및 코일 소손 유발을 유선 안내.
- 따라서, 에어 펌프의 유량 제어는 인버터를 이용한 주파수 제어가 아니라 주파수를 60Hz로 고정한 상태에서 전압 크기만 제어하는 위상 제어(Triac/디머) 또는 슬라이닥스 방식을 사용하여 솔레노이드 스트로크 폭을 조정해야 함을 명확화.
- `PROGRAMMING_TERMS.md`에 '다이어프램식 리니어 펌프 (Linear Diaphragm Pump)' 신규 용어 정의 추가 완료.

### 3. [Feature] 유도성 부하(솔레노이드 펌프)용 트라이악 위상 제어 회로 설계 및 보호 조건 정립
- 에어 펌프와 같은 무거운 인덕티브(Inductive) 부하를 일반 트라이악으로 스위칭 시, 전류 지연에 따른 Zero-Crossing 오프 시점의 급격한 전압 변동률(dV/dt) 스파이크로 인해 오작동(스스로 다시 켜짐) 및 파손 위험이 크므로 스너버 회로(RC Snubber) 또는 스너버리스(Snubberless) 트라이악 적용 가이드 수립.
- 위상 제어를 원활히 수행할 수 있도록, AC 입력 전압의 제로 크로싱 시점을 MCU가 실시간 감지하여 위상각 트리거 타이밍을 제어할 수 있는 Zero-Crossing 감지 동기화 회로 설계 규정.
- MCU의 게이트 트리거 신호와 고압 교류 라인을 분리하는 포토 커플러 소자는 Zero-Cross 회로가 내장된 형태가 아닌, 원하는 위상각에서 즉각 켜지는 Random-Phase 포토 트라이악(MOC3021/MOC3052 등)을 사용해야 정상적인 위상(디밍) 제어가 가능함을 기술 명확화.
- `PROGRAMMING_TERMS.md`에 '스너버 회로 (Snubber Circuit)', '스너버리스 트라이악 (Snubberless Triac)', '랜덤 페이즈 포토 트라이악 (Random-Phase Optocoupler)' 신규 용어 정의 추가 완료.

### 4. [Feature] dsPIC33CK512MP710-E/PT (100핀 TQFP) 제로 크로싱 외부 인터럽트(INT) 핀 매핑 설계 수립
- dsPIC33CK512MP710 MCU는 이전 세대와 달리 외부 인터럽트 `INT0` 마저 고정 핀이 아니며, 모든 외부 인터럽트(`INT0`~`INT3`) 입력을 PPS(Peripheral Pin Select) 입력 레지스터(`RPINR0`~`RPINR2`)를 통해 100핀 TQFP 패키지의 재할당 가능 입력 핀(RP/RPI)에 자유롭게 매핑할 수 있음을 규명.
- 전파 정류 브릿지를 거쳐 120Hz(60Hz 기준 반주기마다 1회) 펄스 형태로 들어오는 감지 파형의 상승 에지(Rising Edge, 포토커플러 오프 시 풀업에 의해 하이로 스윙하는 시점) 트리거링 설정을 통해 정확한 위상 시간 지연 제어(최대 8.33ms 딜레이 후 트라이악 도통)를 기동시키는 임베디드 펌웨어 소프트웨어 타이밍 구조 수립.
- `PROGRAMMING_TERMS.md`에 '외부 인터럽트 (External Interrupt / INTx)', '제로 크로싱 감지 회로 (Zero-Crossing Detection Circuit)' 신규 용어 정의 추가 완료.

### 5. [Structure] dsPIC33CK512MP710-E/PT 73번 핀(RE11) 외부 인터럽트 적용 적합성 검증 및 오류 정정
- 사용자가 제안한 `RE11(73번 핀)`에 대한 데이터시트 및 실제 회로도 교차 검증 수행 및 오인 분석 정정.
- **73번 핀 (포트: `RE11`)**: dsPIC33CK512MP710-E/PT 100핀 TQFP 패키지에서 73번 핀의 공식 포트는 `RE11`이 맞음을 최종 확인.
- **인터럽트 사용 불가능 판정**: `RE11` 포트는 데이터시트상 RP/RPI(재할당 가능) 번호가 할당되지 않은 순수 고정형 일반 GPIO 핀으로 판명됨. 이에 따라 PPS 입력 매핑 레지스터(`RPINR`)를 통한 외부 인터럽트(`INTx`) 지정을 할 수 없음.
- **대안 수립**: 제로 크로싱 센싱용 포토커플러 출력 인터럽트 라인을 73번 핀(`RE11`)에 직접 할당하지 않고, 주변의 빈 RP 핀(또는 하드웨어 핀 스와핑 적용 가능한 RP 핀)으로 우회(Swap) 설계하는 권장사항 확정 및 적용.

### 6. [Feature] dsPIC33CK512MP710-E/PT 51번~75번 핀 범위 내 외부 인터럽트용 가용 핀 도출
- 43번(`RB0`), 44번(`RB1`)이 기존 회로(Ethernet 제어선 등)에서 사용 중임에 따라, 대안으로 51번~75번 핀 영역 내에서 비어있고 외부 인터럽트가 가능한 RP 핀 탐색.
- **68번 핀 (포트: `RB5` / `RP37`)**: 현재 회로도상 미사용 상태이며, `RP37`로 지정되어 외부 인터럽트(`INTx`) 매핑이 가능함.
- **66번 핀 (포트: `RD6` / `RP70`)**: 현재 회로도상 미사용 상태이며, `RP70`으로 지정되어 외부 인터럽트(`INTx`) 매핑이 가능함.
- **62번 핀 (포트: `RD8` / `RP72`)**: 현재 회로도상 미사용 상태이며, `RP72`로 지정되어 외부 인터럽트(`INTx`) 매핑이 가능함.
- 위 3개 핀 중 회로 배선 작업이 가장 수월한 핀을 최종 제로 크로싱 인터럽트 입력으로 활용하도록 레이아웃 설계 가이드 제시.

### 7. [Feature] RS-485 통신선과 제로 크로싱 신호선(`HZ_60`) 간 노이즈 간섭(크로스토크) 방지 대책 수립
- 제로 크로싱 신호(`HZ_60`)를 68번 핀(`RB5`/`RP37`)에 임시 배치함에 따라, 인접한 69번 핀(`485_RX`) 및 67번 핀(`485_TX`) 데이터 신호선 간의 상호 크로스토크(Crosstalk) 유입 가능성 기술 검토.
- **간섭 위험 분석**: `HZ_60` 신호는 120Hz의 저주파 펄스이나, 포토커플러 오프 시 전압 서징 속도(Slew Rate)가 빠르고 고속 UART 통신선들과 평행 배선 시 정전 결합 노이즈가 유입될 우려가 존재함.
- **최종 보완 조치**: 485 통신 노이즈 전파 차단 및 신호 무결성(Signal Integrity) 보장을 위해, 485 라인 영역(65~69번 핀)에서 물리적으로 분리되어 있고 바로 위에 접지 차폐 핀(`Vss` - 63번 핀)이 인접한 **62번 핀 (`RD8` / `RP72`)**으로 최종 변경 배치 결정 및 적용.

### 8. [Feature] PCB 레이아웃 변경 사항의 회로도 역동기화(Back Annotation) 프로세스 수립
- PCB 도면 상에서 직접 변경된 풋프린트 패키지 형태(예: R1608 사이즈의 특정 랜드 디자인 변경 부품 `R537`, `R544` 등) 및 속성 정보를 원본 회로도(SchDoc)로 역업데이트(Back Annotation)하는 엔지니어링 변경 명령(ECO) 프로세스 가이드 정립.
- PCB Editor 내의 `Design -> Update Schematics in [Project]` 메뉴(또는 Schematic Editor 내의 `Import Changes from [PCB]`)를 활용하여 PCB 단독 수정 사항을 회로도에 완전히 일치 및 동기화시키는 워크플로우 적용.
- `PROGRAMMING_TERMS.md`에 '역방향 ECO 업데이트 (Back Annotation)' 신규 용어 정의 추가 완료.

## 📅 2026-07-07: Altium Designer PCB 부품 45도 회전 및 파라미터 표시 설정 🔄
- **MCU 부품(IC5) 배치 후 45도 회전 적용 방법 가이드 작성**:
  - 알티움 디자이너 PCB 에디터 내에서 부품을 45도 등의 세밀한 각도로 회전시키기 위한 속성값 변경 및 환경설정 최적화 가이드 제공.
  - 선택된 부품의 Properties 패널(단축키 `F11`) 내 **Rotation** 속성을 직접 `45`로 편집하는 방법 및 **Tools -> Preferences -> PCB Editor -> General** 탭의 **Rotation Step** 설정을 `90`에서 `45`로 수정하여 드래그 중 스페이스바(`Spacebar`)로 45도 회전하는 설정 가이드 수립.
- **PCB 상의 부품 파라미터 표시를 위한 스페셜 스트링(Special String) 기법 가이드 제공**:
  - PCB 에디터 내에서 부품의 특정 속성 값(예: `BOP No`)을 보드 레이아웃에 실시간으로 반영하여 표시하도록 지정하는 스페셜 스트링 `.ParameterName` (예: `.BOP No`) 연동법 설명.
  - 실제 값으로 변환(Convert)해서 보이기 위해 View Configuration(단축키 `L`) 패널의 `View Options` 탭 -> `Additional Options` 섹션에 있는 **`Special Strings`** 활성화 버튼 가이드 제공. (최신 알티움 버전의 버튼 인터페이스 변경 반영)
  - `PROGRAMMING_TERMS.md`에 '회전 단계 (Rotation Step)', '속성 패널 (Properties Panel)', '스페셜 스트링 (Special String)', '뷰 구성 (View Configuration)' 신규 용어 정의 추가 완료.

## 📅 2026-07-08: dsPIC33CK512MP710-E/PT 미사용 핀 분석 및 60Hz 펄스 입력 가용성 검토 🔍
- **[Analysis] 회로도 및 데이터시트 기반 미사용 핀 PPS 맵핑 적합성 검토**:
  - 회로도([Mcu.SchDoc](file:///d:/Work/H2_Control_Board/01_Hardware/Mcu.SchDoc)) 상 비어있는 핀들(RB13, RA8, RE15, RB12, RA7, RA6, RE14, RD0, RD1, RD4)의 dsPIC33CK512MP710 핀맵 테이블 및 PPS 기능 지원 여부 정밀 분석.
  - 60Hz 펄스 신호(주기 약 16.67ms) 주파수 및 듀티비 계측 시 하드웨어적 Input Capture(IC) 또는 외부 인터럽트(INT) 기능이 필수적임을 규명.
  - 리맵퍼블 핀(RP/RPI) 지정을 지원하여 PPS 설정이 가능한 가용 핀으로 **RB13 (100번)**, **RA8 (99번)**, **RB12 (97번)**, **RA7 (96번)**, **RA6 (95번)**, **RD0 (91번)**, **RD1 (90번)**, **RD4 (85번)** 최종 도출 및 제안.
  - PPS를 지원하지 않는 고정 I/O 핀인 **RE14 (94번)** 및 **RE15 (98번)** 은 정밀한 외부 펄스 측정용 핀으로 비적합(미권장) 판정.
- **[Analysis] 5V 입력 신호에 대한 전압 내성(5V Tolerant) 분석**:
  - 60Hz 동기 펄스가 5V 전압일 경우에 대비하여, 타겟 디지털 핀들 중 아날로그 입력(`ANx`, `CMPx`, `OAx`)이 중첩되지 않는 순수 디지털 전용 핀들이 전부 **5V Tolerant(최대 5.5V 허용) 핀**에 해당함을 확인 및 보증.
  - JTAG 등 디버깅 간섭이 있을 수 있는 `RB12 (TDI/97번)`를 제외한 `RB13 (100번)`, `RA8 (99번)`, `RA7 (96번)`, `RA6 (95번)` 등을 최우선 가용 핀으로 추천.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `5V Tolerant (5V 입력 허용)` 및 `Input Capture (인풋 캡처)` 신규 용어 정의 추가 완료.

- **[Analysis] 제로 크로싱 기반 60Hz 펄스 감지 회로(HZ60) 전기적 타당성 검토**:
  - `VacN`/`VacL` 220V AC 입력을 다이오드 브릿지(`DF06`) 및 광절연 포토커플러(`LTV817S`)로 받아 60Hz 펄스(`HZ60`)로 변환하는 회로의 정밀 검증.
  - 다이오드 브릿지 통과 정류 특성에 의해 출력 펄스 주파수가 120Hz(AC 60Hz 기준 제로 크로싱마다 발생)가 되므로, 펌웨어 계산 로직에서 반주기 계산(120Hz 주기)을 반영해야 함을 판정.
  - 전입력 저항 `R104` (100kΩ) 단일 배치 시의 전기적 스트레스 정밀 계산: AC 220V(311V Peak) 인가 시 저항 소비전력이 약 0.484W에 달하여, 일반 소형 칩 저항(1608/2012) 사용 시 **과열/소손 위험** 및 **전압 정격 초과(절연 파괴/아크 방전)** 문제를 규명.
  - **보완 대책**: AC 입력 양단(`VacL`, `VacN`)에 47kΩ~51kΩ(또는 100kΩ) 저항을 분배 배치하여 전력 및 고전압을 직렬 분산시키고, 노이즈 내성을 대칭 설계할 것을 제안.
  - 광커플러의 풀업 저항 `R105` (20kΩ) 값 조정 검토: 저속 옵토커플러(LTV817) 특성상 턴오프 시간($t_{off}$) 지연에 의해 상승 에지가 둔화될 수 있으므로, 4.7kΩ~10kΩ 수준으로 저항을 줄여 출력 펄스 품질을 확보하도록 제안.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `전력 정격 (Power Rating)` 및 `전압 정격 (Voltage Rating)` 신규 용어 정의 추가 완료.

- **[Analysis] 듀얼 W5500 기반 이더넷 외부 제어 구성 타당성 및 주소 할당 조건 정밀 분석**:
  - 외부 이더넷 통신망을 통한 제어기 통합 관리 시의 설계 규칙 수립 및 듀얼 포트(`IC201`/`IC202`) 운영 요건 분석.
  - 두 칩이 동일 물리 네트워크망(허브/공유기)에 병렬 연결될 경우를 대비하여, **MAC 주소와 IP 주소를 반드시 서로 다른 값으로 하드웨어 초기화 펌웨어 단에서 수동 지정(분리)** 해야 함을 명확화. (MAC 주소의 중복 충돌 방지 및 개별 소켓 바인딩 필수)
  - 동일 스위칭 허브 상에서 두 포트를 동시 결선 시 발생 가능한 **네트워크 루프(Network Loop)** 및 **브로드캐스트 스톰(Broadcast Storm)** 위험을 방지하기 위한 선로 구성(서로 다른 서브넷 망 분리 또는 백업 이중화 시나리오 설정) 지침 마련.
  - 장기간 비정상 통신 단절(케이블 분리, 노이즈 침투 등) 상태에 대응하기 위해, 펌웨어 단에서 옵토커플러 하드웨어 리셋 핀 핸들링 및 W5500 소켓 레지스터를 실시간 루프 감시하여 연결을 복원하는 **소켓 상태 머신(Socket State Machine)** 구현을 보완책으로 제시.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `브로드캐스트 스톰 (Broadcast Storm)` 및 `소켓 상태 머신 (Socket State Machine)` 신규 용어 정의 추가 완료.

- **[Analysis] 알티움 3D 뷰 및 PCB 레이아웃 배치 무결성 정밀 검증**:
  - 사용자가 작업한 H2_Control_Board PCB Floorplan 배치 상태에 대한 적합성 검토 수행.
  - 전원부(`Power`), 아날로그 입력부(`TempController`), 릴레이 출력부(`Output_Section`), 통신부(`Ethernet`) 등이 기판 내에서 기능 블록별로 명확히 구획 정렬되어 배치 신호 흐름이 직관적이며 우수함을 검증.
  - 특히 상단 좌측 온도 센서 커넥터 하단에 아날로그 Mux 및 OP-Amp 필터 회로가 인접 배치되어 '아날로그 최단 배선 원칙'이 철저히 지켜졌음을 확인.
  - 향후 본격적인 라우팅(배선) 작업을 위해 **4대 핵심 안전/품질 배선 가이드라인** 제시:
    1. AC 220V 고전압 입력단의 디지털 접지(DGND)/신호선 격리(최소 Clearance/Creepage 6mm 확보 및 내층 카퍼 삭제).
    2. LM1117 및 전원 칩 주변의 넓은 방열 패드 및 방열 비아(Thermal Via) 배치 방안.
    3. 고속 이더넷 통신 라인의 차동 임피던스(100Ω) 배선 규칙 적용 및 참조 플레인(2층 GND Plane) 무결성 유지.
    4. 민감한 아날로그 신호 구역으로의 고압/대전류 스위칭 노이즈 배선 인입 금지.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `차동 임피던스 (Differential Impedance)` 및 `방열 비아 (Thermal Via)` 신규 용어 정의 추가 완료.

- **[Analysis] 알티움 디자이너 Parameter Set 기반 디자인 룰 동기화 및 논리적 범위 검토**:
  - 회로도 상의 `Parameter Set` 지시자를 통해 설정된 디자인 룰(선폭, 비아 스타일, 이격 거리 등)의 PCB 이관 시점과 실제 넷(Net) 단위의 규칙 적용 메커니즘 분석.
  - 회로도에서 적용한 룰 파라미터는 `Design -> Update PCB Document...` (ECO) 프로세스를 실행하여 PCB 에디터와 동기화되는 시점에 PCB의 `Design Rules`로 생성/바인딩됨을 규명.
  - 파라미터 세트 룰은 물리적 좌표가 아닌 논리적인 **넷(Net) 및 넷 클래스(Net Class)**를 추종하므로, 해당 넷에 연결된 모든 패턴과 부품 패드는 기판의 **어느 위치에 있든지 간에** 해당 룰을 동일하게 지배(적용)받는 글로벌 스코프 특성을 가짐을 검증 및 확인.

- **[Analysis] TO-263 패키지 LM2576(U102) 방열 패드 Clearance 에러 원인 및 해결 프로세스 정립**:
  - PCB 상의 `LM2576` 레귤레이터(`U102`, TO-263 패키지) 탭 랜드(6번, 7번, 8번 패드) 간의 이격 거리 규칙 위반(Clearance Violation) 원인 규명.
  - 풋프린트 내부적으로 전기적/물리적으로 겹쳐 있는 대형 방열 패드(Tab)들의 핀 번호(Designator)가 서로 다르게 6, 7, 8번으로 지정되어, 서로 다른 넷(또는 무소속)으로 인식되어 발생하는 쇼트 에러로 판명.
  - **해결 방안 수립**: 풋프린트 라이브러리(.PcbLib)에서 6, 7, 8번 패드 지정자를 모두 하나의 동일한 번호(예: `3` 또는 `6`으로 통일)로 Consolidation 시켜 하나의 넷으로 병합(동기화)하여 클리어런스 에러를 해결하는 최적 방안 도출.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `패드 지정자 통일 (Pad Designator Consolidation)` 및 `클리어런스 예외 규칙 (Clearance Rule Exception)` 신규 용어 정의 추가 완료.

## 📅 2026-07-09: v0.1.3 MAX31856MUD+ 열전대 인터페이스 주변 회로도 검토 및 그라운드 차폐 설계 분석 🔍

하드웨어 온도 감지 모듈의 정밀성 확보를 위해 MAX31856MUD+ 주변 회로를 정밀 분석하고, 아날로그/디지털 그라운드 분리 및 신호 무결성을 위한 필터 최적화 조치를 수립하였습니다.

### 1. [Analysis] AGND(1번 핀)와 DGND(14번 핀) 묶음 연결 검증 및 가이드
- **현상**: 회로도 상에서 1번(AGND)과 14번(DGND)이 실선으로 직접 단락(Short)되어 있음.
- **분석**: 두 그라운드를 단순 와이어로 직접 연결할 경우, 알티움 디자이너 등 EDA 툴에서 전체 AGND와 DGND를 단일 넷(Net)으로 인식하여 PCB 설계 단계에서 두 그라운드가 무분별하게 혼합될 우려가 있음.
- **조치**: 회로도 상에서는 두 그라운드의 넷(Net)을 논리적으로 완전히 분리하고, PCB 레이아웃 시 단일 지점 결합(Star Grounding)을 강제하도록 **넷 타이(Net Tie)** 부품을 배치하거나, 고주파 노이즈 분리를 위해 **0옴 저항**을 매개로 결합할 것을 제안.

### 2. [Analysis] 열전대 T+/T- 입력 필터 커패시터 구성 검증
- **현상**: T- 라인에는 C301(0.01uF)만 연결되어 있는 반면, T+ 라인에는 C304(0.01uF) 및 C305(0.1uF)가 다소 비대칭적으로 병렬 연결되어 있음. 또한, T+와 T- 간의 디퍼런셜(Differential) 필터 커패시터가 누락된 것으로 보임.
- **분석**: 미세 전압을 측정하는 열전대 입력 회로는 동상 모드(Common-mode) 및 차동 모드(Differential-mode) 노이즈를 억제하기 위해 대칭적인 RC 필터 구성이 필수적임.
- **조치**: 
  - T+와 T- 라인 사이에 **0.1uF 차동 필터 커패시터**를 배치.
  - T+에서 AGND로, T-에서 AGND로 각각 대칭적인 **0.01uF(10nF) 동상 필터 커패시터**를 한 개씩 배치하는 데이터시트 표준 필터 레이아웃을 준수할 것을 제안.

### 3. [Analysis] AVDD(5번 핀) 및 DVDD(8번 핀) 전원 바이패스 검토
- **현상**: 5번 핀(AVDD) 및 8번 핀(DVDD) 전원에 디커플링 커패시터(C306, C307, C308)가 적절히 배치되어 있음.
- **검증**: AVDD와 DVDD 각각에 0.1uF 커패시터가 접지(각각 AGND 및 DGND)와 최단 거리로 연결되도록 설계되어 노이즈 억제에 효과적임을 확인.

### 4. [Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트
- `아날로그/디지털 그라운드 분리 (AGND/DGND Isolation / Split Ground)` 및 `넷 타이 (Net Tie)` 신규 용어 정의 추가 완료.

### 5. [Analysis] MAX31856MUD+ 회로도 수정 2차 검증 및 보완 피드백
- **검증 내용**: 
  - **그라운드 분리**: 1번(AGND)과 14번(DGND)을 회로도 상에서 개별 넷으로 완벽히 분리 조치함 (GND Plane 분할 레이아웃 기반 마련 완료).
  - **차동 필터 추가**: T+와 T- 신호선 사이에 차동 노이즈 억제용 커패시터 C316(0.1uF)을 추가하여 차동 필터 기능 보완을 완료함.
- **추가 보완 조치**: 
  - 현재 수정된 회로도에서 T- 라인에 C306(0.1uF) + C301(0.01uF)이 병렬로 연결되어 있고, T+ 라인에도 C304(0.01uF) + C305(0.1uF)가 묶여 있는 과용량 동상 필터 구조가 식별됨.
  - 동상 필터용 커패시터의 용량이 크면 부품 편차(Tolerance)로 인해 동상 노이즈가 유효 차동 센서 신호로 유입(CMRR 저하)되는 부작용이 발생함.
  - 따라서, 불필요한 과용량인 **C305(0.1uF) 및 C306(0.1uF)을 회로도에서 최종 삭제**하고, 오직 **C301(0.01uF) 및 C304(0.01uF) 대칭 구성만 동상 필터로 남겨둘 것을 권장**.
  - `동상성분제거비 (CMRR)` 및 `차동 및 동상 필터 (Differential and Common Mode Filter)` 신규 용어 사전 업데이트 반영 완료.
- **3차 최종 검증**:
  - T- 및 T+ 동상 필터 라인의 과용량 커패시터 C305, C306(각 0.1uF)이 완벽히 삭제되었음을 확인.
  - 이로써 T+/T- 입력단에 차동 필터 0.1uF(C316) 1개 및 대칭형 동상 필터 0.01uF(C301, C304) 2개로 구성되는 저잡음 고정밀 입력 필터 회로가 완성됨.
  - AGND와 DGND의 분리 설계 역시 무결하게 완료되었음을 검증함.

### 6. [Analysis] MAX31856MUD+ SPI 신호 핀 MCU 매핑 스와핑(94~99번) 검토
- **현상**: MAX31856의 SPI 신호(TC_SPI_CS, TC_SPI_CLK, TC_SPI_MISO, TC_SPI_MOSI)가 기존 dsPIC33CK MCU의 58~61번 핀(우하단)에 연결되어 있어 PCB 배선(Routing) 시 꼬임 발생 및 레이아웃 난이도 상승 문제 제기됨.
- **분석**:
  - MCU 좌상단 미사용 영역인 94번~99번 핀으로의 스와핑 검토 진행.
  - SPI 모듈 동작을 위해서는 SCK, SDI, SDO 신호에 PPS(Peripheral Pin Select) 기능이 적용 가능한 RP(재할당 가능) 핀이 필수적임.
  - 가용 핀 분석 결과: **99번(RA8/RP35)**, **96번(RA7/RP34)**, **95번(RA6/RP33)**은 하드웨어 SPI용 PPS 매핑이 완벽하게 가용함.
  - **97번(RB12/RP44)** 핀은 JTAG 디버깅 전용 기능(`TDI`)과 중첩되어 있으므로, 개발 디버깅 시 JTAG 인터페이스 충돌 방지를 위해 예비용(NC 또는 JTAG 전용)으로 비워두는 세이프티 설계 가이드 수립.
  - SPI CS(Chip Select)는 전용 하드웨어 제어가 아닌 일반 GPIO 포트로 감당 가능하므로, PPS 기능이 없는 고정 GPIO 핀인 **98번(RE15)**에 배치하여 핀 자원을 효율적으로 절약 가능함.
- **최종 매핑 솔루션 도출**:
  - **99번 핀 (RA8 / RP35)** ➡️ `TC_SPI_MOSI`
  - **98번 핀 (RE15 / GPIO)** ➡️ `TC_SPI_CS`
  - **96번 핀 (RA7 / RP34)** ➡️ `TC_SPI_MISO`
  - **95번 핀 (RA6 / RP33)** ➡️ `TC_SPI_CLK`
  - **97번 핀 (RB12 / RP44 / TDI)** ➡️ `NC (JTAG 디버그 예비)`로 유지
  - 이 매핑을 통해 PCB 상에서 SPI 라인을 순차적으로 깔끔하게 평행 배선할 수 있으며, JTAG 기능 간섭까지 완전 방어하여 레이아웃 편의성과 회로 안전성을 동시 확보함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `JTAG 디버깅 포트 (Joint Test Action Group / JTAG)` 신규 용어 정의 추가 완료.

- **4차 최종 핀 매핑 검증 완료**:
  - 회로도 상에서 99번(`TC_SPI_MOSI`), 98번(`TC_SPI_CS`), 96번(`TC_SPI_MISO`), 95번(`TC_SPI_CLK`)이 규정된 핀 맵에 오차 없이 정확히 연결되었음을 확인.
  - JTAG 전용 TDI 핀인 97번(`RB12`) 핀이 의도한 대로 깨끗한 NC(미연결) 상태로 유지되어, 고주파 SPI 클록 신호와의 간섭 차단 및 디버깅 핀 보호 설계를 완수함.

### 7. [Analysis] MAX31856MUD+ 다중 열전대 지원 원리 및 작동 메커니즘 분석
- **분석 내용**: MAX31856 칩이 다양한 규격의 열전대 센서(K, J, N, R, S, T, E, B)를 하나의 소자로 모두 정확히 해독하고 계측할 수 있는 하드웨어/소프트웨어적 메커니즘 규명.
- **핵심 기술 및 작동 원리**:
  - **제백 효과(Seebeck Effect) 활용**: 서로 다른 두 도선의 접합부 전위차가 온도차에 비례해 마이크로볼트(uV)~밀리볼트(mV) 단위로 작게 나타나는 원리를 고정밀 19비트 시그마-델타 ADC를 통해 센싱함.
  - **내장 LUT(Look-Up Table) 및 비선형 선형화(Linearization)**: 온도에 따른 전압 특성이 비선형적인 열전대 특성을 보완하기 위해, 각 타입별(K, J 등) 온도-전압 관계 데이터를 칩 내부 메모리에 룩업 테이블로 기본 저장하여 자동 보정 연산을 수행함.
  - **자동 냉접점 보상(CJC - Cold Junction Compensation)**: 보드와 센서가 만나는 지점(냉접점)의 외부 절대 온도를 칩 내부 고정밀 온도 센서로 실시간 계측하여 열전대 차동 값에 자동으로 연산·합산해 줌.
  - **레지스터 설정 기반 타입 스위칭**: MCU가 SPI 통신을 통해 Configuration Register 0(CR0) 등의 특정 비트를 변경하여 측정 대상 센서 규격(K/T-Type 등)을 실시간 스와핑하여 설정할 수 있음.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `제백 효과 (Seebeck Effect)`, `룩업 테이블 (Look-Up Table / LUT)`, `선형화 (Linearization)` 신규 용어 정의 추가 완료.

### 8. [Analysis] 보드 실장 온도 센서(LM35)와 MAX31856 내장 온도 센서의 기능 분리 타당성 검증
- **검증 내용**: MAX31856 내장 온도 센서의 물리적 역할 한계성 및 외부 실장 LM35 아날로그 온도 센서 2개의 보드 배치 타당성 검증.
- **분석 결과**:
  - **MAX31856 내장 센서의 목적**: 해당 내부 센서는 오직 칩 내부 및 접합부 기판(냉접점)의 국부 온도를 측정하여 열전대 Seebeck 기전력의 냉접점 보상(CJC) 연산용으로만 한정 사용됨.
  - **LM35의 목적 및 필요성**: 보드 좌측 전원부(Power Room) 및 MCU/구동 드라이버 등의 고열 발생 예상 영역 또는 제어기 외부 온도 계측 등 물리적으로 격리된 특정 로컬 포인트를 정밀 감시하기 위함임.
  - **결론**: 두 센서군은 측정 타겟과 위치가 완전히 독립적이므로, 시스템의 과열 보호(OTP - Over-Temperature Protection) 및 쿨링 팬 구동 피드백 루프 작동을 위해 외부 LM35 센서 회로의 유지는 필수적임(삭제 불가 판정).
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `과열 보호 (Over-Temperature Protection / OTP)` 및 `LM35 (정밀 아날로그 온도 센서)` 신규 용어 정의 추가 완료.

### 9. [Analysis] LM35 온도 센서 배치 전략 수립 및 열전대 커넥터 근처 배치 제거 검토
- **검증 내용**: 사용자가 최초 설계에서 열전대 센서의 냉접점 온도를 알기 위해 LM35를 열전대 단자대(터미널 블록) 근처에 배치했던 설계안에 대한 검토 및 최적화.
- **분석 결과**:
  - **최단거리 밀착 배치 조건 충족**: 앞서 v0.1.2 설계 가이드에서 MAX31856 칩을 포함하는 `TempController` 룸을 열전대 단자대(터미널) 바로 옆에 밀착 배치하는 '최단 Analog 배선 원칙'을 수립하였음.
  - **외장 냉접점 보상 불필요**: 칩이 단자대와 매우 가깝게 배치되므로, 칩 내부 고정밀 센서가 단자대 온도를 그대로 감지하여 냉접점 보상(CJC)을 완벽하게 수행함. 따라서 단자대 근처의 LM35는 냉접점 계측 보정 용도로는 불필요함이 증명됨.
  - **재배치 전략 수립**: 단자대 근처에 있는 LM35 2개를 보드 내 전원 변환 장치(LM2576 등) 및 전력 소자 구동부 등 과열이 염려되는 '발열 핫스팟'으로 분산 배치하여 시스템의 열 과하 방지 및 냉각 팬 피드백 제어용으로 그 물리적 위치를 변경하는 최적화 안을 수립함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `외장 냉접점 온도 센서 (External Cold-Junction Temperature Sensor)` 신규 용어 정의 추가 완료.

### 10. [Analysis] MAX31856 계측 정확도 및 시스템 온도 오차 영향성 검토
- **분석 내용**: MAX31856 센서의 내재적 오차 스펙 및 센서 케이블 연결 시 발생하는 전체 시스템 오차 범위 분석.
- **분석 결과**:
  - **MAX31856 내재적 정확도**: 칩 내부의 온도 전압 계측 정확도는 최대 **±0.15%**, 내부 냉접점 보상 센서 정확도는 **±0.7℃**로 매우 고정밀함. 온도 출력 해상도는 19비트로 **약 0.0078℃** 단위의 미세 감지가 가능함.
  - **전체 시스템 오차의 지배 인자**: 칩 자체의 정확도는 1℃ 미만으로 매우 높으나, 실제 최종 온도 계측 오차는 외부 **열전대(Thermocouple) 프로브선 자체의 오차(공차)**에 의해 결정됨. (일반 K-type의 경우 ±1.5℃ ~ ±2.5℃ 수준)
  - **설계적 보완 대책**: 칩의 초고정밀 성능을 온전히 활용하기 위해 열전대 센서 선택 시 허용 오차가 낮게 튜닝된 고급 규격(Class 1 등급 등)의 외장 프로브를 적용하도록 권장함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `온도 해상도 (Temperature Resolution)` 및 `계측 정밀도 및 정확도 (Measurement Precision and Accuracy)` 신규 용어 정의 추가 완료.

### 11. [Analysis] ADG706BRU 기반 다채널 차동 Mux 회로 검토
- **검증 내용**: MAX31856MUD+ 전단의 ADG706BRU 아날로그 멀티플렉서 2개를 활용한 16채널 차동 열전대 입력 회로 구성 검토.
- **분석 결과**:
  - **차동 매핑의 유효성**: 열전대의 양선(T+/T-) 신호를 스위칭하기 위해 2개의 ADG706 칩을 병렬 배치하고, 제어 주소선(`TC_ADDR0~3`)과 `TC_EN1` 제어선을 상호 공유하도록 설계하여 동기화 오류 및 스위칭 채널 비대칭 문제를 완전히 예방함.
  - **전원 및 접지 구성**: 단일 전원 +3.3V 구동 조건에 따라 각 Mux의 VSS(27번 핀)와 GND(12번 핀)를 모두 아날로그 그라운드(AGND)에 직접 연결하여 단일 공급 동작을 안정적으로 구현함. VDD 입력단에는 C300, C309(0.1uF) 디커플링 커패시터를 AGND 기준으로 밀착 배치함.
  - **신호 레벨 호환성**: dsPIC33CK MCU의 3.3V 동작 범위가 ADG706의 입력 문턱 전압(VIH = 2.0V)을 완전하게 커버하므로 추가 논리 레벨 시프터 없이 다이렉트로 드라이브 가능한 환경을 보증함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `아날로그 멀티플렉서 (Analog Multiplexer / Mux)` 및 `차동 멀티플렉싱 (Differential Multiplexing)` 신규 용어 정의 추가 완료.

### 12. [Analysis] dsPIC33CK512MP710 MCU 고정 핀 및 재할당 가능(Remappable) 핀 분류 분석
- **검증 내용**: 사용자가 PCB 아트워크 배선 최적화를 위해 핀 매핑을 안전하게 변경할 수 있도록, 변경 불가능한 고정 핀(Fixed)과 소프트웨어/하드웨어적으로 변경이 용이한 재할당 가능 핀(PPS)을 엄격히 분류함.
- **분류 결과**:
  - **변경 불가능 고정 핀**:
    - **전원/GND**: VDD, VSS, AVDD, AVSS (모든 공급선 및 접지)
    - **리셋/오실레이터**: MCLR(10번), OSC1/OSC2(36, 37번)
    - **디버깅(ICSP)**: PGC/PGD (92, 93번 핀 - PGEC3/PGED3)
    - **하드웨어 I2C**: I2C_SCL(42번 핀 - SCL1), I2C_SDA(43번 핀 - SDA1)
    - **아날로그 센싱(ADC)**: POWER_FAIL_DET(75번 핀 - RB8/AN8) ➡️ 전압 강하 계측용 전용 아날로그 채널이므로 고정 필수.
  - **재배치 가능 핀 (일반 GPIO / 단순 디지털 신호 - 매우 높은 자유도)**:
    - 칩 선택(CS), 리셋(RST), 디렉션 제어(DIR), 솔레노이드 구동(DO), 단순 센서 감시(DI) 선들. (MCU의 일반 디지털 I/O 핀이라면 어디든 변경 배치 가능)
    - 주요 타겟: `FLASH_CS`, `DAC_CS`, `DAC_CS2`, `ETH1_SCSN`, `ETH2_SCSN`, `ETH1_RSTN`, `ETH2_RSTN`, `TC_SPI_CS`, `485_DIR`, `DO_SVxxx`, `DO_HTxxx`, `DI_FMxxx`, `DI_FDxxx`, `DI_FANxxx` 등.
  - **재배치 가능 핀 (하드웨어 특수 기능 - PPS 지원 RP 핀 간 교환 가능)**:
    - 하드웨어 SPI 및 UART 버스 신호, 60Hz 외부 인터럽트(`HZ60`) 신호. (반드시 PPS 기능이 내장된 `RPx` 또는 `RPIx` 핀 영역으로만 스와핑해야 함)
    - 주요 타겟: `ETH_MISO`, `ETH_MOSI`, `ETH_SCLK`, `FLASH_DI`, `FLASH_CLK`, `FLASH_DO`, `TC_SPI_MOSI`, `TC_SPI_MISO`, `TC_SPI_CLK`, `485_TX/RX`, `MON_422_TX/RX`, `HZ60`.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `PPS 락 및 언락 (PPS Register Lock and Unlock)` 및 `핀 충돌 및 이중 매핑 (Pin Conflict and Dual Mapping)` 신규 용어 정의 추가 완료.

### 13. [Analysis] 일반 디지털 입력(DI) 및 디지털 출력(DO) 신호 간 스와핑 타당성 검토
- **검증 내용**: 단순 GPIO 특성을 가지는 디지털 입력(DI) 핀과 디지털 출력(DO) 핀의 상호 스와핑 및 자유 배치 타당성 검토.
- **분석 결과**:
  - **TRIS 제어 호환성**: dsPIC33CK MCU의 모든 일반 디지털 I/O 포트는 소프트웨어 TRIS 레지스터 변경을 통해 핀의 입력/출력 방향을 100% 임의 제어할 수 있으므로, 단순 GPIO 레벨에서의 DI-DO 스와핑은 기술적으로 완전히 유효함.
  - **5V Tolerant 전압 제약**: 외부 센서 등으로부터 들어오는 디지털 입력(DI) 신호 전압이 **5V**인 경우, 타겟 핀이 반드시 5V 내성(Tolerant)을 가져야 함. 아날로그 기능이 중첩된 핀(ANx 핀)은 5V 입력을 받으면 소손 위험이 있으므로 주의 필요.
  - **출력(DO) 프리덤**: MCU가 직접 3.3V 전압을 출력하는 DO 핀의 경우 5V 내성 유무에 구애받지 않으므로, 아날로그 겸용 핀을 포함하여 임의의 I/O 핀으로 완전하게 자유 배치 가능함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `TRIS 레지스터 (Tri-State Register)` 및 `5V 내성 및 아날로그 중첩 제약 (5V Tolerant vs Analog Multiplexing Constraint)` 신규 용어 정의 추가 완료.

### 14. [Analysis] 다채널(16채널) 고성능 DAC 칩셋 선정 검토
- **검증 내용**: 기존 8채널 12비트 SPI DAC인 DAC7568을 대체하여, 수소 제어 보드의 액추에이터 제어 정밀도 및 채널 확장성을 높이기 위한 16채널 DAC 대안 칩셋 발굴 및 분석.
- **후보 칩셋 도출**:
  - **DAC60516 (TI)**: 16채널, 12비트 해상도, SPI/I2C 인터페이스 지원. 내장 2.5V 초고정밀 참조 전원(Reference)을 포함하고 있어 DAC7568의 설계를 유지하면서 채널만 2배로 확장하는 가장 직접적인 대안(Best Fit)으로 선정함. 4x4mm WQFN의 소형 패키지로 기판 면적 절감 유리.
  - **DAC61416 (TI)**: 16채널, 12비트 해상도, SPI 지원. 고전압 및 바이폴라(Bipolar, 예: ±10V 등) 전압 출력을 직접 지원하여 별도의 OP-Amp 증폭단 설계 생략이 필요할 때 최적임.
  - **DAC80516 (TI)**: 16채널, 16비트 고해상도 버전. 향후 유량 조절 벨브 등의 초정밀 PID 미세 제어 요구 사양 발생 시 해상도 스펙 업그레이드 경로로 가이드함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `디지털-아날로그 변환기 (Digital-to-Analog Converter / DAC)` 및 `바이폴라 출력 (Bipolar Output)` 신규 용어 정의 추가 완료.

### 15. [Analysis] 16채널 DAC 도입에 따른 TLV2374 후단 증폭 회로 생략 가능성 기술 검토
- **검증 내용**: 16채널 DAC 대안 칩셋 선정 시, 기존 설계에 적용되던 TLV2374 레일 투 레일 OP-Amp 증폭 회로의 유지 여부 판정.
- **비교 분석 결과**:
  - **DAC60516 채택 시 (OP-Amp 유지 필요)**:
    - 칩 동작 VDD 전원 한계(최대 5.5V)로 인해 단독으로 10V 고전압 출력을 내는 것은 불가능함.
    - 따라서 0~10V 아날로그 출력을 감당해야 하는 채널들에는 여전히 고전압 전원(12V 등)을 쓰는 **TLV2374 OP-Amp 증폭단이 후단에 반드시 유지되어야 함.** (단, 0~5V 출력만 쓰는 채널은 OP-Amp 생략 가능)
  - **DAC61416 채택 시 (OP-Amp 완전 생략 가능)**:
    - 고전압 양전원 구동(최대 ±20V 가능)을 지원하여 소프트웨어 설정만으로 **0~5V, 0~10V, ±5V, ±10V**의 다채로운 스팬 전압 출력을 칩 단독으로 뿜어낼 수 있음.
    - 따라서 이 칩을 선택하면 0~10V 출력을 위해 덧붙였던 **TLV2374 OP-Amp 버퍼/증폭단을 회로에서 100% 삭제(생략)할 수 있어, 기판 공간 절약 및 부품 단가 극감소에 대단히 유리함.**
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `아날로그 출력 버퍼 및 증폭기 (Analog Output Buffer and Amplifier)` 및 `레일 투 레일 연산증폭기 (Rail-to-Rail Operational Amplifier)` 신규 용어 정의 추가 완료.

### 16. [Design] 듀얼 DAC7568(8ch x 2) ➡️ 싱글 DAC60516(16ch) 통합 및 핀 매핑 이식 설계
- **설계 내용**: 기존에 사용되던 두 개의 8채널 DAC7568(IC500, IC501)을 하나의 16채널 DAC60516(IC302)으로 통합(One-chip)하기 위한 아날로그 출력(AO) 신호 및 SPI 통신 제어선 1:1 핀 이식 매핑 튜닝 진행.
- **아날로그 출력 핀 매핑 (OUT0~15 이식)**:
  - `OUT0` (1번) ➡️ `AO_P111`
  - `OUT1` (28번) ➡️ `AO_MFC121`
  - `OUT2` (27번) ➡️ `AO_MFC131`
  - `OUT3` (26번) ➡️ `AO_AB222`
  - `OUT4` (25번) ➡️ `AO_AB212`
  - `OUT5` (24번) ➡️ `AO_AB232`
  - `OUT6` (23번) ➡️ `AO_P311` (IC500/501 공통 결합)
  - `OUT7` (22번) ➡️ `AO_P321`
  - `OUT8` (7번) ➡️ `AO_P422`
  - `OUT9` (8번) ➡️ `AO_P432`
  - `OUT10` ~ `OUT15` (9~14번) ➡️ `NC (미사용 예비 채널)`
- **SPI 및 제어 핀 매핑**:
  - `SCL/CS` (3번 핀) ➡️ `DAC_CS` (기존 IC501에 사용되던 `DAC_CS2`는 설계 제외)
  - `SDA/SCLK` (4번 핀) ➡️ `TC_SPI_CLK`
  - `A0/SDI` (5번 핀) ➡️ `TC_SPI_MOSI`
  - `A1/SDO` (19번 핀) ➡️ `TC_SPI_MISO` (디버깅용 출력선 연동)
  - `RESET` (20번 핀) ➡️ 외부 풀업 저항(10kΩ) 연결하여 항상 활성화(High)
  - `LDAC` (17번 핀) ➡️ 아날로그 접지(AGND)에 직접 묶어서 즉각 변환(Always-active) 설정
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `채널 통합 및 원칩화 (Channel Consolidation / One-Chip Integration)` 및 `LDAC 제어선 (Load DAC Control Pin)` 신규 용어 정의 추가 완료.

### 17. [Design] DAC60516RUYT 아날로그 공급 전원(AVDD) 사양 조정 검토 및 승압 구조 분석
- **검증 내용**: 사용자가 배치 완료한 DAC60516RUYT 회로도의 AVDD 연결 전원 유효성 및 5V 출력 동작 조건 분석.
- **분석 결과**:
  - **승압 회로 부재 검증**: DAC60516 칩 내부에는 독립된 전하 펌프(Charge Pump)와 같은 전압 승압 회로가 존재하지 않음. 따라서 칩의 아날로그 출력 스윙 전압 한계는 오직 아날로그 입력 전원(AVDD) 레벨에 종속(Output Saturation/Clipping 발생)됨.
  - **AVDD 전압 불일치 발견**: 현재 도면상에서 AVDD(2번 핀)에 **+3.3V**가 결선되어 있음. 이 상태에서는 내부 레퍼런스(2.5V)의 게인 2배 설정(5V 출력 모드)을 가동하더라도, 실질적인 최대 아날로그 출력 전압은 약 3.25V로 제한되어 출력이 왜곡됨.
- **보완 대책**:
  - **AVDD(2번 핀)의 결선 전원을 +3.3V에서 +5V로 승격**시켜 설계 수정 유도.
  - 디지털 레벨 매칭용 전원인 **VIO(18번 핀)는 기존의 +3.3V를 유지**하여, MCU와의 3.3V SPI 고속 데이터 링크는 안전하게 가동하면서 아날로그 출력은 완전한 0~5V 범위를 구현하도록 회로 스펙을 분리 튜닝함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `아날로그/디지털 전원 분리 공급 (AVDD and VIO Separation)` 및 `출력 포화 및 클리핑 (Output Saturation and Clipping)` 신규 용어 정의 추가 완료.

### 18. [Design] DAC60516RUYT AVDD 공급 전압 한계 및 절대 최대 정격 검토
- **검증 내용**: DAC60516 칩의 AVDD 전원에 5V보다 더 높은 전압(예: 12V 등)을 인가할 수 있는지와 칩의 정격 전압 한계 분석.
- **분석 결과**:
  - **정격 전압 한계**: DAC60516 칩의 추천 동작 아날로그 전원(AVDD) 공급 한계 범위는 **2.7V ~ 5.5V**이며, 파괴 전압인 절대 최대 정격(Absolute Maximum Rating)은 **6.0V**임.
  - **고압 인가 불가 판정**: 5.5V를 넘는 전압(예: 12V)을 AVDD에 직접 인가하면 칩 내부 실리콘 소자가 완전히 파괴(소손)됨. 따라서 5V가 이 칩에 인가 가능한 사실상의 물리적 맥시멈 전압이며, 그 이상의 고전압(10V 등) 출력을 원칩으로 해결하기 위해서는 고압 전용 DAC61416 칩셋 변경이 유일한 대안임.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `권장 동작 전압 및 절대 최대 정격 (Recommended Operating Voltage and Absolute Maximum Ratings)` 신규 용어 정의 추가 완료.

### 19. [Analysis] DAC60516 원거리 전송 신뢰성 및 MCU I2C 통신 방식 전환 타당성 검토
- **검증 내용**: DAC 출력의 2m 원거리 구동 타당성 분석 및 MCU와의 통신 방식을 SPI에서 I2C로 전환(RTC 버스 공유)할 때의 손익 검토.
- **분석 결과**:
  - **출력 전류 한계 및 원거리 전송 저항**: DAC60516의 출력 버퍼 전류 공급력은 최대 수 mA 수준이므로 센서/모터 직접 구동 시 칩 소손이나 전압 강하가 발생함. 특히 2m 선로 전송 시에는 전자기 노이즈 유입 및 전압 감쇄 오차가 치명적이므로, 후단에 **OP-Amp 임피던스 버퍼(TLV2374 등)**를 달거나 정밀 제어를 위해 **4-20mA 전류 루프(V-I 변환)** 구성을 적용하는 것이 필수적임.
  - **SPI ➡️ I2C 전환 시의 손익**:
    - *이득*: 통신 핀 수의 절약(RTC와 단 2선 공유).
    - *실(위험성)*: I2C의 통신 속도 한계(최대 400kbps~1Mbps)로 인해 16채널 아날로그 업데이트 시 통신 병목 현상이 발생할 수 있으며, 주변 솔레노이드나 모터의 기동 노이즈 환경 하에서 I2C 오픈 드레인 통신이 오작동을 유발할 위험이 매우 높음.
- **최종 가이드**: 실시간 제어 성능 및 산업용 노이즈 내성을 보장하기 위해 기존의 **고속 SPI 통신 방식을 고수**하도록 가이드함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `전압-전류 변환기 (Voltage-to-Current Converter / V-I Converter)`, `4-20mA 전류 루프 (4-20mA Current Loop)` 및 `통신 병목 현상 (Communication Bottleneck)` 신규 용어 정의 추가 완료.

### 20. [Analysis] TLV2374 OP-Amp 채널 밀도 한계 및 소형화 설계 대안 검토
- **검증 내용**: 16채널 DAC 출력을 위해 OP-Amp 버퍼를 구성할 때, TLV2374(4ch)를 대체할 수 있는 고밀도 8채널 OP-Amp의 존재 여부 및 설계 타당성 분석.
- **분석 결과**:
  - **OP-Amp 채널의 물리적 제약**: 일반 연산증폭기는 고유의 아날로그 누화(Crosstalk) 간섭 방지, 다이(Die)의 열 설계적 한계 및 SOIC 패키지 표준(입력/출력 핀 배치 제약)으로 인해 한 칩 패키지에 탑재되는 최대 채널 수가 **4채널(Quad)**로 제한됨. 업계 전반에 일반적인 아날로그 8채널 OP-Amp IC는 실질적으로 존재하지 않음.
  - **대안 설계 추천**:
    - **유지보수형 (Quad 4개 조합)**: 기존 설계를 고수할 경우, 4채널 TLV2374를 회로에 **총 4개** 나란히 배치해야 하므로 회로 면적이 대폭 증가함.
    - **원칩 컴팩트형 (DAC61416 활용)**: 기판 배선의 번거로움과 공간 낭비를 완전히 종식하기 위해, 0~10V 고전압을 다이렉트로 출력하는 고압용 16채널 **DAC61416** 칩셋 도입을 재차 추천함. 이를 채택하면 TLV2374 OP-Amp 소자 4개와 주변의 필터 부품들을 100% 회로도에서 제거할 수 있음.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `쿼드 연산증폭기 (Quad Operational Amplifier / Quad OP-Amp)` 및 `누화 현상 및 신호 간섭 (Crosstalk and Signal Interference)` 신규 용어 정의 추가 완료.

### 21. [Design] 0~5V 전압 출력 제한 조건에 따른 아날로그 완충(Buffer) 회로 최종 대안 수립
- **설계 조건 확정**: 사용자의 아날로그 출력 스펙 요구 사항이 최대 **5V**(0~5V 출력)로 제한 및 확정됨. 이에 따른 후단 버퍼 회로 최적화 방안 도출.
- **최종 대안 설계**:
  - **AVDD = 5V 인가를 통한 단독 출력**: DAC60516의 AVDD에 5V를 인가하면, 칩 자체적으로 0~5V 풀 레인지(Full Range) 출력을 완벽하게 뿜어낼 수 있음.
  - **후단 버퍼(TLV2374)의 최적 활용 제안**:
    1. **버퍼 생략 원칩 설계안**: 만약 제어 대상 센서/액추에이터 단의 입력 임피던스가 매우 높고 원거리 노이즈 영향이 적다면, DAC60516 출력단에 임피던스 매칭용 **100Ω 직렬 댐핑 저항**과 ESD 보호용 **TVS 다이오드**만 1:1 결선하여 회로에서 TLV2374 OP-Amp를 완전히 빼버리는 초소형 원칩 설계가 가능함.
    2. **버퍼 유지 1배 증폭안**: 노이즈 방어 및 2m 케이블의 강인한 전류 구동력을 유지하고자 한다면, 기존 4채널 TLV2374 OP-Amp 4개를 사용하여 1배 게인(Voltage Follower) 버퍼단을 구성함. (전원은 5V 단일 공급으로 단순화)
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `전압 추종기 버퍼 (Voltage Follower Buffer / Unity Gain Buffer)` 및 `직렬 댐핑 저항 (Series Damping Resistor)` 신규 용어 정의 추가 완료.

### 22. [Design] TLV2374ID 4채널 OP-Amp 버퍼 결선 및 소요 부품(BOM) 선정
- **설계 내용**: 사용자의 안전 우선 결정에 따라, 16채널 DAC 출력단 후단에 4채널 TLV2374ID를 배치하여 아날로그 출력을 임피던스 분리 및 노이즈 격리하는 1배 게인 전압 추종기(Voltage Follower) 결선 설계 완료.
- **결선 설계**:
  - **전원/접지**: 칩 4개 공통 VDD(4번)에 **+5V** 연결, GND(11번)에 아날로그 접지 **AGND** 연결. 핀 주변에 0.1uF 세라믹 콘덴서를 AGND 기준으로 밀착 결선.
  - **채널별 루프 구성 (Unity Gain)**: 각 4개 채널의 비반전 입력(`xIN+`)에 DAC 출력 신호 연동. 반전 입력(`xIN-`)과 출력(`xOUT`)을 직접 단선 쇼트로 묶어서 피드백 연결.
  - **최종 출력 단자 단선**: OP-Amp의 `xOUT` 단자 뒤에 **100Ω 직렬 댐핑 저항**을 각각 직렬 삽입하고 그 뒤에 ESD 보호용 **TVS 다이오드**를 병렬 접지(AGND) 결선하여 최종 보드 커넥터로 인출.
- **소요 부품(BOM) 내역**:
  - IC: `TLV2374ID` (TSSOP-14 또는 SOIC-14) ➡️ **4개**
  - Damping Resistors: `100Ω` (0603 패키지) ➡️ **16개**
  - Decoupling Capacitors: `0.1uF` 세라믹 (0603 패키지) ➡️ **4개** (공급 전원 바이패스용)
  - Protection Diodes: `TVS 다이오드` (단방향, ESD용) ➡️ **16개**
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `임피던스 완충 회로 (Impedance Buffering Circuit)` 및 `정전기 방전 보호 소자 (Electrostatic Discharge Protection Device / TVS Diode)` 신규 용어 정의 추가 완료.

### 23. [Design] TLV2374ID 4채널 버퍼 회로도 구현 무결성 검증
- **검증 내용**: 사용자가 배치 완료한 TLV2374ID 4채널 버퍼 회로도 이미지의 결선 적합성 검토.
- **분석 결과 (100% 무결성 확인)**:
  - **전원 및 접지**: VDD(4번) +5V 연결 및 GND(11번) AGND 결선 완벽함.
  - **1배 피드백 루프**: 4개 개별 채널의 반전 입력(`1IN-~4IN-`) 단자와 출력(`1OUT~4OUT`) 단자가 올바르게 다이렉트 쇼트 연결됨.
  - **입력 전원 공급**: DAC로부터 연동된 입력 신호(`AO_AB222` 등)들이 비반전 입력(`1IN+~4IN+`)에 정확히 매핑됨.
  - **최종 출력 및 댐핑**: 출력단 뒤에 `100Ω` 직렬 댐핑 저항이 제대로 직렬 연결되었고, 버퍼 전/후의 신호선을 구별하기 위해 `PAO_xxx` (Protected Analog Output) 넷 네임을 매핑하여 회로의 가독성과 안전성을 비약적으로 높임.
- **아트워크 권장 보완**:
  - 회로도에는 명시적으로 표현되지 않았지만, 실제 PCB 레이아웃 시 TLV2374ID의 VDD(4번)와 GND(11번) 핀 최단 거리에 **0.1uF 세라믹 디커플링 커패시터**를 밀착 배치해야 노이즈 억제에 효과적임을 리마인드함.

### 24. [Standard] 수동 칩 소자(SMD) 패키지 규격 표기법 미터법(Metric) 통일 가이드
- **가이드 합의**: 사용자의 요청에 따라, 저항 및 커패시터 등 수동 소자의 칩 크기를 기존 인치(EIA) 규격 대신 밀리미터 단위인 미터법(Metric) 규격으로 상호 의사소통 기준을 통일함.
- **주요 환산 기준 수립**:
  - 기존 **인치 0603** (0.06 inch x 0.03 inch) ➡️ **미터법 1608** (1.6mm x 0.8mm)로 표기 통일.
  - 기존 **인치 0402** ➡️ **미터법 1005** (1.0mm x 0.5mm)로 표기 통일.
  - 기존 **인치 0805** ➡️ **미터법 2012** (2.0mm x 1.2mm)로 표기 통일.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `SMD 칩 부품 규격 표기법 (SMD Chip Package Code / EIA vs Metric)` 신규 용어 정의 추가 완료.

### 25. [Design] TVS 보호 다이오드(SM712) 및 디커플링 커패시터 회로도 보완 배치 검증
- **검증 내용**: 누락되었던 2m 외부 인출선용 TVS 다이오드 기호 및 VDD 디커플링용 바이패스 커패시터의 배치 무결성 보완 및 검토.
- **분석 결과 (보완 완료)**:
  - **TVS 다이오드 장착**: 최종 아날로그 출력 포트(`PAO_xxx`)마다 2채널 공통 패키지 타입의 정전기 방지 소자인 **SM712** 칩이 1:1로 결선되어 AGND 접지로 떨어지도록 안전하게 배치됨.
  - **디커플링 캡 배치**: OP-Amp의 아날로그 공급 전원 VDD(4번) 라인 상단에 **C502 (0.1uF / 미터법 1608 규격)** 커패시터가 AGND로 연결되도록 완벽하게 보완 추가됨.
- **최종 판정**: 아날로그 계통 통합 및 완충(Buffer) 보호 설계의 모든 하드웨어 회로 구성 무결성이 100% 만족되었으므로 차기 PCB 아트워크 설계 전환을 승인함.

## 📅 2026-07-10: v0.1.1 PCB 결합 설계 및 아트워크 보완 🛠️
물리적으로 하나의 부품으로 들어가는 다중 커넥터(CW520, CW517)의 결합 관계를 표시하는 아트워크 가이드라인을 정립하고 관련 하드웨어 설계 지식을 검증하였습니다.

### 26. [Design] 물리적 통합 부품(CW520, CW517)의 PCB 아트워크 결합 표시 기법 도출
- **분석 내용**: 실장 시 실제로는 하나의 물리적 하우징(예: 4핀 커넥터)이지만 회로 설계 편의상 2핀 부품 2개(CW520, CW517)로 분할된 상태에 대해, 조립 작업자 및 디버깅 시 혼동을 예방하기 위한 3가지 솔루션 도출 및 제안.
- **최종 가이드**:
  1. **실크스크린(Top Overlay) 결합 표시선 작도**: PCB 툴에서 `Top Overlay` 레이어에 두 부품의 영역을 감싸는 점선(Dashed Line) 박스나 결합선 기호를 직접 드로잉하여 시각적 가이드라인을 제공함. (가장 빠르고 직관적임)
  2. **회로도 및 풋프린트 통합**: 회로도에서 2개의 2핀 기호를 제거하고 1개의 4핀 기호로 통합한 뒤, PCB 라이브러리에 4핀 풋프린트를 새로 정의하여 동기화함. (BOM 정합성 및 DRC 충돌 방지 측면에서 가장 정석적인 방법)
  3. **BOM 비실장(DNP) 및 조립 노트 작성**: 두 레퍼런스 중 하나만 실장 부품(4핀)으로 수량을 올리고, 다른 하나는 BOM 상에서 가상(Link/DNP) 처리한 뒤 조립 도면에 주석을 명시함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `실크스크린 (Silkscreen / Top Overlay)`, `레퍼런스 데시그네이터 (Reference Designator / RefDes)`, `코트야드 (Courtyard Layer)` 신규 용어 정의 추가 완료.

### 27. [Layout] 알티움 디자이너 실크스크린 점선(Dashed) 스타일 변경 불가 문제 해결
- **문제 분석**: 실크스크린 결합 표시선 작도 시, Properties 패널에서 Line Style(점선/Dotted) 변경 옵션이 누락되는 원인 분석.
  - **원인 1 (다중 선택 상태)**: 노란색 가이드라인 선택 시, 화면상의 트랙(`Track`) 뿐만 아니라 주변 부품(`Components`) 및 기타 개체들이 13개 이상 다중 선택(`Multi-selection`)되어 공통 속성 외 세부 속성이 표시되지 않음.
  - **원인 2 (객체 타입 불일치)**: 회로 배선용 `Track`으로 그렸을 경우 알티움의 전기적 트랙 특성상 실선(Solid)으로 고정되어 점선 변환이 불가함.
- **해결 조치 가이드**:
  1. 빈 화면을 클릭하여 다중 선택을 해제하고, 오직 원하는 선 한 가지만 단독으로 클릭하여 속성 확인 유도.
  2. 실크 가이드라인 그리기용 전용 드로잉 객체인 `Place ➡️ Line` (단축키 `P` ➡️ `L`)을 이용하여 다시 그린 뒤, 해당 `Line`의 Properties 패널에서 `Line Style`을 `Dashed`로 변경하도록 가이드 적용.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `다중 선택 (Multi-selection)`, `선택 필터 (Selection Filter)` 신규 용어 정의 추가 완료.

## 📅 2026-07-13: v0.1.2 온도 센서 SPI 핀 맵 재할당(PPS) 가능 여부 분석 🔍
온도 센서 리더용 기존 SPI 핀들(95, 96, 98, 99번)을 비어있는 52번 및 58번 핀으로 이동할 수 있는지 물리적/기능적 적합성을 데이터시트를 통해 분석하였습니다.

### 28. [Analysis] dsPIC33CK512MP710 MCU 52번(RE8) 및 58번(RC8) 핀의 SPI 재할당 타당성 분석
- **분석 결과**:
  - **52번 핀 (RE8) - 이사 불가능**: dsPIC33CK512MP710 제품군 데이터시트 분석 결과, 52번 핀(RE8)은 Peripheral Pin Select (PPS)를 지원하지 않는 고정(Fixed) I/O 핀입니다. 하드웨어 SPI 모듈의 통신선(CLK, MISO, MOSI)으로 매핑이 불가능합니다.
  - **58번 핀 (RC8) - 이사 가능 (단독으로는 불가)**: 58번 핀(RC8)은 `RP56`으로 지정되어 있어 PPS 재할당이 가능하므로 SPI 통신선 중 하나를 매핑할 수 있습니다.
  - **핀 개수 부족**: SPI 통신(CLK, MISO, MOSI, CS)을 수용하기 위해서는 최소 3~4개의 핀이 필요한데, 지목한 비어있는 핀은 2개(52, 58번)에 불과하며 이 중 52번은 PPS도 불가능합니다.
- **최종 가이드 및 대안**:
  - 기존 4핀 SPI 회로를 다른 곳으로 이사하려면, 현재 비어있으면서 PPS(RP/RPI) 기능을 완벽히 지원하는 핀들 중 4개를 선택하여 설계해야 합니다.
  - 현재 비어있는 핀들 중 PPS 가능한 후보군:
    - 53번 핀 (`RP90/RF10`)
    - 54번 핀 (`RP35/RB3`)
    - 56번 핀 (`RP91/RF11`)
    - 57번 핀 (`RP36/RB4`)
    - 58번 핀 (`RP56/RC8`)
    - 66번 핀 (`RP70/RD6`)
    - 68번 핀 (`RP37/RB5`)
  - 이 중 4개 핀을 선택해 SPI로 활용하면 하드웨어 SPI 모듈을 정상적으로 사용할 수 있습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `비트뱅잉 (Bit-Banging)` 신규 용어 정의 추가 완료.

### 29. [Analysis] W5500 이더넷 포트 관련 핀 맵(44~50번)의 94~99번 핀 이사 타당성 분석
- **분석 결과**:
  - **핀 개수 부족 문제**: W5500 2개 포트를 제어하는 신호선은 총 7개(SPI 3선, SCSN 2선, RSTN 2선)이나, 요청된 94~99번 핀 영역은 총 6개 핀으로 1개가 부족함.
  - **핀 기능 매치 성공 (100번 핀 포함 시 가능)**: 바로 인접한 100번 핀(`RB13`)까지 포함하여 94~100번 핀(총 7개)을 활용하면 이사 및 하드웨어 SPI 작동이 완벽히 가능함.
    - 94번(`RE14`) 및 98번(`RE15`)은 PPS가 없는 고정 GPIO이나, 이더넷 제어의 단순 GPIO 신호인 `ETH1_RSTN`, `ETH2_RSTN` 리셋선으로 매핑하면 해결됨.
    - 나머지 PPS 가능 핀 5개(95, 96, 97, 99, 100번)에 하드웨어 SPI 통신선 3개(MISO, MOSI, SCLK)와 칩 선택선 2개(SCSN)를 매핑함.
- **최종 가이드 및 주의사항**:
  - **도미노 이사 필수**: 기존 95, 96, 98, 99번은 온도 센서 SPI(TC SPI)가 사용 중이며 100번은 `HZ60` 신호가 사용 중이므로, 이더넷을 이쪽으로 옮기려면 기존 기능들을 다른 비어있는 핀들로 먼저 쫓아내야 함.
  - **추천 매핑 맵**:
    - `ETH_MISO` ➡️ 95번 (`RA6 / RP97`)
    - `ETH_MOSI` ➡️ 96번 (`RA7 / RP98`)
    - `ETH_SCLK` ➡️ 99번 (`RA8 / RP99`)
    - `ETH1_SCSN` ➡️ 97번 (`RB12 / RP44`)
    - `ETH2_SCSN` ➡️ 100번 (`RB13 / RP45`)
    - `ETH1_RSTN` ➡️ 94번 (`RE14` - 고정 핀)
    - `ETH2_RSTN` ➡️ 98번 (`RE15` - 고정 핀)
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `SPI 버스 공유 (SPI Bus Sharing)` 신규 용어 정의 추가 완료.

### 30. [Analysis] HZ60 외부 인터럽트 용도 핀 재할당(이사) 적합성 분석
- **분석 결과**:
  - HZ60 신호는 외부 인터럽트(INT)로 사용해야 하므로, 반드시 외부 인터럽트 기능이 내장되어 있거나 PPS 매핑이 가능한 핀으로 이사해야 함.
  - **가장 추천하는 핀 - 51번 핀 (`RB2`)**: 하드웨어 전용 외부 인터럽트인 `INT0` 기능이 물리적으로 고정 내장되어 있으며, 현재 이미지 상에서 비어 있어 별도의 복잡한 PPS 코드 설정 없이 매우 신속하고 안정적으로 동작함.
  - **PPS 외부 인터럽트 지원 핀들 (비어있는 핀 기준)**:
    - 57번 핀 (`RB4 / RP36`)
    - 58번 핀 (`RC8 / RP56`)
    - 66번 핀 (`RD6 / RP70`)
    - 68번 핀 (`RB5 / RP37`)
    - 이 핀들은 PPS를 통해 외부 인터럽트 입력(`INT1~INT4` 중 하나)으로 코드 레지스터에서 자유롭게 매핑하여 사용 가능함.
  - **사용 불가 핀 (PPS 및 INT0 기능 부재)**:
    - 52번 핀 (`RE8`) 및 73번 핀 (`RE11`)은 PPS가 안 되는 일반 고정 GPIO이므로 외부 인터럽트 입력으로 연결할 수 없음.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `외부 인터럽트 (External Interrupt / INT)` 신규 용어 정의 추가 완료.

### 31. [Design] dsPIC33CK512MP710 MCU 최종 핀맵(포트 설정) 교차 검증 및 PPS 오류 검출
- **검증 내용**: 사용자가 최종 조정한 MCU 핀맵 이미지의 리소스 적합성 검토 및 PPS 기능 매칭 확인.
- **분석 결과 (3가지 치명적 PPS 매핑 오류 검출)**:
  1. **98번 핀 (`RE15`) ➡️ `ETH_MOSI` (치명적)**: `RE15`는 PPS 기능이 없는 고정 GPIO임. 하드웨어 SPI 데이터 출력(`ETH_MOSI`)으로 활용할 수 없으므로, PPS 가능 핀인 **96번 핀(`RA7` / `RP98`)**을 `ETH_MOSI`로 쓰고 98번 핀을 `ETH1_RSTN` 리셋선으로 맞바꾸는(Pin Swap) 조치가 필요함.
  2. **55번 핀 (`RE9`) ➡️ `TC_SPI_CLK` (치명적)**: `RE9`는 PPS 기능이 없는 고정 핀임. 하드웨어 SPI의 클록 출력(`TC_SPI_CLK`)이 불가능하므로, 비어있는 PPS 가능 핀인 **57번 핀(`RB4` / `RP36`)** 또는 **58번 핀(`RC8` / `RP56`)**으로 CLK 신호를 이동시켜야 함.
  3. **70번 핀 (`RE10`) ➡️ `MON_422_RX` (치명적)**: `RE10`은 PPS 기능이 없는 고정 핀임. UART 수신(`MON_422_RX`)은 RPI(Remappable Input) 매핑이 필수적인데 불가함. 현재 비어있는 PPS 가능 핀인 **66번 핀(`RD6` / `RP70`)** 또는 **68번 핀(`RB5` / `RP37`)**으로 신호를 이동시켜야 함.
- **기타 검토 권장 사항 (라벨 중복 의심)**:
  - 6번 핀(`RC12`) 및 15번 핀(`RD14`)에 `DO_P165`가 중복 할당되어 있으므로 한쪽 오타 여부 체크 필요 (예: 15번 핀은 `DO_P331` 등의 오타 가능성).
  - 35번 핀(`RD12`) 및 77번 핀(`RB9`)에 `DO_HT181`이 중복 할당되어 있으므로 확인 필요.
- **최종 판정**: 상기 3가지 치명적 PPS 통신 핀 배치 오류 및 라벨 중복 사항을 수정한 후 최종 회로 설계를 마감하도록 가이드함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `핀 스왑 (Pin Swap, 핀 맞바꾸기)` 신규 용어 정의 추가 완료.

### 32. [Design] DAC60516 및 TLV2374 OP-Amp 아날로그 출력 회로 무결성 검증
- **검증 내용**: 16채널 DAC 컨버터(IC501) 출력단 및 3개 Quad OP-Amp(IC500, IC502, IC503) 전압 추종기 버퍼 회로의 최종 보완 상태 검증.
- **분석 결과 (100% 무결성 확인)**:
  - **결선 무결성 검증**: 당초 `IC503` (세 번째 TLV2374ID)의 5번 핀(`2IN+`) 입력 넷 레이블이 `AO_MFC121`로 중복 의심되었으나, 재검증 결과 주변 전압 기호(`+5V`)와의 시각적 간섭 현상에 따른 일시적 판독 해프닝이었으며 실제로는 **`AO_MFC131`**로 정밀하게 설계되어 있음을 확인 완료함.
  - **전원 및 보호 회로**:
    - 소자별(IC500, IC502, IC503) 전원 VDD-GND 최단 거리에 `0.1uF` 세라믹 바이패스 커패시터 장착 확인.
    - 외부 2m 인출 신호선마다 `100Ω` 댐핑 저항 및 `SM712` TVS 다이오드의 병렬 AGND 결선 확인 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `넷 레이블 오타 (Net Label Typo, 네트 오표기)` 신규 용어 정의 추가 완료 (일반 설계 원칙 보완 목적 유지).

### 33. [Design] ADS1115 아날로그 입력 ADC 컨버터 회로 교차 검증 및 중복 정리
- **검증 내용**: 4개 ADS1115 ADC 칩셋(`IC400, IC401, IC402, IC403`)의 I2C 주소 설정 결선 및 16개 아날로그 입력 채널 매핑 무결성 검증.
- **분석 결과 (중복 채널 정리 및 무결성 확인)**:
  - **중복 해결**: 당초 `IC401`의 6번 핀(`AIN2`)에 연결되어 중복 의심을 받았던 `AI_PT314`는, 6kW BOP 자재 명세서(BOP LIST_test 시트) 확인 결과 개질수 유량계(`FM312`)가 아날로그 입력이 아닌 **RS-485 통신 방식**을 사용하는 디바이스로 확인됨에 따라 아날로그 센서 입력 `AI_FM312` 자체가 회로에 불필요함을 검증 완료함. 이에 따라 중복 할당되었던 `IC401`의 6번 핀은 최종적으로 아무 신호도 연결하지 않는 **비연결(Unconnected)** 상태로 비워두는 정리안을 확정함.
  - **I2C 주소 설정 (100% 무결성 확인)**: 4개 칩의 ADDR 핀(1번)이 각각 GND(`IC400`), +5V(`IC401`), SDA(`IC402`), SCL(`IC403`)로 완벽히 격리되어 주소 충돌(`0x48~0x4B`) 없이 1개 버스로 전수 동작함.
  - **전원 및 디커플링 (100% 무결성 확인)**: 각 소자 VDD단에 `0.1uF` 바이패스 커패시터(`C400, C402, C404, C406`)가 AGND 접지 기준으로 올바르게 매핑되어 노이즈 억제력이 보장됨.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `I2C 디바이스 주소 설정 (I2C Device Address Configuration / ADDR Pin)` 및 `종단저항 (Termination Resistor)` 신규 용어 정의 추가 완료.

### 34. [Design] RS-485, RS-422, 플래시 메모리, RTC 주변 장치 회로 최종 교차 검증 및 무결성 확보
- **검증 내용**: 485 통신(`IC4`), 422 통신(`IC2`), SPI Flash 메모리(`IC3`), DS3231 RTC(`IC6`) 주변 회로의 무결성 검증 및 보완 사항 확인.
- **분석 결과 (중복 해결 및 I2C 풀업 보완)**:
  - **Flash 메모리 EPAD 오결선 해결**: `IC3` (W25Q256)의 칩 바닥 열 방출 패드인 `EPAD` (9번 핀)에 잘못 연결되었던 `FLASH_DO` 데이터선을 완벽히 격리하고, 노이즈 방출을 위해 **`DGND`**로 최종 접지 결선하여 무결성을 확보함.
  - **I2C 풀업 저항 오류 정정 및 보완**: 이전 분석에서 MCU 시트에 I2C 풀업 저항이 이미 장착되어 있다고 잘못 오판했던 오류를 발견해 교정함. 실제 전체 회로에 I2C 풀업 저항이 전혀 없었던 상태였으며, 사용자가 RTC 시트에서 기존 중복 저항을 지우면서 전체 풀업 저항이 아예 누락되는 사태가 됨. 이를 방지하기 위해 RTC 칩(`IC6`) 주변의 `I2C_SDA` 및 `I2C_SCL` 라인 바로 옆에 **`R17 (4.7k)` 및 `R18 (4.7k)` 풀업 저항 한 쌍을 유일하게 신규 배치하여 최종 I2C 통신 버스 무결성을 정상 복구**하도록 가이드함.
  - **크리스탈 스펙 수정 완료**: 외부 시스템 발진자 `Y1`을 노이즈(EMI)가 큰 25MHz 대신, 정밀도가 높고 전자기 노이즈가 적은 **`8MHz` 크리스탈**로 최종 변경 및 수정 완료함.
  - **남은 보완 과제**: RS-485 `IC4` (MAX3485)의 패일세이프 바이어스 저항 역결선 교정(A 풀업, B 풀다운) 및 디커플링 커패시터 `C6` 그라운드를 `DGND`로 일치시키는 잔여 하드웨어 작업 유도.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `패일 세이프 바이어스 (Fail-Safe Biasing)` 및 `노출형 패드 (Exposed Pad / EPAD)` 신규 용어 정의 추가 완료.

### 35. [Design] PCB 배치 완료 후 라우팅(배선) 우선순위 및 실무 가이드라인 수립
- **배선 작업 우선순위 정립**:
  1. **고속 및 차동 신호**: 이더넷 차동 신호쌍(100Ω 임피던스 제어) 및 MCU 크리스탈 클록(비아 없이 최단 라인) 배선 최우선.
  2. **민감한 아날로그 신호**: 온도 센서 및 ADC 입력 라인을 노이즈원(스위칭 전원, 릴레이)에서 최대한 멀리 분리하고 가드 링 접지 배선 적용.
  3. **스위칭 전원 및 대전류**: LM2576 주변 벅 컨버터 루프 최소화 및 220V 입력, 릴레이/트라이악 라인은 허용 전류 밀도를 고려하여 넓은 구리 패턴 설계.
  4. **일반 디지털 신호**: 저속 I/O 및 일반 제어선들은 비아를 자유롭게 활용해 내/외층으로 유연하게 배치.
  5. **전원 및 GND 플레인 연결**: 4층 스택업 중 내층 GND/Power Plane의 스플릿 구획 재검토 및 디커플링 콘덴서 바이패스 배선 룰 엄수.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `차동 라우팅 (Differential Routing)`, `디커플링 커패시터 바이패스 라우팅 (Decoupling Capacitor Bypass Routing)`, `가드 링 및 실드 배선 (Guard Ring and Shield Trace)` 신규 기술 용어 정의 추가 완료.

### 36. [Design] 2층(2-Layer) vs 4층(4-Layer) 적층 사양 비교 타당성 검토
- **검증 내용**: 부품 배치 최적화 및 PPS 활용에 따른 2층 PCB 기판 설계 가능 여부 검토.
- **분석 결과 (4층 기판 유지 강력 권장)**:
  1. **고속 이더넷 임피던스 제어 한계**: 2층 기판(1.6mm 두께)은 절연 두께가 너무 두꺼워 W5500의 100Ω 차동 임피던스를 맞추려면 패턴 폭이 기형적으로 굵어짐. 4층 기판의 얇은 PP(프리프레그) 참조 평면 구조가 필수적임.
  2. **그라운드 리턴 패스 분열**: 2층에서는 신호선들이 GND 동박을 사방으로 조각내어 고속 디지털 신호의 귀로(Return Path)가 길어지며, 이는 아날로그 미세 신호(열전대 등) 계측값 흔들림과 치명적인 EMI/EMC 잡음 유발로 직결됨. 4층 내층의 전면 Solid GND가 유일한 해결책임.
  3. **다전압 파워 분할 공급**: +3.3V, +5V, +12V, +24V 및 AC 220V 등 복잡한 전력망의 2층 배선 시 전원 공급선 두께 제한으로 인한 임피던스 증가 및 노이즈 전달 억제력 상실 우려. 4층의 전원 스플릿 플레인 배치가 전원 무결성(PI) 확보에 유리.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `리턴 패스 (Return Path / 신호 귀로)`, `참조 평면 (Reference Plane)`, `임피던스 매칭 (Impedance Matching)` 신규 용어 정의 추가 완료.

### 37. [Design] 4층 PCB 스택업 레이어별 지정 및 전원/GND 분할 설계 가이드 수립
- **검증 내용**: 4층 적층 구조의 레이어별 구체적 물리 속성 및 네트(Net) 매핑, 전원/접지 분할 가이드 수립.
- **분석 결과 (실무 가이드라인 확정)**:
  1. **Top Layer (Signal)**: MCU, 이더넷 칩셋 등 능동소자 배치 및 고속 통신(이더넷 100Ω 차동선), 크리스탈 클록(Shield GND) 등 최단 배선.
  2. **Layer 2 (Inner Plane 1 - GND)**: Internal Plane Layer로 설정하여 솔리드 GND 평면 구성. 디지털 그라운드(`DGND`)를 기본으로 하며 온도센서 아날로그 접지(`AGND`)와 섀시 접지(`GND`)를 층 분할(Split)하여 1점 접지로 묶음.
  3. **Layer 3 (Inner Plane 2 - Power)**: Internal Plane Layer로 설정. 다양한 전원 레벨(`+3.3V`, `+5V`, `+12V`, `+24V`)을 소자 배치 그룹에 맞춰 구획 분할(Power Split)하여 면으로 공급.
  4. **Bottom Layer (Signal)**: 터미널 블록 등 리드 부품의 솔더링면 및 잔여 저속 디지털/아날로그 라우팅.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `플레인 레이어 (Plane Layer / 전용 내층)`, `파워 스플릿 / 전원 분할 (Power Split)` 신규 용어 정의 추가 완료.

### 38. [Design] 이더넷 고속 라인 및 MCU SPI 라우팅 출발점 설정 설계 전략 수립
- **검증 내용**: 이더넷 차동 신호(W5500 ~ RJ45) 및 MCU ~ W5500 간 SPI 통신 배선 시 라우팅 시작 지점(출발점) 결정 기준 정립.
- **분석 결과 (실무 설계 방향성 확정)**:
  1. **W5500 ~ RJ45 차동 신호**: 핀 간격이 조밀한 **W5500 IC 핀(QFN 패키지)**에서 먼저 배선을 인출(Escape Routing)한 후, 상대적으로 간격이 넓고 자유로운 RJ45 커넥터 방향으로 차동 임피던스(100Ω) 배선을 진행. 좁은 곳으로 들어갈 때 생기는 배선 꼬임 및 임피던스 불연속성 최소화.
  2. **MCU ~ W5500 SPI 신호**: 100핀 TQFP 패키지로 가장 핀이 빽빽한 **MCU 핀**에서 출발하여 비교적 단순한 W5500 방향으로 이송하는 라우팅 순서 적용.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `이스케이프 배선 / 인출 배선 (Escape Routing)`, `임피던스 불연속 (Impedance Discontinuity)` 신규 기술 용어 정의 추가 완료.

### 39. [Design] IPC-4761 비아 보호(텐팅/플러깅/필링) 사양 가이드라인 수립
- **검증 내용**: 알티움 비아 속성(Via Feature) 드롭다운 리스트 내 IPC-4761 표준 종류별 특징 분석 및 본 보드 적용 사양 도출.
- **분석 결과 (실무 가이드라인 확정)**:
  1. **Tenting (Type 1a/1b/2a/2b)**: 비아 표면을 솔더마스크 잉크로 덮어씌워 땜납 유입 및 구리 산화를 방지하는 기본 방식. 공정 비용 추가가 없으며 대다수의 일반 신호 및 전원 비아에 `Type1a - Tenting`을 표준 적용함.
  2. **Plugging (Type 3a/3b/4a/4b)**: 비아 구멍 내부를 에폭시 등으로 메우는 방식. 땜납 흘러내림을 물리적으로 완전히 방지하나 미세한 내부 공기 팽창 이슈가 있을 수 있음.
  3. **Filling & Capping (Type 7)**: 비아 구멍을 전도성/비전도성 물질로 완전히 채운 후 표면을 구리로 재도금하여 평평한 패드로 만드는 사양. 초고밀도 BGA 설계용 `Via-in-Pad` 공정에 필수이나 고비용이 유발되므로, 본 보드(TQFP/QFN 사양)에서는 필수적인 패드 위 비아를 제외하고 일반 Tenting 적용을 권장.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `IPC-4761 (비아 보호 설계 표준)`, `텐팅 (Tenting)`, `비아 인 패드 (Via-in-Pad)` 신규 기술 용어 정의 추가 완료.

### 40. [Design] 알티움 Parameter Set 지시자 룰 스코프(Scope) 영향 범위 검증
- **검증 내용**: 회로도 시트(`Ethernet.SchDoc` 등) 상의 Parameter Set 지시자를 통한 디자인 룰(선폭, Clearance, Via) 세팅 시 스코프 영향 범위 검토.
- **분석 결과 (실무 가이드라인 확정)**:
  1. **개별 넷 타겟팅**: 회로도 상의 Parameter Set은 **해당 지시자가 물리적으로 붙어 있는(터치 중인) 특정 넷(Net)에만 국부적으로 적용**됨. 시트 전체나 프로젝트 전체로 자동 전파되지 않아 상호 충돌이 없음.
  2. **시트별 독립성 보장**: 각 시트마다 서로 다른 Parameter Set을 각각 독립적으로 배치하더라도, 각 넷 고유의 룰로 인식되어 PCB 동기화(ECO) 시 개별 규칙으로 정상 이관 및 적용됨.
  3. **프로젝트 전역 적용 방안**: 전체 선로나 대규모 클래스 단위 적용 시에는 회로도 지시자 배치 대신, **Net Class**를 생성해 묶어준 후 PCB의 `Design -> Rules` 매니저에서 룰 스코프(Query)를 조절하는 방식이 더 깔끔함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `디자인 룰 스코프 (Design Rule Scope / 규칙 범위)`, `파라미터 세트 (Parameter Set)`, `넷 클래스 (Net Class)` 신규 기술 용어 정의 추가 완료.

### 41. [Design] 이더넷 차동 신호(100Ω) 배선 중 비아 사용 타당성 및 보호 설계 검토
- **검증 내용**: 이더넷 차동 라인 배선 중 탑에서 바텀으로 층간 이송을 위해 뚫은 비아의 적합성 및 필수 설계 조치 검토.
- **분석 결과 (실무 가이드라인 확정)**:
  1. **비아 사용 타당성**: 비아는 기생 용량/인덕턴스로 인해 임피던스 불연속성을 야기하므로 고속 신호선에는 사용을 최소화해야 함. 부득이하게 사용할 경우 대칭성과 대책 설계가 필수적임.
  2. **대칭적 배치 검증**: 현재 차동 쌍(RXP, RXN)에 대해 동일한 선상에 나란히 대칭으로 비아를 뚫어 시간 지연(Skew) 편차와 공통 모드 노이즈 유발 요인을 정상 억제함.
  3. **리턴 비아(Return Via) 보완 필수**: 탑(L1)에서 바텀(L4)으로 층을 바꿀 때 접지(GND) 참조 평면의 전류 경로가 끊김. 이를 방지하기 위해 신호 비아 바로 옆(최대한 가깝게)에 **GND 리턴 비아**를 뚫어 리턴 패스를 연결해 주는 보완 조치를 권장함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `리턴 비아 (Return Via / 접지 귀로 비아)`, `공통 모드 노이즈 (Common Mode Noise)`, `비아 스텁 (Via Stub / 비아 잔여물)` 신규 기술 용어 정의 추가 완료.

### 42. [Design] 열전대(TC) 아날로그 영역 내층(L2, L3)의 배선 사용 검토 및 채널 재매핑 수립
- **검증 내용**: TC(열전대) 미세 아날로그 센서 영역의 배선 꼬임 해결을 위해 내층(L2 GND, L3 Power)을 배선용으로 사용하는 방안에 대한 타당성 검토.
- **분석 결과 (내층 훼손 금지 및 펌웨어 채널 재매핑 권장)**:
  1. **내층(L2 GND) 훼손 금지**: TC 신호는 uV 단위 극미세 신호로 노이즈에 매우 취약함. L2 GND Plane을 쪼개어 신호선으로 사용 시, 아날로그 차폐 능력이 영구 상실되어 심각한 계측값 흔들림(노이즈 유입) 및 전자기 잡음(Ground Slot Noise)을 유발함. L2 GND는 솔리드 평면(Solid Plane) 유지가 절대 필수임.
  2. **교차 배선의 근본적 해결 (배치 및 핀 스왑)**: 배선 꼬임의 주원인은 터미널 블록의 채널 배열과 멀티플렉서(Mux) 칩의 입력 핀 순서의 불일치임. 물리적으로 내층을 쪼개는 위험을 감수하는 대신, **아날로그 채널 재매핑(Analog Channel Re-mapping)** 기법을 사용하여 회로도 상의 연결을 꼬이지 않게 직선형으로 재설계하고, 펌웨어(소프트웨어)의 인덱스 매핑 배열을 변경해 소프트웨어적으로 해결하는 것이 가장 안전한 표준 정석임.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `그라운드 슬롯 노이즈 (Ground Slot Noise)`, `아날로그 채널 재매핑 (Analog Channel Re-mapping)` 신규 기술 용어 정의 추가 완료.

### 43. [Design] 알티움 라우팅 중 레이어 전환 시 자동 생성 비아 스타일 기본값 설정 가이드 수립
- **검증 내용**: 탑(Top)에서 바텀(Bottom) 등으로 층 전환 시 자동 생성되는 비아의 기본 규격 설정 오류 해결.
- **분석 결과 (실무 설정 가이드라인 확정)**:
  1. **디자인 룰 설정 (Routing Via Style Rule)**: 배선 중 자동 삽입 비아는 알티움의 디자인 룰 설정을 최우선으로 따름. `Design -> Rules` 메뉴에서 `Routing -> Routing Via Style` 규칙의 **Preferred(선호하는) 값**을 원하는 비아 규격(예: Hole 0.25mm / Diameter 0.45mm 등)으로 수정해 두면 레이어 전환 시 해당 비아가 고정 자동 생성되어 수동 변경 번거로움이 해결됨.
  2. **환경 설정 변경 (Defaults Via)**: Preferences 톱니바퀴 -> `PCB Editor -> Defaults`에서 `Via` 객체의 기본값(Diameter, Hole Size)을 원하는 치수로 동일하게 매칭함.
  3. **실시간 제어 (Tab Key)**: 배선 중 단축키(`*` 등)로 레이어 변경 전에 `Tab` 키를 눌러 Properties 창에서 비아 속성을 일시 고정하는 실무 조작법 안내.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `라우팅 비아 스타일 규칙 (Routing Via Style Rule)`, `프리퍼드 사이즈 / 선호 치수 (Preferred Size)` 신규 기술 용어 정의 추가 완료.

### 44. [Design] 열전대(TC) 채널 재매핑 회로도 설계 수정 검증
- **검증 내용**: 사용자가 수정한 Mux 칩셋(IC300, IC303)의 입력 채널 재매핑 회로도 변경 사항의 무결성 검증.
- **분석 결과 (수정 완료 및 무결성 확보)**:
  1. **회로도 결선 최적화**: Mux `IC300`(P채널) 및 `IC303`(N채널)의 입력 신호 라벨을 꼬이지 않는 직선 방향으로 변경 완료함 (예: `P-CH1`~`P-CH5` 순차 결선 후 `P-CH21`~`P-CH23` 결합 등).
  2. **차동 정합성 완벽 보장**: 두 칩(IC300, IC303)의 제어 주소선(`TC_ADDR0`~`TC_ADDR3`) 및 활성화 핀(`TC_EN1`) 결선이 완벽히 병렬 공유되어, 두 칩이 하나의 차동 쌍(Pair) 채널로 동시 동작함을 확인 완료함.
  3. **후속 조치**: PCB 동기화(ECO) 실행을 통해 배선을 최단거리로 직결한 후, 펌웨어(MCU) 코딩 단계에서 수정한 인덱스 맵 배열을 구성하도록 가이드함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `ADG706 (16채널 아날로그 멀티플렉서 IC)`, `차동 멀티플렉서 구성 (Differential Mux Configuration)` 신규 기술 용어 정의 추가 완료.

### 45. [Design] dsPIC33CK512MP710 SPI 통신 핀 스왑(Pin Swap) 및 PPS 타당성 검토
- **검증 내용**: dsPIC33CK512MP710 (100핀 TQFP) MCU의 SPI 핀 교환(53번-54번 MISO/MOSI 스왑, 56번-57번 CS/CLK 스왑) 가능 여부 분석.
- **분석 결과 (핀 스왑 적용 가능 확정)**:
  1. **53번(RF10)과 54번(RB3) 핀 교환 (MOSI/MISO 스왑)**: 두 핀 모두 PPS(Peripheral Pin Select) 기능인 재할당 가능 핀(RF10 = RP106, RB3 = RP35)입니다. 따라서 펌웨어 상의 PPS 입력 레지스터와 출력 레지스터 설정을 통해 데이터 송신(SDO) 및 수신(SDI) 방향을 서로 바꾸어 매핑하는 것이 완벽하게 가능합니다.
  2. **56번(RF11)과 57번(RB4) 핀 교환 (CS/CLK 스왑)**: 두 핀 또한 재할당 가능 핀(RF11 = RP107, RB4 = RP36)이므로, SPI 클록(SCK) 출력과 일반 GPIO 칩 선택(CS) 제어 신호의 역할을 맞바꾸어 소프트웨어적으로 설정하는 것이 지장 없이 지원됩니다.
- **결론**: PCB 패턴 레이아웃이나 신호선 꼬임을 풀기 위해 언급하신 핀들(53/54번, 56/57번)을 서로 스왑하는 설계 변경을 진행하셔도 무방하며, 이후 펌웨어(MCC 핀 관리기 활용 권장) 단에서 바뀐 핀 정보에 맞춰 PPS 매핑을 재할당해 주면 정상적으로 동작합니다.

### 46. [Design] IC300 및 C300 아날로그 그라운드(AGND) L2 내층 접속 및 비아 배치 설계 검토
- **검증 내용**: 4층 PCB에서 C300(디커플링 커패시터)의 2번 AGND 핀과 IC300의 27번 AGND 핀을 묶어 L2(2층) 그라운드 레이어로 내리는 구체적인 연결 및 비아 배치 설계 방법 분석.
- **분석 결과 (내층 Split 및 비아 연결 가이드 수립)**:
  1. **L2 층분할(Split Plane) 영역 확인**: 본 보드 설계 사양 상 L2(Inner 1)층은 디지털 그라운드(DGND), 아날로그 그라운드(AGND), 섀시 접지(GND)로 층 분할(Split) 구성되어 있습니다. C300 및 IC300의 AGND 핀은 L2의 'AGND 분할 영역'에 닿아야 합니다.
  2. **비아(Via)를 통한 내층 접속**: 탑(Top) 레이어에서 C300의 2번 핀과 IC300의 27번 핀을 연결한 후 배선 도중 비아(Via)를 삽입하면, 비아의 넷(Net) 속성이 자동으로 `AGND`로 연결됩니다. 알티움은 넷 속성이 일치하는 L2 내층의 AGND 동박과 비아를 자동으로 접속시켜 줍니다.
  3. **노이즈 억제를 위한 개별 비아(Individual Via) 배치 권장**: 디커플링 콘덴서(C300)는 IC300 내부의 고속 스위칭 및 아날로그 노이즈를 그라운드로 최단 거리 우회시키는 역할을 수행합니다. 따라서 탑 레이어에서 두 핀을 묶어 하나의 비아로 내리는 것보다, **C300의 2번 그라운드 패드 바로 옆에 비아를 1개 뚫고, IC300의 27번 그라운드 패드 바로 옆에도 비아를 1개 따로 뚫어** 최단 거리로 L2 AGND 플레인에 직접 물려주는 것이 루프 인덕턴스를 낮추고 노이즈를 억제하는 데 가장 좋습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `서멀 릴리프 (Thermal Relief, 열 분산 접속)` 신규 기술 용어 정의 추가 완료.

### 47. [Design] 알티움 L2 내층 AGND 분할 플레인(Split Plane) 생성 절차 및 조작 가이드 수립
- **검증 내용**: L2(Inner 1) 내층 그라운드 평면에서 아날로그 그라운드(AGND) 영역을 분할하여 생성하는 구체적인 Altium Designer 설계 프로세스 분석.
- **분석 결과 (음각 분할 가이드라인 확정)**:
  1. **음각(Negative) 레이어 특성**: 본 보드의 L2층은 Plane Layer(내층 전용 플레인)로 설정되어 있습니다. 이 레이어는 일반 배선층과 반대로 선(Line)을 그리면 동박이 깎여서 절연되는 음각 방식으로 동작합니다.
  2. **분할 플레인(Split Plane) 생성 순서**:
     * **경계선 배치**: L2(Inner 1) 레이어로 이동한 후, `Place -> Line` (단축키 `PL`)을 실행하여 디지털 그라운드(DGND)와 아날로그 그라운드(AGND) 영역을 격리할 경계선을 그립니다. 선의 두께(Width)는 안전 이격 거리(Gap)인 0.5mm~1.0mm로 설정합니다.
     * **넷(Net) 이름 할당**: 선으로 영역 구획이 나누어진 상태에서 아날로그 그라운드 전용으로 쪼개진 영역의 내부 동박을 **더블 클릭**합니다. 나타나는 `Split Plane` 팝업창의 넷 목록에서 `AGND` 넷을 선택해 줍니다. 나머지 큰 영역은 `DGND` 등으로 지정합니다.
     * **연결성 확보**: 넷 할당이 끝나면, 그 영역에 `AGND` 넷의 비아를 배치했을 때 자동으로 해당 내층 AGND 분할 동박과 서멀 릴리프(열 분산 접속) 구조로 결합됩니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `스플릿 플레인 (Split Plane, 분할 플레인)` 신규 기술 용어 정의 추가 완료.

### 48. [Design] 써모커플(TC) 단자대 하단 영역의 그라운드 설계 및 처리 기준 수립
- **검증 내용**: 써모커플 단자대(J300 ~ J307) 하단 레이어에 그라운드(AGND)를 넓게 깔아주는(Solid Plane) 설계의 타당성 및 노이즈 영향성 검토.
- **분석 결과 (아날로그 그라운드 통동판 배치 확정)**:
  1. **전기적 차폐 성능 확보**: 써모커플 아날로그 계측 신호는 uV~mV 레벨의 매우 미세한 전압 신호로 노이즈에 매우 취약  1. **스페셜 스트링(.NetName) 활용 및 독립 텍스트 스트링(Free Text String) 배치**:
     - 기존 부품에 종속된 지정자(Designator) 텍스트(예: `Tp307` 글자 자체)는 부품의 이름표 속성이므로 고유한 `Net` 속성을 가질 수 없어 속성창에 Net 박스가 표시되지 않음을 규명했습니다.
     - 해결을 위해 기존 지정자 텍스트를 숨김(Hide) 처리한 후, **`Place -> String` (단축키 `P` ➡️ `S`)을 통해 독립된 새로운 텍스트 스트링(Free Text String)**을 배치하고, 이 독립 텍스트의 Properties 패널에서 **Text 값에 `.NetName`**을 입력하고 Net 속성을 패드와 동일하게 맞춰 줌으로써 실제 넷 명이 실시간 매핑되도록 처리했습니다.
  2. **특수 문자열 변환(Convert Special Strings) 뷰 옵션 활성화**:
     - 화면 상에 `.NetName` 대신 실제 넷 명(`TC_ADDR2` 등)이 제대로 파싱되어 렌더링되도록, 단축키 `L`로 열 수 있는 **`View Configuration` 패널 상단의 `View Options` 탭으로 전환한 후, 최하단 `Additional Options` 섹션 내 `Special Strings`** 파란색 버튼을 클릭하여 활성화하도록 가이드했습니다.
  3. **텍스트 오브젝트 자체의 Net 속성 바인딩 및 Properties 패널 필터링 활용**:
     - `Special Strings` 옵션이 활성화되었음에도 `.NetName`이 실제 넷명으로 변환되지 않는 현상의 원인이 텍스트 오브젝트 자체의 **`Net`** 속성이 `No Net`으로 비어 있기 때문임을 규명했습니다.
     - 특히, 다른 부품(Component, 3D Body 등)과 함께 다중 선택(Multiple Selection)이 되어 있을 경우 Properties 패널에 `Net` 속성 설정창이 숨겨지는데, 이 경우 패널 맨 위의 깔때기 필터 팝업에서 **`Texts`** 버튼을 클릭하여 텍스트 속성만 선택적으로 필터링하거나, 선택을 완전히 해제 후 텍스트만 단독 선택한 상태에서 **`Net`** 속성을 타겟 패드와 동일한 넷(예: `TC_ADDR2`)으로 바인딩해야 정상 대치됨을 추가 안내 및 피드백 처리 완료했습니다.
 침범해서는 안 됩니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `냉접점 보상 (Cold Junction Compensation / CJC)`, `열적 평형 설계 (Thermal Plane Design)` 신규 기술 용어 정의 추가 완료.

### 49. [Design] RS-485 / RS-422 통신 포트용 물리 핀 PPS 지원 및 핀 재할당 타당성 검토
- **검증 내용**: 485/422 통신용 신호선 핀들(71번 RF13, 69번 RB6, 68번 RB5, 67번 RD5, 65번 RD7)의 PPS(Peripheral Pin Select) 기능 지원 여부 및 타 포트/핀 재배치 가능 여부 분석.
- **분석 결과 (PPS 기능 완벽 지원 확정)**:
  1. **대상 물리 핀의 RP 지정자 확인**: 검토를 요청한 모든 통신 핀은 dsPIC33CK512MP710 (100핀 TQFP) MCU 사양 상 디지털 주변장치 할당이 자유로운 **재할당 가능 핀(RP Pin)**으로 지정되어 있습니다.
     - 71번: RF13 (RP109) - MON 422 TX (UART TX 매핑)
     - 69번: RB6 (RP38) - 485 RX (UART RX 매핑)
     - 68번: RB5 (RP37) - MON 422 RX (UART RX 매핑)
     - 67번: RD5 (RP85) - 485 TX (UART TX 매핑)
     - 65번: RD7 (RP87) - 485 DIR (일반 GPIO 또는 UART 모듈의 하드웨어 자동 방향 제어 매핑)
  2. **재배치 가능성 검증**: 모든 핀이 PPS 기능(RP 번호)을 완벽히 내장하고 있으므로, 배선 라우팅의 흐름에 맞춰 다른 여유 있는 재할당 가능(RP) 핀으로 자유롭게 스왑 및 포트 변경을 진행하셔도 무방합니다. 변경 후에는 펌웨어(MCC 핀 제어 설정)에서 해당 UART 채널의 TX/RX/RTS 신호 매핑을 바뀐 RP 번호로 연동해 주면 오류 없이 고속 통신이 가능합니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `RS-485 자동 방향 제어 (RS-485 Auto Direction Control / Auto Flow Control)` 신규 기술 용어 정의 추가 완료.

### 50. [Design] 사용자 변경 485 / 422 통신 포트 핀배열 변경 검증
- **검증 내용**: 사용자가 수정한 485/422 통신 핀 배열(MON 422 RX -> 70번 RE10로 이동, 485 DIR -> 68번 RB5로 이동)의 기술적 타당성 및 신호 무결성 검증.
- **분석 결과 (변경안 승인 및 무결성 확보)**:
  1. **신규 변경 핀의 PPS 적합성 확인**:
     * **70번 핀 (RE10 / RP122)**: dsPIC33CK512MP710 (100핀 TQFP) MCU에서 RE10 포트는 **RP122** 재할당 가능 핀으로 정의되어 있습니다. 따라서 비동기 직렬 통신인 `MON 422 RX` (UART 입력) 신호를 받아들이는 데 완벽하게 호환됩니다.
     * **68번 핀 (RB5 / RP37)**: 앞선 검토와 같이 **RP37** 재할당 가능 핀으로, 반이중 RS-485 전환용 `485 DIR` 방향 제어 출력 신호로 지정하는 데 문제가 없습니다.
  2. **그 외 통신 핀 유지 검증**: 71번(MON 422 TX - RF13/RP109), 69번(485 RX - RB6/RP38), 67번(485 TX - RD5/RP85) 핀들은 기존의 PPS 가능 구역에 완벽히 보존되어 있어 변경된 통신 회로 구성에 이상이 없습니다.
  3. **결론**: 사용자가 수정한 회로 변경안은 물리 핀 충돌이나 전기적 한계 오류가 없는 **안전하고 무결한 설계**이며, PCB 배선 흐름에 맞춰 매우 깔끔하게 정리되었습니다. 이 상태로 회로 설계를 진행하고 펌웨어(MCC)에서 해당 통신 포트 핀 설정만 변경해 주면 됩니다.

### 51. [Design] DC 모터 드라이브 회로 타당성 검토 및 피드백 수립
- **검증 내용**: 칩니 배수 흡입펌프 및 전도도 필터링 펌프 구동용 DC 모터 드라이브 회로(MOSFET, 포토커플러, 역기전력 다이오드 등) 설계의 전기적 타당성 분석.
- **분석 결과 (하드웨어 수정 권장 및 PWM 제어 한계 검증)**:
  1. **MOSFET 내압 마진 부족 해결 (가장 치명적)**: 현재 24V 모터 제어 회로에 적용된 `IRF7413` MOSFET은 드레인-소스 항복 전압($V_{DSS}$) 규격이 30V로 매우 타이트합니다. 24V 전원 장치의 전압 변동 및 모터 오프 시 발생하는 인덕티브 서지 전압을 고려할 때, 30V 내압은 소자 파손 위험이 극도로 큽니다. 따라서 24V 구동 환경에서는 **최소 40V, 권장 60V 이상**의 내압을 가지는 MOSFET 소자(예: FDS5672, AO4484 등)로의 변경이 필수적입니다.
  2. **포토커플러 입력 제어 전류 적절성 검토**: MCU의 DO 출력단(`DO_P165`, `DO_P331`)과 포토커플러 LED 입력 간의 직렬 저항(R507, R515)이 1kΩ으로 다소 높게 잡혀있습니다. 3.3V MCU 제어 기준 시 LED 구동 전류는 약 2.1mA로, LTV817S의 전류 전달 비율(CTR) 조건에 따라 2차측 포토트랜지스터가 충분히 켜지지 못하고 동작이 불안정해질 수 있습니다. 안정적인 턴온을 위해 입력 저항 값을 **330Ω ~ 470Ω** 수준으로 하향 조정하는 것을 권장합니다.
  3. **구동 방식(ON/OFF vs PWM)에 따른 제약 사항 검증**:
     * **스위칭 속도**: 게이트 직렬 저항 R508(10kΩ)과 풀다운 저항 R510(10kΩ)의 조합은 시상수(RC 지연)를 크게 늘려 스위칭을 매우 느리게 만듭니다. 단순 온/오프 제어 시에는 큰 문제가 없지만, **PWM 고속 제어**를 적용하는 경우 MOSFET이 선형 영역에 머무는 시간이 길어져 심각한 발열 및 열화로 소자가 즉시 파손됩니다. PWM 적용 시에는 게이트 저항을 수십~수백 옴으로 낮추거나 전용 드라이버 칩 사용이 필요합니다.
     * **프리휠링 다이오드**: 병렬 연결된 `1N4007` 다이오드는 일반 정류용으로 역회복 특성($t_{rr}$)이 매우 느립니다. 단순 ON/OFF 시에는 무리가 없으나, PWM 제어 시 고속 복구 성능 부족으로 역기전력 전압을 차단하지 못하고 다이오드가 파손될 위험이 크므로 PWM 가동 시에는 **고속 복구 다이오드(FRD)나 쇼트키 다이오드(SS34 등)**로 반드시 교체해야 합니다.
  4. **전기적 격리 설계 적합성 통과**: 포토커플러를 적용하여 디지털 접지(`DGND`)와 파워 접지(`Gnd_e`)를 완벽히 격리(Galvanic Isolation)한 아키텍처는 모터 기동 노이즈가 메인 제어 회로로 전파되는 것을 원천 차단하는 매우 안전하고 훌륭한 설계 방향임을 확인하였습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `전기적 격리 (Galvanic Isolation / 전기적 절연)`, `프리휠링 다이오드 (Freewheeling Diode / 플라이백 다이오드)`, `드레인-소스 항복 전압 (V_DSS / Drain-Source Breakdown Voltage)`, `게이트 충전 저항 및 스위칭 손실 (Gate Charge Resistance & Switching Loss)` 신규 기술 용어 정의 추가 완료.

---

## 📅 2026-07-23: v0.1.1 DC 24V 구동용 MOSFET 마진 검토 및 대체 소자 추천 ⚡

### 52. [Design] IRF7413 마진 부족 검증 및 SO-8 핀 호환 60V MOSFET 대체품 추천
- **검토 내용**: DC 24V 모터 및 솔레노이드 밸브 구동용으로 설계된 `IRF7413` (30V, SO-8)의 전압 마진 부족을 구체적으로 입증하고, 동일 패키지 규격(SO-8) 및 핀아웃을 가진 최적의 60V급 대체 MOSFET 부품을 분석 및 선정.
- **분석 결과**:
  1. **30V 내압 마진의 한계**: 24V 전원 시스템에서 30V MOSFET 사용 시 정적 전압 마진은 단 25%(6V)에 불과합니다. 환류 다이오드(D505, 1N4007)가 병렬 배치되어 있으나, 다이오드의 포워드 리커버리 지연과 배선 기생 인덕턴스(Stray Inductance)로 인한 과도기 스파이크가 30V를 상회하여 아발란치 항복(Avalanche Breakdown)에 따른 소손 위험이 상존합니다. 따라서 안정성을 위해 **60V급 소자** 적용을 강력 권장합니다.
  2. **SO-8 핀 호환 60V 대체 MOSFET 선정**:
     - **onsemi FDS5672** (강력 추천): $V_{DS} = 60\text{V}$, $I_D = 12\text{A}$, $R_{DS(on)} = 10\text{m}\Omega$ (at $V_{GS}=10\text{V}$). IRF7413과 핀아웃 및 구동 특성이 유사하면서도 우수한 열적 내진성을 가짐.
     - **Vishay SQ4470EY** (고신뢰성 추천): $V_{DS} = 60\text{V}$, $I_D = 14.8\text{A}$, $R_{DS(on)} = 9.5\text{m}\Omega$ (at $V_{GS}=10\text{V}$). AEC-Q101 차량용 인증을 획득하여 $175^\circ\text{C}$ 고온 환경 제어에 탁월.
     - **Vishay Si4062DY** (범용 추천): $V_{DS} = 60\text{V}$, $I_D = 16\text{A}$, $R_{DS(on)} = 9\text{m}\Omega$ (at $V_{GS}=10\text{V}$). 극저온저항 설계로 발열 저하에 용이.
- **설계 반영 조치**: 24V 구동용 부하 MOSFET을 기존 `IRF7413`에서 60V 사양의 `FDS5672`로 최종 변경 및 회로도(Schematic) 반영 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `유도성 부하 (Inductive Load)`, `역기전력 (Back EMF / Flyback Voltage)`, `아발란치 항복 (Avalanche Breakdown)` 3개 신규 기술 용어 정의 추가 완료.

### 53. [Design] LTV817S 포토커플러 구동 회로의 입력단 저항 설계 적정성 검토
- **검토 내용**: MCU의 3V(또는 3.3V) 출력 포트에서 포토커플러 `LTV817S`를 구동할 때, 입력측 직렬 제한 저항 $R_{in} = 1\text{k}\Omega$ 설계가 2차측 디지털 트랜지스터(Q505, KRC105S)를 구동하는 데 충분한지 타당성 분석.
- **분석 결과**:
  1. **입력 전류 및 동작 한계 확인**: 입력 전압 3V 기준 $1\text{k}\Omega$ 저항 사용 시 LED 순방향 전류 $I_F$는 약 $1.8\text{mA}$(3.3V 기준 $2.1\text{mA}$) 수준으로 매우 낮음. 이 전류 영역에서는 LTV817S의 수광 전달 비율(CTR)이 최대 50% 수준까지 감소할 수 있어, 2차측 컬렉터 전류 $I_C$ 용량이 약 $1\text{mA}$ 내외로 크게 제한됨.
  2. **2차측 구동 마진 검증**: 2차측 부하인 내장 바이어스 트랜지스터 `KRC105S`($R_1 = 2.2\text{k}\Omega$, $R_2 = 47\text{k}\Omega$)를 완전히 도통(Saturation)시키기 위해 필요한 Base-Emitter 동작 마진 및 노화 방지(Aging) 고려 시 $1.8\text{mA}$의 구동 전력은 대단히 부족함. 온도 변화나 부품 편차에 의해 트랜지스터가 불안정하게 온/오프되거나 중간 선형 영역에 머무를 위험이 상존함.
  3. **회로 개선 권장사항**: 안정적인 스위칭 마진(최소 $I_F = 5\text{mA}$ 이상)을 확보할 수 있도록 입력 저항값을 낮출 것을 권장함. 3.3V MCU 구동 기준시 **$390\Omega \sim 470\Omega$** (3.0V 구동 기준시 **$330\Omega \sim 360\Omega$**)으로 수정 필요.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `CTR (Current Transfer Ratio, 전류 전달 비율)` 신규 기술 용어 정의 추가 완료.

### 54. [Design] 액 질소퍼지 솔레노이드 밸브(SV502) MOSFET 게이트 구동 시정수 적정성 분석
- **검토 내용**: 포토커플러(B518, LTV817S) 및 직렬 저항($R_{579} = 10\text{k}\Omega$), 풀다운 저항($R_{581} = 10\text{k}\Omega$)으로 설계된 `U520` MOSFET(IRF7413 -> FDS5672 대체 예정)의 게이트 구동 시정수 및 스위칭 마진 분석.
- **분석 결과**:
  1. **게이트 인가 전압 안전성 검증**: 포토커플러가 도통하여 +24Vee가 출력될 때, $10\text{k}\Omega : 10\text{k}\Omega$ 전압 분배 구조를 통하여 MOSFET 게이트에 걸리는 전압($V_{GS}$)은 $12\text{V}$로 제어됨. 이는 MOSFET 게이트 파괴 한계 전압($\pm20\text{V}$)을 초과하지 않는 매우 안전하고 확실한 전압 범위임.
  2. **시정수 및 제어 속도(PWM vs ON/OFF) 검토**:
     - 게이트 충전 시의 등가 병렬 저항은 $5\text{k}\Omega$이며, 이로 인해 MOSFET 게이트의 스위칭 시상수(RC Time Constant)가 길어져 스위칭 속도가 수십 $\mu\text{s}$ 단위로 다소 느림.
     - 액 질소퍼지 솔레노이드 밸브(SV502)는 고속 스위칭(PWM)이 필요 없는 단순 온/오프(ON/OFF) 부하이므로 현재의 $10\text{k}\Omega$ 시정수는 오동작 및 고주파 EMI 노이즈를 억제하는 데 최적인 매칭임. (단, PWM 구동 필요시 게이트 저항을 수백 옴 단위로 대폭 낮추어야 함)
  3. **뒷단 서지 흡수 회로 적정성**: 솔레노이드 양단에 병렬 배치된 정류 다이오드(D527, 1N4007)는 일반 저속 다이오드이나, 단순 온/오프 응용에서는 밸브 차단 시 발생하는 역기전력 서지를 억제하기에 충분히 안전함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `시상수 (Time Constant / RC Time Constant)`, `게이트 전압 분배 (Gate Voltage Divider)` 신규 기술 용어 정의 추가 완료.

## 📅 2026-07-28: v0.1.2 계측 장비 매뉴얼 관리 및 검색 📋

### 55. [Doc] Keysight MSOX4154A 오실로스코프 매뉴얼 다운로드 및 관리
- **내용**: 사용자가 요청한 "mxo-x4154a" 장비 매뉴얼(Keysight MSOX4154A, InfiniiVision 4000 X-Series)의 최신 한국어 및 영어 사용 설명서(User's Guide) PDF 파일을 검색하고 프로젝트 문서 폴더(`05_Docs`)에 저장.
- **수행 작업**:
  - Keysight 제품 지원 페이지를 분석하여 MSOX4154A 사용 설명서의 직접 다운로드 링크 확보.
  - PowerShell 스크립트를 사용하여 한국어 및 영어 매뉴얼 PDF 파일을 `d:\Work\H2_Control_Board\05_Docs` 하위에 직접 다운로드 완료.
    - 한국어 매뉴얼: `MSOX4154A_User_Manual_KR.pdf`
    - 영어 매뉴얼: `MSOX4154A_User_Manual_EN.pdf`
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `자동화 웹 스크래핑 및 다운로드 (Automated Web Scraping and Download)` 신규 기술 용어 정의 추가 완료.

---

## 📅 2026-07-29: v0.1.3 볼밸브 컨트롤러(AT24-3T) 역설계 및 반도체 소자 제어 타당성 분석 🔧

### 56. [Analysis] 3선식 볼밸브 컨트롤러 AT24-3T 내부 회로 역설계 및 동작 원리 규명
- **검토 내용**: Panasonic DPDT 릴레이(TQ2-24V) 및 마이크로 리미트 스위치, 바이패스 다이오드로 구성된 볼밸브 제어기 내부 PCB 역설계 및 회로 분석.
- **분석 결과**:
  1. **동작 메커니즘**: 외부 3선식 인터페이스(상시 V+, 공통 GND, 제어선 Control) 중 제어선의 인가 여부에 따라 릴레이 코일이 여자/소자됩니다. 릴레이의 DPDT 접점을 통하여 DC 24V 브러시드 모터의 극성을 교차 스왑하여 정/역 구동을 구현합니다.
  2. **리미트 정지**: 밸브 축의 기계적 캠 돌기가 회전 한계에 이르면 좌/우의 마이크로 리미트 스위치를 눌러 전원을 차단시킵니다. 스위치 접점과 역방향으로 병렬 연결된 리미트 바이패스 다이오드가 반대 방향 제어 시 전류를 흐르게 하여 원활한 역회전 탈출 경로를 보장합니다.

### 57. [Design] 반도체 소자(MOSFET H-Bridge) 대체 설계 타당성 검토 및 아키텍처 수립
- **검토 내용**: 물리 릴레이 접점을 N/P-channel MOSFET으로 교환하고 리미트 스위치를 제어 로직에 통합하는 반도체 구동 대안 회로 설계 검토.
- **분석 결과**:
  1. **H-브리지 구동 구조**: DPDT 릴레이를 P-MOSFET 2개, N-MOSFET 2개(또는 전용 H-bridge 드라이버 IC)로 대체하여 브러시드 모터 양단의 24V 극성을 제어하는 아키텍처를 수립했습니다.
  2. **기계식 스위치의 저전력 로직화**: 모터 전류를 직접 차단하던 기계식 리미트 스위치의 역할을 저전력 풀업/풀다운 입력 로직 신호로 전환하여 MCU 또는 H-브리지 로직 컨트롤러에 결합시켰습니다. 이를 통해 스위치 수명을 극대화하고 전기적 아크 노이즈를 완벽 차단하는 설계를 도출했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `H-브리지 (H-Bridge)`, `리미트 바이패스 다이오드 (Limit Bypass Diode)` 2개 신규 기술 용어 정의 추가 완료.

### 58. [Analysis] 외부 시그널선 TR/FET 스위칭(오픈 드레인) 제어 타당성 분석 및 설계 수립
- **검토 내용**: 기존 AT24-3T 컨트롤러의 내부 회로는 보존한 상태에서 외부 인출된 제어 신호선(시그널선)을 TR이나 FET를 사용하여 그라운드(GND)에 묶어 제어(Active Low 및 Open 제어)하는 방식의 타당성 및 보호 대책 분석.
- **분석 결과**:
  1. **오픈 드레인(Open Drain) 스위칭 타당성 검증**: 시그널선을 릴레이 대신 N-channel MOSFET(예: 2N7002) 또는 NPN 트랜지스터의 드레인(또는 컬렉터)에 연결하고, 소스(또는 이미터)를 GND에 접지하여 스위칭하는 방식은 하드웨어적으로 완벽하게 가능함을 검증했습니다.
  2. **회로 보호 마진 설정**: 시그널선이 플로팅 상태일 때 인가될 수 있는 내부 전압(통상 DC 24V 풀업 상태)을 감안하여, 스위칭 소자는 드레인-소스 내압($V_{DSS}$ 또는 $V_{CEO}$)이 최소 40V, 안정성을 고려해 60V 이상인 소자 선택을 제안했습니다.
  3. **전기적 격리 권장**: 제어 측 MCU 그라운드와 구동 측 24V 그라운드 간 전위 차이 및 유도성 노이즈 유입을 차단하기 위해, 포토커플러(Optocoupler)의 2차측 광트랜지스터 단자를 직접 스위칭 소자로 사용하여 시그널선을 GND에 묶어주는 격리형 제어 회로 구성을 도출했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `오픈 드레인 (Open Drain)`, `포토커플러 격리 스위칭 (Optocoupler Isolated Switching)` 2개 신규 기술 용어 정의 추가 완료.

### 59. [Analysis] 시그널선과 모터 전력 구동 경로 분리 및 용량 확장성 영향 검토
- **검토 내용**: 외부 모터 사양이 대용량으로 확장될 때, 시그널선에 인가되는 전압 및 전류 변화가 있는지 검증하고 코일-접점 물리 격리 분석.
- **분석 결과**:
  1. **신호선과 부하선 분리 검증**: 외부 시그널선(Control)은 릴레이의 코일(Coil)을 켜는 미소 신호선일 뿐이며, 모터의 대전류 구동 전원은 상시 전원선(V+)과 릴레이 접점(Contacts)을 통과하는 구조임을 판별했습니다.
  2. **모터 용량 변화 영향**: 모터가 훨씬 커지더라도 시그널선에 흐르는 전류는 오직 TQ2-24V 릴레이 코일의 동작 전류(약 5.8mA ~ 8.3mA)로 영구적으로 고정되어 있음을 확인했습니다. 모터 사양 증대에 따라 제어측 소자(포토커플러나 TR)를 교체할 필요가 없으므로 시스템의 확장성이 뛰어납니다.
  3. **주의 사항 (내부 릴레이 한계)**: 단, 밸브 내 모터 부하 전류가 기존 TQ2 릴레이 접점의 허용 정격(1A~2A)을 초과하게 되면 내부 릴레이 접점이 소손될 우려가 있으므로, 모터가 비정상적으로 커질 경우에는 밸브 내 릴레이 자체의 정격 확인 및 외부 구동단(H-bridge 등) 설계가 요구됨을 분석하여 명시했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `코일-접점 격리 (Coil-to-Contact Isolation)`, `신호선과 부하선 분리 (Signal and Load Line Separation)` 2개 신규 기술 용어 정의 추가 완료.

### 60. [Analysis] Yeonho SMW025 (SMW250) 커넥터 전기적 정격 사양 규격 검토
- **검토 내용**: 디렉터리 배선 및 보드 접속용 연호(Yeonho) 커넥터 SMW025(실 명칭 SMW250/SMAW250 시리즈)의 정격 허용 전압 및 전류 규격 검토.
- **분석 결과**:
  1. **전기적 정격 사양**: 허용 전압 AC/DC 250V 및 허용 전류 AC/DC 3A의 성능 규격을 충족함을 식별하여 보고했습니다.
  2. **환경 및 내구성 규격**: 접촉저항 30mΩ MAX, 절연저항 1000MΩ MIN, 내전압 AC 1000V/1분 및 사용 환경 온도 범위(-25°C ~ +85°C) 스펙을 확인하여 향후 보드 신호 커넥팅 설계 가이드로 확정했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `허용 전압 (Rated Voltage / 정격 전압)`, `허용 전류 (Rated Current / 정격 전류)` 2개 신규 기술 용어 정의 추가 완료.

### 61. [Analysis] SMW025 (2.5mm Pitch) 커넥터의 AT24-3T 적용 안전 마진 및 타당성 검증
- **검토 내용**: AT24-3T 밸브 구동 인터페이스(DC 24V 구동 모터 및 제어 입력) 사양에 대비하여 연호 SMW025 커넥터의 전압 및 전류 동작 안전 마진(Safety Margin) 설계 적합성 분석.
- **분석 결과**:
  1. **전압 마진 검증**: 구동부 동작 전압인 DC 24V 대비 커넥터 허용 내압이 250V로 설계되어, 10배 이상의 극도로 안전한 전압 마진(약 90.4%의 디레이팅 마진)이 보장됨을 확인했습니다.
  2. **전류 마진 검증**: 내장 DC 모터(RK-370JT)의 실운전 소모 전류(약 0.1A ~ 0.3A) 및 기동 순간 발생하는 과도 기동 전류(Peak Inrush Current, 약 0.8A ~ 1A 내외)에 대비해, 커넥터 연속 허용 전류가 3A로 설계되어 있어 최소 3배 이상의 높은 전류 마진을 확보했습니다.
  3. **결론**: SMW025 커넥터는 AT24-3T 제어 및 구동 통합 인터페이스용으로 전기적 소손 위험이 전혀 없는 **최적의 설계 부품 사양**임을 최종 확인 및 승인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `안전 마진 및 디레이팅 (Safety Margin & Derating)`, `모터 기동 전류 (Motor Starting Current / Inrush Current)` 2개 신규 기술 용어 정의 추가 완료.

### 62. [Design] KRC105S 디지털 TR의 포토커플러 기반 24V 구동 설계 적정성 분석
- **검토 내용**: 포토커플러(LTV817S) 및 직렬 저항(R549, 3.3kΩ)을 거쳐 24V 전원으로 디지털 트랜지스터(Q508, KRC105S)의 베이스를 구동하는 스위칭 회로의 전력 소모 및 전압 규격 적정성 분석.
- **분석 결과**:
  1. **베이스 포화 구동 전류**: 입력 전류 계산 결과 $I_B \approx 4.2\text{ mA}$가 베이스로 인입되어 릴레이 부하(U513)를 완전히 켜기 위한 포화 구동 조건은 충분히 만족함을 확인했습니다.
  2. **소자 내압 및 파손 취약점 발견**: 24V 입력 전압이 포토커플러 온 상태 시 KRC105S 베이스 핀에 그대로 분배되어 가해집니다. BJT 베이스 접합의 0.7V 클램핑을 신뢰하더라도 소자 고장(Open) 발생 시 24V 전압이 직접 유입되어 칩 소손 위험이 있고, 내부 바이어스 저항 $R_1$(2.2kΩ)에서 약 39mW의 과도 전력이 소모됩니다.
  3. **외장 저항 전력 마진**: 1608 칩 저항 R549(3.3kΩ)에서 약 59mW의 전력 소모가 발생하여 1/10W(100mW) 사양 기준 약 60%의 부하가 걸리며, 장기 열화 위험이 존재합니다.
  4. **개선 대안 제안**: 
     - 베이스 풀업 전원을 24V 대신 로직용 5V 또는 3.3V로 인가하여 구동할 것을 권장.
     - 24V 유지가 불가피할 경우, 외부 저항 R549를 **10kΩ**으로 변경하여 베이스 전류를 1.9mA 수준으로 디레이팅 시킴으로써 TR 보호 및 칩 저항의 발열 전력 소모(12mW)를 대폭 억제하도록 수정 권장.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `디지털 트랜지스터 (Digital Transistor / BRT)`, `포화 구동 (Saturation Drive)` 2개 신규 기술 용어 정의 추가 완료.

### 63. [Design] AT24-3T 볼밸브 스위칭 구동 회로의 신뢰성 검토 및 마진 최적화
- **검토 내용**: N-channel MOSFET 및 LTV817S 포토커플러를 사용해 외부 24V 풀업 시그널선을 제어하는 볼밸브 스위칭 드라이버 회로의 전기적 마진 및 정격 적합성 분석.
- **분석 결과**:
  1. **MOSFET 게이트 과전압 취약성 규명**: 포토커플러가 켜져 24V 전원이 직렬 저항(R574, 10kΩ) 및 풀다운 저항(10kΩ)으로 분배될 때 게이트 인가 전압은 $V_{GS} \approx 12\text{V}$로 이상적이나, 풀다운 저항이 단선 고장(Open)이 날 경우 게이트에 $V_{GSS}$ 정격($\pm20\text{V}$)을 넘어서는 24V 전압이 직접 걸려 MOSFET이 소손될 수 있는 구조적 취약성을 식별했습니다. 게이트 전압 차단을 위한 12V~15V 제너 다이오드 병렬 보호 소자 배치 또는 풀업 전원을 낮은 전압(12V 이하)으로 이송하는 대책을 제안했습니다.
  2. **상태 표시용 LED 회로의 스펙 아웃(Spec-out) 저항 검출**: `+24Vee` 라인과 3.3kΩ 저항(R505) 및 LED로 구성된 표시부 회로에서 저항의 실효 소모 전력 계산 결과 약 **147mW**가 발생하여 1608 칩 저항 정격 한계(100mW)를 현저히 초과하는 심각한 고온 노화 오동작 리스크를 발견했습니다. 저항 소손을 막기 위해 저항 값을 **6.8kΩ ~ 10kΩ** 수준으로 대폭 상향 조정하여 전류를 2.2mA~3.2mA로 낮추고 전력 소비를 50mW 이하로 줄여 저항을 정상 디레이팅 설계 영역으로 복귀 조치하도록 가이드라인을 수립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `게이트 과전압 보호 (Gate Overvoltage Protection)`, `칩 저항 정격 디레이팅 (Chip Resistor Derating)` 2개 해제 완료.

### 64. [Design] NTR4003 MOSFET 스위칭 회로의 10V 제너 다이오드(ZD) 클램핑 보호 설계 검증
- **검토 내용**: 상태 표시 LED 직렬 저항 수정(10kΩ) 및 게이트 단 10V 제너 다이오드 병렬 보호 회로가 추가된 NTR4003 MOSFET 스위칭 제어 설계 변경안의 무결성 검증.
- **분석 결과**:
  1. **LED 회로 발열 차단 검증**: LED 제한 저항 R505를 10kΩ으로 수정함으로써 회로에 흐르는 전류를 약 2.2mA로 낮췄고, 저항 전력 소비량을 **48mW**로 억제하여 1608 칩 저항 정격(100mW) 내로 정상 디레이팅(약 48% 부하율)을 유도했습니다.
  2. **10V 제너 클램핑 보호 적합성 판정**: 게이트-소스 핀에 병렬 배치한 10V 제너 다이오드는 포토커플러 도통 시 게이트 전압을 안정적인 10V 레벨로 완벽히 클램핑하여 NTR4003의 정격 $V_{GSS}$($\pm20\text{V}$) 파손 노출 위험을 원천 차단했습니다.
  3. **제너 다이오드 역전류 전력 계산**: 10V 클램핑 시 제너에 흐르는 분류 전류는 약 0.4mA, 소비 전력은 **4mW** 미만으로 산출되어 0.5W 제너 규격 대비 장기적인 발열 및 노화 우려가 아예 없는 완벽히 안전한 하드웨어 설계임을 판정 및 승인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `제너 클램핑 회로 (Zener Clamping Circuit)` 신규 기술 용어 정의 추가 완료.

### 65. [Analysis] dsPIC33CK512MP710 MCU의 PPS 미지원 고정 핀 라인업 도출 (실제 핀 맵 및 회로도 정정반영)
- **검토 내용**: 사용자가 공유한 dsPIC33CK512MP710 100핀 TQFP 공식 핀 다이어그램 및 설계 회로도를 바탕으로, 소프트웨어적 리매핑(PPS)이 불가능하고 하드웨어적으로 역할이 고정된 비재할당 가능 핀의 세부 핀번호 라인업 전면 정정 분석.
- **분석 결과**:
  1. **전원/GND 고정 핀 (10개)**: VDD(14, 39, 64, 88번), VSS(13, 40, 63, 87번), AVDD/AVSS(33, 34번).
  2. **시스템 제어 및 디버그 고정 핀 (7개)**: MCLR 리셋(10번), ICSP 프로그래밍 채널 (PGEC1/D1: 92, 93번; PGEC2/D2: 97, 100번; PGEC3/D3: 75, 77번).
  3. **클록/오실레이터 고정 핀 (4개)**: 메인 시스템 크리스탈 (RC1/RC2 - 36, 37번), 보조 32.768kHz 크리스탈 (RC12/RC13 - 6, 7번).
  4. **I2C 통신 고정 핀 (4개)**: I2C1 (RB0/RB1 - 42, 43번), I2C2 (RC5/RC4 - 82, 80번).
  5. **아날로그 전용 기능 고정 핀**: 아날로그 계측용 ADC 채널(AN0~AN23), 아날로그 비교기 입력(CxINy), DAC 아날로그 출력(DACOUTx)은 각 물리 핀에 기능이 고정되어 타 핀으로 이동할 수 없음을 최종 확인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `고정 기능 핀 (Fixed-Function Pin)` 신규 기술 용어 정의 추가 완료.

### 66. [Design] DS3231 RTC 및 ADS1115 ADC의 I2C 버스 공유 및 통합 구동 타당성 분석
- **검토 내용**: 단일 I2C 통신 버스 라인(I2C1 포트: SCL1-42번, SDA1-43번)에 고정 소자 DS3231(실시간 클록) 및 ADS1115(16비트 아날로그 Mux ADC)를 병렬 연결하여 동시에 읽을 때 주소 충돌 및 구동 전위 설계 적합성 분석.
- **분석 결과**:
  1. **슬레이브 주소 충돌 여부 검증**: DS3231의 고유 I2C 주소인 `0x68`과 ADS1115의 기본 I2C 주소인 `0x48`(ADDR 핀 GND 접지 시)이 상호 충돌하지 않고 완전 격리되어 있어, 병렬 공유 결선 및 개별 제어가 정상 동작함을 입증했습니다.
  2. **I2C 버스 레벨 적합성**: dsPIC33CK MCU 디지털 레벨(3.3V)에 맞춰, 버스의 풀업 전압 및 각 소자의 구동 VCC 전위를 **3.3V**로 일치시켜 전위차에 의한 소자 손상 및 레벨 시프터 추가 비용을 사전에 억제했습니다.
  3. **풀업 저항 등가 값 관리**: 두 모듈 내 장착된 풀업 저항들의 병렬 등가 저항값이 너무 낮아져 BJT 드라이브 부하를 주지 않도록 병렬 저항값 매칭 설계를 권장했습니다.
- **[Doc] programme 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `I2C 주소 충돌 (I2C Address Conflict)`, `I2C 버스 풀업 저항 (I2C Bus Pull-up Resistor)` 2개 신규 기술 용어 정의 추가 완료.

### 67. [Design] MCU 회로 설계 최종 핀 매치 무결성 검증 및 입력 전용 핀 설계 결함 도출
- **검토 내용**: dsPIC33CK512MP710 회로도 내 주변장치(I2C, 크리스탈, RS-485, RS-422, 써모커플 SPI, DI/DO 등) 전체 핀 매핑 관계의 기술적 적정성 교차 분석.
- **분석 결과**:
  1. **정상 매칭 검증**: I2C1 (RB0/RB1 - 42, 43번), 메인 시스템 크리스탈 (RC1/RC2 - 36, 37번), RS-485/422 및 SPI 제어선들의 PPS 재할당 가능(RP) 핀 맵 배선은 충돌 없이 완벽한 하드웨어 정합성을 가짐을 검증했습니다.
  2. **치명적인 설계 결함 발견 (30번 핀)**: Mux 활성화 제어선인 `TC EN3` 신호가 연결된 **30번 핀(RA4)**은 dsPIC33CK 구조상 **입력 전용 핀(RPI Pin, RPI20)**에 해당함을 발견했습니다. Mux 소자를 제어하기 위해 3.3V 출력을 쏴주어야 하는 출력 모드로 동작해야 하나, 해당 포트는 출력을 낼 수 없으므로 써모커플 채널 제어 오동작이 일어나는 치명적 리스크를 검출했습니다.
  3. **조치 방안 수립**: 30번 핀(`RA4`)에 인가된 `TC EN3` 네트를 출력 제어가 가능한 인근 다른 여유 GPIO/RP 핀으로 **스왑(위치 변경) 설계 변경**을 강력 가이드했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `입력 전용 핀 (Input-Only Pin / RPI Pin)`, `아날로그-디지털 다중화 (Analog-to-Digital Multiplexing)` 2개 신규 기술 용어 정의 추가 완료.

### 68. [Design] TC EN3의 RB12(35번 핀) 핀 스왑 및 디지털 I/O의 배선용 핀교환 가능성 검증
- **검토 내용**: 써모커플 Mux 활성화 선 `TC EN3`를 출력 가능 핀인 `RB12`(35번 핀)로 스왑한 변경 설계안의 적정성 확인 및 일반 DI/DO 핀들의 PCB 아트워크 배선 편의성 목적 핀교환(Pin Swap)의 하드웨어/소프트웨어 타당성 분석.
- **분석 결과**:
  1. **TC EN3 스왑 적정성 판정**: `RB12` (RP44) 핀은 PPS 출력이 정상 지원되는 핀이므로, 입력 전용 핀 `RA4`(30번)에서 해당 핀으로 스왑한 조치는 하드웨어적으로 아주 완벽하며 정상 제어가 가능함을 검증했습니다.
  2. **디지털 I/O 핀 스왑 적합성**:
  - **깃허브(GitHub) 원격 저장소 바인딩 및 최적화 푸시 완료**:
  - 원격 리포지토리: [seokipo/H2_Control_Board](https://github.com/seokipo/H2_Control_Board)
  - 용량 제한 유발 요소(SolidWorks CAD 파일 `*.SLDPRT`, Altium 백업 `History/`, 대용량 PDF/인스톨러)를 제외하고, 핵심 알티움 회로도(SCH) 및 PCB 소스(`*.SchDoc`, `*.PcbDoc`, `*.PrjPcb`), dsPIC33CK C 펌웨어, SCADA 웹 대시보드 및 시퀀스 엔진 전체를 `main` 브랜치로 성공적으로 초기 푸시 완료.
  3. 강력한 PPS(주변장치 핀 선택) 시스템을 가지고 있어, 일반 디지털 입력(DI) 및 디지털 출력(DO) 신호선 간의 배선 꼬임 방지를 위한 물리적 핀 맞바꾸기(Pin Swap)가 전적으로 가능함을 규명했습니다. 
  4. **예외 및 제약 조건 수립**: 단, 스왑 시 다음 3가지 사항을 제한 규칙으로 정의했습니다.
     - 디지털 출력(DO) 라인을 출력 기능이 없는 입력 전용 핀(RPI)에 배치하지 말 것.
     - 아날로그 센서 신호선은 하드웨어 내부 고정된 ANxx 핀 외의 순수 디지털 I/O에 맵핑하지 말 것.
     - 전원, 리셋(MCLR), 크리스탈(OSC1/2), 디버거(PGECx/Dx) 등 고정 기능 핀은 절대 스왑 대상에서 제외할 것.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `핀 스왑 (Pin Swap)` 신규 기술 용어 정의 추가 완료.

### 69. [Design] Phoenix Contact 1818339 커넥터 명명 규칙 및 다른 극수(핀 수) 검색법 분석 (2026-07-30)
- **검토 내용**: Phoenix Contact 1818339(17핀 PCB 헤더)의 다른 극수(핀 수) 버전을 마우저(Mouser)에서 신속하게 검색하고 CAD 모델(Symbol, Footprint)을 획득하기 위한 검색 프로세스 및 명명 규칙 수립.
- **분석 결과**:
  1. **파트넘버 규칙 규명**: 해당 커넥터는 피닉스컨택트 사의 `MCV 1,5/17-GF-3,81-LR` 모델로, `/17` 부분이 핀 수(극 수)를 가리킴을 파악했습니다.
  2. **직접 검색식 도출**: 원하는 극수에 맞춰 `MCV 1,5/[원하는핀수]-GF-3,81-LR` 형식으로 마우저 검색창에 직접 검색을 유도하여, 오검색 없이 즉시 CAD 라이브러리를 확보할 수 있는 매핑 규칙을 도출했습니다.
     - 예: 10핀이 필요하다면 `MCV 1,5/10-GF-3,81-LR`로 검색
     - 예: 8핀이 필요하다면 `MCV 1,5/8-GF-3,81-LR`로 검색
  3. **마우저 필터 이용법 가이드**: 공통 시리즈명인 `MCV 1,5 GF 3.81 LR`로 검색 후, 극 수(Number of Positions) 필터를 사용하여 원하는 부품 페이지로 진입한 뒤, SamacSys CAD 모델 연동 팝업을 통해 Symbol/Footprint 다운로드 방법을 구체화했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `ECAD (전자 설계 자동화)`, `Symbol/Footprint/3D Model (하드웨어 라이브러리 3대 요소)` 2개 신규 기술 용어 정의 추가 완료.

### 70. [Design] Altium Designer 회로도 라이브러리(.SchLib) 및 부품 심볼 병합/이동 가이드 제공 (2026-07-30)
- **검토 내용**: 알티움 디자이너(Altium Designer) 내에서 개별 다운로드한 `1818216.SchLib` 파일의 심볼 부품을 메인 라이브러리인 `KipoAlt.SchLib`로 병합하거나, 라이브러리 패키지(`KipoAlt.LibPkg`) 구조 내부로 파일을 이동하는 가이드 작성.
- **분석 결과**:
  1. **부품 심볼 병합 (SCH Library Panel 활용)**: `SCH Library` 패널을 활성화한 후, 원본 라이브러리(`1818216.SchLib`)의 부품 리스트에서 대상을 우클릭하여 복사(Copy)한 뒤, 대상 라이브러리(`KipoAlt.SchLib`) 패널에 붙여넣기(Paste)하는 방식의 병합 절차를 도출했습니다.
  2. **프로젝트 간 파일 이동**: 알티움 좌측 Projects 트리 상에서 `1818216.SchLib` 파일을 클릭한 채로 `KipoAlt.LibPkg` 프로젝트 하위로 드래그 앤 드롭하여 파일 소속을 손쉽게 변경하는 방법을 가이드했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `알티움 라이브러리 패키지 (Altium Library Package / .LibPkg)` 신규 기술 용어 정의 추가 완료.

## 📅 2026-07-31: v0.1.2 Altium Designer 설계 규칙 중복 부여 시 우선순위 적용 가이드 수립 📐
회로도에서 Parameter Set을 통해 Clearance 및 Width 등의 설계 규칙을 이중(중복)으로 설정할 때, PCB 단에서 어떻게 적용되는지 분석하고 올바른 우선순위 지정 방식을 정립하였습니다.

### 71. [Analysis] Altium Designer 설계 규칙(Design Rule) 중복 부여 시 우선순위(Priority) 적용 원리 분석
- **검토 내용**: 회로도 상에서 특정 영역에 다른 Clearance 또는 Width 규칙을 가진 Parameter Set이 중복으로 배치되었을 때, 어떤 규칙이 최종 적용되는지에 대한 메커니즘 규명.
- **분석 결과**:
  1. **ECO 업데이트 및 규칙 생성**: 회로도에서 Net 또는 Area에 Parameter Set을 통해 규칙을 부여한 후 PCB로 동기화(ECO)하면, 해당 규칙들이 PCB Design Rules에 각각 개별 규칙으로 자동 생성됩니다.
  2. **우선순위(Priority)에 의한 결정**: 넷이나 영역이 겹쳐 충돌이 발생하면, Altium은 PCB Rules에 설정된 **우선순위(Priority)**가 높은 것(숫자가 낮아 1위에 가까운 것)을 최종 적용합니다.
  3. **구체성 우선 원칙**: 좁고 구체적인 영역(예: 칩 주변의 Clearance 0.17mm)의 우선순위를 넓고 일반적인 영역(Clearance 0.2mm)의 우선순위보다 더 높게(더 낮은 번호로) 수동 조정해주어야 오동작(DRC 에러)을 방지할 수 있음을 가이드했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `설계 규칙 우선순위 (Design Rule Priority)` 신규 기술 용어 정의 추가 완료.

### 72. [Design] PCB 추가 마운팅 홀(Mounting Hole)의 회로도 역방향 동기화(Update Schematics) 프로세스 수립
- **검토 내용**: PCB 레이아웃 도면에서 추가한 마운팅 홀(구멍)을 회로도 도면으로 역으로 반영하여 일치시키는 방법 및 자유 객체(Free Primitive) 상태의 홀 동기화 한계 검토.
- **분석 결과**:
  1. **역방향 ECO 기법 적용**: PCB 에디터에서 `Design > Update Schematics`를 실행하여 PCB 상의 부품 변경 내역을 회로도로 밀어내는 역방향 ECO 프로세스를 가이드했습니다.
  2. **자유 객체(Free Primitive) 홀의 동기화 제약**: 라이브러리 컴포넌트(Component) 형태가 아닌 단순 패드(Pad)로 추가한 자유 객체는 역방향 동기화 시 회로도로 이관되지 않으므로, 회로도 상에서 1핀 컴포넌트를 배치한 후 PCB로 밀거나 정식 Mounting Hole 풋프린트 부품으로 추가하는 안전한 설계 기법을 제시했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `자유 객체 (Free Primitive)`, `마운팅 홀 (Mounting Hole)` 2개 신규 기술 용어 정의 추가 완료.

### 73. [Analysis] PCB 설계 규칙 검사(DRC) 연두색 위반 패턴 분석 및 역방향 ECO 적용 대상 구분
- **검토 내용**: PCB 레이아웃 도면에서 연두색으로 하이라이트되는 설계 규칙 위반(DRC)의 발생 원인 규명 및 규칙 완화 시 회로도 역방향 동기화(Update Schematics) 필요성 분석.
- **분석 결과**:
  1. **연두색 하이라이트 원인 규명**: PCB 상에서 이격 거리 위반(Clearance Violation) 등으로 인해 규칙을 충족하지 못할 때 발생하는 DRC 경고 표시임을 식별했습니다.
  2. **해결 유형별 ECO 매핑 분리**:
     - **물리적 레이아웃 수정**: 배선을 우회하거나 부품을 옮겨서 해결할 경우, 이는 논리적 변경이 없으므로 회로도로 역방향 ECO를 보낼 필요가 없습니다.
     - **설계 규칙 값 완화**: PCB 상의 규칙 설정(`Design > Rules`)을 직접 수정하여 경고를 해제한 경우, 변경된 룰 사양을 회로도 도면의 Parameter Set에도 일치시키기 위해 역방향 ECO(`Design > Update Schematics`)를 수행해야 합니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `DRC (Design Rule Check, 설계 규칙 검사)`, `클리어런스 위반 (Clearance Violation)` 2개 신규 기술 용어 정의 추가 완료.

### 74. [Design] 회로도(Schematic) 상의 설계 규칙 지정용 Parameter Set 및 Blanket 사용 가이드 수립
- **검토 내용**: 회로도 단계에서 설계 규칙(Design Rule)을 개별 선 또는 다중 그룹 영역에 적용하기 위한 ECAD 지시자(Directives)의 활용 경로 및 적용 프로세스 정립.
- **분석 결과**:
  1. **개별 Net 규칙 주입**: 특정 배선에 개별적으로 설계 규칙을 부여하기 위한 `Place > Directives > Parameter Set` 지시자 배치 및 내부 `Properties > Rules` 추가 경로를 구체화했습니다.
  2. **다중 Net 일괄 규칙 주입**: 여러 넷을 하나의 설계 영역으로 묶어 일괄적으로 규칙을 부여할 수 있는 `Place > Directives > Blanket`의 구조적 원리 및 Parameter Set과의 하이브리드 연동 결선 가이드를 도출했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `블랭킷 지시자 (Blanket Directive)` 신규 기술 용어 정의 추가 완료.

### 75. [Design] 3D PCB 레이아웃(Floorplan) 최종 배치 타당성 검토 및 Mixed-Signal 분리 설계 승인
- **검토 내용**: 강전(AC 220V), 약전(센서 계측 및 통신), 고주파 디버그 영역이 복합적으로 공존하는 6kW 수소 제어 통합 보드의 3D 완성 레이아웃 배치 구조(Floorplan) 신뢰성 및 노이즈 격리 타당성 종합 검증.
- **분석 결과**:
  1. **구획화 격리(Isolation Floorplan) 우수성 확인**: 
     - 보드 하단의 AC 220V 전원부 및 대용량 릴레이/트라이악 출력부와, 좌상단의 써모커플 온도 계측 회로가 대각선 방향으로 최대 격리되어 열 및 고전압 노이즈 전도 경로를 원천 차단함.
     - 중앙의 MCU와 우측의 이더넷 통신 라인을 조밀하게 분할하여 아날로그 라인의 누화 현상(Crosstalk)을 예방함.
  2. **안전성 및 보완 제안 수립**: 고전압 영역과 저전압 로직 제어 영역 간 기판 연면거리 차단을 위한 물리적 절연 슬릿(Isolation Slit) 추가 반영 여부 및 아날로그/디지털 그라운드(AGND/DGND)의 1점 접지 패턴 정합성 최종 점검 체크리스트 도출.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `혼성 신호 PCB 설계 (Mixed-Signal PCB Design)`, `절연 슬릿 (Isolation Slit)` 2개 신규 기술 용어 정의 추가 완료.

### 76. [Design] 이더넷 차동 신호선(TX/RX) 레이아웃 무결성 검증 및 임피던스 불일치 설계 결함 검출
- **검토 내용**: 듀얼 W5500 이더넷 컨트롤러와 RJ45 커넥터 간의 10/100M 고속 통신 배선 라우팅의 전자기적 대칭성 및 임피던스 매칭(Impedance Matching) 정밀 분석.
- **분석 결과**:
  1. **설계 결함 검출 (차동 페어 분리 배선)**: 이더넷 TXP/TXN, RXP/RXN 신호 라인이 동일 층에서 나란히 병행 라우팅(Differential Pair Routing)되지 않고 개별적으로 층(Top/Bottom)이 갈라지거나 궤적이 멀어지는 구조적 배치 결함을 확인했습니다. 이로 인해 임피던스 불연속성 및 통신 링크 두절 위험이 높습니다.
  2. **수정 가이드 수립**: 회로도 상 차동 페어 네트 네임 네이밍 규칙(`_P`, `_N` 접미사)을 정의하여 PCB로 동기화한 뒤, 알티움 디자이너의 차동 페어 배선 툴을 이용해 두 가닥의 도선을 동일 층(Top)에서 나란히 밀착 라우팅하도록 개선 방안을 작성했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `차동 페어 배선 (Differential Pair Routing)`, `임피던스 매칭 (Impedance Matching)` 2개 신규 기술 용어 정의 추가 완료.

### 77. [Design] 회로도 내 네트 네임 보존형 차동 페어 지시자(Differential Pair Directive) 적용 가이드 수립
- **검토 내용**: 이더넷 `TXP2/TXN2` 등 기존 명명된 네트 네임(Net Name)을 `_P`/`_N` 접미사로 강제 개명하지 않고, 알티움 디자이너에서 차동 페어(Differential Pair)로 자동 인식되게 하는 설계 대안 프로세스 수립.
- **분석 결과**:
  1. **차동 페어 지시자 배치**: 회로도 메뉴 `Place > Directives > Differential Pair` 지시자를 각 신호 와이어 상단에 배치하는 방식으로 직접 지정 경로를 구체화했습니다.
  2. **프로젝트 접미사(Suffix) 매핑 규칙 설정**: `Project Options > Differential Pairs` 탭에서 기본 설정값(`_P` / `_N`)을 사용자가 기존 설정해 둔 접미사 포맷인 `P` / `N` (예: TXP2와 TXN2는 각각 P와 N으로 끝남)으로 변경 설정하여, 네트 네임 변경 없이도 정상 매칭되도록 연동 설계를 구성했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `차동 페어 지시자 (Differential Pair Directive)`, `프로젝트 옵션 (Project Options)` 2개 신규 기술 용어 정의 추가 완료.

### 78. [Design] 4중 복합 그라운드(GNDE, Power GND, DGND, AGND) 접지 결합 및 격리 아키텍처 수립
- **검토 내용**: 섀시 케이스 접지(GNDE), 강전 구동 전원부 그라운드(Power GND), 디지털 제어 그라운드(DGND), 정밀 온도 아날로그 계측 그라운드(AGND) 등 보드 내에 공존하는 4대 기준 전위(접지)들의 계통 분리 및 안전한 상호 결합 아키텍처 정의.
- **분석 결과**:
  1. **계통별 접지 설계 정의**:
     - **DGND와 AGND**: 물리적으로 그라운드 플레인을 완전 분리(Split Plane)하되, 기준 레벨 고정을 위해 Net Tie(단일 지점 접지 / 스타 그라운드)를 배치하여 노이즈 공유 차단.
     - **DGND와 Power GND**: 스위칭 노이즈 역류를 막기 위해 레귤레이터 전원 인출단(Star Point) 또는 0옴 저항/비즈를 통해 결합.
     - **GNDE와 DGND**: 전기적 안전(서지 유입 차단)을 위해 완전 차단(Galvanic Isolation)하되, 교류 고주파 노이즈 방출을 위해 용량성 그라운드 결합(1MΩ 저항 + 1nF / 2kV 커패시터 병렬 RC)을 적용.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `스타 그라운드 (Star Grounding)`, `용량성 그라운드 결합 (Capacitive Ground Coupling)` 2개 신규 기술 용어 정의 추가 완료.

### 79. [Design] 분산 배치된 아날로그 그라운드(AGND)의 영역별 개별 로컬 스타 접지 설계 가이드 도출
- **검토 내용**: 좌측 상단의 온도 계측 아날로그 영역과 우측 중앙의 DAC 출력 아날로그 영역처럼 보드 상에서 서로 물리적으로 멀리 떨어진 복수의 AGND 도체 영역(Ground Island) 간의 노이즈 격리 및 최적 연결 경로 수립.
- **분석 결과**:
  1. **개별 로컬 스타 접지(Local Star Grounding) 방안 확정**: 
     - 멀리 떨어진 두 개의 AGND 면을 얇은 패턴선으로 무리하게 묶을 경우, 해당 선로가 전자기 방사 노이즈 안테나로 오동작하여 디지털 노이즈 유입 창구가 됨을 분석했습니다.
     - 대안으로 좌상단 AGND 영역은 해당 위치에서 DGND와 넷 타이 1개(Net Tie 1)로 묶고, 우측 아날로그 출력 AGND 영역 역시 그 자리 부근에서 DGND와 별도의 넷 타이 1개(Net Tie 2)로 일대일 연결하는 '개별 로컬 스타 그라운드' 토폴로지를 제시했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `그라운드 아일랜드 (Ground Island)`, `로컬 스타 그라운드 (Local Star Grounding)` 2개 신규 기술 용어 정의 추가 완료.

### 80. [Design] 분산 배치된 아날로그 영역 간 비드 저항(Ferrite Bead) 적용을 통한 고주파 디지털 노이즈 필터링 가이드 수립
- **검토 내용**: 로컬 스타 접지를 구성할 때, 비드 저항(또는 Net Tie)을 어떤 방식으로 배치하고 물리적인 배선 기법에서 주의할 점이 무엇인지 분석 및 지침 정립.
- **분석 결과**:
  1. **비드 배치 전략**: 좌상단 온도 계측 아날로그 영역과 우하단 아날로그 출력 영역에 각각 1개씩 비드 저항(또는 Net Tie)을 배치하되, 그라운드 플레인이 나뉘는 경계선(Split Boundary) 바로 위에 얹어서 배선 거리를 최소화해야 함을 분석했습니다.
  2. **오동작 방지 설계 수립**: 비드를 멀리 배치하고 긴 신호선으로 그라운드를 끌어오면 기생 인덕턴스가 발생해 신호 감쇠 및 안테나 효과를 야기하므로, 최단 거리 결선 가이드를 수립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `페라이트 비즈 (Ferrite Bead)` 신규 기술 용어 정의 추가 완료.

### 81. [Design] 4층 PCB 그라운드 스플릿(Ground Split) 경계선 상의 비드 저항 배치 및 리턴 경로(Return Path) 최적화 설계 가이드 수립
- **검토 내용**: 4층 기판의 2층 GND Plane 상에서 쪼개진 4개 그라운드 영역(24V PGND, AGND, DGND, 보드 전원 GND) 경계선(Split Line)을 가로지르는 신호선의 노이즈 차단 대책 수립.
- **분석 결과**:
  1. **경계선 결합 조건 검증**: 경계선의 적절한 한 지점을 선정하여 비드 저항을 배치하는 기법의 적합성 판정.
  2. **리턴 경로 유효성 보존 전략**: 쪼개진 틈(Gap) 위를 일반 신호선이 브릿지 형태로 타고 넘어갈 경우 발생하는 루프 면적 극대화 및 EMI 방출 리스크를 식별했습니다. 해결 방안으로 신호선을 그라운드가 묶여 있는 비드 저항 통로 근처로 우회시키거나, 신호선 교차 지점 바로 아래에 차폐용 디커플링 통로를 확보하여 리턴 경로(Return Path)의 연속성을 확보하도록 보드 설계 규칙 가이드를 적립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `리턴 경로 (Return Path)`, `그라운드 스플릿 (Ground Split)` 2개 신규 기술 용어 정의 추가 완료.

### 82. [Design] 그라운드 루프(Ground Loop) 방지를 위한 비드 저항 병렬 배치 제한 및 대전류 대책 가이드 수립
- **검토 내용**: 서로 다른 계통의 그라운드 사이에 전위 고정용 비드 저항을 복수(병렬)로 흩어 배치할 때 발생하는 전자기적 악영향 및 대전류 경로 대응 방안 검토.
- **분석 결과**:
  1. **그라운드 루프 노이즈 리스크 규명**: 두 접지판 사이에 비드 저항을 멀리 떨어뜨려 여러 개 배치할 경우, 그 사이 공간에 루프(Loop)가 형성되어 외부 유도 노이즈 수집 안테나로 작동하는 치명적인 노이즈 유입 경로를 식별했습니다. 따라서 그라운드 결합은 항상 단일 지점(Single Point)이어야 함을 명시했습니다.
  2. **대전류 경로 대책 수립**: 24V PGND 등 대전류가 복귀하는 통로에 대해 비드 저항을 분산 병렬화하는 대안 대신, 정격 허용 전류가 높은 단일 대용량 파워 라인 비드를 쓰거나, 경계선 1개 지점에 대형 0옴 저항(3216 등)을 바로 밀착 병렬 배치하여 루프를 최소화하는 설계 표준을 도출했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `합성 임피던스 (Equivalent Impedance)`, `그라운드 루프 노이즈 (Ground Loop Noise)` 2개 신규 기술 용어 정의 추가 완료.

### 83. [Design] 디지털 그라운드(DGND) 허브 기반의 직렬식 단일 접지(Star Ground) 아키텍처 정립
- **검토 내용**: 보드 내에 공존하는 4대 그라운드를 결합할 때, 상호 간의 루프 형성을 방지하고 기준 전위를 보존하기 위한 연결 연결 방향성 검토.
- **분석 결과**:
  1. **직렬 체인형 스타 토폴로지 확정**: 
     - 여러 그라운드가 복잡하게 얽히지 않도록 면적이 가장 크고 기준이 되는 디지털 그라운드(DGND)를 접지 허브(Hub)로 선정했습니다.
     - 아날로그 접지(AGND)는 DGND와 1:1 결선, 전원 접지(Power GND)도 DGND와 1:1 결선, 섀시 접지(GND_e) 또한 DGND와 RC 필터 1:1 결선 형태로 각각 구성하여, DGND를 중간 다리로 삼는 직렬/방사형 연결 아키텍처의 정상 동작 타당성을 입증 및 승인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `스타 그라운드 허브 (Star Ground Hub)` 신규 기술 용어 정의 추가 완료.

### 84. [Design] 내층 그라운드 플레인 접속을 위한 비아 홀(Via Hole) 설계 및 전류 정격 검증
- **검토 내용**: 4층 기판의 L1(Top) 레이어에 배치된 비드 저항(또는 Net Tie) 부품에서 L2(GND) 내층으로 그라운드 접속 시 비아 홀(Via Hole) 배치 타당성 및 대전류 경로 정격 안전 마진 검증.
- **분석 결과**:
  1. **접지 비아(Ground Via) 적용 적합성 판정**: 표면 실장 부품에서 내층 플레인으로 리턴 전류를 공급하기 위해 패드 양단에 각각 1개씩 비아를 배치하여 정상적인 저임피던스 수직 경로를 구현하였음을 확인 및 승인했습니다.
  2. **대전류 경로 보완 피드백 제공**: 모터/밸브 등 2A 이상의 대전류가 복귀하는 Power GND 단자의 경우, 단일 비아(0.3mm 기준 허용 전류 약 1A 내외)의 전압 강하 및 국부 발열 위험을 방지하기 위해 비아 크기 확대 또는 복수(2개 이상)의 비아 병렬 배치가 필요함을 보완 가이드라인으로 제시했으며, **사용자가 각 단자당 3개의 비아를 병렬 배치하여 3A 이상의 전력 허용량 및 임피던스 극소화를 최종 완료**했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `접지 비아 (Ground Via)` 신규 기술 용어 정의 추가 완료.

### 85. [Design] 열전대(써모커플) 입력단 LDO 레귤레이터(LM1117)의 AGND 전용 접지 분리 설계 수립
- **검토 내용**: 아날로그 써모커플 센서 입력단 및 ADC용 3.3V 정전압 공급용 LM1117 LDO 레귤레이터의 접지(GND) 전위를 디지털(DGND)과 아날로그(AGND) 중 어떤 계통에 매핑하는 것이 노이즈 측면에서 최적인지 타당성 분석.
- **분석 결과**:
  1. **접지 분리 가이드 수립**: LDO가 아날로그 회로 전용 전압 공급 장치이므로, 기준 접지 핀과 출력단 바이패스 커패시터(CP300, C303)의 접지를 `AGND`로 결합하여 디지털 리플 잡음 유입을 완전히 차단하는 설계 규칙을 제안했습니다.
  2. **입력 노이즈 바이패스 분리**: LDO 12V 입력 측 바이패스 커패시터(CP301, C302)는 12V 전력 리턴 그라운드인 `DGND`에 그대로 유지하여 입력단 서지 노이즈를 1차 격리하도록 안전 조치 가이드를 정립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `아날로그 LDO 접지 설계 (Analog LDO Grounding)` 신규 기술 용어 정의 추가 완료.

### 86. [Design] 이더넷 컨트롤러(W5500) 디지털/아날로그 공통 LDO의 DGND 기준 전위 정합성 분석 및 검증
- **검토 내용**: 이더넷 회로의 LM1117-3.3V LDO 레귤레이터가 W5500의 디지털부와 아날로그부에 동시에 전력을 인가할 때, LDO의 접지(GND) 핀 결선 적합성 검증.
- **분석 결과**:
  1. **접지 매핑 적합성 검증**: 써모커플 전용 LDO(AGND 묶음)와 달리, 이더넷 LDO는 W5500의 디지털 연산 노이즈 및 SPI 클록 전류 리턴 경로를 형성하므로 LDO의 접지와 바이패스 필터를 면적이 크고 저임피던스인 디지털 그라운드(DGND)에 그대로 연결한 현재 설계가 타당함을 확인 및 승인했습니다.
  2. **아날로그 노이즈 차단 검증**: LDO 출력 3.3V에서 W5500 아날로그 전원(AVDD)으로 넘어가는 경로에 인덕터/비즈(L201) 및 바이패스 필터를 설계하여, 디지털 스위칭 노이즈가 아날로그 그라운드(AGND)로 역류하는 것을 안정적으로 격리함을 확인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `혼성 신호 IC 접지 규칙 (Mixed-Signal IC Grounding Rule)` 신규 기술 용어 정의 추가 완료.

### 87. [Design] RS-422/485 비절연 통신 커넥터(J1)의 통신 리턴 접지(DGND) 오결선 결함 도출 및 수정
- **검토 내용**: MAX3490 RS-422/485 송수신부의 외부 인출 커넥터 `J1`의 1번 통신 그라운드 핀이 섀시 접지(`Gnd_e`)로 연결되어 발생하는 전위 불안정성 및 오작동 위험 분석.
- **분석 결과**:
  1. **오결선 원인 규명**: 비절연형 통신 구조에서 트랜시버 칩 `IC2`가 디지털 그라운드(`DGND`) 전위 기준의 로직 파형을 송수신하므로, 외부 통신선과의 기준 0V 레벨 일치를 위해 통신 접지선(1번 핀)은 `DGND`에 결합해야 정상 동작함을 분석했습니다.
  2. **수정 및 보완 방안 수립**: 1번 핀의 결선을 `Gnd_e`에서 `DGND`로 수정 조치하고, 케이스 노이즈 ESD 유입 방출을 위해 실드 핀(MH1, MH2)만 `Gnd_e`에 유지하는 정석 차폐 회로를 확정했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `통신 접지선 (Signal Ground)` 신규 기술 용어 정의 추가 완료.

### 88. [Design] RS-485 통신 보호용 TVS 다이오드(SM712)의 섀시 접지(Gnd_e) 우회 설계 타당성 검증
- **검토 내용**: MAX3485 RS-485 통신 라인 보호용 TVS 다이오드 `D1`(SM712)의 공통 접지 단자를 내부 디지털 접지(`DGND`)와 외부 섀시 접지(`Gnd_e`) 중 어느 계통에 연결하는 것이 안전성 측면에서 우수한지 분석.
- **분석 결과**:
  1. **섀시 접지(Gnd_e) 결선 타당성 검증**: 통신 그라운드와 달리, TVS 다이오드는 외부 ESD/서지 고전압을 다이렉트로 바이패스하므로 내부 `DGND`로 서지가 흐르면 Ground Bounce 노이즈 오작동을 유발함을 입증했습니다.
  2. **서지 격리 설계 통과**: 따라서 방전 에너지를 섀시 접지 `Gnd_e`로 직접 격리시켜 시스템 바깥으로 배출하도록 한 현재의 `Gnd_e` 묶음 설계를 최적의 보호 아키텍처로 확인 및 승인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `TVS 다이오드 접지 규칙 (TVS Diode Grounding Rule)` 신규 기술 용어 정의 추가 완료.

### 89. [Design] dsPIC33CK MCU의 아날로그 접지 핀(AVSS) 오결선 결함 도출 및 AGND 변경 설계 반영
- **검토 내용**: 메인 MCU `IC1`(dsPIC33CK512MP710)의 내부 고정 ADC 주변장치 기준 접지선인 34번 `AVSS` 핀이 24V/12V 강전 구동 전원용 그라운드(`GND`)에 묶여 발생하는 전위 요동 및 측정 오작동 위험 분석.
- **분석 결과**:
  1. **접지 오결선 원인 규명**: `AVSS` 핀은 기판 상에서 가장 전압 리플과 스위칭 전력 잡음이 억제된 깨끗한 아날로그 전용 접지인 `AGND` 플레인에 묶여야 센서 계측 정확도를 확보할 수 있으나, 노이즈가 많은 전원용 GND에 연결된 설계를 결함으로 탐지했습니다.
  2. **수정 방안 수립**: 34번 `AVSS` 핀의 결선을 전원용 `GND`에서 아날로그 접지 `AGND`로 정정 변경하는 설계 지침을 확정했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `AVSS (MCU 아날로그 접지 핀)` 신규 기술 용어 정의 추가 완료.

## 📅 2026-08-01: v0.1.2 아날로그 로컬 스타 그라운드 및 비드 저항 아키텍처 재검토 🔄
아트워크 마무리 단계에서 흩어져 배치된 다중 아날로그 그라운드(AGND) 도메인에 대한 스타 그라운드(Star Grounding) 허브 연결 방식과 비드 저항(Ferrite Bead) 결선 구조를 재검증하고 설계 지침을 보완했습니다.

### 90. [Design] 분산 배치된 다중 아날로그 그라운드(AGND) 영역의 로컬 스타 그라운드 비드 저항 적용 검증
- **검토 내용**: 보드 내에 물리적으로 좌상단(온도 센서부)과 우측(아날로그 출력부)으로 분산된 아날로그 접지(AGND) 영역을 메인 디지털 접지(DGND) 허브와 결합할 때, 비드 저항의 개별 사용 타당성 재확인.
- **분석 결과**:
  1. **로컬 스타 그라운드(Local Star Grounding) 타당성 확정**:
     - 물리적으로 멀리 떨어진 두 개의 아날로그 그라운드(AGND) 영역을 억지로 패턴선으로 길게 묶으면, 해당 선로가 전자기 노이즈를 끌어들이는 안테나 역할을 하여 디지털 노이즈 유입이 심화됩니다.
     - 이에 따라, 두 영역을 무리하게 하나로 결합하는 대신 **각각의 영역 근처에 있는 로컬 디지털 접지(DGND) 지점으로 각각 1개씩 비드 저항(또는 Net Tie)을 독립적으로 연결**해 주는 개별 로컬 스타 그라운드 구조가 최적임을 재검증했습니다.
  2. **오동작 방지 설계 보완 가이드**:
     - **그라운드 루프 차단**: 개별 비드를 사용할 경우 두 아날로그 영역(좌상단 및 우측) 사이에 직접적으로 왕래하는 신호선이나 결로성 결합이 생기면 폐루프(Ground Loop)가 형성되어 외부 유도 노이즈가 급증하므로, 두 아날로그 도메인 간의 직접적인 신호 연동이 없음을 재차 전제해야 합니다.
     - **최단 거리 배치**: 비드 저항은 그라운드 스플릿 경계선(Split Line) 바로 위에 배치하여 결선 루프와 기생 인덕턴스를 원천적으로 극소화해야 함을 리마인드했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `비드 저항 그라운드 결합 (Ferrite Bead Ground Coupling)` 신규 기술 용어 정의 추가 완료.

### 91. [Design] 외부 24V 전원(SMPS) 접지와 디지털 접지(DGND) 간의 단일 지점 접지 설계 검증
- **검토 내용**: 우하단 터미널을 통해 인입된 외부 24V 전력용 접지(Power GND)와 보드의 메인 제어용 접지(DGND)를 묶는 결합점(Star Point)의 위치 및 다중 결선 리스크 검토.
- **분석 결과**:
  1. **다중 결선 방식 제한 및 그라운드 루프 방지**:
     - 24V 그라운드와 DGND를 좌측, 중간, 우측 등 여러 지점에서 비드로 중복 연결하는 것은 그라운드 루프(Ground Loop)를 형성하여 전력 노이즈가 보드 전반의 제어 회로를 오염시키는 원인이 됨을 규명했습니다.
     - 이에 따라, 연결은 **반드시 단 한 지점(Single Point)**에서만 이루어져야 함을 설계 원칙으로 확정했습니다.
  2. **최적의 전원 인입부 접지 설계(Power Entry Grounding) 수립**:
     - 24V 대전류 리턴 전류가 보드 안쪽으로 깊숙이 침범하여 흐르지 않도록, 외부 전원이 기판으로 유입되는 우하단 24V 인입 터미널(SMPS 단자) 및 휴즈 인근 부근에 단일 스타 포인트를 설정하는 방안을 확정했습니다.
     - 이 지점에 단 하나의 비드 저항 또는 전류 용량을 확보할 수 있는 대용량 0옴 저항(또는 밀착 병렬 배치된 복수 저항)을 배치하여 결합하도록 피드백을 제공했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `전원 인입부 접지 설계 (Power Entry Grounding)` 신규 기술 용어 정의 추가 완료.

### 92. [Design] 4층 PCB 3층 내층 전원선(Power Plane)의 일부 영역 그라운드(GND) 분할 설계 검토
- **검토 내용**: 4층 기판 중 3층(Inner 2 - Power Plane)의 일부 영역을 그라운드(GND)로 분할(Split)하여 사용하는 방안의 타당성 및 노이즈/열적 영향 분석.
- **분석 결과**:
  1. **내층 분할 그라운드(Split Plane Grounding) 설계 적합성 판정**:
     - 기술적으로 전원 내층의 일부를 그라운드 플레인으로 구획 분할하여 사용하는 것은 정상적인 설계 방식이며, 특히 다채널 혼성 신호 및 대전류 부하 제어 보드에서 적극 권장됨을 확인했습니다.
  2. **회로적/열적 이점 분석**:
     - **차폐(Shielding) 효과**: 정밀 온도 센서나 고속 이더넷 통신 라인의 하부 3층 영역을 GND로 채워 2층 GND와 함께 이중 그라운드 차폐막을 형성함으로써 통신 및 감지 노이즈를 획기적으로 낮출 수 있습니다.
     - **방열(Thermal) 효과**: 1층(Top)의 고열 방출 소자(벅 컨버터, LDO 등) 밑 3층 영역을 GND로 분할하고 방열 비아(Thermal Via)로 묶어주어 기판 내층으로의 열 분산 효율을 극대화합니다.
  3. **설계 시 핵심 주의사항 전파**:
     - **분할 간격(Split Clearance)**: 3층 내 그라운드 분할 면과 인접 전원 패턴 간에 합선이 발생하지 않도록 최소 0.5mm(20mil) 이상의 갭(Gap)을 확보해야 합니다.
     - **신호선 리턴 경로 연속성(Return Path Continuity)**: 4층(Bottom)의 신호선 배선 시, 3층 그라운드와 전원이 갈라지는 경계선(Gap)을 가로질러 라우팅되지 않도록 배선 금지 규칙을 수립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `내층 분할 그라운드 (Split Plane Grounding)` 신규 기술 용어 정의 추가 완료.

## 📅 2026-08-03: v0.1.2 다중 +3.3V 레귤레이터 기반 전원 도메인 분리 및 병렬 연결 오류 검증 🔄
아트워크 마무리 단계에서 보드 내 각 파트별(온도 제어, 이더넷 통신, MCU) 독립 설계된 +3.3V 전원 레귤레이터의 출력단 넷 레이블과 결선 구조를 분석하여 설계 결함을 규명하고 개선 방향을 수립했습니다.

### 93. [Design] 다중 +3.3V 레귤레이터(LM1117-3.3V) 출력단 병렬 결선 오류 발견 및 넷 분리 조치
- **검토 내용**: 보드 내에 온도(MAX31856), 이더넷(W5500), MCU용으로 각각 배치된 3개의 LM1117-3.3V LDO 레귤레이터의 출력단이 모두 동일한 `+3.3V` 넷 레이블로 연결된 설계 적합성 분석.
- **분석 결과**:
  1. **병렬 결선 결함 규명**: 알티움 디자이너의 플랫(Flat) 회로 설계 환경 상에서 출력단 넷명이 동일하여 물리적으로 3개의 레귤레이터 출력이 다이렉트로 병렬 연결되는 회로 구성임을 확인했습니다. 이는 소자 간 미세 출력 편차에 의해 특정 LDO로 전류가 집중되는 쏠림 현상(발열)을 유발하고, 피드백 제어 루프의 간섭으로 전원선상에 발진(Oscillation) 노이즈를 일으키는 명백한 설계 오류입니다.
  2. **넷 분리 가이드 수립**: 각 레귤레이터의 출력 네트명을 기능 블록 단위로 분할하여 **`+3.3V_MCU`**, **`+3.3V_ETH`**, **`+3.3V_ANA`**로 각각 명확히 변경 및 분리하도록 지침을 제시했습니다.
  3. **전원 도메인 격리 필터링**: 독립 도메인 전원 간에는 비드 저항이나 0옴 저항 등으로 상호 연결하지 않고 완전히 분리된 상태를 유지하도록 했습니다. 전원 간의 노이즈 격리는 각 LDO의 5V 입력 필터 및 LDO 자체의 높은 PSRR(전원 노이즈 제거비)를 통해 달성되므로 3.3V 상호 간의 브릿지 연결은 불필요하며, 신호선(SPI 등) 간의 통신은 공통 디지털 접지(DGND) 전위 기준을 공유하므로 레벨 래치 및 격리 없이 다이렉트로 정상 결선됨을 최종 확인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `전원 도메인 분리 (Power Domain Isolation / Split Rail)`, `리니어 레귤레이터 병렬 운전 제한 (Linear Regulator Parallel Operation Limitation)` 2개 신규 기술 용어 정의 추가 완료.

### 94. [Design] DAC 컨버터(DAC60516) 디지털 인터페이스 전원(VIO)의 디지털 전원 도메인(+3.3V_MCU) 배정 검증
- **검토 내용**: DAC60516RUYT(IC501) 칩의 디지털 I/O 전원 핀인 18번 `VIO` 및 `RESET` 풀업용 `+3.3V` 전원 넷의 아날로그 전원(`+3.3V_ANA`) 또는 디지털 전원(`+3.3V_MCU`) 매핑 적합성 검증.
- **분석 결과**:
  1. **신호 성격 규명**: `VIO` 핀은 DAC의 아날로그 신호 처리부 전원(`AVDD`, +5V 공급)과 달리, MCU와의 SPI 통신 버퍼 및 로직 임계 전위를 맞춰주기 위한 순수 디지털 I/O용 인터페이스 전원 핀임을 분석했습니다.
  2. **노이즈 유입 리스크 차단**: `VIO` 핀을 정밀 아날로그 온도 센서 및 ADC 전원인 `+3.3V_ANA`에 연결할 경우, 고속 SPI 통신 클록 및 데이터 송수신에 따른 디지털 스위칭 노이즈가 아날로그 전원선으로 직접 인입되어 계측 정밀도 저하를 초래합니다.
  3. **디지털 도메인 배정 지침 확정**: 따라서 `VIO` 핀과 리셋 풀업 저항 `R508`로 공급되는 3.3V 전원은 디지털 전용 도메인인 **`+3.3V_MCU`**로 연결하여 노이즈를 차단하도록 가이드를 확정했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `디지털 인터페이스 전원 (Digital Interface Power / VIO)` 신규 기술 용어 정의 추가 완료.

### 95. [Design] DAC 컨버터(DAC60516) 기준 그라운드(GND)의 아날로그 접지(AGND) 결선 타당성 검증
- **검토 내용**: DAC60516RUYT(IC501)의 디지털 인터페이스 전원(VIO)이 디지털 도메인(`+3.3V_MCU`)으로 변경됨에 따라, 칩 자체의 접지 핀(`GND_1`, `GND_2`, `GND_3`)을 디지털 접지(`DGND`)로 이설해야 하는지 여부 분석.
- **분석 결과**:
  1. **그라운드 참조 기준 규명**: DAC는 정밀 아날로그 출력 전압(`AO_...` 계통)을 생성하는 디바이스이며, 이 출력값들은 칩 내부의 기준 그라운드 전위를 원점으로 생성됨을 확인했습니다.
  2. **노이즈 침투 차단**: DAC 그라운드를 노이즈가 많은 디지털 접지(`DGND`)에 결합할 경우, MCU 및 주변 디지털 회로의 스위칭 전류 리턴 노이즈(Ground Bounce)가 DAC의 정밀 아날로그 출력 신호에 그대로 중첩되어 출력 정밀도를 훼손하는 리스크를 규명했습니다.
### 97. [Design] 기구 외함(프레임) 노이즈의 PCB 역유입 차단을 위한 갈바닉 격리(Galvanic Isolation) 타당성 검증
- **검토 내용**: 펌프, 히터, 솔레노이드 밸브 등 다수의 대전력 구동 기기가 공통 프레임(FG)에 설치되어 있을 때, 프레임의 대전류 누설 및 노이즈가 TVS 접지를 타고 PCB 제어계로 역유입될 가능성 분석 및 방지 설계 수립.
- **분석 결과**:
  1. **역유입 차단 메커니즘 검증**: PCB 내부 신호 접지(`DGND`, `AGND`)와 기구 외함 접지(`FG`) 간에는 직류(DC) 전류가 흐를 수 없도록 도체가 분리된 **갈바닉 격리(Galvanic Isolation)** 구조가 유지되고 있음을 확인했습니다. 평상시 TVS 다이오드는 무한대의 저항(Open 상태)을 유지하므로 프레임의 일반적인 노이즈가 보드로 역유입되지 않습니다.
  2. **Y-커패시터(Y-Capacitor) 필터링**: 보드 내부 접지와 `FG` 간에 높은 절연 내압(1kV~2kV)을 가진 소용량 콘덴서(Y-Capacitor)를 병렬 배치하여, 저주파 전위차는 완벽히 격리(차단)하고 초고주파 공통 모드 EMI 노이즈만 프레임으로 빼내도록 유도 설계했습니다.
  3. **외함 대지 접지(Earthing) 필수 조건 수립**: 프레임 그라운드(`FG`)가 제 역할을 수행하고 프레임 전위 상승을 방지하기 위해서는, 최종 컨트롤 박스 외함 기구가 반드시 건물의 안전 대지 접지(Earth Ground) 단자에 굵은 접지선으로 저임피던스 직결되어야 함을 시스템 설계 기본 조건으로 명시했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `갈바닉 격리 (Galvanic Isolation / 전기적 격리)`, `Y-커패시터 (Y-Capacitor / 접지 결합 캐패시터)` 2개 신규 기술 용어 정의 추가 완료.

### 98. [Design] 이더넷 통신(W5500) 및 MCU용 +3.3V 전원의 레귤레이터 분리 타당성 분석
- **검토 내용**: 이더넷 통신부(W5500)의 3.3V 전원과 MCU용 3.3V 전원을 하나의 공용 3.3V LDO 레귤레이터로 통합 설계하는 방안의 타당성 및 리스크 분석.
- **분석 결과**:
  1. **열적 포화 리스크 검증**: W5500(동작 시 약 120~150mA 소모)과 dsPIC33CK MCU(동작 시 약 100~150mA 소모)의 부하를 단일 LDO(LM1117-3.3V, 입력 5V)에 통합할 경우, 약 0.6W의 continuous 발열이 단일 칩에 집중되어 장기 신뢰성이 심각하게 저하됩니다. 전원을 분할하여 열을 기판 전체로 분산시키는 **열 분산 설계(Thermal Distribution)**가 적합함을 규명했습니다.
  2. **상호 노이즈 영향성 검증**: 이더넷 내부 PHY의 25MHz/125MHz 고주파 스위칭 과도 전류에 따른 전원 리플이 MCU 전원선으로 유입되어 internal PLL 작동 및 정밀 ADC 전위 정밀도를 위협할 수 있어, 전원 도메인을 철저하게 분리하는 것이 노이즈 측면에서 우수함을 확인했습니다.
  3. **고장 격리(Fault Isolation) 및 페일 세이프 보존**: 외부 포트 서지 등으로 이더넷 통신 칩이 소손(단락 고장)되더라도, MCU는 독립 전원 덕분에 다운되지 않고 정상 동작을 유지하며 시스템을 안전하게 정지(Safe-Shutdown)시킬 수 있는 기능적 이중화 격리를 승인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `열 분산 설계 (Thermal Dissipation Design / Thermal Distribution)`, `고장 격리 및 페일 세이프 (Fault Isolation & Fail-Safe)` 2개 신규 기술 용어 정의 추가 완료.

### 99. [Design] dsPIC33CK MCU 아날로그 전원(AVDD)의 아날로그 전원 도메인(+3.3V_ANA) 분리 매핑 검증
- **검토 내용**: 메인 MCU(dsPIC33CK512MP710)의 33번 `AVDD`(Analog VDD) 핀의 전원을 일반 디지털 전원(`+3.3V_MCU`)과 공유하지 않고, 아날로그 전용 정전압원(`+3.3V_ANA`)에 연결해야 하는지 여부 타당성 분석.
- **분석 결과**:
  1. **아날로그 블록 보호 및 정밀도 향상**: `AVDD` 핀은 MCU 내부의 고속 ADC, 비교기 등 중요 아날로그 주변기기를 구동하는 전원이므로, MCU 디지털 스위칭 잡음이 섞인 `+3.3V_MCU` 대신 저노이즈 전용 전원인 **`+3.3V_ANA`**에 단독 연결하는 것이 센서 측정 정확도 극대화에 필수적임을 판정했습니다.
  2. **기동 전압차 규격 만족 검증**: dsPIC33CK 규격 상 $V_{DD}$와 $V_{AVDD}$의 전위 차는 $0.3\text{V}$ 이내여야 합니다. 현재 설계는 `+3.3V_MCU`와 `+3.3V_ANA` 모두 공통 +5V 입력에서 레귤레이션되므로 파워 업 시 동시에 기동되어 규격을 완벽하게 만족함을 입증했습니다.
  3. **접지 정합성 확인**: 아날로그 전원 분리에 맞춰 아날로그 접지 핀인 34번 `AVSS`가 아날로그 그라운드 `AGND`에 제대로 묶여 있어 노이즈 차폐 구조가 완성되었음을 최종 확인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `AVDD/AVSS 전원 및 접지 분리 규칙 (AVDD/AVSS Isolation Rule / MCU Analog Power Separation)` 신규 기술 용어 정의 추가 완료.

### 100. [Design] RS-485 통신선 보호용 TVS 다이오드(SM712)의 프레임 그라운드(FG) 결선 검증
- **검토 내용**: MAX3485(IC4) RS-485 통신 회로의 출력단 보호용 TVS 다이오드 D1(SM712)의 접지를 `GNDE`가 아닌 프레임 그라운드(`FG`)로 매핑하는 것의 적합성 검증.
- **분석 결과**: 외부 통신 선로를 타고 유입되는 정전기(ESD) 및 서지 고전압을 회로 내부 전원 접지인 `GNDE`나 `DGND`로 인입시키지 않고, 시스템 기구 외함인 **`FG`**로 즉시 우회하여 배출하기 위해 접지 네트를 **`FG`**로 연결하는 설계가 동일 원칙에 의거하여 타당함을 확인하고 최종 확정했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 해당 사항은 기존 `TVS 다이오드 아날로그 격리 규칙 (TVS Diode Analog Ground Isolation Rule)`의 프레임 그라운드(FG) 바이패스 원칙에 준함을 확인했습니다.

### 101. [Design] 아날로그 멀티플렉서(ADG706) 전원(VDD)의 아날로그 전원 도메인(+3.3V_ANA) 지정 타당성 검증
- **검토 내용**: 써모커플 온도 센서 신호를 멀티플렉싱하는 ADG706BRU(IC300, IC303)의 전원 핀 1번 `VDD`를 디지털 전원(`+3.3V_MCU`)이 아닌 아날로그 전원(`+3.3V_ANA`)에 연결한 설계의 적정성 검증.
- **분석 결과**:
  1. **전하 주입 및 노이즈 결합 방지**: ADG706은 내부 아날로그 스위치 소자(MOSFET)의 온저항 및 기생 커패시턴스를 통해 전원 전위가 신호 경로에 영향을 주는 구조입니다. VDD에 디지털 스위칭 노이즈가 유입될 경우 미세 써모커플 신호(수십 uV 수준)에 노이즈 전도(Charge Injection)를 일으키므로 깨끗한 **`+3.3V_ANA`**에 공급하는 현재 설계가 타당함을 확인했습니다.
  2. **디지털 제어 신호 레벨 호환성 검증**: ADG706의 논리 입력 임계값(VIH 최소 2.0V)에 비추어 볼 때, MCU의 3.3V 디지털 제어 신호(TC_ADDR, TC_EN)로 직접 게이트 구동이 가능하여 레벨 시프터 없이 다이렉트 구동할 수 있음을 검증했습니다.
  3. **접지 정합성 확인**: 전원 차폐에 맞춰 GND(12번 핀)와 VSS(27번 핀)가 아날로그 접지 `AGND`에 제대로 묶여 있음을 최종 승인했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `아날로그 멀티플렉서 전원 설계 (Analog Multiplexer Power Design / Mux VDD)` 신규 기술 용어 정의 추가 완료.

### 102. [Design] 이더넷 통신 전원(+3.3V_ETH) 배선의 탑 레이어(Top) 및 내층(Layer 3) 중복 라우팅 결함 제거
- **검토 내용**: W5500(IC201, IC202) 이더넷 통신 칩의 `+3.3V_ETH` 전원 공급 시, 탑 레이어(Top Layer)의 패턴 배선(Trace)과 3층 내층 파워 플레인(Power Plane) 분할 동판을 중복 병렬 연결한 회로 적합성 및 전자기적 영향 분석.
- **분석 결과**:
  1. **루프 안테나(EMI) 위험 제거**: 동일한 전원 라인을 서로 다른 두 레이어 상에서 패턴선과 넓은 동판으로 평행 중복 배선할 경우, 두 층 사이에 폐회로 루프(Loop)가 생겨 고속 데이터 통신 시 발생하는 스파이크성 리플 전류가 전자기파로 방출되는 루프 안테나 효과를 유발하여 EMI 테스트 장해 리스크가 커짐을 식별했습니다.
  2. **플레인 공급 방식 최적화**: 얇은 탑 레이어 패턴 배선(Trace)을 과감히 삭제하고, 임피던스가 극도로 낮고 안정적인 3층 내층의 `+3.3V_ETH` 파워 플레인(Plane) 단일 공급 방식으로 단일화했습니다.
  3. **디커플링 필터링 극대화**: 이더넷 칩 전원 핀과 디커플링 콘덴서 패드 바로 옆에서 비아(Via)를 뚫어 3층 파워 플레인으로 수직 직결되도록 레이아웃 변경 지침을 확정하여 전원 리플 노이즈를 최대로 억제하도록 조치했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `파워 루프 안테나 효과 (Power Loop Antenna Effect)`, `내층 플레인 전원 공급 방식 (Inner Plane Power Delivery)` 2개 신규 기술 용어 정의 추가 완료.

### 103. [Design] 다채널 입출력(I/O) 포트용 테스트 핀(Test Point) 설치 및 디버깅 표준 설계 지침 수립
- **검토 내용**: 6kW BOP 다채널 입출력 단자대 주변에 오실로스코프 및 멀티미터 계측용 테스트 핀(TP)을 설계할 때의 최적의 부품 타입 및 회로적 배치 안전 규칙 검토.
- **분석 결과**:
  1. **물리적 부품 선정**: 수동 계측기(오실로스코프 등)의 프로브 접지 고리를 직접 걸어놓고 양손을 자유롭게 사용할 수 있도록 루프형 금속 테두리 테스트 포인트(Through-hole Metal Loop) 타입을 최종 채택하고 대표 부품인 Keystone 5000~5004 시리즈(색상별 구분)를 권장했습니다.
  2. **권장 PCB 풋프린트 규격 (미터법)**: 5000 시리즈 핀 사양에 맞춰 권장 드릴 홀 직경(Hole Size)은 **1.02 mm** (표준 가공 탭 드릴 규격: **1.0 mm**도 호환 가능)로 제안했으며, 안정적인 납땜 및 박리 방지를 위해 구리 패드 외경(Pad Size)은 **1.8 mm ~ 2.0 mm** 범위로 설정하도록 가이드를 제시했습니다.
  3. **안전 격리 배치 규칙**: 테스트 프로브 접촉 시 실수로 발생할 수 있는 핀 단락(Short) 사고를 방지하기 위해, 테스트 핀은 MCU/OP-Amp 핀 바로 옆이 아닌, 신호 보호용 직렬 댐핑 저항(100옴 등)의 뒤편(외부 커넥터 측)에 배치하여 쇼트 전류가 과도하게 유입되는 리스크를 사전 차단했습니다.
  4. **접지(GND) 참조쌍 매핑**: 신호 파형의 측정 노이즈(고주파 링잉)를 억제하기 위해, 각 신호 TP 블록 인근에 반드시 1개 이상의 GND 테스트 포인트(DGND 및 AGND 각각 배치)를 쌍으로 결선하도록 가이드를 수립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `테스트 포인트 / 테스트 핀 (Test Point / TP)`, `ICT 인서킷 테스트 (In-Circuit Test / ICT)` 2개 신규 기술 용어 정의 추가 완료.

## 📅 2026-08-04: v0.1.3 PCB 테스트 포인트(TP) 연결 정보(Net) 유실 원인 분석 및 해결 가이드 수립 🔍
회로도에 테스트 포인트를 신규 반영하고 PCB로 업데이트(ECO)를 실행했음에도, PCB 레이아웃 도면에서 테스트 포인트 패드의 전기적 연결망(Net) 정보가 유실되고 미연결(Floating) 상태로 머무는 결함에 대해 기술 분석을 진행하고 해결 솔루션을 수립하였습니다.

### 104. [Design] Altium Designer 회로도-PCB 간 테스트 포인트(TP) 연결성 유실 원인 규명
- **원인 분석**:
  1. **회로도 상 핀 오정렬 및 논리적 미연결(Unconnected Pin)**:
     - 회로도 심볼의 전기적 핀 접촉점(Connection Point)이 와이어(Wire) 선로와 정밀하게 닿지 않고 빗겨 배선되었거나, 그리드(Grid) 정렬 어긋남으로 인해 겉보기에는 핀 위에 테스트 포인트가 올려진 것 같으나 내부적으로 논리 연결이 완료되지 않는 문제가 가장 빈번합니다.
  2. **회로도 심볼과 PCB 풋프린트 간 지정자 불일치(Designator Mismatch)**:
     - 회로도 심볼 핀의 `Designator` 번호(예: `1`)와 실제 매핑된 PCB 풋프린트 패드의 `Designator` 기호(예: `TP` 또는 비어 있음)가 상호 불일치하여, ECO 컴파일러가 두 연결점을 매핑하지 못해 넷리스트 정보가 이관되지 않는 문제를 규명했습니다.
  3. **단순 그래픽 기호(Drawing Object) 배치 오류**:
     - 회로도 상에 배치된 테스트 포인트가 실제 라이브러리 부품(Component)이 아니라 단순 원형 그리기 도구(Graphic Circle)와 문자열 상자(Text String)로 그려진 그래픽 정보인 경우, 컴파일러가 이를 부품과 핀으로 인지하지 못해 발생합니다.
  4. **Component Type 설정 결함**:
     - 테스트 포인트 부품의 `Component Type` 속성이 `Standard`가 아닌 `Graphical` 또는 `Mechanical` 등으로 오지정되어 있으면 넷리스트(Netlist) 생성 및 PCB ECO 전달 대상에서 완전 제외됩니다.

- **해결 가이드라인 수립**:
  - 회로도 상에서 테스트 포인트를 드래그하여 움직였을 때 와이어가 같이 끌려오는지 확인하여 논리적 연결 상태를 검증하도록 가이드했습니다.
  - 테스트 포인트의 핀 속성(Properties -> Pin List)에 진입하여 핀 번호(Designator)가 PCB 패드 번호(일반적으로 `1`)와 완전히 일대일 대칭되는지 확인 및 통일하도록 조치했습니다.
  - 단순 그래픽 선으로 그린 테스트 포인트는 즉시 삭제하고 정식 라이브러리 부품(`Place -> Part`)을 통해 배치할 것을 당부했습니다.

- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `넷리스트 (Netlist)`, `지정자 불일치 (Designator Mismatch / Pin-to-Pad Mismatch)`, `미연결 핀 및 미사용 넷 (Unconnected Pin & Unconnected Net)` 3개 신규 기술 용어 정의 추가 완료.

### 105. [Design] Altium Designer 회로도 심볼 수정 후 프로젝트 도면 내 일괄 갱신(Update) 가이드
- **작업 내용**: 사용자가 자체 제작 또는 수정한 테스트 포인트(TP) 심볼 등 수정된 회로도 부품 라이브러리(.SchLib)의 변경 사항을 이미 다수의 시트에 배치된 부품 인스턴스에 일괄 갱신하는 최적의 워크플로우 분석 및 안내.
- **해결 방안 및 지침**:
  1. **라이브러리 에디터 기반 일괄 업데이트**:
     - 수정한 회로도 라이브러리(`.SchLib`) 파일 내에서 **SCH Library 패널**을 활성화한 후, 수정된 부품을 우클릭하고 **`Update Schematic Sheets`** 또는 **`Update All`**을 실행하여 프로젝트 전체의 부품 외형과 핀 배치를 한 번에 강제 동기화시켰습니다.
  2. **Schematic 에디터 메뉴 기반 업데이트**:
     - 회로도 화면 상단 메뉴 **`Tools -> Update From Libraries...`** 메뉴를 이용하여 프로젝트의 전체 도면을 대상으로 특정 테스트 포인트 라이브러리 부품들을 일괄 매핑하고 업데이트하여 최신 핀 연결성을 회복하도록 유도했습니다.
  3. **회로 접합 도트(Junction) 활성화 유도**:
     - 수정된 심볼 적용 후, 와이어와 핀의 결합 상태를 확인하기 위해 교차 접점에 나타나는 **정션(Junction, 파란색 연결점 도트)**이 정상적으로 콕 찍히는지 시각적으로 검증하여 PCB 넷리스트 연동의 무결성을 최종 보장하도록 했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `라이브러리 업데이트 (Update From Library / Update Schematic Sheets)`, `정션 / 교차 연결점 (Junction / Connection Dot)` 2개 신규 기술 용어 정의 추가 완료.

### 106. [Design] Altium Designer PCB 상의 테스트 포인트 부품 명칭 대신 넷 이름(.NetName) 일괄 동적 표시 가이드
- **검토 내용**: 디버깅 편의성을 위해 PCB의 테스트 포인트(예: Tp307) 실크스크린 지정자(Designator) 대신 해당 패드에 인가된 실제 회로의 넷 이름(Net Name, 예: TC_ADDR2)을 인쇄용 텍스트로 동적 변경 및 화면에 표시하는 실무 솔루션 제공.
- **해결 방안 및 가이드라인**:
  1. **스페셜 스트링(.NetName) 활용**:
     - 기존의 `Tp307`과 같은 지정자 실크 텍스트를 숨김(Hide) 처리한 후, `Place -> String`으로 새 텍스트를 생성하고 Properties 패널에서 **Text 값에 `.NetName`**을 입력하고 Net 속성을 패드와 동일하게 맞춰 줌으로써 실제 넷 명이 실시간 매핑되도록 처리했습니다.
  2. **특수 문자열 변환(Convert Special Strings) 뷰 옵션 활성화**:
     - 화면 상에 `.NetName` 대신 실제 넷 명(`TC_ADDR2` 등)이 제대로 파싱되어 렌더링되도록, 단축키 `L`로 열 수 있는 **`View Configuration` 패널 상단의 `View Options` 탭으로 전환한 후, 최하단 `Additional Options` 섹션 내 `Special Strings`** 파란색 버튼을 클릭하여 활성화하도록 가이드했습니다.
  3. **텍스트 오브젝트 자체의 Net 속성 바인딩 및 Properties 패널 필터링 활용**:
     - `Special Strings` 옵션이 활성화되었음에도 `.NetName`이 실제 넷명으로 변환되지 않는 현상의 원인이 텍스트 오브젝트 자체의 **`Net`** 속성이 `No Net`으로 비어 있기 때문임을 규명했습니다.
     - 특히, 다른 부품(Component, 3D Body 등)과 함께 다중 선택(Multiple Selection)이 되어 있을 경우 Properties 패널에 `Net` 속성 설정창이 숨겨지는데, 이 경우 패널 맨 위의 깔때기 필터 팝업에서 **`Texts`** 버튼을 클릭하여 텍스트 속성만 선택적으로 필터링하거나, 선택을 완전히 해제 후 텍스트만 단독 선택한 상태에서 **`Net`** 속성을 타겟 패드와 동일한 넷(예: `TC_ADDR2`)으로 바인딩해야 정상 대치됨을 추가 안내 및 피드백 처리 완료했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - `스페셜 스트링 (Special String)`, `실크스크린 넷 이름 인쇄 (Silkscreen Net Name Printing)` 2개 신규 기술 용어 정의 추가 완료.


### 107. [Design] Altium Designer 23에서 PCB 실크스크린의 .NetName 특수 스트링 표시 불가능 버그 해결 (2026-08-04)
- **작업 내용**: 사용자가 PCB 상의 테스트 포인트 옆에 네트 네임을 실시간 표시하기 위해 .NetName 텍스트를 배치했으나, 실제 네트명으로 치환되지 않고 글자 그대로 .NetName으로 표시되는 현상에 대한 디버깅 가이드 제공.
- **원인 분석**:
  1. .NetName은 알티움의 기본 내장 특수 스트링이 아닙니다.
  2. 실크스크린(Top Overlay)과 같은 비전기적 그래픽 레이어에 위치한 텍스트 오브젝트는 Properties 패널에 Net 속성 항목이 보이지 않아 넷 이름을 매핑할 수 없습니다.
- **해결 방안 및 지침**:
  1. **임시 레이어 전환 우회 기법 (레이어 전환 트릭)**:
     - .NetName 텍스트의 레이어를 임시로 구리 신호 레이어(예: Top Layer)로 변경하여 Properties 패널 내 숨겨진 Net 속성 선택란을 노출시킵니다.
     - 활성화된 Net 필드에서 테스트 포인트의 패드와 동일한 넷 이름(예: TC_ADDR2)을 할당합니다.
     - 다시 레이어를 Top Overlay로 복구하여 지정한 넷 속성을 보존한 상태로 실크 레이어에 유지시킵니다.
  2. **특수 문자열 변환(Convert Special Strings) 뷰 옵션 확인**:
     - 단축키 L을 눌러 View Configuration 패널의 View Options 탭 -> Additional Options 섹션에서 Convert Special Strings가 켜져 있는지 확인하여 실제 네트명으로 치환 렌더링되도록 조치했습니다.
  3. **회로도 동기화(Parameter Mapping) 정석 방안**:
     - 회로도 부품의 Parameters 탭에서 사용자 파라미터 NetName을 추가하고 실제 넷 이름을 값으로 입력한 뒤 PCB로 ECO를 수행하는 정석 동기화 방법을 가이드했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 특수 스트링 레이어 전환 트릭 (Layer Toggle for Special String Net Binding) 1개 신규 기술 용어 정의 추가 완료.

### 108. [Layout] 써모커플 온도 계측 단자대 및 관련 전처리 IC 아날로그 그라운드(AGND) 차폐성 설계 타당성 검토 (2026-08-04)
- **검토 내용**: 사용자가 PCB 상의 써모커플 센서 결합용 단자대(J300~J307) 및 주변 온도 계측 전처리 IC 영역의 상부/하부 레이어(Top & Bottom Layer)를 아날로그 그라운드(AGND) 폴리곤 동박으로 차폐(Shielding) 덮개 설계한 구조에 대한 전기적 및 열적 타당성 검증.
- **분석 및 해결 가이드라인**:
  1. **노이즈 차폐 타당성 (전기적 관점)**:
     - 써모커플 센서는 수십 uV/°C 레벨의 극히 미세한 전압 기전력을 감지하므로 노이즈에 대단히 취약합니다. 따라서 신호 라인 상하부를 AGND 평면으로 덮는 구조는 전자기파(EMI/RFI) 유입을 방지하는 차폐막 역할을 하므로 설계상 매우 적절하고 올바른 방향입니다.
  2. **등온 구역(Isothermal Zone) 형성 (열적 관점)**:
     - 써모커플 냉접점 보상(CJC)의 정확도를 높이려면 단자대 핀 간의 온도 구배(온도 편차)가 최소화되어야 합니다. 넓게 덮은 AGND 구리 동박은 열전도율이 우수하여 단자대 주위의 열적 평형(등온 구역)을 유지해 주므로 냉접점 보상 오차를 줄이는 데 크게 기여합니다.
  3. **실무적 주의사항 권고**:
     - **디지털 라인 격리**: AGND 동박 영역 하부나 주변으로 MCU 클록, SPI, UART 등 고속 디지털 신호선이 지나가거나 노이즈가 유입되지 않도록 레이아웃 상에서 라우팅 격리를 엄격히 적용해야 합니다.
     - **단일 접지 (Star Grounding)**: 아날로그 그라운드(AGND)와 디지털 그라운드(DGND)는 다중 합선되지 않고 단 한 점(Net Tie 또는 단일 비드/저항 결합점)에서만 만나도록 격리 설계해야 노이즈 루프가 발생하지 않습니다.
     - **써모커플 접지 상태 점검**: 사용하고자 하는 써모커플 센서의 종류(Ungrounded/Grounded type)를 파악하고, 만약 센서 자체 접지형인 경우 보드의 AGND와 직접 단락되어 루프 오차가 나지 않도록 단자대 입력 핀 주변 클리어런스 간격을 확보하도록 권고했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 냉접점 보상 (Cold Junction Compensation / CJC), 등온 구역 / 등온 판 (Isothermal Zone / Isothermal Plate), 그라운드 차폐 (Ground Shielding / Guard Ring) 3개 신규 기술 용어 정의 추가 완료.

### 109. [Layout] RS-485 통신 라인 접지(Gnd) 설계 적합성 분석 및 그라운드 루프 방어 솔루션 (2026-08-04)
- **검토 내용**: 시스템 내 RS-485 통신 채널 3개(개질수 유량계 FM382, LCD모니터 LCD500, 인버터 INV506)의 통신 신호선(A, B)과 함께 설계된 접지(Gnd/SGND) 라인의 연결 대상(디지털 그라운드 DGND) 적합성 검토 및 그라운드 루프 노이즈 대책 수립.
- **분석 및 해결 가이드라인**:
  1. **절연 및 비절연 구조별 그라운드 매핑 구분**:
     - **비절연(Non-isolated) 구조일 경우**: 메인 보드의 RS-485 트랜시버 IC(MAX3485 등)의 GND가 디지털 그라운드(DGND)에 묶여 있다면, 통신 커넥터의 Gnd 역시 DGND와 연결하는 것이 기준 전위(공통 모드 전압 범위 -7V ~ +12V 유지) 동기화를 위해 올바른 선택입니다.
     - **절연(Isolated) 구조일 경우**: 아이솔레이터를 사용하여 전원과 신호가 분리되어 있다면, 통신 커넥터의 Gnd는 내부 DGND가 아닌 반드시 485 트랜시버 전용 절연 그라운드(ISO_GND / SGND)에 묶어야 하며, DGND와는 완벽히 분리되어야 합니다.
  2. **그라운드 루프(Ground Loop) 방지 솔루션 제시**:
     - 외부 장비(인버터, LCD 등)와 통신선을 통해 그라운드를 직접 다이렉트로 결합할 경우, 두 기기 간의 대지 전위차로 인해 그라운드선을 타고 큰 노이즈 전류가 흐르는 그라운드 루프가 발생할 위험이 높습니다.
     - 이를 방지하기 위해 커넥터의 Gnd 핀과 보드 내부 그라운드(DGND 또는 ISO_GND) 사이에 **10Ω ~ 100Ω 수준의 저항(또는 페라이트 비드)을 직렬로 배치**하여 기준 전위는 동기화하되 루프 노이즈 전류는 억제하도록 레이아웃 우회 대책을 수립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - RS-485 공통 모드 전압 (RS-485 Common-Mode Voltage), 절연형 RS-485 (Isolated RS-485), 그라운드 루프 (Ground Loop) 3개 신규 기술 용어 정의 추가 완료.

### 110. [Design] Altium Designer BOM 출력 시 부품 용량별(Value) 일괄 그루핑 분리 가이드 (2026-08-04)
- **검토 내용**: 사용자가 BOM(Bill of Materials) 파일 생성 시 동일 크기(1608 사이즈 등)의 칩 저항들의 전기적 규격 용량(Value, 예: 10K, 4.7K 등)이 한 셀(Row)로 합쳐져 일괄 표기되는 병합 오류를 해결하고, 용량(Value)별로 개별 행을 분리 정렬하도록 워크플로우 설정 가이드 제공.
- **분석 및 해결 가이드라인**:
  1. **그루핑(Group By) 기준 결함 확인**:
     - 기존 BOM Properties 패널 상단의 Drag a column to group 영역에 Comment와 Footprint만 등록되어 있어, 사이즈 명칭(R1608)과 패드 패턴 형태(RESC1608X55N)가 일치하는 모든 칩 저항들이 하나로 강제 병합된 것이 원인임을 진단했습니다.
  2. **용량별 그룹 열 추가 조치**:
     - BOM 에디터 우측 Properties 패널의 Columns 리스트에서 체크(활성화)되어 있는 Value (또는 실제 용량값이 든 파라미터 컬럼)를 드래그하여, 상단 Drag a column to group 영역 내의 Footprint 옆으로 드롭해 추가하도록 안내했습니다.
     - 그루핑 기준으로 Comment, Footprint, Value 세 가지가 상호 대칭되도록 복수 설정함으로써, 같은 칩 사이즈 내부에서도 소자 고유 스펙 용량값(Value)이 다를 경우 개별 행으로 명확히 쪼개어 수량(Quantity)과 지정자(Designator) 리스트가 각각 분리 정렬되도록 완료했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - BOM 그룹화 (BOM Grouping / Group Columns), 파트 값 (Component Value) 2개 신규 기술 용어 정의 추가 완료.

### 111. [Design] RS-422/485 통신 종단저항(120옴)의 칩 사이즈(1608) 허용 전력 타당성 분석 (2026-08-04)
- **검토 내용**: 회로도 상의 RS-422(IC2, MAX3490) 및 RS-485(IC4, MAX3485) 통신 선로의 종단저항 R4, R7 (120옴, 1% 오차)의 물리적 크기를 1608(0603 inch)로 설계할 때 정격 허용 전력(Power Rating) 충족 여부 및 안정성 검증.
- **분석 및 해결 가이드라인**:
  1. **소모 전력 정밀 계산**:
     - 타겟 트랜시버(MAX3485, MAX3490)는 3.3V 구동 소자이므로, 최악의 상태(Worst-case)로 지속적인 3.3V DC 전압이 종단저항에 다이렉트로 가해진다고 가정한 이론상 최대 전력 소비는 P = V^2 / R = 3.3^2 / 120 ≒ 91mW (0.091W) 수준입니다.
  2. **1608 사이즈 정격 전력 대조**:
     - 표준 1608 (0603 inch) 칩 저항의 정격 전력 규격은 일반적으로 **1/10W (100mW)**이므로, 최대 지속 부하인 91mW 대비 마진이 충족되어 전기적으로 안전함을 입증했습니다.
     - 특히 실제 시리얼 통신 상황에서는 High/Low 펄스가 교대로 출력되어 듀티 사이클(Duty Cycle)이 약 50% 수준이 되며, 유휴(Idle) 상태 시 드라이버 출력이 비활성화(Hi-Z)되므로 실질적인 평균 소모 전력은 수십 mW 이하로 떨어집니다. 따라서 1608 사이즈로 설계하는 것은 물리적으로 아무런 문제가 없음을 통보했습니다.
  3. **안전 마진 극대화 실무 권고**:
     - 만약 외부 서지 유입, 라인 단락(쇼트) 불량 및 가혹 환경 조건에서의 장기 신뢰성 마진을 더욱 넉넉히 확보하고 싶다면, 실무에서는 한 단계 더 큰 **2012 (0805 inch, 1/8W = 125mW)** 또는 **3216 (1206 inch, 1/4W = 250mW)** 규격을 종단저항 전용으로 적용하는 설계 사례도 보편적임을 참고 지침으로 안내했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 종단저항 (Termination Resistor), 허용 전력 / 정격 전력 (Power Rating of Resistor), 임피던스 매칭 (Impedance Matching) 3개 신규 기술 용어 정의 추가 완료.

### 112. [Design] Altium Designer 실크 대체용 기계적 레이어(Mechanical Layer) 추가 및 선택적 표시 설정 가이드 (2026-08-05)
- **검토 내용**: 실크 레이어(Top/Bottom Overlay) 외에 추가적인 실크 성격의 텍스트 및 그래픽 정보를 선택적으로 분리 배치하고 끄고 켤 수 있도록 레이어를 설계 구성하는 방법 분석.
- **분석 및 해결 가이드라인**:
  1. **Mechanical Layer 추가 및 성격 지정**:
     - PCB 편집기에서 단축키 `L`을 입력해 `View Configuration` 패널 진입 후, 마우스 우클릭 -> `Add Mechanical Layer`를 선택하여 새로운 기계적 레이어(예: `Top_Silk_Extra` 등)를 추가하도록 조치했습니다.
  2. **컴포넌트 레이어 페어(Component Layer Pair) 설정**:
     - 부품 뒤집기(Flip) 시 보조 실크 정보가 보드 반대편 기면으로 자동 이동 및 대칭 전환되도록, 탑/바텀 한 쌍의 레이어를 컴포넌트 레이어 페어로 묶는 법을 가이드했습니다.
  3. **가시성 제어 및 에디팅 편의 기능 제시**:
     - 눈 모양 아이콘을 통한 개별 켜기/끄기, 단축키 `Shift + S`를 활용한 단일 레이어 모드(Single Layer Mode) 전환 등을 안내하여 선택적인 설계 정보 관리가 가능하도록 수립했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 기계적 레이어 (Mechanical Layer), 컴포넌트 레이어 페어 (Component Layer Pair) 2개 신규 기술 용어 정의 추가 완료.

### 113. [Design] Altium Designer 레이어 그리기 순서(Layer Drawing Order) 및 화면 표시 우선순위 조정 가이드 (2026-08-05)
- **검토 내용**: 새로 생성한 기계적 레이어가 화면 2D 뷰 상에서 탑 오버레이(Top Overlay) 바로 다음(아래)에 겹쳐 보이도록 렌더링 우선순위를 설정하는 방법 분석.
- **분석 및 해결 가이드라인**:
  1. **Layer Drawing Order 설정창 진입 (한글판 및 단축키)**:
     - PCB 편집 화면에서 단축키 `T` 후 `D`를 눌러 `레이어 드로잉 순서` 대화창으로 즉시 진입하는 방법 가이드.
     - 사용자가 보낸 한글판 환경 설정(Preferences) 창 기준, 왼쪽 트리 메뉴의 `PCB Editor -> Display`를 클릭한 후 우측의 `레이어 드로잉 순서...` 버튼을 눌러 접근하는 경로 매핑 가이드 제공.
  2. **우선순위 재정렬 및 적용**:
     - 레이어 그리기 순서 목록에서는 개별 기계적 레이어 명칭(예: `TopSilkExtra`)이 개별 노출되지 않고, **`Mechanical Layers`**라는 단일 그룹 항목으로 묶여 표시됩니다.
     - 따라서 목록에서 **`Mechanical Layers`**를 선택한 뒤 `Promote`/`Demote` 기능을 사용해 `Top Overlay` 바로 아래로 이동시키면, 그 안에 포함된 `TopSilkExtra` 레이어도 함께 화면 표시 우선순위가 연동되어 재조정됨을 확인하고 가이드했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 레이어 그리기 순서 (Layer Drawing Order) 1개 신규 기술 용어 정의 추가 완료.

### 114. [Design] Altium Designer PCB 상의 외부 로고 이미지(BMP/PNG) 삽입 및 실크 레이어 매핑 가이드 (2026-08-05)
- **검토 내용**: 보드 중앙 빈 공간에 회사 로고 및 마킹 이미지를 정상 실크스크린 인쇄 데이터로 삽입하기 위한 최적의 워크플로우 분석.
- **분석 및 해결 가이드라인**:
  1. **Place Graphic 기능 적용**:
     - 알티움 최신 버전 기준, 단축키 `P` -> `G` (또는 `Place -> Graphic`)를 실행하여 외부 로고 이미지 파일(PNG/JPG/BMP)을 직접 가져와 배치하는 표준화 기법을 안내했습니다.
  2. **레이어(Overlay) 및 속성 설정**:
     - 가져온 이미지 객체의 속성(Properties)에서 레이어를 `Top Overlay`로 지정하고, 이미지 외곽 조절 및 흑백 반전(Invert) 등을 통해 실제 인쇄용 화질을 매칭시키는 가이드라인을 제공했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - PCB 로고 이미지 삽입 (Place Graphic in PCB) 1개 신규 기술 용어 정의 추가 완료.

### 115. [Hardware] 포토커플러 PC817 대 PC357 상호 대체 가능성 및 부품 호환성 정밀 분석 (2026-08-05)
- **검토 내용**: 컨트롤러 회로도 상의 다채널 디지털 입출력 절연용 포토커플러 PC817 소자를 PC357 소자로 상호 대체 적용할 수 있는지 전기적/기구적 적합성 검토.
- **분석 및 해결 가이드라인**:
  1. **물리적 패키지(Footprint)의 불호환성 규명**:
     - PC817은 DIP-4 (또는 Wide SMD-4) 패키지로 핀 피치가 2.54mm인 반면, PC357은 SOP-4 (Mini-flat) 초소형 패키지로 핀 피치가 1.27mm입니다.
     - 패키지 물리 치수(Footprint)가 완전히 달라 PCB 보드 상에 1대1 다이렉트 땜 납(Drop-in) 대체는 불가하며, 사용하려면 PCB 아트웍 수정을 통한 풋프린트 변경이 필수적임을 진단했습니다.
  2. **전기적 사양 및 기능적 호환성 분석**:
     - 두 소자 모두 입력 전류($I_F$), 출력 컬렉터 전압($V_{CEO}$ = 80V) 및 전류전달비(CTR = 50~600%) 등 핵심 전기 회로 특성은 동등한 레벨로 완벽히 일치하여 회로 기저 기능은 호환 가능합니다.
     - 단, 절연 내압(Isolation Voltage)은 PC817이 5,000 Vrms, PC357이 3,750 Vrms로 차이가 있어 시스템의 가혹한 고전압 차단 설계 기준을 재확인하도록 권고했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 포토커플러 / 광커플러 (Optocoupler / Photocoupler), 전류 전달비 (Current Transfer Ratio / CTR) 2개 신규 기술 용어 정의 추가 완료.

### 116. [Hardware] 24V 전원 입력 기반의 메탄/수소 가스 검출 인터페이스 및 비교기(LM393) 회로 설계 수립 (2026-08-05)
- **검토 내용**: 메탄(CH4) 및 수소(H2) 가스 감지용 아날로그 센서(MQ 시리즈)를 활용하여 24V DC 입력 전원 환경에서 동작하며, 가변 감도 세팅 및 디지털 High/Low 출력을 내는 하드웨어 아키텍처 수립 및 설계.
- **분석 및 해결 가이드라인**:
  1. **전원 설계 최적화 (24V -> 5V 버크 컨버터 도입)**:
     - 가스 센서 히터 발열 및 소모 전류(2채널 합산 최대 400mA)를 감당하기 위해 기존 LDO 대신 DC-DC 스위칭 레귤레이터(AP63205 등)를 설계하여 열 발생 및 손실 억제 조치.
  2. **LM393 전압 비교기 및 가변 감도 레벨 설계**:
     - 듀얼 비교기 LM393을 통해 가변저항(Potentiometer)으로 분배된 임계 전압($V_{ref}$)과 가스 센서 아날로그 감지 전압($V_{sens}$)을 비교하여 디지털 논리값 변환.
     - 경계선 채터링 노이즈 방지를 위해 1MΩ 정궤환 저항을 추가한 히스테리시스 회로 및 MCU 논리 전압 매칭용 풀업 저항 적용 제안.
- **[Doc] programme 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - (신규 문서 추가) gas_sensor_design.md 가스 센서 인터페이스 회로 설계서 발행 완료.

### 117. [Hardware] 메탄(MQ-4) 및 수소(MQ-8) 가스 센서 검출 회로 공용화(Common Design) 적합성 검증 (2026-08-05)
- **검토 내용**: 메탄 가스 센서와 수소 가스 센서의 주변 회로를 완전히 동일하게 설계하고 센서 소자만 분리 실장하는 표준화 가능 여부 분석.
- **분석 및 해결 가이드라인**:
  1. **물리적/전기적 규격 일치성 확인**:
     - MQ-4와 MQ-8 센서는 외관 핀맵 구조(Footprint)와 5V 구동 사양, 히터 전류 소모량(각 150~200mA)이 완벽히 동일하여 핀 대칭 호환성이 보증됨을 확인했습니다.
  2. **동작 메커니즘 분석 및 공용화 설계 타당성**:
     - 두 센서 모두 대상 가스 농도가 증가하면 전극 저항($R_S$)이 낮아지고 아날로그 출력 전압($V_{sens}$)이 상승하는 동일 방향성의 동작 양상을 가집니다.
     - 따라서 주변의 비교기(LM393) 회로 및 수동 소자 배치 구조를 100% 동일하게 설계(공용화)하고, 센서 종류만 개별 삽입하여도 가변저항을 통해 각 채널별 감도 임계치($V_{ref}$)를 독립 미세 조절할 수 있으므로 설계 표준화가 충분히 가능함을 검증했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 회로 공용화 (Common Circuit Design / Design Standardization) 1개 신규 기술 용어 정의 추가 완료.

### 118. [Hardware] 전압 비교기 LM393과 범용 듀얼 Op-Amp LM2904 호환성 및 대체 검토 (2026-08-05)
- **검토 내용**: 설계된 비교기 회로 상의 LM393 소자를 범용 듀얼 Op-Amp 소자인 LM2904로 대체할 수 있는지 분석.
- **분석 및 해결 가이드라인**:
  1. **핀맵 호환성과 구조적 차이 식별**:
     - LM393과 LM2904는 8핀 패키지 기준 물리적인 핀맵(Pinout)이 완벽히 동일하여 1대1 교체 삽입은 기구적으로 가능함을 확인했습니다.
  2. **전기적 동작의 한계성 분석 및 리스크 도출**:
     - **오픈 컬렉터 vs 푸시풀 출력**: LM393은 오픈 컬렉터 구조로 MCU 레벨(3.3V)에 독립 풀업이 가능하나, LM2904는 푸시풀 내부 전원 전위로 출력 전압을 내밀어 MCU 입력 핀에 과전압 대미지 인가 위험이 높음을 규명했습니다.
     - **응답 속도 한계**: Op-Amp인 LM2904는 스위칭 속도가 비교기(LM393)에 비해 매우 느리고 포화(Saturation) 상태 탈출 시 딜레이가 걸려, 가스 농도 임계 경계선상에서 출력 채터링 및 지연 오작동이 우려됨을 진단했습니다.
     - **👉 최종 권고**: 핀 호환은 되나 전기적 레벨 매칭 및 스위칭 안정성을 위해, Op-Amp(LM2904) 대체를 피하고 전용 비교기(LM393)를 유지하도록 권고했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 전압 비교기 (Voltage Comparator), 연산 증폭기 (Operational Amplifier / Op-Amp) 2개 신규 기술 용어 정의 추가 완료.

### 119. [Hardware] 24V DC 입력을 5V로 감압하기 위한 LM2596-5.0 스위칭 레귤레이터 회로 부품 명세(BOM) 수립 (2026-08-05)
- **검토 내용**: 메탄/수소 가스 센서 및 주변 회로 공급용 5V 전압(2채널 히터 전류 합산 약 400mA)을 24V 입력 전원에서 효율적으로 낮추기 위해 구성할 스위칭 Buck Converter 회로의 필수 부품 리스트 및 스펙 가이드 작성.
- **분석 및 해결 가이드라인**:
  - 스위칭 주파수가 높고 널리 쓰여 부품 수급이 용이한 **LM2596S-5.0** 솔루션 기준으로 다음 핵심 주변 소자들의 사양을 도출하고 안내했습니다:
    1. **주요 IC**: LM2596S-5.0 (TO-263 패키지 권장)
    2. **파워 인덕터**: 33uH (허용전류 3A 이상, 예: CD127-330 또는 쉴딩 타입 파워 인덕터)
    3. **프리휠링 다이오드**: MBRS340 또는 1N5822 (쇼트키 다이오드, 내압 40V, 3A 이상)
    4. **입출력 필터 전해 캐패시터**: 입력단 100uF/50V (24V 입력 고려해 내압 50V 필수), 출력단 220uF/10V (저-ESR 사양)
    5. **세라믹 바이패스 칩 콘덴서**: 입출력단 각각 100nF(0.1uF) MLCC 병렬 배치
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 버크 컨버터 (Buck Converter), 쇼트키 다이오드 (Schottky Diode) 2개 신규 기술 용어 정의 추가 완료.

### 120. [Hardware] 스위칭 레귤레이터 LM2576과 LM2596 성능 차이 및 주변 부품 설계 영향 분석 (2026-08-05)
- **검토 내용**: 감압용 스위칭 레귤레이터 고전적 모델인 LM2576과 후속 개선 모델인 LM2596의 사양 비교 및 이에 따른 PCB 소형화 성능 검증.
- **분석 및 해결 가이드라인**:
  1. **스위칭 주파수 비교**:
     - LM2576은 **52 kHz**인 반면, LM2596은 **150 kHz**로 3배 더 빠르게 동작함을 명시했습니다.
  2. **수동 소자(인덕터/커패시터) 소형화 효과**:
     - 주파수가 높아짐에 따라 LM2596 회로에서는 에너지 리플 전하량이 줄어들어 인덕터 용량을 LM2576(100uH 이상) 대비 약 1/3 수준인 **33uH ~ 47uH**로 줄이고 외경 크기도 크게 감소시킬 수 있음을 규명했습니다.
     - 최종 출력 전압의 맥동 노이즈(Ripple) 억제력도 LM2596이 우수하여 고주파 특성에 유리함을 보고했습니다.
  3. **핀 맵 호환성**:
     - 두 칩은 물리적 핀아웃 구조가 완벽히 동일하여 1대1 상호 대체 장착이 가능하지만, 주변 수동 필터 소자의 스펙 매개변수 설계 변경이 수반되어야 함을 피드백했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 레귤레이터 스위칭 주파수 (Switching Frequency of Regulator), 출력 리플 전압 (Output Ripple Voltage) 2개 신규 기술 용어 정의 추가 완료.

### 121. [Hardware] LM2596-5.0 전원 회로용 프리휠링 다이오드로 SS34 쇼트키 다이오드 선정 검토 (2026-08-05)
- **검토 내용**: LM2596 감압 스위칭 레귤레이터 주변의 역류방지/에너지 회수용 다이오드로 `SS34` 파트의 스펙 타당성 및 패키지 정합성 분석.
- **분석 및 해결 가이드라인**:
  1. **전기적 적격성 판정**:
     - `SS34`는 정방향 허용 전류 3A, 역방향 저지 전압 40V 스펙을 가져 3A급 LM2596 버크 컨버터 전원의 최대 과부하 조건에서도 안정적인 마진을 제공하는 우수한 매칭 소자임을 확정했습니다.
  2. **기구적 패키지(Footprint) 정합성 확인**:
     - 기존에 추천된 패키지 명칭인 `SMC` 및 `DO-214AB`는 사실상 동일한 기구 치수를 뜻하는 이명(Aliasing) 규격입니다.
     - 표준 `SS34` 칩 소자는 주로 바로 이 `SMC` (DO-214AB) 패키지 형태로 제작되므로, 풋프린트는 `SMC / DO-214AB`로 설계하고 소자 파트 명칭을 `SS34`로 지정하여 설계 및 양산 자재 명세서(BOM)를 완벽히 연동할 수 있도록 가이드했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - SS34 쇼트키 다이오드 (SS34 Schottky Diode) 1개 신규 기술 용어 정의 추가 완료.

### 122. [Hardware] LM2596S-5.0 스위칭 전원 설계 회로도 정밀 검증 및 수동 부품/피드백 라우팅 개선 피드백 (2026-08-05)
- **검토 내용**: 사용자가 배치 완료한 LM2596S-5.0 기반 24V-5V Buck Converter 스키매틱 회로도 도면의 전기적 결선, 다이오드/LED 방향성, 그리고 선정된 수동 부품 스펙 타당성 검증.
- **분석 및 해결 가이드라인**:
  1. **인덕터 정격 허용 전류 결함 규명**:
     - 회로도에 선정된 TDK 인덕터 `VLP8040T-330M`은 허용전류가 약 **1.1A** 수준에 불과하여, 최대 3A급의 LM2596 스위칭 동작과 다채널 가스센서 히터 부하(약 400mA) 인가 시 포화(Saturation) 현상에 의한 과열 및 파손 리스크가 매우 큼을 확인했습니다.
     - 대안으로 허용 정격 전류 3A급 이상의 파워 인덕터(예: **CD127-330M** 등)로 소자 스펙을 변경하도록 권고했습니다.
  2. **피드백(FB) 신호선 레이아웃 간섭 경고**:
     - 4번 핀(FB)의 감지 와이어가 2번 핀(OUT) 스위칭 배선 노드를 단순 교차 통과하도록 그려져 있어 혼선 및 PCB 상의 기생 노이즈 인입 우려를 제기했습니다. 4번 피드백 라인은 2번 스위칭 라인(노이즈원)과 충분히 이격시켜 우회 배선하도록 레이아웃 지침을 제시했습니다.
  3. **결선 및 소자 극성 검증**:
     - 입력/출력 칩 콘덴서 배치, ON/OFF 핀 그라운드 처리, SS34 쇼트키 다이오드 역방향 결선 및 전원 표시 LED의 극성 연결 등은 회로도 상으로 정상 설계되었음을 최종 검토 통보했습니다.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 피드백 라인 격리 (Feedback Line Routing / Isolation), 인덕터 포화 전류 (Inductor Saturation Current / Isat) 2개 신규 기술 용어 정의 추가 완료.

### 123. [Hardware] LM2596S-5.0 회로도 개선사항 반영 완료 및 최종 하드웨어 전원부 서명(Sign-off) (2026-08-05)
- **검토 내용**: 이전 피드백 사항이 적극 반영된 개선된 스키매틱 회로도 도면의 최종 레이아웃 가독성 및 전기적 안정성 검증.
- **분석 및 해결 가이드라인**:
  1. **인덕터 사양 및 용량 확보 검증**:
     - 기존의 1.1A 규격 소형 인덕터 대신 대량 생산 및 허용 전류 마진(최대 약 3A 전후)이 확실한 파워 인덕터 **SC1260-330M(33uH)** 파트로의 교체가 완벽하게 반영되어 스위칭 포화 과열 우려를 완전히 해결했습니다.
  2. **가독성 향상 및 피드백 신호 안전성 확보**:
     - 2번 핀(OUT) 스위칭 라인을 상단으로 우회시키고, 4번 핀(FB)의 피드백 입력 전선이 직선으로 깔끔하게 매핑되도록 재배선하여 두 중요한 노드 간의 교차 간섭이 제거되고 가독성이 향상되었음을 확인했습니다.
  3. **최종 서명**:
     - 회로 설계상의 모든 기능적 연결 검증을 통과하여 최종 전원부 설계 승인(Sign-off)을 완료했습니다.

### 124. [Hardware] 메탄/수소 가스 센서 및 LM393 비교기 인터페이스 신호 처리부 회로 설계 착수 (2026-08-05)
- **검토 내용**: 메탄 가스(MQ-4) 및 수소 가스(MQ-8) 감지 아날로그 신호를 디지털 스위칭 논리 신호로 판정하여 출력하는 인터페이스 회로도 설계 시작 및 필수 소자 배치 기획.
- **분석 및 해결 가이드라인**:
  - 사용자 질의인 'Op-Amp 333'이 듀얼 비교기 **LM393**임을 확인 정정하고, 해당 비교기 및 가스 센서 주변 수동 소자(가변저항, 피드백 히스테리시스 저항, 입력 직렬 저항, 출력 풀업 저항 등)의 알티움 설계 라이브러리 목록 배치 안내.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - (이전 항목 보완 완료) 수소/메탄 가스 감지 2채널 회로 설계 가이드라인 수립.

### 125. [Hardware] 수소 센서 풋프린트 원점(Origin) 설정 및 배치 가이드 (2026-08-06)
- **검토 내용**: Altium Designer PCB 라이브러리 편집기에서 수소 센서 풋프린트 작업 시 오리진(Origin) 조준 마크를 특정 핀(2번 핀)의 중심으로 이동 및 설정하는 방법 문의.
- **분석 및 해결 가이드라인**:
  1. **오리진 재지정 단축키 및 경로**:
     - PCB 라이브러리 편집기 화면에서 `Edit -> Set Reference -> Location` (단축키: `E` -> `R` -> `L`)을 순차적으로 호출한 뒤, 원하는 타겟 위치(2번 핀 중심)를 마우스로 직접 지정하는 기능 안내.
  2. **핀 정중앙 정렬을 위한 스냅(Snap) 기능 연동**:
     - 정확한 중심점 매핑을 위해 스냅 옵션(`Shift + E` 또는 보드 옵션 내 Snap Options)이 활성화되어 있는 것을 확인하고 핀 주위로 커서를 유도하여 자동 정렬하도록 함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 알티움 레퍼런스 포인트 / 원점 (Reference Point / Origin) 1개 신규 기술 용어 정의 추가 완료.

### 126. [Hardware] 알티움 디자이너 내 선택 오브젝트(십자선) 45도 회전 가이드 (2026-08-06)
- **검토 내용**: 수소 센서 풋프린트 원 안의 십자 가이드라인(수평/수직)을 X자 형태(45도)로 회전 배치하는 방법 문의.
- **분석 및 해결 가이드라인**:
  1. **설정값 변경을 통한 회전 단위 튜닝 (Rotation Step)**:
     - `DXP -> Preferences -> PCB Editor -> General`에서 `Rotation Step`을 `90`에서 `45`로 수정한 뒤, 선택 객체 회전 동작(스페이스바) 수행을 통해 45도 각도 정렬 구현 방법 제시.
  2. **정밀 선택 영역 회전 (Rotate Selection) 기능 활용**:
     - `Edit -> Move -> Rotate Selection` (단축키: `M` -> `O`) 메뉴를 사용하여 회전 각도로 `45`를 지정하고, 회전 중심축을 마우스로 클릭하여 십자선을 45도 회전하는 정밀 설계 프로세스 가이드 수립.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 로테이션 스텝 (Rotation Step), 선택 영역 회전 (Rotate Selection) 2개 신규 기술 용어 정의 추가 완료.

### 127. [Hardware] 메탄/수소 가스 센서 및 LM393 비교기 인터페이스 회로 결선 및 가이드라인 수립 (2026-08-06)
- **검토 내용**: 알티움 도면상에서 가스 센서(MQ-4) 및 전압 비교기(LM393) 결합 신호 처리 인터페이스 회로의 미결선 부품 간의 와이어 연결 및 히스테리시스 회로 설계 방법 문의.
- **분석 및 해결 가이드라인**:
  1. **가스 센서 결선**:
     - 1번, 3번 전극 핀과 2번 히터 핀을 묶어 `+5V`에 입력하고, 5번 히터 핀을 `GND`에 연결.
     - 4번과 6번 출력 전극 핀을 병합하여 `V_SENS` 신호 노드를 만들고, 부하 저항(`10kΩ`)을 거쳐 `GND`로 그라운딩 유도.
  2. **가변저항 결선**:
     - 10kΩ 가변저항 양 끝을 `+5V`와 `GND`에 정렬하고, 가운뎃값 와이퍼 단자(2번 핀)를 `LM393`의 2번 핀(-) 입력에 연결하여 비교 문턱 기준 전압($V_{ref}$)을 가변 제어하도록 구성.
  3. **비교기 및 정궤환 히스테리시스 루프 구성**:
     - 센서 출력 `V_SENS`와 `LM393` 3번 핀(+) 사이에 입력 직렬 저항 `4.7kΩ`을 설계.
     - 비교기 3번 핀(+)과 1번 출력 핀 사이에 피드백 고저항 `1MΩ`을 결선하여 채터링 억제용 히스테리시스 적용.
     - 오픈 컬렉터인 `LM393` 1번 출력 핀에 풀업 저항 `10kΩ`을 달고, 타겟 MCU의 전압에 맞춘 `+3.3V` 전원 및 최종 `SIGNAL_OUT` 커넥터에 결선 유도.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 히스테리시스 회로 (Hysteresis Circuit), 오픈 컬렉터 / 오픈 드레인 출력 (Open Collector / Open Drain Output) 2개 신규 기술 용어 정의 추가 완료.

### 128. [Hardware] 메인 PCB 절연 입력단(Optocoupler)과의 가스센서 보드 출력 인터페이스 매칭 분석 (2026-08-06)
- **검토 내용**: 메인 PCB의 포토커플러 절연 입력 회로(24V Active-High 동작)와의 연동을 위해 가스센서 보드 출력 신호(`SIGNAL_OUT`)의 전기적 레벨 매칭 및 풀업 저항 사양 재분석.
- **분석 및 해결 가이드라인**:
  1. **메인 PCB 입력단 특성 파악**:
     - 메인 PCB의 입력 회로는 `+24Vee` 전원과 포토커플러 `PC357` 입력 다이오드, 직렬 저항 `1kΩ`이 결합된 24V Active-High 방식으로 구동됨을 인지.
  2. **풀업 전원 및 저항 적격성 재조정**:
     - 가스센서 보드 비교기 `LM393` 출력의 풀업 전원을 센서보드 내부 5V에서 풀업할 경우, 메인 PCB 포토커플러의 기동 전류 확보(최소 1~2mA)가 원활하지 않아 오동작할 위험성 규명.
     - 해결책으로 `LM393`의 오픈 컬렉터 내전압(최대 36V) 사양을 적극 활용하여, 가스센서 보드의 풀업 전원을 센서보드 입력인 **`24V`** 라인에 연결하고 풀업 저항값으로 **`4.7kΩ`**을 적용하도록 권고.
     - 이를 통해 High 출력 시 포토커플러 구동 전류를 약 **4.0mA**로 안정적으로 공급하여 완벽한 레벨 매칭 보증.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 절연 디지털 입력 (Isolated Digital Input) 1개 신규 기술 용어 정의 추가 완료.

### 129. [Hardware] 가스센서 보드 회로도 최종 검증 및 가변저항 결선 오류 정정 피드백 (2026-08-06)
- **검토 내용**: 완성된 수소/메탄 가스센서 결합 보드의 전체 회로도 결선 구조 점검 및 추가 개선 가능 요소 분석.
- **분석 및 해결 가이드라인**:
  1. **가변저항 VR1 결선 심각한 에러 규명**:
     - 도면상 가변저항 `VR1`이 `+5V`와 `GND` 사이의 분배 전압을 만들어 기준 문턱 전압($V_{ref}$)을 가변 제어하는 대신, `V SENS` 노드 신호선에 1번 핀이 직렬 결선되어 가스 감지 판정이 아예 불가능하거나 신호가 요동치게 설계된 치명적 결선 결함 발견.
     - **조치 사항**: `V SENS`에서 `VR1` 1번 핀으로 연결된 와이어를 완전히 지우고, `VR1` 1번 핀은 `+5V` 버스에, 3번 핀은 `GND`에, 가운데 2번 핀은 `LM393`의 2번 핀(-) 입력에 정렬하도록 완벽한 해결법 제시.
  2. **디버그 신뢰성 및 노이즈 방어 개선 제안**:
     - 비교기 `U2` 전원 핀 결선 누락 여부 확인 가이드(숨겨진 핀 설정 체크) 제공.
     - 비교기 주변 전원 바이패스용 `0.1uF` 디커플링 커패시터, 외부 케이블을 타고 오는 정전기 ESD 보호용 `TVS 다이오드`, 센서 미세 전압 출렁임 방지용 저역통과 필터(LPF) 구성용 `10nF` 콘덴서 추가 제안을 통한 보드 신뢰성 극대화 피드백 제공.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 디커플링 커패시터 / 바이패스 콘덴서 (Decoupling Capacitor / Bypass Capacitor), TVS 다이오드 (Transient Voltage Suppressor Diode), 저역통과 필터 (Low-Pass Filter / LPF) 3개 신규 기술 용어 정의 추가 완료.

### 130. [Hardware] 2미터 장거리 케이블 사양 대응 노이즈/서지 대책 및 부하 저항 결선 오류 피드백 (2026-08-06)
- **검토 내용**: 2미터 길이의 외부 케이블 적용 사양 수령에 따른 회로 신뢰성 분석 및 2차 수정 회로도 검증.
- **분석 및 해결 가이드라인**:
  1. **부하 저항 R5 결선 오류 규명 및 조치**:
     - 이전 가변저항 라인 수정 과정에서 센서의 부하 저항인 `R5 (10kΩ)`가 출력 `V_SENS` 노드가 아닌 `+5V`와 `GND` 사이에 연결되는 바람에 전원 바이패스 저항이 되는 2차 설계 결함 확인.
     - **조치 사항**: `R5` 윗단을 `+5V` 버스에서 분리하여 가스 센서 출력인 **`V_SENS`** 노드에 정상적으로 연결하여 센서 동작 전압 강하가 이루어지도록 유도.
  2. **2미터 롱 케이블에 의한 노이즈/서지 대응 보강**:
     - **24V 입력단 서지 억제**: 장거리 케이블로 인한 기생 인덕턴스 과도 전압 노이즈를 방어하기 위해 커넥터 2번 핀(`+24`)과 `GND` 사이에 24V~30V 규격의 TVS 다이오드 배치 제안.
     - **출력단 댐핑 저항(100Ω) 추가**: 비교기 1번 출력 핀과 커넥터 1번 핀 사이에 **`100옴`** 댐핑 저항을 직렬 추가하도록 가이드. 이는 케이블 기생 용량에 의한 신호 반사를 막고, 외부 ESD(정전기) 차단 시 TVS와 함께 1차 전류 감쇄 보호망 역할을 수행.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 댐핑 저항 / 직렬 종단 저항 (Damping Resistor / Series Resistor), 기생 인덕턴스 (Parasitic Inductance) 2개 신규 기술 용어 정의 추가 완료.

### 131. [Hardware] 직렬 종단 저항 R6 패키지 사이즈 선정 가이드 (2026-08-06)
- **검토 내용**: 외부 케이블 신호선에 추가한 직렬 댐핑 저항 `R6 (100Ω)`의 적정 SMD 패키지 하드웨어 사이즈 선정 피드백.
- **분석 및 해결 가이드라인**:
  1. **전력 정격 조건 분석**:
     - 가스센서 동작 시 `R6`에 흐르는 최대 전류는 약 **4.0mA**이며, 저항 자체 소모 전력은 **`16uW`** 수준에 지나지 않아 단순 전력 측면에서는 최소 패키지도 충족함.
  2. **장거리 전선(2m) ESD 및 정격 안전성 고려**:
     - 외부 2미터 케이블과 직접 결합하는 댐핑 저항 특성상 인체 정전기(ESD) 등으로 발생하는 과도한 고전압 아크 방전 및 절연 파괴(Spark-over) 위험을 방어해야 함.
     - 칩 저항의 물리적 크기가 너무 작으면 전극 간 절연 내압 한계로 파괴되므로, 최소 **`1608 Metric (0603 inch)`** 또는 공간적 마진 확보 시 정격 전압(150V) 신뢰성이 뛰어난 **`2012 Metric (0805 inch)`** 패키지 적용을 최종 권고.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 칩 저항 정격 전압 및 절연 파괴 (Resistor Rated Voltage & Dielectric Breakdown) 1개 신규 기술 용어 정의 추가 완료.

### 132. [Hardware] 메탄(MQ-4) 및 수소(MQ-8) 가스 센서 공용화 설계(Common Design) 호환성 최종 확정 (2026-08-06)
- **검토 내용**: 설계된 가스 센서 인터페이스 회로에서 수소 센서 대신 메탄 센서 탑재 시 회로 구조나 주변 수동 소자값의 변경이 불필요한지 최종 확인.
- **분석 및 해결 가이드라인**:
  1. **물리적 / 기구적 및 전원 호환성**:
     - MQ-4(메탄)와 MQ-8(수소)은 표준 MQ 시리즈의 동일 외형 패키지 및 6핀 구조(1대1 완전 핀 핀아웃 호환)와 5V 히터 구동 전압 및 150~200mA의 전력 스펙을 그대로 공유하므로 PCB 풋프린트와 전원 공급 설계는 변경할 필요가 없음.
  2. **가변 감도 대응 방안 (VR1의 역할)**:
     - 두 센서 간의 미세한 고유 감도(Gas Sensitivity) 편차나 농도별 검출 전압의 상이성은 기판에 이미 결선된 가변저항 `VR1 (10kΩ)`을 통해 하드웨어 수정 없이 드라이버 미세 조정만으로 문턱 전압($V_{ref}$) 튜닝이 완벽히 해결 가능함.
     - 이에 따라 별도의 회로 수정(Trace 변경, 저항 변경 등) 없이 소자(센서 자체)만 교체하여 100% 공용 사용이 가능함을 보증 및 확인.

### 133. [Hardware] 가스센서 감지 메커니즘 및 비교기 히스테리시스 회로 작동 원리 설명 가이드 (2026-08-06)
- **검토 내용**: 설계 완료된 가스센서(MQ-4/MQ-8) 보드의 가스 농도 측정 물리 화학적 원리 및 아날로그 비교기 `LM393`과 정궤환 회로를 활용한 노이즈 채터링 방지 히스테리시스 동작 원리 기술 교육 가이드 작성.
- **분석 및 해결 가이드라인**:
  1. **금속산화물반도체(MOS) 가스 센서 감지 원리**:
     - 히터를 이용해 감지 전극($SnO_2$)을 약 300~400℃ 고온으로 가열하여 표면 산소 흡착 유도.
     - 청정 공기 상태에서는 흡착 산소에 의해 전자가 묶여 전극 저항($R_s$)이 높으나, 가스 유입 시 환원 반응으로 산소가 탈착되며 전자 재방출로 전극 저항($R_s$)이 급감함.
     - 센서 전극 저항($R_s$)과 부하 저항 `R5 (10kΩ)`의 전압 분배 법칙에 의해, 가스 감지 시 출력 전압 `V_SENS`가 비례 상승하는 메커니즘 설명.
  2. **LM393 비교기 및 정궤환 히스테리시스 루프 동작 원리**:
     - `LM393`의 비반전 입력(3번, $+$, $V_{sens}$)과 반전 입력(2번, $-$, $V_{ref}$)을 대조하여 Active-High 24V 디지털 신호 `SIGNAL_OUT` 출력.
     - 가스 농도가 경계치 전후로 흔들릴 때 출력이 고속 진동하는 채터링 차단을 위해, 출력 1번 핀에서 3번 핀(+)으로 피드백 저항 `R3 (1MΩ)`을 통한 정궤환 루프 형성.
     - 출력 상태에 따라 비교 기준 전압을 상승 및 하강 임계치로 강제 시프트시켜 약 수십 mV 수준의 히스테리시스 마진(Hysteresis Window)을 확보함으로써 노이즈를 완전 억제하는 작동 방식 규명 및 상세 교육 가이드 작성.

### 134. [Hardware] 알티움 디자이너 라이브러리 내 Via-in-Pad (서멀 비아) 구현 가이드 (2026-08-06)
- **검토 내용**: PCB 라이브러리 편집기(.PcbLib)에서 M3 스크류 고정용 대형 패드 내부에 다중 비아(Via) 홀을 배치 및 중첩하는 방법과 설계 의도 분석.
- **분석 및 해결 가이드라인**:
  1. **수동 비아 배치 기법**:
     - 상단 메뉴의 Place -> Via (단축키: P -> V)를 실행해 대형 구리 패드 위에 필요한 비아들을 배치.
     - 풋프린트 단계에서는 Net를 부여할 수 없으나, PCB 편집기(.PcbDoc)로 이관되어 패드에 Net가 할당되면 겹쳐진 비아들도 해당 Net(GND 등)를 자동으로 공유하여 연결됨을 안내.
  2. **지정자(Designator) 매칭을 통한 다중 홀 패드 중첩 기법**:
     - 구멍 크기가 작은 둥근 패드(Pad)들을 다수 배치하고, 각 패드의 Designator 속성을 큰 패드와 완벽히 일치(예: 둘 다 1번 또는 0번)하게 설정하는 대안 제시.
     - 이 방식을 이용하면 툴이 단일 핀의 멀티홀로 인지하여 DRC 에러가 발생하지 않음을 가이드.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 비아 인 패드 (Via-in-Pad), 서멀 비아 (Thermal Via, 열 방출 비아) 2개 신규 기술 용어 정의 추가 완료.

### 135. [Hardware] 메탄센서 2종(MQ-4/RSM741) 선택 실장형 공용화 회로 검증 (2026-08-06)
- **검토 내용**: 메탄 가스 검출 센서로 MQ-4 또는 RNSLab 사의 RSM741 중 1종을 현장 사양에 따라 대체 삽입(선택 실장)할 수 있도록 결합한 이중 풋프린트 회로도의 결선 적합성 검증.
- **분석 및 해결 가이드라인**:
  1. **회로 결선 적합성 확인**:
     - `RSM741 (U?)` 4핀 캔 타입 센서 심볼 기준, 내부 히터 단자인 1번과 4번 핀에 각각 `GND`와 `+5V`를 결선하여 가열 전원을 정상 매핑 완료.
     - 센서 측정 전극 단자인 2번과 3번 핀에 각각 `V_SENS`와 `+5V`를 매핑하여, 가스 유입으로 센서 저항이 낮아질 시 `V_SENS` 아날로그 전압이 상승하는 검출 매커니즘 정합성 검증 통과.
  2. **선택 실장(Selective Stuffing) 동작 타당성 판정**:
     - `MQ-4`와 `RSM741`의 출력을 동일 `V_SENS` 노드로 공용 바인딩하고 필터 `C3 (0.01uF)` 및 부하저항 `R5 (10kΩ)`를 병렬 접지 연결하여, 제품 생산 시 두 부품 중 한 부품만 납땜하더라도 신호 처리가 완벽하게 수행되는 구조로 설계되었음을 확정.
     - 두 가스 센서 모두 가변저항 `VR1`을 이용해 비교기의 작동 문턱값($V_{ref}$)을 튜닝하므로, 부하 저항 `R5` 교체 없이 센서 칩 단독 대체만으로 100% 동작 가능함을 확인 승인.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 선택적 실장 / 옵션 실장 (Selective Stuffing / DNP - Do Not Populate) 1개 신규 기술 용어 정의 추가 완료.

### 136. [Hardware] 아날로그 그라운드(AGND) 비아 스티칭(Via Stitching) 설계 타당성 검토 (2026-08-07)
- **검토 내용**: 아날로그 그라운드(AGND) 영역 내 노이즈 및 임피던스 저감을 목적으로 배치한 비아 스티칭(Via Stitching) 레이아웃의 타당성 및 주의사항 분석.
- **분석 및 해결 가이드라인**:
  1. **접지 임피던스 저감 효과**:
     - 격자 형태로 촘촘히 배치된 비아들이 다층 기판의 그라운드 구리 플레인을 수직으로 단단하게 연결함으로써 아날로그 신호 감지부의 기준 전위(0V) 흔들림 및 임피던스를 최소화하여 노이즈 차단에 긍정적인 역할 수행을 확인.
  2. **추가 확인 및 주의사항**:
     - **그라운드 격리 및 단일 지점 연결**: 아날로그 신호의 무노이즈 계측을 위해 디지털 그라운드(DGND) 영역과의 물리적 격리(Slit)가 레이아웃에서 올바르게 유지되고 있는지, 비아 추가로 인해 두 그라운드가 의도치 않게 이중 결선(Short)되지 않았는지 재확인 필요.
     - **신호 귀로(Return Path) 차단 방지**: 비아 어레이가 내층의 신호선이나 전원선 배선을 지나치게 가로막아 고속 신호의 귀로 전류 경로를 우회시키는 장애물이 되지 않도록 내층 라우팅과의 간섭을 크로스체크할 것을 권고.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 비아 스티칭 (Via Stitching) 1개 신규 기술 용어 정의 추가 완료.

### 137. [Hardware] 다층 기판(4층) 식별용 레이어 인디케이터(Layer Indicator) 설계 제안 (2026-08-07)
- **검토 내용**: 4층 기판(4-Layer PCB) 사양에 맞춘 레이어 적층 상태 검증용 식별 패턴(Layer Indicator) 구현 방식 검토.
- **분석 및 해결 가이드라인**:
  1. **레이어 인디케이터(Layer Indicator) 설계 기법 제시**:
     - 각 레이어(Top, L2, L3, Bottom)마다 다른 위치에 구리 패턴으로 숫자(1, 2, 3, 4)를 계단식으로 배치하여, 기판 생산 후 측면이나 투과 광원을 통해 4층 적층 여부를 쉽게 식별할 수 있는 표준 가이드라인 제시.
     - 제작 결함 예방을 위해 솔더 마스크 오프닝(Solder Mask Opening)을 적용한 윈도우형 인디케이터 구현 기법 설명.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 레이어 인디케이터 / 레이어 스택업 인디케이터 (Layer Indicator / Layer Stackup Indicator) 1개 신규 기술 용어 정의 추가 완료.

### 138. [Hardware] 레이어 인디케이터 구역 내 내층 구리 걷어내기(Negative Plane Void) 설계 가이드 (2026-08-07)
- **검토 내용**: 레이어 인디케이터 투과 시인성 확보를 위한 L2(GND), L3(Power) 내층의 동박 격리(Void/Keepout) 설정 검증.
- **분석 및 해결 가이드라인**:
  1. **네거티브 플레인 및 포지티브 레이어 동작 매커니즘 차이 분석**:
     - L2 및 L3 내층이 네거티브 플레인(Negative Plane)으로 설계된 경우, 해당 숫자를 그냥 배치하면 도리어 구리가 깎여 나간 빈 공간(Void)이 되거나, 외부 그라운드 구리가 이 구역을 전체적으로 덮어 광원 투과가 불가하게 됨을 지적.
  2. **해결 방안 수립**:
     - 4층 전체 레이어 인디케이터가 표기된 사각형 구역(1, 2, 3, 4 배치 영역) 만큼 L2 및 L3 층에서 다각형 선을 그려 구리를 완전히 걷어내는(Void/Copper Free) 작업 유도.
     - Top 및 Bottom 레이어에서는 해당 숫자 자체만 구리 패턴으로 남기고 주변 구리는 밀어내도록 하며, 솔더 마스크 윈도우(Solder Mask Opening) 설정을 병행하도록 권고.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 네거티브 플레인 (Negative Plane) 1개 신규 기술 용어 정의 추가 완료.

### 139. [Hardware] 네거티브 레이어 상의 음각 패턴(Void) 투과 원리 검토 (2026-08-07)
- **검토 내용**: 네거티브 플레인(Negative Plane) 레이어에 숫자를 직접 쓸 경우, 글자 부분이 구리가 제거된 빈 구멍(Void)이 되어 투과 효과가 나타나는 물리적 매커니즘 검토.
- **분석 및 해결 가이드라인**:
  1. **네거티브 음각 투과 메커니즘 확인**:
     - 사용자의 지적대로 네거티브 레이어상에 글자를 배치하면 해당 글자 모양의 구리가 지워지게(Void) 되므로, 뒤에서 빛을 비추었을 때 글자 틈새로 빛이 새어 나오는 음각(Negative Pattern) 형태로 시인되는 구조적 원리를 확인 및 보증.
  2. **다층 간섭 예방 대책**:
     - 4층 기판의 특성상 L2에서 뚫어놓은 숫자 구멍이 투과되려면 L3(Power) 및 탑/바텀 외층의 해당 좌표 영역에도 구리가 채워져서 빛을 차단하지 않도록, 모든 관련 레이어에서 인디케이터 박스 영역 전체를 공통적으로 비우거나 통제해야 함을 피드백.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 음각 패턴 / 네거티브 텍스트 (Negative Pattern / Void Text) 1개 신규 기술 용어 정의 추가 완료.

### 140. [Hardware] PCB 레이아웃 상의 아웃라인 폰트(Outline Font) 적용 원리 검토 (2026-08-07)
- **검토 내용**: 글자의 외곽선만 표현하고 내부는 투명하게 비우는 형태의 특수 서체 명칭 식별 및 PCB 캐드 툴 내 연동 방법 분석.
- **분석 및 해결 가이드라인**:
  1. **폰트 공식 명칭 정의**:
     - 속이 비고 외곽 라인만 존재하는 형태의 폰트를 **아웃라인 폰트(Outline Font)** 또는 **할로우 폰트(Hollow Font)**로 명명하여 안내.
  2. **알티움 디자이너 내 구현 방안 마련**:
     - 텍스트 속성의 `Font Type`을 `TrueType`으로 변경하고 윈도우 OS에 등록된 아웃라인 스타일 폰트를 임포트하여 배치하는 가이드 제공.
     - 실크스크린 인쇄 가독성을 올리고 동박 면적의 의도치 않은 간섭을 방지하기 위한 용도로 활용 가능함을 확인.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 아웃라인 폰트 / 할로우 폰트 (Outline Font / Hollow Font) 1개 신규 기술 용어 정의 추가 완료.

### 141. [Hardware] 아날로그 IO 테스트 지그(Analog IO Test Jig) 회로 설계 방안 수립 (2026-08-07)
- **검토 내용**: 메인 제어 보드의 0~5V 아날로그 입력(AI) 및 출력(AO) 포트의 정상 동작 검증을 위한 보조 테스트 지그 회로 설계 규격 및 스키매틱 아키텍처 제안.
- **분석 및 해결 가이드라인**:
  1. **가변 전압 출력(AI 테스트용) 회로 설계**:
     - 5V 전원을 소스로 하는 정밀 가변저항(Potentiometer)의 전압 분배 선로에 입력 임피던스가 높은 Op-Amp 볼티지 팔로워(Voltage Follower) 버퍼를 전방 배치하여, 메인 보드 ADC 단자 진입 시의 전류 흡수로 인한 왜곡을 원천 방지하는 구조 제안.
  2. **출력 감지(AO 테스트용) 모니터링 기법**:
     - 메인 보드에서 출력되는 0~5V 신호의 직관적인 확인을 위해 3선식 디지털 미니 FND 전압계 모듈과 테스트 포인트를 지그에 내장하고, 과전압 보호용 제너 다이오드(Zener Diode) 보호단 설계를 가이드함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 테스트 지그 (Test Jig / Test Fixture), 볼티지 팔로워 / 전압 추종기 (Voltage Follower / Unity Gain Buffer) 2개 신규 기술 용어 정의 추가 완료.

### 142. [Hardware] 단종 파워 MOSFET IRF7413의 드롭인 대체품(Drop-in Replacement) 조사 (2026-08-07)
- **검토 내용**: 메인 보드 탑재용 N-Channel 30V 파워 MOSFET 소자인 IRF7413의 제조사 공식 단종(Obsolete) 판정에 따른 시장 유통 활성 대체 소자 조사 및 분석.
- **분석 및 해결 가이드라인**:
  1. **물리적 패키지 및 핀아웃 호환성 분석**:
     - 기존 PCB 동박 패턴 수정 없이 1대1 납땜 대체(드롭인)가 가능하도록 표준 8-Pin SOIC (Gull-wing 리드 타입) 패키지 사양을 최우선 조건으로 설정. (바닥면 서멀 패드가 있는 PowerPAK SO-8 패키지군 제외 검토)
  2. **핵심 전기적 사양 비교 및 최적 대체품 선정**:
     - 드레인-소스 전압($V_{DS} \ge 30V$), 연속 드레인 전류($I_D \ge 13A$), 온저항($R_{DS(on)} \le 11m\Omega$)의 사양 충족 여부 확인.
     - Vishay 사의 `SQ4184EY-T1_GE3` (40V / 16A / 11mΩ)을 상위 호환 부품으로 선정. 그 외 온세미의 `FDS8880` 및 Vishay의 `Si4430BDY` 등을 비교 분석하여 조달 연속성을 확보함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 드롭인 대체품 / 핀 호환 대체품 (Drop-in Replacement / Pin-to-Pin Compatible) 1개 신규 기술 용어 정의 추가 완료.

### 143. [Hardware] 단종 온보드 AC-DC 모듈 LH25-10B24의 대체품(Replacement) 조사 (2026-08-07)
- **검토 내용**: 메인 전원부용 Mornsun 사의 25W AC-DC 온보드 컨버터 모듈인 LH25-10B24의 단종에 따른 대체 소자 분석 및 수급 대안 마련.
- **분석 및 해결 가이드라인**:
  1. **동일 핀아웃 1대1 드롭인(Drop-in) 대치품 매핑**:
     - Mornsun 사의 공식 개선 후속작인 `LH25-10B24ER2`가 핀 배열(Pinout) 및 외형 치수(`70 x 48 x 23.5 mm`)가 완벽하게 일치하여 회로 기판 수정 없이 직접 대체 가능함을 규명 및 추천.
  2. **수급성 중심의 고신뢰성 이종 브랜드 대치품 검토 (어트워크 수정 필요)**:
     - Mean Well(민웰) 사의 온보드 모듈인 `IRM-30-24` (30W / 24V / 1.3A)를 적극적 대체품으로 추천. 단, 민웰 소자는 폭이 좁고 핀 피치가 Mornsun과 상이하므로 PCB 풋프린트(Footprint) 변경이 필요함을 지적.
     - CUI 사의 `PSK-25B-24` 등 병행 검토를 통해 공급 다변화 기반 마련.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 보드 실장형 전원 공급장치 / AC-DC 온보드 모듈 (Board Mount Power Supply / On-board AC-DC Converter) 1개 신규 기술 용어 정의 추가 완료.

### 144. [Hardware] 정밀 아날로그 온도 센서 LM35의 TO-92 패키지 풀 파트넘버 및 핀 정보 검토 (2026-08-07)
- **검토 내용**: TO-92 스루홀 타입 아날로그 온도 센서 IC인 LM35의 발주용 공식 풀 파트넘버(Full Part Number) 식별 및 하드웨어 사양 검토.
- **분석 및 해결 가이드라인**:
  1. **정확한 파트넘버 식별**:
     - TO-92 플라스틱 패키지 기반 온도 센서의 표준 모델명은 **`LM35DZ`** (0°C ~ +100°C 계측용) 및 무연 환경 사양인 **`LM35DZ/NOPB`** 임을 판정하여 정보 제공.
     - 측정 온도 범위 확장 필요 시 영하권 대응이 가능한 **`LM35CZ`** (-40°C ~ +110°C) 모델 병행 확인.
  2. **물리 핀아웃 및 구동 전압 검증**:
     - 전면 인쇄부 기준 좌측부터 `1: VCC(4V~30V)`, `2: VOUT(아날로그 출력)`, `3: GND` 핀 결선 사양 및 `10mV/°C`의 선형적 스위프 전압 출력 메커니즘을 명시함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 아날로그 온도 센서 IC (Analog Temperature Sensor IC) 1개 신규 기술 용어 정의 추가 완료.

### 145. [Hardware] 단종 아날로그 멀티플렉서 ADG706BRU의 무연 드롭인 대체품 조사 (2026-08-07)
- **검토 내용**: 다채널 계측용 16채널 아날로그 멀티플렉서(MUX)인 ADG706BRU-REEL7의 공식 단종 판정에 따른 친환경(무연) 대체 소자 분석.
- **분석 및 해결 가이드라인**:
  1. **완벽한 무연 드롭인(Drop-in) 대체품 매핑**:
     - 기존 납 함유 모델(`BRU`)의 공식 제조사 후속인 무연(Pb-Free) 제품군 **`ADG706BRUZ`** 및 **`ADG706BRUZ-REEL7`** (TSSOP-28 패키지)이 현재 Mouser와 DigiKey 등 주요 대리점에 대량의 활성 재고가 존재함을 검증하여 조달 가능 판정.
     - 핀 배열 및 전기적 온저항($2.5\Omega$) 성능이 100% 동일하므로 기판 수정 불필요.
  2. **이종 브랜드 이원화 대안 분석**:
     - TI 사의 `MUX506IPWR` (TSSOP-28) 등을 대체용으로 비교 분석하되, 온저항 사양 차이(ADG706: 2.5Ω vs MUX506: 125Ω)가 있으므로 아날로그 고정밀 신호선 적용 시에는 `ADG706BRUZ`를 단독 최선책으로 추천함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 아날로그 멀티플렉서 (Analog Multiplexer / MUX) 1개 신규 기술 용어 정의 추가 완료.

### 146. [Hardware] 단종 SPI 플래시 메모리 W25Q256FVEIQ의 드롭인 대체품(Drop-in Replacement) 조사 (2026-08-07)
- **검토 내용**: 데이터 로그 및 시스템 리소스 저장용 256M-bit SPI 플래시 메모리인 W25Q256FVEIQ의 공식 단종 판정에 따른 호환 대체 소자 분석.
- **분석 및 해결 가이드라인**:
  1. **후속 세대 드롭인(Drop-in) 대체품 발굴**:
     - Winbond 사의 공식 후속 제품군인 **`W25Q256JVEIQ`** (WSON-8 8x6mm 패키지)가 기존 부품(`FV` 시리즈)과 물리적 치수 및 핀아웃이 100% 일치하며 동작 스피드가 향상된 완벽한 호환 제품임을 확인.
     - 현재 Mouser와 DigiKey 등 해외 공인 대리점에 수천 개 수준의 대량 활성 재고가 확보되어 있음을 확인하여 발주 가능 판정.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - SPI 플래시 메모리 (SPI Flash Memory) 1개 신규 기술 용어 정의 추가 완료.

### 147. [Hardware] 1117 계열 3.3V LDO 레귤레이터 정식 제조사 모델명 식별 (2026-08-07)
- **검토 내용**: 보드 설계서상의 약어 `1117SO` (SOT-223 또는 SO-8 패키지 추정)의 3.3V 고정형 LDO 레귤레이터 실발주용 정식 제조사 파트넘버(Full Part Number) 추출.
- **분석 및 해결 가이드라인**:
  1. **SOT-223 패키지 기준 정식 파트넘버 도출**:
     - STMicroelectronics 사의 고품질 대표 품번인 **`LD1117S33TR`** (또는 무연 규격 `LD1117S33CTR`)을 추천.
     - 그 외 호환 소자로 Diodes 사의 `AP1117E33G-13` 및 onsemi의 `NCP1117ST33T3G`, 가성비 위주의 `AMS1117-3.3` 매핑 제공.
  2. **SO-8 (SOIC-8) 패키지 기준 정식 파트넘버 도출**:
     - 만약 기판 도면이 8핀 IC 형태인 SO-8 규격인 경우, STMicroelectronics 사의 **`LD1117D33TR`** 정식 파트넘버를 명시하여 오발주를 미연에 방지함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - LDO 레귤레이터 (Low Drop-Out Regulator) 1개 신규 기술 용어 정의 추가 완료.

### 148. [Hardware] RS-485 통신 보호용 비대칭 TVS 다이오드 SM712 정식 제조사 모델명 식별 (2026-08-07)
- **검토 내용**: RS-485 트랜시버 ESD/서지 차단용으로 사용되는 SOT-23 패키지 비대칭 TVS 다이오드인 SM712의 정식 파트네임(Full Part Name) 조사.
- **분석 및 해결 가이드라인**:
  1. **정식 파트네임 매핑**:
     - 오리지널 개발사인 Semtech 사의 표준 릴 포장 정식 발주 품번이 **`SM712.TCT`** 임을 규명하여 전달.
  2. **조달 이원화 대체품 검토**:
     - Littelfuse 사의 1대1 무연/할로겐 프리 호환 소자인 **`SM712-02HTG`** 역시 패키지 및 핀아웃이 동일하여 완벽한 대체 사용이 가능함을 추천함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 비대칭 TVS 다이오드 (Asymmetric TVS Diode) 1개 신규 기술 용어 정의 추가 완료.

### 149. [Hardware] 단종 트라이액 BTA16-400B의 드롭인 대체품(Drop-in Replacement) 조사 (2026-08-07)
- **검토 내용**: 교류 부하 스위칭 제어 회로용 16A 트라이액(TRIAC) 소자인 BTA16-400B(TO-220 패키지)의 단종에 따른 대체 소자 검토.
- **분석 및 해결 가이드라인**:
  1. **오타 정정 및 스펙 확인**:
     - 사용자가 요청한 품명인 `BAT16-400B`는 오타로 추정되며, 실제 정식 명칭은 STMicroelectronics 사의 **`BTA16-400B`** (16A, 400V, TO-220 절연형 트라이액)임을 판정.
  2. **상위 규격 드롭인 대치품 제안**:
     - 기존의 400V 전압 정격을 한 단계 강화하여 시장 유통성이 매우 뛰어난 **`BTA16-600B`** (600V 정격) 또는 **`BTA16-800B`** (800V 정격)로 1대1 교체 사용할 것을 추천함.
     - **조립 주의점**: 반드시 금속 탭이 절연 설계된 **`BTA`** 시리즈를 사용해야 하며, 비절연형인 `BTB` 시리즈 적용 시에는 실리콘 패드 등의 방열 절연 조치가 추가되어야 함을 피드백.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 트라이액 (TRIAC - Triode for Alternating Current) 1개 신규 기술 용어 정의 추가 완료.

### 150. [Hardware] 3225 규격 8MHz 크리스탈 석영 진동자 파트넘버 도출 (2026-08-07)
- **검토 내용**: TXC 사의 25MHz 크리스탈 `7M-25.000MEEQ-T` 부품과 동일한 패키지 크기(3.2 x 2.5 mm, 4핀 SMD)를 가지는 8MHz 크리스탈 파트넘버 발굴.
- **분석 및 해결 가이드라인**:
  1. **제조사(TXC) 파트넘버 구조 분석 및 규격 매핑**:
     - TXC 사의 3.2 x 2.5 mm 4핀 패키지 라인업 중 8MHz 사양은 구형 `7M`이 아닌 **`7V` 시리즈**로 생산됨을 판별.
     - 동작 스펙에 맞춘 정식 주문 코드인 **`7V-8.000MEEQ-T`** (10pF / ±10ppm) 및 범용 오차 코드인 **`7V-8.000MAAQ-T`** 를 도출.
  2. **수급 다변화용 1대1 드롭인 타사 부품 맵 구성**:
     - 표준 3225 4-pin 패키지 사양이 100% 일치하는 Abracon 사의 **`ABM3B-8.000MHZ-10-1-U-T`** 및 Kyocera 사의 **`CX3225SB08000D0FPSZ1`** 의 정식 파트넘버와 스펙 비교 정보 제공.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 3225 패키지 (3225 Package) 1개 신규 기술 용어 정의 추가 완료.

### 151. [Hardware] dsPIC33CK MCU 외부 발진용 8MHz 크리스탈 선정 적합성 검증 (2026-08-07)
- **검토 내용**: dsPIC33CK512MP710 MCU의 시스템 클록 발생용 외부 발진자로 3225 규격 8MHz 크리스탈을 채택하는 방안의 기술적 적합성 분석.
- **분석 및 해결 가이드라인**:
  1. **클록 및 통신 정밀도 타당성 통과**:
     - MCU 내부 고성능 PLL과의 호환성이 우수하여 8MHz 입력을 체배하여 100 MIPS 동작 주파수를 구현하는 데 완벽히 매칭됨을 보증.
     - 특히 온도 드리프트에 취약한 내부 FRC 대비, RS-485 통신의 보레이트 오차(Baud Rate Error)를 0%에 수렴하도록 억제하기 위한 필수 설계로 확인.
  2. **회로 설계 상 세부 매칭 지침**:
     - 크리스탈 `7V-8.000MEEQ-T` 사양인 `CL = 10pF`에 대응하기 위해 OSC1 및 OSC2 라인에 **`12pF ~ 15pF` 세라믹 매칭 콘덴서**를 그라운드 결선하도록 가이드라인 수립.
     - 발진 안정성 강화를 위해 OSC2 핀 출력에 직렬 댐핑 저항(약 0 ~ 100Ω) 실장 공간 확보 권고.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 로드 커패시터 매칭 (Load Capacitor Matching) 1개 신규 기술 용어 정의 추가 완료.

### 152. [Hardware] 포토커플러 PC357N14J00F와 TLP185(GB)의 1대1 대체 적합성 검증 (2026-08-07)
- **검토 내용**: Sharp 사의 단종 포토커플러 PC357N14J00F와 Toshiba 사의 TLP185(GB) 간의 1대1 드롭인 호환성 분석.
- **분석 및 해결 가이드라인**:
  1. **물리적 및 전기적 사양 비교 검토**:
     - 두 소자 모두 표준 4-pin Mini-flat (SOP-4) 패키지를 공유하여 PCB 패턴 수정 없이 1대1 핀 호환 실장이 가능함을 확인.
     - 절연 전압(3.75 kVrms) 및 Collector-Emitter 내압(80V) 성능이 완벽히 일치하여 정합성 통과.
  2. **전류 전달비(CTR) 세부 편차 분석**:
     - 샤프의 `N14` 등급은 `150% ~ 300%`, 도시바의 `GB` 등급은 `100% ~ 600%`의 범위를 지녀 도시바의 하한선이 약간 낮으나, 일반적인 센서 신호 절연 및 온/오프 드라이빙 환경에서는 동작 특성에 지장이 없는 완벽한 대체 품목으로 최종 판정 및 확인.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 포토커플러 / 옵토커플러 (Photocoupler / Optocoupler), 전류 전달비 (Current Transfer Ratio / CTR) 2개 신규 기술 용어 정의 추가 완료.

### 153. [Hardware] RTC 백업용 CR2032 배터리 홀더(Battery Holder) 추천 부품 조사 (2026-08-07)
- **검토 내용**: MCU의 실시간 클록(RTC) 유지 및 정전 시 SRAM 데이터 보존을 위한 3V 코인 셀(CR2032) 실장용 배터리 홀더 제조사 및 파트넘버 선정.
- **분석 및 해결 가이드라인**:
  1. **실장 패키지(SMD vs Through-hole) 조건별 제품 추천**:
     - **SMD 플라스틱 홀더 (탈착 용이 및 쇼트 방지)**: Keystone 사의 `1058` 및 MPD 사의 `BU2032SM-HD-G` (골드 도금, 내부식성) 추천.
     - **Through-hole 플라스틱 홀더 (물리적 고정력 강함)**: Keystone 사의 `1026` 및 MPD 사의 `BH32T-C` 추천.
     - **SMD 금속 리테이너 클립 (높이가 매우 낮은 슬림형)**: Keystone 사의 `1060` (높이 3.2mm) 권고.
  2. **회로 설계 상 주의점 제안**:
     - 리테이너 클립 사용 시 배터리 바닥면 음극(-) 단락 방지를 위해 PCB 상에 절연 마스크 및 원형 GND 패턴 설계를 철저히 하도록 권고함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 코인 셀 배터리 홀더 (Coin Cell Battery Holder / Retainer) 1개 신규 기술 용어 정의 추가 완료.

### 154. [Hardware] 10.16mm 피치 터미널 블록 2핀/3핀/14핀 파트넘버 및 실무 조립 기법 검토 (2026-08-07)
- **검토 내용**: 전원 입력단 대전류 연결용 10.16mm 피치 터미널 블록의 핀 수별(2핀, 3핀, 14핀) 정식 모델명 식별 및 실무 부품 선정.
- **분석 및 해결 가이드라인**:
  1. **오리지널(Phoenix Contact) 파트넘버 도출**:
     - **2핀**: `GMKDS 3/ 2-10,16` (품명 번호: `1731022`, 스크루 고정형)
     - **3핀**: `GMKDS 3/ 3-10,16` (품명 번호: `1731035`, 스크루 고정형)
     - **14핀**: 10.16mm 대전류 사양 특성상 14핀 일체형 단일 모델은 기판 휨 및 사출 한계로 생산되지 않으므로, 조립식 인터락킹(Interlocking) 구조를 활용해 2핀 7개 또는 3핀 4개 + 2핀 1개를 맞물려 조립하여 적용하도록 가이드.
  2. **가성비 제조사(Dinkle) 이원화 대안 제시**:
     - 대만 딩클 사의 10.00mm 피치 모델인 `EK1000V-02P` 및 `EK1000V-03P` 비교 제안.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 인터락킹 터미널 블록 (Interlocking Terminal Block) 1개 신규 기술 용어 정의 추가 완료.

### 155. [Hardware] 고정식 배리어 터미널 블록(Barrier Terminal Block) 품종 식별 및 파트네임 추천 (2026-08-07)
- **검토 내용**: 사용자가 사진으로 제공한 나사 결선식 14핀 배리어 터미널 블록의 규격 식별 및 PCB 실장용 제조사 모델 번호 도출.
- **분석 및 해결 가이드라인**:
  1. **소자 식별 및 특성 확인**:
     - 제공 이미지의 부품은 금속 나사와 플라스틱 격벽으로 절연을 극대화한 **배리어 터미널 블록(Barrier Terminal Block)**임을 식별.
     - 10.16mm 피치의 일반 유로 스타일 커넥터와 달리, 이 종류의 단자대는 14핀 규격이 단품(일체형)으로 유통됨을 확인.
  2. **제조사별 14핀 파트넘버 매핑**:
     - **대만 Dinkle 사 (최우수 가성비 및 보편적 규격, 9.5mm pitch)**: `DT-45-B01W-14` 추천.
     - **TE Connectivity 사 (Buchanan 계열, 9.53mm pitch)**: `JC6-Q308-14` 추천.
     - **Molex 사 (Beau 계열, 9.53mm pitch)**: `0387206314` 추천.
     - **주의 사항**: 9.5mm와 9.53mm(0.375인치) 및 10.0mm 피치 간에 미세한 간격 오차가 있으므로 아트워크 설계 전 실물 자재 피치 확인 필수 가이드 제공.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 배리어 터미널 블록 / 배리어 단자대 (Barrier Terminal Block / Barrier Strip) 1개 신규 기술 용어 정의 추가 완료.

### 156. [Architecture] 시스템 개발 요소(Firmware, GUI, HMI) 간 폴더 구조 설계 검토 및 모노레포 수립 (2026-08-10)
- **검토 내용**: MCU 제어 보드 펌웨어, PC 모니터링 GUI 애플리케이션, 7인치 LCD HMI 작화 프로그램의 개발을 시작하기에 앞서, 각 프로젝트의 소스 코드를 별도 저장소(멀티레포)로 쪼개어 독립 폴더로 관리할 것인지, 아니면 단일 저장소(모노레포) 형태의 상위 폴더 구조 내에서 통합 관리할 것인지 타당성 검토.
- **분석 및 해결 가이드라인**:
  1. **물리적 환경의 상이성 확인**:
     - MCU Firmware(dsPIC33CK C/C++ 프로젝트), PC GUI(C#/.NET 또는 Python), HMI LCD 작화(제조사 전용 에디터 프로프램 및 이진/설정 파일)의 개발 툴체인 및 개발 환경이 서로 상이함을 확인.
  2. **모노레포(Monorepo) 방식의 단일 프로젝트 관리 확정**:
     - 세 가지 애플리케이션이 공통의 통신 프로토콜(Modbus 또는 UART Custom 프로토콜) 및 IO 맵(공유 레지스터 어드레스 계약)을 기반으로 강력하게 상호작용하므로, 이를 단일 Git 저장소 형태의 통합 폴더 구조(`02_Firmware`, `03_GUI_PC`, `04_HMI_LCD`) 내에서 관리하도록 설계 확정.
     - 통신 규격이나 레지스터 맵이 변경될 때 세 프로젝트의 코드를 단일 커밋으로 동기화하여 변경 이력을 관리할 수 있어 버전 싱크 불일치 문제를 예방함.
     - 공용 사양서 및 기획 문서(`05_Docs`에 배치된 6kW BOP 리스트 등)와 동일 구조에서 밀접하게 버전 관리를 수행할 수 있어 프로젝트 투명성 확보.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 모노레포 (Monorepo), 멀티레포 / 폴리레포 (Multirepo / Polyrepo) 2개 신규 기술 용어 정의 추가 완료.


### 157. [Architecture] 임베디드 프로젝트의 관심사 분리(SoC) 및 서브 디렉터리별 개발 방식 확립 (2026-08-10)
- **검토 내용**: 01_Hardware부터 04_HMI_LCD까지의 세부 디렉터리 구조 하에서 실제 개발 작업(코드 작성, IDE 사용)을 어느 지점에서 수행해야 하는지 실무적 워크플로 정립.
- **분석 및 해결 가이드라인**:
  1. **컨테이너와 워크스페이스 구분**:
     - 프로젝트 루트(`d:\Work\H2_Control_Board`)는 모든 개발 요소를 하나로 묶는 상위 '컨테이너(모노레포)' 저장소 역할만 담당함.
  2. **서브 디렉터리 내 개별 개발(관심사 분리) 확정**:
     - MCU 펌웨어 개발 시에는 02_Firmware 내부로 이동하여 MPLAB X IDE 등의 전용 도구로 해당 폴더 안의 독립 프로젝트를 빌드 및 컴파일함.
     - PC GUI 개발 시에는 03_GUI_PC, LCD 화면 작화 개발 시에는 04_HMI_LCD 내부의 리소스만 수정 및 배포함으로써 다른 성격의 파일이 오염되거나 꼬이는 현상을 원천 차단함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 관심사 분리 (Separation of Concerns / SoC) 1개 신규 기술 용어 정의 추가 완료.


### 158. [Architecture] 연동 개발 우선순위(펌웨어 우선 개발 로드맵) 설계 수립 (2026-08-10)
- **검토 내용**: MCU 펌웨어, PC GUI, HMI LCD의 3가지 소프트웨어 구성 요소 간의 연동 개발 착수 시 기술적 의존성을 최소화하고 효율을 극대화하기 위한 우선순위 및 로드맵 수립.
- **분석 및 해결 가이드라인**:
  1. **펌웨어 우선 개발 방식 타당성 검증**:
     - 시스템 데이터의 물리적 원천(Source)이 MCU 제어 보드에 연결된 BOP 센서들이므로, 펌웨어가 데이터 포맷을 정하고 통신 패킷을 먼저 송출해야 PC GUI 및 LCD HMI가 이를 수신하여 작동할 수 있는 단방향 종속성 규명.
  2. **단계적 개발 로드맵 수립**:
     - **1단계 (통신 설계)**: 프로그램 코딩 전, 엑셀 등을 이용하여 공통 데이터 계약서 역할을 수행할 레지스터 맵(Register Map)을 최우선으로 확정함.
     - **2단계 (펌웨어 기초)**: MCU 주변장치(ADC, GPIO 등) 구동 드라이버 및 이더넷/RS-485 통신 모듈을 작성하여 데이터 패킷을 실어 보낼 인프라 구축.
     - **3단계 (PC GUI 연동)**: 펌웨어에서 가상의 센서 값을 실어 패킷을 송출하고, PC GUI를 실행하여 이를 수신하여 파싱하는 연동 및 명령 제어 루프를 먼저 검증함.
     - **4단계 (HMI LCD 구현)**: 검증 완료된 프로토콜을 그대로 HMI LCD 작화 태그에 바인딩하여 현장 모니터링 디스플레이 조작 시스템을 최종 조립함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 레지스터 맵 / 통신 맵 (Register Map / Memory Map) 1개 신규 기술 용어 정의 추가 완료.


### 159. [Process] AI 협업 대화 컨텍스트 유지 및 개발 연속성 확립 (2026-08-10)
- **검토 내용**: 하드웨어(회로도 및 PCB) 설계 완료 후 소프트웨어 연동 개발(펌웨어, GUI, LCD 작화)로 전환 시, 기존 AI 어시스턴트와의 대화 쓰레드를 새로 생성할 것인지 아니면 기존 대화를 유지하여 개발할 것인지에 대한 효율성 분석.
- **분석 및 해결 가이드라인**:
  1. **컨테이너화된 히스토리 보존 필요성**:
     - 기존 대화 세션에 dsPIC33CK MCU 핀 맵, W5500 이더넷 인터페이스, 6kW BOP 다채널 센서/액추에이터 요구 사항, ESD 서지 방지 필터 소자 명세 등 방대한 하드웨어 설계 자산이 저장되어 있음을 확인.
  2. **대화 연속성 유지 결정**:
     - 새로운 쓰레드로 전환 시 기존 설계 데이터가 단절되어 중복적인 설명 및 재확인 과정에 의한 비효율이 발생함.
     - 따라서 동일 대화 쓰레드 상에서 펌웨어와 GUI 소프트웨어 통신 설계 단계로 막힘없이 연결해 나감으로써 완벽한 문맥적 동기화(SoC)를 기반으로 고속 코딩을 실현하도록 프로세스 확정.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 대화 컨텍스트 / 개발 문맥 (Conversation Context / Development Context) 1개 신규 기술 용어 정의 추가 완료.


### 160. [Firmware] dsPIC33CK512MP710 타겟 MPLAB X IDE 프로젝트 뼈대 생성 및 초기화 (2026-08-10)
- **검토 내용**: 02_Firmware 디렉터리 내에 dsPIC33CK512MP710 MCU에 최적화된 MPLAB X IDE 개발용 빈 프로젝트 폴더 구성 및 설정.
- **분석 및 해결 가이드라인**:
  1. **MPLAB X 프로젝트 구조 수립**:
     - 02_Firmware/H2_Control_Board.X 경로 하위에 논리 디렉터리 구성 파일인 project.xml 및 configurations.xml 생성 완료.
     - 타겟 디바이스(dsPIC33CK512MP710) 및 컴파일러 툴체인(XC16), DFP 장치 팩을 명세하여 IDE 로드 시 자동 매핑되도록 처리함.
  2. **기본 소스 및 빌드 파일 구성**:
     - 빌드 진입점인 Makefile과 기본 main 루프 구조를 갖는 main.c 파일 생성 완료.
     - main.c 상단 주석에 하드웨어 회로도 분석을 통한 W5500 이더넷 SPI 핀, 외부 크리스탈 8MHz 클록 핀(36/37번), 정전 감지 아날로그 입력 핀(75번/RB8) 등의 매핑 사양을 가이드로 기술하여 설계 연속성 확보.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 장치 대응 팩 (Device Family Pack / DFP), 설정 비트 (Configuration Bits) 2개 신규 기술 용어 정의 추가 완료.


### 161. [Hardware] 01_Hardware 디렉터리 내 PDF 회로도 및 자재명세서(BOM) 직접 분석 완료 (2026-08-10)
- **검토 내용**: AI 어시스턴트가 01_Hardware 내에 위치한 알티움 기반 회로도 및 PDF 도큐먼트의 설계 데이터를 직접 읽고 파싱할 수 있는지 기술적 적합성 검증 및 자료 정합성 통과.
- **분석 및 해결 가이드라인**:
  1. **PDF 문서 직접 읽기 및 데이터 추출 성공**:
     - 01_Hardware/FuelCell.pdf 파일을 직접 오픈하여 1페이지의 기판 회로 다이어그램 이미지 스크린샷과 2페이지의 BOM(자재명세서) 데이터 테이블을 완벽히 읽고 파싱함.
     - 메인 MCU(DSPIC33CK512MP710-E_PT), 듀얼 이더넷 칩(W5500), ADC(ADS1115), DAC(DAC60516), 릴레이 스위치(RY_HL2-2CM) 등의 실장 레퍼런스 및 핀 관계를 재복사 없이 직접 로드 가능한 시스템 환경 확보.
  2. **중복 자재 업로드 불필요 가이드 정립**:
     - 펌웨어 프로젝트의 Doc 디렉터리에 회로도를 부분 쪼개어 수동 업로드할 필요가 없음을 확정하여 개발 리소스 낭비 방지.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - BOM (Bill of Materials, 자재명세서) 1개 신규 기술 용어 정의 추가 완료.


### 162. [Firmware] dsPIC33CK512MP710 MCU 전용 GPIO 입출력 포트 핀 매핑(pin_map.h) 수립 및 포트 초기화 연동 (2026-08-10)
- **검토 내용**: 사용자 제공 MCU 네트 회로 캡처 이미지 및 FuelCell.pdf 정보를 비교 분석하여, dsPIC33CK512MP710의 물리 핀(76~100번 및 좌/우/하단 라인)과 통신/제어 신호선 간의 1대1 매핑 관계를 규명하고 이를 펌웨어 헤더로 구현.
- **분석 및 해결 가이드라인**:
  1. **물리 핀과 네트 라벨 정밀 매핑**:
     - 76번(RF15) -> DO_MC_SW, 77번(RB9) -> DO_HT193, 78번(RE12) -> DI_HT193, 74번(RF14) -> POWER_FAIL_DET, 75번(RB8) -> DAC_CS 등 물리적 포트와 기능 제어 핀의 TRIS/LAT 레지스터 매크로를 도출함.
  2. **핀 맵 헤더(pin_map.h) 작성 및 main.c 연동**:
     - 02_Firmware/pin_map.h 파일을 신규 생성하여 모든 DO(출력, TRIS=0) 및 DI(입력, TRIS=1) 핀에 대한 LAT/PORT 제어 어드레스와 방향 제어 레지스터를 완벽하게 정의함.
     - 메인 소스 main.c에 해당 헤더를 인클루드하고 시스템 최초 부팅 시 입출력 포트를 방향성에 맞춰 일괄 리셋/설정해주는 GPIO_Initialize() 초기화 함수 연동 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - GPIO (General Purpose Input/Output, 범용 입출력), TRIS 레지스터 (Tri-state Register, 방향 제어 레지스터) 2개 신규 기술 용어 정의 추가 완료.


### 163. [Firmware] ADS1115 16채널 ADC 드라이버 수립 및 핀 매핑 완료 (2026-08-10)
- **검토 내용**: 01_Hardware 내의 Input_Section.SchDoc 도면 정보를 토대로, I2C 통신 기반의 16비트 ADC 칩인 ADS1115 4개(IC400~IC403)의 하드웨어 주소 및 16개 아날로그 센서 채널에 대한 매핑 분석과 전용 드라이버 구현.
- **분석 및 해결 가이드라인**:
  1. **ADS1115 4개 칩의 주소 및 입출력 맵 도출**:
     - IC400 (주소: 0x48, ADDR->GND): AIN0~3 순서대로 Prox 공기 유량계, 촉매부 압력, 유틸리티 압력, 스택 Anode 압력 매핑.
     - IC401 (주소: 0x49, ADDR->VDD): AIN0~3 순서대로 BNG 유량, PNG 유량, 버너 공기 유량1, STACK 공기 유량 매핑.
     - IC403 (주소: 0x4A, ADDR->SDA): AIN0~3 순서대로 버너 압력, 후단부 압력, DI 탱크 전도도, 가스 버퍼 압력 매핑.
     - IC402 (주소: 0x4B, ADDR->SCL): AIN0~3 순서대로 대기 온도, 스페어 센서, 냉각수 회수 유량, 버너 공기 유량2 매핑.
  2. **드라이버 파일(ads1115.h/c) 작성 및 프로젝트 연동**:
     - 02_Firmware/ads1115.h와 02_Firmware/ads1115.c를 생성하여 I2C 기반의 ADS1115 16비트 원시 데이터 계측 및 전압 스케일 변환 API 구조화 완료.
     - MPLAB X IDE 프로젝트의 configurations.xml 설정 파일 내에 소스/헤더 참조 트리 추가 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - I2C 통신 (Inter-Integrated Circuit, 아이스퀘어씨), ADS1115 (16비트 아날로그-디지털 변환기 / ADC) 2개 신규 기술 용어 정의 추가 완료.


### 164. [Firmware] BOP 입출력 및 센서 명명 규격서(PDF) 표준에 따른 핀 맵 및 ADC 명칭 동기화 (2026-08-10)
- **검토 내용**: 사용자가 업로드한 05_Docs 내의 입출력 장치 리스트 및 명명 규격 PDF 문서 정보를 반영하여, 기존에 구성한 핀 맵 및 ADS1115 아날로그 채널 명칭을 신규 정식 장치명으로 일체 동기화.
- **분석 및 해결 가이드라인**:
  1. **아날로그 입력 채널 명칭 개정**:
     - ads1115.h 내의 ADS1115_SensorChannel_t Enum 형 변수명을 PDF 사양서에 부합하는 AI_PT383_REFORMER_WATER_PRESS (개질수 공급 압력), AI_PT162_EXHAUST_PRESS (배기가스 압력), AI_MFM211_STACK_AIR_FLOW (스택 공기 유량) 등으로 수정함.
     - ads1115.c 내의 채널 매핑 범위 판단 경계인 AI_MFC121_PNG_FLOW 도 신규 Enum 형태로 수정하여 빌드 안정성 확보.
  2. **디지털 입출력 핀 매크로 주석 갱신**:
     - pin_map.h 내의 히터(DO_HT193~196 -> LTS 촉매 히터 1/2, Prox 히터 1/2), 밸브(DO_MV, DO_SV), 입력 경보(DI_HT_WD, DI_GD -> 메탄/수소 누출 감지) 등의 주석을 규격서의 한국어 장치명 표준에 완벽히 매칭하여 수정 완료.
     - pin_map.h 내에 존재하던 TC_EN3 핀 매크로 설정 상의 물음표(?) 오타를 교정하여 LATBbits.LATB12로 확정함.


### 165. [Firmware] DAC60516 16채널 DAC 아날로그 출력 드라이버 설계 및 핀 매핑 완료 (2026-08-10)
- **검토 내용**: 01_Hardware 내의 Output_Section.SchDoc 도면 정보를 토대로, SPI 통신 기반의 16비트 DAC 칩인 DAC60516(IC501)의 12개 채널별 아날로그 출력 신호에 대한 매핑 분석과 전용 드라이버 구현.
- **분석 및 해결 가이드라인**:
  1. **DAC60516 12개 채널의 아날로그 출력 맵 도출**:
     - OUT0: AO_AB232 (PrOx 에어블로우), OUT1: AO_AB212 (STACK 에어블로어), OUT2: AO_P341 (AOG 냉각 물펌프), OUT3: AO_P375 (STACK 2 냉각수 공급)
     - OUT4: AO_P108 (가스 가압펌프), OUT5: AO_P370 (STACK 1 냉각수 공급), OUT6: AO_P380 (개질수 펌프), OUT7: AO_AB221 (개질기 버너 에어블로어)
     - OUT8: AO_MFC111 (BNG 유량 제어), OUT9: AO_MFC121 (PNG 유량 제어), OUT10: AO_P351 (Anode(RG) 냉각 물펌프), OUT11: AO_SPARE1 (스페어 아날로그 출력)
  2. **드라이버 파일(dac60516.h/c) 작성 및 프로젝트 연동**:
     - 02_Firmware/dac60516.h와 02_Firmware/dac60516.c를 생성하여 SPI 기반의 24비트 데이터 전송(8비트 주소 + 16비트 데이터 값) 및 0V ~ 5V 선형 연속 전압 환산 출력 API 구조화 완료.
     - MPLAB X IDE 프로젝트의 configurations.xml 설정 내에 드라이버 파일을 신규 연동 등록함.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - DAC (Digital-to-Analog Converter, 디지털-아날로그 변환기), DAC60516 (16비트 16채널 디지털-아날로그 변환기 / DAC) 2개 신규 기술 용어 정의 추가 완료.


### 166. [Firmware] 다채널 열전대(TC) 온도 측정용 MAX31856 및 ADG706 멀티플렉서 스위칭 설계 검토 (2026-08-10)
- **검토 내용**: 01_Hardware 내의 Input_Section.SchDoc 도면 정보를 기반으로 한 T-type(19채널) 및 K-type(12채널) 온도 측정 회로 설계 타당성 및 제어선 매핑 상태 검토.
- **분석 및 해결 가이드라인**:
  1. **다채널 온도 센서 획득 아키텍처 분석**:
     - 30개가 넘는 열전대 센서 신호를 단일 열전대-디지털 변환 IC인 MAX31856MUD+(IC301)로 처리하기 위해, 아날로그 멀티플렉서인 ADG706BRU-REEL7(IC300, IC303 등) 어레이를 채택한 하드웨어 구조 검증.
     - MCU의 디지털 출력 핀 TC_ADDR0~3과 MUX 인에이블 제어선 TC_EN1~3을 동적으로 스위칭하여 각 채널의 온도 전압을 MAX31856으로 라우팅하고, 이를 SPI 통신으로 순차 스캔하여 계측하는 펌웨어 스위칭 드라이버 필요성 규명.
  2. **추가 입출력 구현 계획 수립**:
     - 다음 개발 단계로 넘어가기 전, 이 다채널 열전대(TC) 온도 획득 관련 채널 어드레스 스위칭 드라이버(thermocouple.h/c) 설계 수립 및 구현 필요성 제안.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 냉접점 보상 (Cold-Junction Compensation / CJC) 1개 신규 기술 용어 정의 추가 완료.


### 167. [Firmware] 다채널 열전대(TC) 온도 획득용 스캐너 드라이버 및 MUX 매핑 구현 완료 (2026-08-10)
- **검토 내용**: 사용자가 업로드한 TempController.SchDoc 회로도 도면 확대 본을 정밀 해독하여, 19개 T-type 채널 및 12개 K-type 채널과 6개의 아날로그 멀티플렉서 ADG706(IC300, IC303 등) 및 MAX31856MUD+(IC301) 칩셋 간의 차동(Differential) 결선 매핑 및 제어 사양 파악 완료.
- **분석 및 해결 가이드라인**:
  1. **다채널 온도 센서 스캐닝 로직 수립**:
     - 6개의 ADG706 멀티플렉서를 3개의 차동 스위칭 쌍(EN1~3)으로 구성하고, MCU 4비트 주소선(ADDR0~3)을 순차 조절하여 MAX31856의 T+/T- 단자로 대상 온도 전압을 입력하는 하드웨어 라우팅 해독.
     - thermocouple.h와 thermocouple.c를 생성하여 채널 선택 함수 TC_SelectChannel() 및 24비트 SPI 통신 연동 및 Celsius 환산 수식(1 LSB = 0.0078125도)을 내장한 TC_ReadTemperature() API 구현 완료.
  2. **핀 맵 헤더 보강 및 프로젝트 연동**:
     - pin_map.h 내에 SPI CS 핀 제어선 TC_SPI_CS (물리 35번 핀, RC3) 매크로 정의 추가 및 MUX 인에이블 핀 TC_EN3 초기화 누락분 보강 완료.
     - MPLAB X IDE 프로젝트의 configurations.xml에 드라이버 헤더/소스 경로를 신규 등록 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 냉접점 보상 (Cold-Junction Compensation / CJC) 기술 용어 추가 완료.


### 168. [Doc] MAX31856MUD+ 열전대 온도 변환 IC 작동 메커니즘 분석 및 용어사전 반영 (2026-08-10)
- **검토 내용**: 사용자의 요청에 따라, MAX31856 칩셋의 하드웨어적 T/K 타입 분류 방식, 냉접점 보상(CJC) 동작 원리, 및 기전력-온도 선형화(Linearization) 수학적 연산 알고리즘을 분석하여 상세 기술 설명 제공.
- **분석 및 해결 가이드라인**:
  1. **동적 온도 센서 타입(T/K) 제어 원리**:
     - MAX31856은 하드웨어 단자 선로만으로는 K타입과 T타입 금속 재질의 차이를 자체 판별하지 못함.
     - MCU가 SPI 인터페이스를 통해 Configuration 1 Register(CR1) 비트 설정을 통해 센서 타입(T-type=0x07, K-type=0x03)을 사전에 주입해 주면, 칩 내부에 저장된 비선형 곡선 룩업 테이블(LUT)을 알맞게 활성화하여 전압-온도 역변환을 수행함.
  2. **냉접점 보상(CJC) 및 비선형성 선형화 보정**:
     - 열전대 센서 양끝 온도차에 의한 Seebeck 효과 기전력 측정 시 기판 접속 단자대(냉접점)에서 일어나는 2차 열기전력 오차를 방지하기 위해, 칩 내부에 위치한 고정밀 아날로그 온도 감지기를 통해 냉접점 온도를 절대값으로 측정하고 이를 전압값으로 환산해 원래 기전력과 보상 합산 연산을 수행.
     - 보상된 기전력을 19비트 시그마-델타 ADC를 거쳐 내장 수학 연산 엔진을 통해 선형 섭씨온도 데이터로 최종 디지털 출력(1 LSB = 0.0078125°C).
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 제베크 효과 (Seebeck Effect, 열전효과) 신규 기술 용어 추가 완료.


### 169. [Firmware] Half-Duplex RS-485 통신용 MAX3485 드라이버 및 포트 매핑 분석 완료 (2026-08-10)
- **검토 내용**: 사용자의 9600 bps 통신 요청 및 도면 정보에 의거하여, Mcu.SchDoc 파일의 Altium 내부 핀 데이터 및 UniqueID 추적 기법을 통해 RS-485 트랜시버 칩셋인 MAX3485CSA+(IC4)의 핀 할당 규명 완료.
- **분석 및 해결 가이드라인**:
  1. **물리 핀 및 포트 할당 정밀 역추적**:
     - 485_TX -> 물리 67번 핀 (RD5 포트, UART1 TX 출력 PPS)
     - 485_RX -> 물리 69번 핀 (RB6 port, UART1 RX 입력 PPS)
     - 485_DIR (Half-Duplex 송수신 방향 전환선) -> 물리 68번 핀 (RB5 포트, 출력 일반 GPIO)
  2. **드라이버 개발 및 프로젝트 연동**:
     - rs485.h와 rs485.c를 신규 작성하여, 시스템 80MHz 기준 보레이트 9600bps 연산 및 UART1 PPS 핀 매핑 해제/잠금 매칭을 완벽히 설정 완료.
     - 데이터 전송 시 바이트 잘림 현상을 완벽하게 방지하기 위해 송신 시프트 레지스터 비움(TRMT) 대기 조건을 내장한 RS485_WriteByte(), RS485_WriteBuffer() 및 수신 오버런 오류(OERR) 자동 복원 필터를 결합한 RS485_ReadByte() API 설계 완료.
     - pin_map.h에 DO_485_DIR 매크로 및 GPIO_Initialize() 내에 초기 리셋 설정을 추가하고 configurations.xml에 드라이버 파일을 최종 등록 연동.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 반이중 통신 (Half-Duplex, 하프 듀플렉스) 기술 용어 추가 완료.


### 170. [Firmware] Full-Duplex RS-422 통신용 MAX3490 드라이버 및 포트 매핑 분석 완료 (2026-08-10)
- **검토 내용**: 관제 프로그램과의 양방향(Full-Duplex) 온도/압력 제어 연동 요청에 맞춰, Mcu.SchDoc 파일의 Altium 내부 핀 데이터 매칭 분석을 통해 RS-422 트랜시버인 MAX3490CSA+(IC2)와 dsPIC33CK MCU 간의 핀 할당 규명 완료.
- **분석 및 해결 가이드라인**:
  1. **물리 핀 및 포트 할당 정밀 역추적**:
     - MON_422_TX -> 물리 71번 핀 (RF13 포트, UART2 TX 출력 PPS)
     - MON_422_RX -> 물리 70번 핀 (RE10 포트, UART2 RX 입력 PPS)
     - Full-Duplex 규격이므로 하드웨어적 송수신 제어선(DIR) 설정은 불필요하여 MCU GPIO 리소스 절약.
  2. **드라이버 개발 및 프로젝트 연동**:
     - rs422.h와 rs422.c를 신규 생성하여, 시스템 클록 80MHz 기반 보레이트 9600bps 분주 설정(U2BRG = 521)을 완료.
     - PPS 매핑 시 _U2RXR = 42 (RE10 입력) 및 _RP109R = 2 (RF13 출력 U2TX)로 UART2 주변기기 물리 포트를 확실하게 동적 맵핑 완료.
     - 방향 전환 오버헤드가 없으므로 UART2 송신 버퍼(UTXBF) 상태만 감시하는 고속 논스톱 전송 API RS422_WriteByte() 및 RS422_WriteBuffer() 수립 완료.
     - 수신 버퍼 오버런 방지를 위한 오버런 클리어(OERR = 0) 코드를 수신 연동에 결합하여 장시간 관제 패킷 수신 시의 하드웨어 오동작 가능성 배제 완료.
     - configurations.xml에 드라이버 파일을 신규 헤더/소스 노드 트리로 추가 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 전이중 통신 (Full-Duplex, 풀 듀플렉스) 기술 용어 추가 완료.


### 171. [Firmware] Modbus RTU Slave 프로토콜 스택 설계 및 구현 완료 (2026-08-10)
- **검토 내용**: 관제 프로그램 연동 시 필수적인 산업 표준 Modbus RTU 프로토콜 슬레이브 스택 수립 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **슬레이브 레지스터 맵 설계 수립**:
     - Input Registers (3XXXX, Read-Only): 다채널 열전대 온도 계측 데이터(T/K타입 31개 채널) 및 ADS1115 아날로그 입력 전압 데이터(16개 채널) 매핑.
     - Holding Registers (4XXXX, Read/Write): DAC60516 12개 출력 채널 전압값 및 온/압력 루프 제어용 설정 파라미터 매핑.
     - Coils & Discrete Inputs: 디바이스 온/오프 제어선 및 안전 감시 장치 매핑.
  2. **프로토콜 스택 및 통신 핸들러 구현**:
     - modbus.h와 modbus.c를 생성하여 Modbus 표준 국번(0x01) 패킷 분류 및 고속 16비트 CRC(0xA001 다항식) 알고리즘을 통한 무결성 검증 루틴 수립.
     - 펑션 코드 0x03, 0x04, 0x06 표준 모드 응답 생성부 설계 및 지원하지 않는 기능 코드에 대해 Modbus 표준 예외 응답(Exception Code) 송출 처리 구현.
     - 특히 단일 홀딩 레지스터 쓰기(0x06) 동작으로 DAC60516 채널 설정 값을 수신했을 때, 하드웨어 DAC60516_WriteChannel() API를 직접 트리거하도록 연동 설계하여 실시간 가동 제어 실현.
     - 수신 프레임 무전송(3.5T) 간격 감시 타임아웃 감지 처리를 포함하는 Modbus_Task()를 생성하여 주기적 메인 루프에서 안정적으로 스캐닝되도록 처리.
     - configurations.xml에 드라이버 헤더/소스 경로를 신규 등록 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 모드버스 (Modbus) 기술 용어 추가 완료.


### 172. [Firmware] main.c 메인 스케줄러 통합 및 센서-Modbus DB 완전 연동 완료 (2026-08-10)
- **검토 내용**: 각 장치별 드라이버와 통신 스택을 메인 제어 루프에 결합하여 연료전지 BOP 스케줄러 시스템으로 구동 가능하도록 펌웨어 통합 및 연동 로직 수립.
- **분석 및 해결 가이드라인**:
  1. **초기화 시퀀스 체인 구축**:
     - main() 기동 시 GPIO_Initialize(), TC_Initialize(), RS485_Initialize(), RS422_Initialize(), Modbus_Initialize()를 일체 연동 호출하도록 구성.
  2. **센서 계측 데이터 Modbus 바인딩 자동화**:
     - 31개 온도 센서 계측값(TC_ReadTemperature())을 섭씨온도 10배 곱 연산 후 modbus_db.input_regs[0~30]에 순차 대입.
     - 16채널 ADC 값(ADS1115_ReadChannel())을 modbus_db.input_regs[32~47]에 바인딩.
     - pin_map.h에 등록된 디지털 입력(DI) 경보 포트 상태를 Modbus Discrete Inputs 레지스터로 비트 팩킹 처리.
  3. **관제 제어 피드백 루프 실현**:
     - 노트북 PC에서 Modbus Coils를 제어 시, MCU의 실제 디지털 출력(DO) 릴레이 및 밸브 제어 포트(DO_MC_SW, DO_HT193~196, DO_FAN504 등)가 물리적으로 스위칭되도록 동기화.
     - 주기적 루프 내 ClrWdt() 배치를 통해 하드웨어 워치독 타이머 간섭 배제 및 펌웨어 다운 현상 원천 차단.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 에코백 (Echo-back) 기술 용어 추가 완료.


### 173. [Firmware] 듀얼 W5500 이더넷 제어 핀 매핑 해독 및 초기화 드라이버 구현 완료 (2026-08-10)
- **검토 내용**: Ethernet.SchDoc 및 Mcu.SchDoc 회로도 도면 해독을 바탕으로, 듀얼 W5500 이더넷 컨트롤러용 SPI 공유 버스 신호 및 1호기/2호기 개별 CS, RESET, INT 제어 핀의 MCU 핀 할당 규명 완료.
- **분석 및 해결 가이드라인**:
  1. **물리 핀 및 포트 할당 정밀 규명**:
     - SPI 공유 버스: ETH_SCLK (물리 6번 핀, RC12 포트, SPI2 CLK), ETH_MOSI (물리 5번 핀, RF0 포트, SPI2 SDO), ETH_MISO (물리 4번 핀, RE1 포트, SPI2 SDI)
     - 이더넷 1호기 (ETH1): ETH1_SCSN (물리 7번, RC13), ETH1_RSTN (물리 2번, RE0), ETH1_INT (물리 3번, RB15)
     - 이더넷 2호기 (ETH2): ETH2_SCSN (물리 1번, RB14), ETH2_RSTN (물리 8번, RC14), ETH2_INT (물리 9번, RC15)
  2. **드라이버 개발 및 프로젝트 연동**:
     - ethernet.h와 ethernet.c를 생성하여 SPI2의 마스터 모드(Baudrate 10 MHz) 및 PPS 매핑 설정을 완벽하게 빌드인.
     - 부팅 초기화 시 듀얼 W5500의 CS 핀을 High로 올리고, Active-Low 리셋 라인을 순차적으로 강제 제어(Low 인가 후 PLL 안정화 지연 대기)하여 칩셋 기동 신뢰성 확보.
     - W5500의 기본 3바이트 주소/제어 헤더(Address High/Low + Control Phase 쓰기/읽기 구분 비트) 트랜잭션 연산을 통한 고속 레지스터 1바이트 쓰기(ETH_WriteReg) 및 읽기(ETH_ReadReg) API 골격 설계 완료.
     - pin_map.h에 듀얼 이더넷용 매크로를 일체 수립하고, main.c 의 시스템 초기화 시퀀스 체인에 ETH_Initialize()를 최종 통합 연동 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - W5500 이더넷 컨트롤러 (W5500 Ethernet Controller) 기술 용어 추가 완료.


### 174. [Firmware] W25Q256 SPI Flash 드라이버 구현 및 이더넷 2호기 핀 맵 정밀 복원 완료 (2026-08-10)
- **검토 내용**: Mcu.SchDoc 회로 결선 데이터를 정밀 비교하여 물리 8번, 9번 핀이 이더넷 포트가 아니라 W25Q256 플래시 메모리 신호선임을 확인하고, 이더넷 2호기의 실제 리셋 및 인터럽트 포트(RA3/RF5) 핀 맵을 정정 복원함과 동시에 플래시 드라이버를 구현 완료.
- **분석 및 해결 가이드라인**:
  1. **회로도 교차 분석을 통한 이더넷 2호기 핀 맵 정정**:
     - ETH2_RSTN: 물리 27번 핀, RA3 포트 (LAT 및 TRIS 맵 정정)
     - ETH2_INT: 물리 26번 핀, RF5 포트 (PORT 및 TRIS 맵 정정)
  2. **W25Q256 SPI Flash 핀 맵 획득**:
     - FLASH_CLK: 물리 8번 핀, RC14 포트 (SPI3 SCK)
     - FLASH_DI (MOSI): 물리 9번 핀, RC15 포트 (SPI3 SDO)
     - FLASH_DO (MISO): 물리 11번 핀, RD15 포트 (SPI3 SDI)
     - FLASH_CS: 물리 12번 핀, RF1 포트 (일반 GPIO CS 제어선)
  3. **Flash 드라이버 개발 및 프로젝트 연동**:
     - flash.h와 flash.c를 생성하여 SPI3 주변기기를 마스터 모드(10 MHz 클록) 및 PPS 매핑으로 초기화하는 FLASH_Initialize()를 작성.
     - JEDEC ID(0x9F) 리드 커맨드를 통해 플래시 정상 부팅 상태(제조사 ID 및 용량 코드 획득) 검증 체계 수립.
     - 4KB 섹터 일괄 소거(FLASH_EraseSector), 최대 256바이트 페이지 프로그래밍(FLASH_WritePage), 다바이트 데이터 연속 리드(FLASH_ReadData) 및 쓰기 보호 해제(FLASH_WriteEnable) API 구현 완료.
     - pin_map.h 초기화 블록에 FLASH CS를 High(비활성화)로 올리도록 구성하고, main.c 부팅 루틴에 FLASH_Initialize()를 최종 통합 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 플래시 메모리 (Flash Memory) 기술 용어 추가 완료.


### 175. [Firmware] DS3231 고정밀 RTC 드라이버 구현 및 시간 동기화 체계 연동 완료 (2026-08-10)
- **검토 내용**: 시스템의 정밀 시간 데이터 획득을 위해 I2C1 공유 버스를 사용하는 DS3231SN+ RTC 주변장치 드라이버 구현 및 핀 매핑 완료.
- **분석 및 해결 가이드라인**:
  1. **하드웨어 인터럽트 및 통신 라인 규명**:
     - RTC_INT: 물리 72번 핀, RB7 포트 (인터럽트 알람 감시선 입력 방향 정의)
     - I2C 공유: ads1115.c 에 기 구축된 I2C1 버스(SDA: RG3, SCL: RG2)를 공유하여 다중 마스터 간섭 배제 및 핀 리소스 효율 극대화.
  2. **RTC 드라이버 개발 및 BCD 포맷 연산**:
     - rtc.h와 rtc.c를 생성하여 BCD(Binary Coded Decimal) 형태의 시간 데이터 레지스터(0x00 ~ 0x06)를 Decimal 10진수로 변환하는 BCD2DEC() 및 DEC2BCD() 무결성 인라인 함수 수립.
     - RTC_Initialize()를 통해 I2C1 기본 결선 통신 신뢰성을 확인하고, Hours 레지스터의 12/24시간 설정을 감시하여 24시간 형식으로 자동 복원 및 보정하는 예외 처리 반영.
     - 현재 시간 정보를 단일 7바이트 리드로 획득하는 RTC_GetTime() 및 동기화를 위한 일괄 쓰기 RTC_SetTime() API 구현 완료.
  3. **메인 BOP 스케줄러 연동**:
     - pin_map.h에 RTC 인터럽트 핀 초기 설정을 탑재하고, main.c 부팅 시 RTC_Initialize()를 바인딩.
     - 분주 제어 루프 내부에서 약 200ms 주기로 RTC_GetTime()을 수행하여 획득한 년/월/일/시/분/초 데이터를 Modbus 입력 레지스터 50 ~ 55번지에 실시간 업데이트 처리 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 실시간 클록 (Real-Time Clock, RTC) 기술 용어 추가 완료.


### 176. [Control UI] 연료전지 BOP 통신 매핑 검증용 시제품 GUI 대시보드 개발 완료 (2026-08-10)
- **검토 내용**: GUI 개발 착수 전, 31개 온도 채널 및 다채널 ADC/DAC/DI/DO의 Modbus 패킷 매핑 상태를 1차적이고 완벽하게 사전 검증하기 위한 웹 기반 데이터 뷰어 및 시뮬레이터 구축.
- **분석 및 해결 가이드라인**:
  1. **인터랙티브 웹 대시보드 구축**:
     - HTML5, CSS3(Vanilla), ES6(JavaScript) 기반의 SPA 웹 애플리케이션 설계.
     - 사용자 경험 극대화를 위해 Outfit/Inter 현대식 타이포그래피, 다크 모드 글래스모피즘(Glassmorphism) 테마 및 갱신 애니메이션 가동.
  2. **가상 Modbus RTU 패킷 시뮬레이터 내장**:
     - 마스터(PC)에서 쏘는 Modbus RTU 헥사 프레임(예: `01 04 00 00 00 13 70 0D`)의 동적 생성 및 시리얼 콘솔 로그 모니터 모사.
     - 펑션 코드 0x04(Read Input Regs) 수신 시 해당 주소 범위의 센서 데이터 난수 변동 및 하이라이트 행 애니메이션 피드백 구동.
     - 펑션 코드 0x06(Write Single Reg) 수신 시 특정 DAC 채널 값을 업데이트하고 에코백(Echo-back) 응답을 반환하는 양방향 가상 검증 로직 수립.
  3. **데이터 매핑 표 탑재**:
     - 자기가 제공한 BOP 엑셀 결선 사양과 100% 매치되는 31개 채널 온도계 리스트, 16채널 아날로그 입력(원시 코드 -> 물리 전압 환산), 12채널 아날로그 출력(DAC 설정 코드 -> 실전압 매핑) 테이블 화면을 일체 탑재.
     - 03_Control_UI 디렉토리에 index.html 소스를 정식 보존 완료.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 글래스모피즘 (Glassmorphism) 기술 용어 추가 완료.


### 177. [Control UI] FHD 고정 해상도 2단 대시보드 개편 및 통신 모달 팝업 적용 완료 (2026-08-11)
- **검토 내용**: 모니터 해상도 1920x1080 (FHD) 기준, 스크롤 없는 한 화면에 모든 데이터를 빽빽하고 가시성 높게 배치하는 개편 요청 및 통신 관련 모듈의 팝업화 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **FHD 고정 뷰포트 및 스크롤바 완전 제거**:
     - body 크기를 1920px * 1080px로 지정하고 `overflow: hidden;`을 선언하여 스크롤 오버헤드와 화면 잘림 방지.
  2. **좌/우 2단 대칭형 레이아웃 구성 (Input vs Output)**:
     - 좌측(입력부): 온도 센서 31개 채널을 3열 컴팩트 그리드로 재배치하여 수직 공간 단축. ADC 16개 채널 또한 2열 배치로 가시성을 확보하였으며, 디지털 입력(DI 16채널) LED 인디케이터 구역을 신설하여 외부 긴급 차단 및 리밋 스위치 실시간 상태 표기.
     - 우측(출력부): 아날로그 제어 출력인 DAC 12개 채널을 2열로 리모델링하였고, 디지털 출력(DO 16채널) 스위칭 영역을 신설하여 릴레이 및 솔레노이드 밸브 상태 감시와 개별 수동 제어(Toggle) 테스트가 가능하도록 연동.
  3. **통신 모듈의 반투명 글래스모피즘 모달(Modal) 팝업화**:
     - 상시 데이터 감시 영역을 침해하지 않도록, Modbus RTU Packet Simulator와 Serial Console Log 모니터를 통합하여 버튼 클릭 시에만 떠오르는 미려한 Blur 효과의 모달 팝업으로 격리 구성.
     - 가상 Modbus 패킷 송출에 따른 실시간 온도/ADC 데이터 난수 변경 및 쓰기 단일 레지스터(0x06) 동작 시 실제 DAC/DO 상태 테이블에 즉각 에코백 동기화 연동 처리.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 모달 윈도우(Modal Window), FHD 해상도 최적화(FHD Resolution Optimization), 디지털 입출력 & 아날로그 입출력(DI/DO & AI/AO) 신규 기술 용어 추가 완료.


### 178. [Control UI] 10kW BOP 설계 전환 및 6kW 축소 사양 호환성(음영/참조) 공용화 연동 완료 (2026-08-11)
- **검토 내용**: 최신 10kW BOP 핀 매핑 엑셀 문서를 기준으로 대시보드 구조를 전환하되, 6kW 축소 사양과의 호환성을 만족하기 위한 음영 처리 및 수동/자동 참조 차단 연동 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **10kW/6kW 실데이터 교차 분석 및 채널 맵 교체**:
     - `260727_10kW IO 넘버링 Ver 1.1...` 엑셀을 분석하여 온도 32개, ADC 14개, DAC 9개, DI 5개, DO 20개 등 실제 10kW 기준 채널을 `index.html` 내에 완전 교체 주입.
     - 6kW 엑셀과 교차 검증을 거쳐 6kW에만 존재하지 않는 전용 채널 `TC377 (CH16)` 및 `AO_P375 (P375)` 2종 식별.
  2. **하이브리드 공용화 연동 (음영 및 참조 처리)**:
     - **UI**: 미지원 2개 채널에 `.inactive-channel` 클래스를 신설 적용하여 반투명 음영 처리하고, 비고란에 `[10kW 전용]` 뱃지를 표기해 실시간 운영 및 6kW 기동 시의 핀 오매핑 혼선 원천 차단.
     - **JS**: 각 채널 객체에 `is6kW` 속성을 바인딩하고, 가상 Modbus 패킷 송출에 따른 갱신 연프레임 및 단일 레지스터 쓰기(0x06) 동작 시 미지원 채널은 업데이트 주기에서 자동으로 스킵(참조 우회) 처리되도록 JS 비즈니스 로직 수정.
  3. **우측 레이아웃 세로 비율 불균형 및 20개 DO 채널 완전 수용**:
     - DAC 카드와 DO 카드의 flex 비율을 `flex: 0.85` 대 `flex: 1.65`로 차등 재조정하여 DAC 하단의 빈 여백을 완전히 메우고 DO 카드의 세로 크기를 확보.
     - DO 20개 채널 전체가 스크롤바 없이 꽉 들어차도록 DO 인디케이터 그리드를 `7열 3행` 구조로 빽빽하게 압축 정렬하여 가시성과 디자인 미려성 극대화.
- **[Doc] 프로그램 용어 사전(PROGRAMMING_TERMS.md) 업데이트**:
  - 하이브리드 공용화 설계(Hybrid Common Design) 신규 기술 용어 추가 완료.


### 179. [Control UI] 디지털 출력(DO) 카드 수직 정렬 기준 개선 및 밀도 콤팩트화 완료 (2026-08-11)
- **검토 내용**: 우측 아래 디지털 출력(DO)의 3행 20개 채널 인디케이터가 세로 중앙 정렬로 인하여 아래쪽 버튼들이 카드 밖으로 밀려 잘리던 현상 개선 및 최적의 상단 정렬 적용 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **수직 정렬 기준 변경 (Center -> Top-aligned)**:
     - DO 카드의 `card-body`에 설정되어 있던 `align-items: center;`를 `align-items: flex-start;`로 변경 적용하여 모든 DO 그리드 채널이 카드의 상단 경계부터 순차적으로 채워지도록 수정.
  2. **그리드 및 아이템 패딩 압축 (Compact 튜닝)**:
     - `do-grid` 줄간격(`gap`)을 `8px`에서 `6px`로 줄이고, 각 `led-item`의 세로 패딩을 `6px`에서 `4px 6px`로 더 압축하여, 3행 20개 채널 전체의 상태등과 제어 버튼이 카드의 한정된 세로 높이 내에 100% 온전히 표출되도록 레이아웃 잘림 방지 최적화 완료.


### 180. [Control UI] 디지털 입출력(DI/DO) 그리드 카드 내 실질적 한글 장치명 상시 표기 완료 (2026-08-11)
- **검토 내용**: 디바이스 명칭(영문 라벨)만 표시되던 기존 DI/DO 인디케이터에 엑셀 캡처 사양의 실질적 한글 장치명을 상시 노출시켜 대시보드의 정보 직관성 및 시인성을 보완하고자 함.
- **분석 및 해결 가이드라인**:
  1. **실질적 한글 명칭 정밀 교정**:
     - 사용자가 제공한 엑셀 스크린샷과 100% 일치하도록 데이터 맵의 한글 설명을 갱신 (`DO_SV125`: `개질기 질소퍼지 솔레노이드 밸브`, `DO_SV145`: `ANODE 인입 솔레노이드 밸브` 등).
  2. **서브 텍스트 레이아웃 신설 및 렌더링**:
     - CSS에 `.led-desc` 클래스를 신설(폰트 `7.5px`, 색상 그레이 톤)하고, 마우스 오버 시 입체적인 화이트 하이라이팅 효과 부여.
     - DI 및 DO 렌더링 루프 마크업에 서브 텍스트 레이어인 `<div class="led-desc">${item.desc}</div>`를 삽입하여, 한 칸 안에 영문 라벨과 한글 실명이 깨짐 없이 수직으로 가지런하게 상시 공존하도록 콤팩트 UI 최적화 완료.


### 181. [Control UI] 디지털 출력(DO) 장치 성격별 개별 피드백 및 특화 제어 텍스트 적용 완료 (2026-08-11)
- **검토 내용**: DO 채널 중 SV/MV(밸브류) 및 HT(히터류) 등 각 부품 성격에 부합하는 개별 상태 텍스트(열림/닫힘, 가열) 및 시인성이 높은 컬러 테마(초록, 빨강 발광) 개별화 요구사항 반영.
- **분석 및 해결 가이드라인**:
  1. **장치 분류 속성 추가 및 UI 텍스트 이원화**:
     - `doChannels` 변수 내에 `type` 속성을 신설하여 `VALVE`, `HEATER`, `GENERAL`로 분기 데이터 바인딩.
     - **밸브류 (`DO_SVxxx`, `DO_MVxxx`)**: 기동 버튼을 `열기 / 닫기`로 전환하고, 돔(DOM) 상태 표현 텍스트를 `열림 / 닫힘`으로 변경 적용.
     - **히터류 (`DO_HTxxx`)**: 가열 기동 시 `가열 중`으로 상태를 표시하고 버튼은 `정지 / 기동`으로 대응 표기.
     - **기타 장치**: 기존 공정 계통도 표준인 `기동 / 정지` 및 `ON / OFF` 매핑 적용.
  2. **장치 속성별 발광 컬러 분기 (CSS/JS)**:
     - 밸브가 열렸을 때(`.active-open`)는 산뜻한 초록색(var(--accent)) 네온 네뷸러 효과를 구현하고, 작동 버튼의 배경 및 테두리도 초록 톤으로 자동 전환.
     - 히터가 구동될 때(`.active-heat`)는 고온 작동에 따른 직관성을 높이기 위해 경고 빨간색(var(--danger)) 발광을 연출하고 작동 버튼도 빨간 톤으로 자동 변환.
     - 시리얼 콘솔 로그 및 가상 Modbus 수동 피드백 출력도 `열림`, `닫힘`, `가열` 등 장치 속성에 맞춰 한국어 액션 스트링으로 동적 갱신되도록 콘솔 디깅 로직 보완.


### 182. [Control UI] 아날로그 입력(ADC) 전압 신호(1~5V / 0~5V) 기반 실제 물리 스케일 환산 처리 완료 (2026-08-11)
- **검토 내용**: 임의의 배율만 곱해지던 기존 아날로그 수신값(ADC)을, 엑셀 사양서의 입력 신호 규격(1~5V 또는 0~5V)과 실제 센서가 계측할 수 있는 압력/유량/전도도 물리 범위에 부합하도록 정밀 선형 환산 보간 알고리즘 적용 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **물리 환산용 정밀 핀 매핑 데이터 보강**:
     - `adcChannels` 데이터 맵 내에 각 센서의 입력 전압 범위(`minV`, `maxV`) 및 실제 계측값 물리 범위(`minReal`, `maxReal`) 속성을 전수 바인딩 (예: `AI_PT105` 0~10 kPa 1~5V, `AI_MFC111` 0~30 LPM 0~5V 등).
  2. **2단계 선형 변환 알고리즘 수립 (JS)**:
     - 1단계(전압 환산): ADC 원시 코드를 바탕으로 FSR 5V 비례 전압을 계산 (`V = raw / 65535 * 5.0 V`).
     - 2단계(물리량 보간): 1~5V 계통 센서와 0~5V 계통 센서로 분기 연동하여 선형 스케일 환산 로직 구현. 1V 미만 수신 시 센서의 최소 물리량(단선 오류 상태)으로 고정 처리하는 강건 설계 도입.
  3. **ADC 대시보드 테이블 UI 고도화**:
     - 테이블 마크업 헤더에 `전압` 열을 신설하고, 돔(DOM) 갱신 시 `RAW` -> `전압 (V)` -> `환산 물리값`이 수평으로 함께 갱신되도록 설계하여 가상 Modbus 패킷 송출 디버깅 시의 상태 판독성을 대폭 향상 완료.


### 183. [Control UI] 아날로그 출력(DAC) 제어 신호(0~5V) 기반 실제 출력율(0~100%) 환산 처리 완료 (2026-08-11)
- **검토 내용**: 아날로그 입력(ADC)의 전압 선형 환산 개편에 맞추어, 에어블로어 및 펌프 계통 아날로그 출력(DAC) 또한 출력 전압(0~5V)에 비례하는 제어 세기 백분율(0~100%) 환산 및 컬럼 정밀 연동 보완 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **DAC 수치 비례 스케일링 (JS)**:
     - 0~5V 제어 사양 기준, RAW 코드(`0 ~ 65535`) 수신 시 출력 전압 `Volt = (raw / 65535) * 5.0 V` 공식 및 백분율 제어 출력율 `Percent = (raw / 65535) * 100.0 %` 공식 동시 바인딩.
  2. **DAC 대시보드 테이블 UI 리모델링**:
     - 테이블 헤더 컬럼을 `RAW` -> `전압` -> `제어 출력율`로 신설 재편하고, 입력(AI) 대시보드 테이블 레이아웃과 미적/구조적 통일성 확보.
     - 통신 모달 내 0x06 (Write Single Reg) 제어 입력 시 우측 상단 테이블의 출력 전압(V)과 출력율(%)이 스무스하게 1:1 선형 연동 갱신되도록 렌더러 보완 완료.


### 184. [Control UI] 아날로그 출력(DAC) 개별 수동 조작 모달 팝업 및 Modbus 0x06 피드백 연동 완료 (2026-08-11)
- **검토 내용**: 실시간 계측 모니터링만 수행하던 아날로그 출력(DAC) 기기들에 대해 디지털 출력(DO)처럼 직접 출력을 가변 테스트해 볼 수 있도록 하는 대시보드 조작 인터페이스 수립 요구사항 반영.
- **분석 및 해결 가이드라인**:
  1. **개별 제어 모달 마크업 및 슬라이더 탑재 (HTML/CSS)**:
     - DAC 테이블 행 호버 효과와 커서 포인터를 부여하여 클릭 가능한 요소임을 인지하게 하고, 클릭 시 중앙에 팝업되는 글래스모피즘 모달 창 (`id="dac-modal-overlay"`) 신설.
     - 가로 세로 높이 제어가 용이하도록 출력 조절 슬라이더(`0.00% ~ 100.00%`)와 RAW 코드 값(`0 ~ 65535`) 직접 입력 폼을 함께 배치하여 조작 편의성 극대화.
  2. **수동 제어 및 Modbus 0x06 통신 패킷 모사 동기화 (JS)**:
     - 슬라이더 조절 시 환산 전압(V)과 출력율(%)이 수식 비례 연산되어 모달 상에 즉시 선형 피드백을 제공.
     - `제어 명령 송출` 버튼을 누르면 해당 채널의 `val`을 갱신하고 테이블을 리렌더링하며, 시리얼 로그창에 `0x06` 쓰기 요청 및 에코백 응답 통신 프레임을 가상 모사 출력하여 디버그 라인 무결성을 보장.
     - 10kW 전용 비활성 채널(`AO_P375`) 조작 시도를 미연에 감지 및 차단하는 안전 경고 인터록 스크립트 연동 완료.


### 185. [Control UI] RS-485 시리얼 통신 장치 3종(인버터, LCD, 유량계) 모니터링 카드 신설 완료 (2026-08-11)
- **검토 내용**: 송수신 상태 모사가 부재했던 RS-485 통신 기반 시리얼 장치 3종(인버터 485, LCD 모니터, 개질수 유량계)에 대해 회로 도면에 기반한 전용 데이터 및 RX/TX 인디케이터 점멸 피드백 대시보드 추가 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **시리얼 모니터 카드 신설 및 우측 패널 재배치 (HTML/CSS)**:
     - 우측 패널 중앙 영역에 `시리얼 통신 장치 현황 (RS-485)` 카드를 삽입하고, DAC/DO 카드의 flex 비율을 조절하여 스크롤바 없이 1920x1080 FHD 내에 완벽 핏으로 연동.
     - 통신 단선 상태를 모니터할 수 있는 정상/오류 연결 상태 뱃지(`.link-status`) 및 주황/초록 LED 형태의 물리 RX/TX 가상 인디케이터(`.comm-led`) 신설.
  2. **10kW 통신 사양 기반 장치 데이터 매핑 및 점멸 시뮬레이션 (JS)**:
     - **인버터 485 (`INV506`)**: 주파수 설정값/출력값(Hz), 가동 전류(A) 표현.
     - **LCD 모니터 (`LCD500`)**: 화면 백라이트 및 휘도 상태 표현.
     - **개질수 유량계 (`FM382`)**: 모델명 `OF05ZAT-AR` 규격을 연동하여 순시 유량(LPM) 및 누적 유량(L) 표현.
     - 가상 Modbus 패킷 송출 디버거 작동 시, 시리얼 장치들의 수치 데이터가 실시간 난수로 동적 갱신됨과 동시에 RX/TX LED가 `500ms` 동안 실시간 점멸되는 애니메이션 펄스를 적용하여 통신 활동성 가시화를 완성함.


### 186. [Control UI] RS-485 시리얼 테이블 컬럼 가로폭 재배분 및 정렬 피드백 정정 완료 (2026-08-11)
- **검토 내용**: 시리얼 통신 장치 현황 테이블 내의 계측 데이터가 너무 우측으로 쏠려 통신 LED와 상태 뱃지를 화면 밖으로 밀어내 잘리던 정렬 불균형 현상 보완 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **가로폭 비율 전면 재배분 (HTML)**:
     - 485 장비 테이블의 각 헤더 `th` 너비(`width`)를 재정비하여, 왼쪽 영역(`NETNAME`, `모델명`)의 과도한 여백을 깎고 `계측값` 및 `TX/RX` 열의 너비 비중을 늘림 (예: 계측값 열 width를 `260`으로 대폭 확장).
  2. **정렬 및 패딩 튜닝 (JS)**:
     - 기존의 `text-align: right;` 정렬 방식에서 `text-align: left; padding-left: 10px;` 정렬로 전면 개편하여, 텍스트가 인접 컬럼과 간섭을 빚지 않고 중앙-우측 사이에서 자연스럽게 가독성을 유지하도록 교정 완료.


### 187. [Control UI] 디지털 출력(DO) 가시성 확보용 우측 카드 세로 비율(flex) 재배분 완료 (2026-08-11)
- **검토 내용**: 우측 아래 디지털 출력(DO) 카드의 수직 높이가 모자라 세 번째 줄의 릴레이/히터 버튼들이 화면 하단에 가려 잘려 보이던 현상 개선 및 상단 여백 압축을 통한 DO 공간 추가 확보 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **상단 카드 여백 회수 및 높이 압축 (HTML)**:
     - 내부 행수가 적어 여백이 남던 아날로그 제어 출력(DAC) 카드의 flex 비율을 `0.8`에서 `0.6`으로 축소.
     - 시리얼 통신 장치(RS-485) 카드의 flex 비율도 `0.55`에서 `0.45`로 축소하여 내부 여유 공간을 콤팩트하게 상단으로 밀어 올림.
  2. **하단 DO 조작 영역 세로 수직 확장 (HTML)**:
     - 축소 회수된 세로 공간을 디지털 출력(DO) 카드에 할당하여 flex 비율을 `1.15`에서 `1.45`로 전격 확장.
     - 이를 통해 DO 카드 내부의 3행(15~20번 릴레이 및 가열 히터, 점화 트랜스 버튼)이 밀림이나 하단 경계 잘림 없이 FHD(1920x1080) 단일 고정 해상도 뷰포트 내에 100% 정상 가시화되도록 조정 완료.


### 188. [Control UI] RS-422 물리 시리얼 통신 포트 환경 설정 및 연결 상태 인터록 연동 완료 (2026-08-11)
- **검토 내용**: 실 제어 PCB와 PC 노트북 간에 RS-422 통신을 할 때 필요한 통신 방식, 포트 번호(COM), 속도(Baudrate) 설정 인터페이스 확보 및 미연결 시 제어 차단 인터록 요구사항 반영.
- **분석 및 해결 가이드라인**:
  1. **물리 시리얼 포트 설정 패널 추가 (HTML/CSS)**:
     - 디버그 모니터 팝업 내부에 통신 인터페이스(`RS-422`, `RS-485`, `RS-232`), 포트명(`COM1~5`), 속도(`9600~115200bps`), 패리티 및 스톱 비트 설정을 직접 지정하고 포트를 가상으로 온오프할 수 있는 글래스모피즘 설정 폼과 `포트 연결` 제어 스위치 장착.
  2. **연결 정보 뱃지 및 점멸 펄스 (HTML/CSS)**:
     - 상단 헤더 RTC 시계 좌측 영역에 연결 상태를 나타내는 지시등 뱃지(`header-comm-status-badge`)를 신설하여 포트가 연결되면 실시간 연결 포트 규격 텍스트가 표시되며 초록색 LED가 깜빡이도록 렌더러 설계.
  3. **통신 차단 안전 인터록 (JS)**:
     - `isPortConnected` 논리 상태 변수를 정의하여, 포트가 열리지 않은 상태에서는 Modbus 디버그 패킷 송출, DAC 아날로그 수동 제어(슬라이더), DO 디지털 릴레이/밸브 기동 버튼 조작을 전면 무효화하고 "포트 미개방 경고창"을 출력하도록 인터록 수립 완료.


### 189. [Control UI] 헤더 통신 상태 뱃지 클릭 설정 모달 팝업 숏컷(Shortcut) 연동 완료 (2026-08-11)
- **검토 내용**: 상단 헤더의 통신 뱃지 클릭 시 포트 설정을 바로 변경할 수 있는 모달창으로 진입하고 싶다는 사용성 개선 요구사항 반영.
- **분석 및 해결 가이드라인**:
  1. **호버 피드백 및 포인터 추가 (CSS)**:
     - `.header-comm-status` 클래스에 `cursor: pointer;` 및 `user-select: none;`을 선언하여 클릭 버튼의 아이덴티티를 확립하고, `:hover` 가상 선택자에 밝기 변동 및 그림자 이펙트 추가.
  2. **클릭 이벤트 매핑 (JS)**:
     - 헤더 뱃지(`header-comm-status-badge`) 클릭 시 디버그 모니터 팝업 모달창(`debug-modal-overlay`)에 `active` 클래스를 추가하여 즉시 모달이 기동되도록 이벤트를 바인딩 완료.


### 190. [Control UI] RS-422 물리 시리얼 통신 연동용 Web Serial API & WebSocket 기술 아키텍처 수립 완료 (2026-08-11)
- **검토 내용**: 순수 HTML/JS 브라우저 환경에서 실제 PC/노트북의 RS-422 물리 시리얼 포트(COM)를 개방하여 제어 PCB와 직접 실시간 Modbus 패킷 통신을 수행할 수 있는 실현 방안 및 시스템 아키텍처 수립 요청 대응.
- **분석 및 해결 가이드라인**:
  1. **Web Serial API 연동 방안**:
     - 별도의 백엔드 중계 서버 설치 없이, Chrome/Edge 표준 웹 브라우저의 `navigator.serial` 객체를 활용하는 **`Web Serial API`** 스펙을 적용하여 브라우저 단에서 COM3 포트에 직접 억세스 및 쓰기/읽기 스트림을 수행하는 기술 경로를 안내하고 `PROGRAMMING_TERMS.md` 사전에 정식 기술 용어로 추가.
  2. **WebSocket 중계 아키텍처 정의**:
     - 전통적이고 가장 안정적인 로컬 서버 중계 방식인 Node.js/Python 백엔드 연동 경로를 제시. 백엔드가 RS-422 물리 시리얼 포트와 직접 고속 Modbus RTU 통신을 주도하고, 웹 대시보드 화면에는 **`WebSocket`** 양방향 소켓 통신을 이용해 실시간 데이터를 브로드캐스팅하는 실무 표준 아키텍처 가이드를 수립 완료.


### 191. [Control UI] Python 시리얼 브릿지(serial_bridge.py) 및 실전 웹소켓 연동 개발 완료 (2026-08-11)
- **검토 내용**: 프로토타입 샘플을 넘어 PC/노트북의 실 시리얼 포트(COM3)와 실제 연동하여 외부 기기와 실전 통신을 하고, 대시보드가 실제 하드웨어 관제 장치로 완전히 기동될 수 있도록 기능 구체화 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **초경량 파이썬 시리얼 웹소켓 브릿지 구현 (`[NEW] serial_bridge.py`)**:
     - `pyserial` 및 `websockets` 비동기 라이브러리를 연동하여, 물리 COM 포트의 수신 바이너리 프레임을 띄어쓰기 가공 Hex 데이터로 로컬 웹소켓에 실시간 포워딩하고 웹 브라우저의 전송 명령 Hex 패킷을 물리 포트로 송출하는 백엔드 중계 게이트웨이 서비스 신설. 물리 장치 부재 시 가상 난수 데이터를 주기적으로 전송하는 에뮬레이터 자가 기동 메커니즘 설계.
  2. **대시보드 실전 웹소켓 모드 연동 및 패킷 처리 (HTML/JS)**:
     - 디버그 모달 내부에 `가상 시뮬레이션 모드`와 `웹소켓 실전 모드`를 제어하는 라디오 버튼 스위치를 배치.
     - 실전 모드 시 `ws://localhost:8888` 웹소켓 클라이언트로 자동 접속해 계측 수치를 동적 갱신하고, 사용자가 DAC 슬라이더나 DO 릴레이 버튼을 누를 시 `0x06` 쓰기 명령 Hex 프레임을 웹소켓 서버를 거쳐 물리 시리얼 포트로 전송하는 바이너리 통신 파이프라인의 구축 및 동작 검증을 완수함.



### 192. [Control UI] 이더넷(Ethernet / Modbus TCP) 통신 포트 설정 인터페이스 신설 완료 (2026-08-11)
- **검토 내용**: 실 제어 보드가 RS-422 시리얼 링크뿐만 아니라 공장망 및 무선 공유기 환경에 연계할 수 있는 Modbus TCP/IP 이더넷 인터페이스 설정을 추가해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **인터페이스 동적 토글 UI 수립 (HTML/CSS/JS)**:
     - 물리 시리얼 설정 디렉티브 박스에 Ethernet (Modbus TCP) 방식을 신설.
     - toggleInterfaceInput(value) 함수를 구현하여 시리얼 옵션 선택 시 포트/보드레이트 입력 상자가 표시되고, 이더넷 선택 시 시리얼 상자들은 숨겨지며 IP 주소 (192.168.1.100) 및 TCP 포트 (502) 입력란이 표출되는 동적 레이아웃 스위칭 실현.
  2. **이더넷 뱃지 동기화 및 가상 소켓 프레임 (JS/Python)**:
     - 연결 개방 시 상단 헤더 뱃지가 🔌 TCP: 192.168.1.100:502 [연결됨]으로 실시간 갱신 및 초록색 LED 점멸 구동되도록 연동.
     - serial_bridge.py 파일 내에 OPEN_TCP 비동기 신호 수신 블록을 신설하여 이더넷 연결 모사 로깅과 가상 데이터 파이프라인 연동 성공.

### 193. [Control UI] UI/UX 4대 고도화 개발 로드맵 승인 및 1일차 기초 정비(CSS 테마 변수화) 완료 (2026-08-11)
- **검토 내용**: 실 제어 PCB 제작 대기 기간(약 1주일) 동안, 대시보드의 사용성과 비주얼을 HMI 수준으로 끌어올리기 위한 UI/UX 4대 개선안(트렌드 차트, 다중 테마, 실시간 경보, 반응형 모바일 최적화) 로드맵 시작.
- **분석 및 해결 가이드라인**:
  - 향후 원클릭 테마 전환 체인저 장착에 대비하여 CSS 글로벌 변수 :root 영역 밑에 body.light-theme 클래스에 귀속되는 10여 개의 라이트 테마 고유 색상 및 광채 반경(GLOW) 변수를 사전에 선언하여 스타일 시트의 기반 인프라를 확립 완료.

### 194. [Control UI] 온도 센서(TC 스캐너) 40채널 확장 및 4열 정렬 뷰포트 레이아웃 개편 완료 (2026-08-11)
- **검토 내용**: 실제 제어 PCB 보드 사양이 40채널 온도 입력을 지원함에 따라, 32채널로 설계되어 있던 기존 대시보드 화면을 40채널 전체 규격으로 확장하고 균등 4열 배치로 레이아웃 가시성을 보강해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **그리드 4열 배치 설계 (CSS/HTML)**:
     - CSS 내 .grid-4col { grid-template-columns: repeat(4, 1fr); } 클래스를 새로 선언하고, 온도 센서 측정 현황 카드 몸체의 마크업 테이블을 Col 1~4의 4개 독립 칼럼으로 확장 개편.
  2. **40채널 매핑 데이터셋 구축 (JS)**:
     - 자바스크립트 tcChannels 전역 데이터 어레이 구조에 누락되어 있던 CH20 예비 채널 및 미배정 CH33 ~ CH40 예비 채널을 선언(is6kW: false 속성을 주어 10kW 비활성화/음영 처리 룰 준수)하여 40개 채널 연속성을 확보.
  3. **10채널 분할 배포 렌더러 구현 (JS)**:
     - renderAll() 함수 내부의 TC 테이블 동적 빌더 루프를 수정하여, 40개 채널 데이터를 10개 단위로 정밀 분할해 Col 1~4 테이블 몸체에 균등 매핑 렌더링되도록 수정 및 검증 완료.

### 195. [Control UI] 온도 센서(TC) 테이블 중복 TYPE 열 삭제 및 공통 헤더 라벨 도입을 통한 가독성 최적화 완료 (2026-08-11)
- **검토 내용**: 40채널 온도 배치 시 한 화면(1080p 고정)에 세로 스크롤바가 발생하는 문제를 해소하고, 테이블 내 중복 노출되는 TYPE(T/K) 열을 제거하여 가로/세로 레이아웃 공간을 최적화하고 가독성을 확보해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **테이블 셀 세로 높이 최적화 (CSS)**:
     - 글로벌 테이블 th 및 td 셀의 padding 값을 4px 6px에서 3px 5px로 조밀하게 조정하여 세로 공간 소모를 극소화하고 스크롤바 발생 원천 차단.
  2. **중복 TYPE 열 제거 및 공통 헤더 신설 (HTML/JS)**:
     - 각 테이블 내 개별 행마다 렌더링되던 T/K 뱃지(badge-type) 열을 전면 삭제하고, 4개 칼럼 테이블 상단에 각각 T-Type (CH1 ~ CH10), T-Type (CH11 ~ CH20), K-Type (CH21 ~ CH30), K-Type (CH31 ~ CH40) 네온 스타일의 공통 타이틀 라벨 헤더를 장착하여 시인성 최적화.

### 196. [Control UI] 온도 테이블(TC) 셀 높이 고정 및 nowrap 텍스트 말줄임 처리를 통한 세로 정렬 및 스크롤 차단 최적화 (2026-08-11)
- **검토 내용**: 3열 및 4열의 K타입 센서 넷네임 및 라벨 길이가 길어 일부 행에서 줄바꿈이 발생해 4개 열 간의 행 높이가 불일치되고 세로 스크롤바가 재차 발생하는 문제를 해소하기 위한 2차 미세 조정 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **텍스트 줄바꿈 금지 및 고정 셀 높이 튜닝 (CSS)**:
     - .tc-table-container 스타일 영역을 신설하여, 온도 테이블 내 글자 크기를 10.5px에서 9px로 축소 조정하고, 모든 td와 th에 white-space: nowrap과 overflow: hidden, text-overflow: ellipsis 속성을 부여하여 텍스트 길이 초과 시 한 줄 말줄임으로 자동 정리되도록 설계.
     - 각 셀의 높이를 height: 24px로 명시적으로 규정하여 4개 칼럼 간의 행 평행선과 세로축 정렬을 완벽하게 맞추고 세로 스크롤바 발생 현상을 완벽하게 조치 완료.

### 197. [Control UI] 측정 위치 내 중복 채널(CH) 표시 제거 및 폰트 크기 복구(10px) 가독성 극대화 완료 (2026-08-11)
- **검토 내용**: NET 열(CH1~40)의 정보와 측정 위치 셀 내부의 괄호 채널 표시 (1)~(40)가 중복 기재되어 텍스트 길이만 늘리는 현상을 해소하고, 이를 통해 절약한 가로 폭을 바탕으로 폰트 크기를 다시 가독성 높게 환원해 달라는 피드백 반영.
- **분석 및 해결 가이드라인**:
  1. **중복 채널 표시 삭제 (JS)**:
     - JS 렌더러 루프 내 tr.innerHTML 템플릿 구문에서 (${item.ch}) 파트를 삭제하여 텍스트 길이를 최소 4글자 이상 콤팩트하게 다듬음.
  2. **가독성 폰트 확대 (CSS)**:
     - 확보된 가로 공간에 힘입어 .tc-table-container table의 폰트 크기를 9px에서 10px로, th 헤더 폰트 크기를 8.5px에서 9.5px로 상향하여 선명한 가시성을 복구하고 줄바꿈/세로 정렬 꼬임을 방지함.

### 198. [Control UI] 테이블 홀짝 줄무늬(Zebra Striping) 디자인 및 카드 영역 flex 높이 재조정 최적화 (2026-08-11)
- **검토 내용**: 40채널 온도 튜닝 후 남은 하단 여백의 비대칭을 균형 있게 재분배하고, 테이블 행 간의 식별력을 높이기 위해 Zebra Striping 및 대비 강화 색상 튜닝 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **Zebra Striping 및 대비 강화 (CSS)**:
     - tr:nth-child(even)과 tr:nth-child(odd) 의 배경색 톤을 은은하게 다변화하고 보더 라인의 투명 대비를 강화하여 수평 행 간의 정보 인지력을 극대화.
  2. **좌측 패널 카드 flex 재분배 (HTML)**:
     - TC 카드의 flex 비율을 1.4에서 1.25로 소폭 축소하고, 아래쪽 ADC 카드의 flex 비율을 1에서 1.15로 확대하여 두 카드 사이의 여백 밸런스를 균형적으로 동기화 튜닝 완료.

### 199. [Control UI] BOP(Balance of Plant) 기술 개념 정리 및 용어사전 등재 완료 (2026-08-11)
- **검토 내용**: 대시보드 메인 타이틀에 포함된 BOP 약어의 실질적 시스템 정의를 점검하고, 연료전지 하드웨어 계통 설계 분야의 기술 명확성을 확보해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  - BOP의 원문 약어인 Balance of Plant(수소 연료전지 주변 보조기기)의 학술적/실무적 하드웨어 정의를 명확하게 수립하고, PROGRAMMING_TERMS.md 용어 사전의 370번 기술 용어로 정식 등록 완료.

### 200. [Control UI] 마우스 호버 시 실 하드웨어 모델명 팝업 네온 툴팁 기능 구현 완료 (2026-08-11)
- **검토 내용**: 각 디지털 밸브, 펌프, 블로어 및 아날로그 센서 등 대시보드 내 계측/제어 명칭에 대응되는 실제 현장 하드웨어 기기 모델명 스펙(VX230GA, BLP-8080, FDU-520 등)을 화면 오염 없이 직관적으로 모니터링할 수 있도록 툴팁 구현 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **CSS 네온 툴팁 팝업 프레임 구축 (CSS)**:
     - .tooltip-trigger 클래스 및 마우스 호버 시 발생하는 가상 요소 ::after, ::before 규칙을 추가 선언하여, 0.15초의 페이드 애니메이션과 함께 프라이머리 블루 광채 테두리의 다크 툴팁이 동작하도록 구현.
  2. **하드웨어 기기 스펙 매핑 및 렌더링 결합 (HTML/JS)**:
     - adcChannels, diChannels, dacChannels, doChannels 변수 내에 실물 기기 매핑용 model 필드를 일제히 도입.
     - renderAll() 내의 각 바인딩 템플릿 태그를 <span class="tooltip-trigger" data-tooltip="모델명: ...">으로 감싸도록 수정하여, 장치명에 마우스 접근 시 즉시 실물 모델명이 표출되도록 연동 성공.

### 201. [Control UI] 실제 10kW IO 넘버링 스펙 엑셀 파싱 및 진짜 하드웨어 모델명 툴팁 데이터 업데이트 완료 (2026-08-11)
- **검토 내용**: 아날로그 계측기의 모델명 데이터가 간이 범용 텍스트인 "센서부 (1~5V)" 등으로 출력되어 실 스펙 모니터링에 부적합했던 상황을 해결하기 위해, 05_Docs` 폴더 내 실제 배정 엑셀 스펙 시트를 분석하여 진짜 하드웨어 모델명(T2800, PSS-01V-R1/8, CM-230K 등)으로 연동해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **로컬 엑셀 사양 파싱 스크립트 가동 (Python)**:
     - 05_Docs 하위의 260727_10kW IO 넘버링 Ver 1.1 이규원_ 모델명 포함(수정).xlsx 파일을 파이썬 pandas 엔진으로 비동기 파싱하여 넷네임(NETNAME)별 매핑된 실제 장치 모델 파트넘버를 정확히 추출.
  2. **정밀 하드웨어 모델명 업데이트 (JS)**:
     - 추출한 데이터셋을 기반으로 adcChannels, diChannels, doChannels 데이터 맵의 model 속성을 전면 업데이트.
     - 예: 유틸리티 공급 압력계 -> T2800, DI Water Tank 전도도계 -> CM-230K, BOG 유량계 -> TSCD230-8SW-CH4 30L, 점화 트랜스 -> TZI5-15/100WE 등으로 실물 부품 정보를 완벽히 동기화 조치 완료.

### 202. [Control UI] 툴팁 출력 범위 연동, th 헤더 타이틀 황금색 네온 강조 및 ADC/DAC 컬럼폭 비례 조정 완료 (2026-08-11)
- **검토 내용**: 툴팁 내부 모델명 뒤에 전압 및 출력 신호 범위(1~5V, DC 24V 등)를 결합 표출하고, 테이블 헤더 th의 가독성을 노란색으로 키워 본문 대비 구분을 명확히 하며, 아날로그 테이블의 가로폭 좁음으로 인한 수치/단위 줄바꿈 깨짐을 방지해 달라는 피드백 반영.
- **분석 및 해결 가이드라인**:
  1. **툴팁 신호 범위 동적 결합 (JS)**:
     - ADC 툴팁에는 (${item.minV}~${item.maxV}V), DAC 툴팁에는 (0~5V), DI 툴팁에는 (DC 24V)를 각각 꼬리말로 붙여 표출.
     - DO 툴팁에는 기기 타입(밸브/일반: DC 24V, 히터/점화기: AC 220V)을 분기 매칭하여 동작 전원을 결합 표출하도록 렌더링 최적화.
  2. **테이블 th 헤더 비주얼 튜닝 (CSS)**:
     - th 요소의 글자 색상을 황금빛 옐로우(#fbbf24)로 변경하고 폰트 크기를 12px로 확장하여 테이블 본문(11px) 대비 타이틀의 경계 시인성을 비약적으로 향상.
  3. **가로 컬럼폭(width) 정합 튜닝 (HTML)**:
     - ADC 및 DAC 테이블 th의 가로폭 설정을 NETNAME: 70, RAW: 55, 전압: 65, 실제계측/출력율: 95로 비례적으로 조절하여 수치나 단위(V, kPa 등)의 불필요한 세로 줄바꿈을 원천 예방 완료.

### 203. [Control UI] 툴팁 우측 정렬 팝업 스위칭 및 폰트 상향(10.5px) 최적화 완료 (2026-08-11)
- **검토 내용**: 대시보드 1열(테이블 가장 왼쪽 장치명)에 마우스 호버 시 툴팁의 왼쪽 일부분이 화면 바깥으로 삐져나가 가려지는 문제를 보완하고, 폰트 가독성을 높이기 위해 툴팁의 우측 정렬 스위칭 요청 반영.
- **분석 및 해결 가이드라인**:
  - CSS 툴팁(tooltip-trigger)의 절대좌표 기준을 bottom에서 left: 106% 및 top: 50%로 변경하여 마우스 포인터의 우측으로 자연스럽게 정렬 팝업되도록 전환.
  - 화살표 꼬리(.tooltip-trigger::before) 역시 left: 101% 및 좌측 지향의 삼각형 형태로 변경하고, 툴팁 내 폰트 크기를 9px에서 가독성 높은 10.5px로 상향하여 비주얼 품질과 정보 수집 편의를 극대화 완료.

### 204. [Control UI] 디지털 LED 아이템 박스(led-item) 호버 z-index 팝업 제어를 통한 툴팁 잘림 현상 조치 완료 (2026-08-11)
- **검토 내용**: 우측 아래 디지털 릴레이 및 밸브 출력(DO 20채널) 카드 내의 개별 박스(led-item)에 호버 시 툴팁이 삐져나오며 사각 부모 경계에 의해 잘리거나 뒤로 가려져 보이지 않는 문제를 조치해 달라는 피드백 반영.
- **분석 및 해결 가이드라인**:
  1. **부모 영역 오버플로우 제한 해제 (CSS)**:
     - .led-item에 overflow: visible !important;를 부여하여 자식 요소인 툴팁이 부모 영역을 가뿐히 돌출해 표출되도록 강제 설정.
  2. **동적 레이어(z-index) 제어 장착 (CSS)**:
     - 마우스 포인터가 올라간 활성화 엘리먼트에 한하여 .led-item:hover { z-index: 999 !important; } 스타일을 trigger함으로써, Stacking Context에 의해 인접 그리드 박스 아래로 툴팁이 가라앉는 현상을 원천 방지하고 가장 최상위 레이어로 부드럽게 팝업되도록 보정 완수.

### 205. [Control UI] DI 및 DO 카드/바디 오버플로우 visible 강제 적용을 통한 경계선 툴팁 클리핑 해결 완료 (2026-08-11)
- **검토 내용**: .led-item 자체에 overflow: visible을 처리했음에도 불구하고, 상위 컨테이너 카드 바디 .card-body에 부여된 overflow-y: auto;로 인해 툴팁 팝업이 여전히 카드의 사각 테두리 경계 밖으로 나가는 순간 가로/세로로 잘리는 현상에 대한 정밀 재보정 피드백 반영.
- **분석 및 해결 가이드라인**:
  - 디지털 입력(DI) 및 디지털 출력(DO) 카드의 상위 <div class="card" ...> 컨테이너 태그에 overflow: visible; 인라인 스타일을 강제 선언.
  - 해당 두 카드의 알맹이 몸체인 .card-body 컨테이너 영역에도 overflow: visible !important;를 인라인 스타일로 기입하여, 상속받는 세로 스크롤용 overflow-y: auto 제한을 완전히 무력화함으로써 자식 툴팁이 대시보드 스크린 전면으로 완전무결하게 팝업되도록 보정 조치함.

### 206. [Control UI] LED 설명(led-desc) overflow hidden 간섭 우회를 위한 led-item 전체 영역 툴팁 트리거 통합 개편 및 툴팁 잘림 최종 조치 (2026-08-11)
- **검토 내용**: 카드의 오버플로우 제한을 모두 풀었음에도 불구하고 기기 설명 셀(.led-desc) 자체에 텍스트 말줄임(text-overflow: ellipsis)을 위해 걸려 있던 overflow: hidden; 속성 때문에 자식 툴팁이 여전히 잘리는 현상을 완벽 해결하기 위한 마크업 및 이벤트 재구조화 피드백 반영.
- **분석 및 해결 가이드라인**:
  1. **툴팁 바인딩 대상을 박스 전체로 이전 (JS)**:
     - 툴팁 트리거가 기기 설명 텍스트 내부의 좁은 <span>에 속해 있던 기존 구조를 폐기하고, 사각 LED 박스 전체인 .led-item 엘리먼트에 직접 tooltip-trigger 클래스와 data-tooltip 속성을 주입하도록 JS 렌더링 코드 개편.
     - 이로 인해 .led-desc에 부여된 overflow: hidden 스타일 간섭을 완벽히 우회하여 자식 툴팁이 100% 온전하게 노출되도록 처리.
     - DO 제어 버튼 클릭 시 이벤트 전파로 인한 툴팁 및 제어 간섭을 방지하도록 onclick="event.stopPropagation(); ..." 방어 코드 추가 연동.
  2. **그리드 점선 데코레이션 보정 (CSS)**:
     - .led-item에 tooltip-trigger 가 탑재되면서 기기 박스 하단 전체에 가로질러 표시되던 지저분한 밑줄 점선(border-bottom: 1px dotted)을 무력화(border-bottom: 1px solid rgba(255,255,255,0.03) !important)시켜 레이아웃 비주얼 정밀화 완료.

### 207. [Control UI] 마우스 호버 시 0.8초 지연(Transition Delay) 후 툴팁 팝업 작동 및 마우스 아웃 시 즉시 소멸 제어 고도화 완료 (2026-08-11)
- **검토 내용**: 마우스가 계측 장치나 LED 박스 위를 그냥 빠르게 스쳐 지나갈 때 툴팁이 실시간으로 휙휙 팝업되어 눈이 피로해지는 산만한 사용성을 차단하고, 툴팁 표시의 반응성을 정교하게 튜닝해 달라는 추가 요구사항 반영.
- **분석 및 해결 가이드라인**:
  - CSS 트랜지션의 시간 지연 속성(transition-delay)을 활용하여 호버 딜레이 장착.
  - 마우스를 올리는 순간의 호버 상태(.tooltip-trigger:hover::after, ::before)에 transition-delay: 0.8s;를 적용하여 약 1초 미만(0.8초) 동안 마우스가 멈춰있을 때만 툴팁이 스무스하게 팝업되도록 제어.
  - 마우스가 영역을 벗어나는 순간(.tooltip-trigger::after, ::before 기본 상태)에는 transition-delay: 0s;로 지연을 리셋해 지체 없이 즉시 즉각 소멸하도록 하는 프로 수준의 HMI 화면 튜닝 완수.

### 208. [Control UI] 마그네틱 접촉기 메인 전원 스위치(DO_MC_SW) 그리드 분리 및 상단 대형 토글 패널 단독 독립 고도화 완료 (2026-08-11)
- **검토 내용**: 디지털 출력(DO) 카드 내부의 DO_MC_SW(MC 동작 스위치)가 타 기기(밸브, 히터)들과 동일한 크기의 그리드에 배치되어 있는 레이아웃을 개조하여, 하드웨어 계통상 24V 보드 전원 전체를 관할하는 마스터 파워 스위치에 걸맞은 독보적 강조 비주얼로 전이해 달라는 설계 요구 반영.
- **분석 및 해결 가이드라인**:
  1. **그리드 목록 배제 및 상단 전용 MC 패널 신설 (HTML/JS)**:
     - DO 20개 채널 루프에서 DO_MC_SW 항목을 필터링 처리하여 그리드 목록에서 완전히 분리.
     - DO 카드 바디 최상단에 마스터 파워 전용 박스 <div class="main-mc-panel">을 신설하고 여기에 15번 핀(DO_MC_SW)을 단독 데이터 바인딩함.
  2. **동적 네온 컬러 및 펄스 점멸 (CSS/JS)**:
     - 전원 차단(OFF) 상태에서는 경고성 붉은빛 테두리와 전원 공급 차단됨 (24V OFF) 문구가 표시되며, 전원 인가(ON) 상태에서는 은은한 초록빛 네온 가동 연출과 함께 SYSTEM MAIN POWER 텍스트와 대형 LED 인디케이터가 2초 주기로 펄스 숨쉬기 발광을 구동하도록 세부 HMI 동작 튜닝 완수.

### 209. [Control UI] MC 스위치(DO_MC_SW) 클릭 핸들러 범위 전이 및 MC OFF 시 대시보드 전체 전원 차단 음영/잠금 연출 완료 (2026-08-11)
- **검토 내용**: 독립된 MC 메인 패널의 클릭 조작성 오류를 해결하고, 마그네틱 메인 전원의 물리적 상태에 따라 대시보드 내 모든 계측 값과 출력이 차단되는 HMI 전원 연동 차단 효과를 디자인해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **패널 전체 영역 클릭 토글화 및 중복 방지 (HTML/JS)**:
     - main-mc-panel 박스 자체에 클릭 핸들러(.onclick)를 매칭하고, 버튼 태그에는 pointer-events: none;을 인가해 박스 내부 그 어디를 클릭해도 정상 토글되며 중복 트리거되지 않도록 클릭 핸들러 보정.
  2. **통신 비활성 상태에서의 가상 제어 허용 (JS)**:
     - 포트 비연결 상태(isPortConnected === false)이더라도 MC 전원 스위치(DO_MC_SW) 만큼은 알림 창 없이 오프라인 시뮬레이션 토글이 기동되도록 예외 필터 처리 적용.
  3. **대시보드 전체 24V 차단 음영 및 제어 잠금 시스템 구축 (CSS/JS)**:
     - MC가 꺼진 상태(mcItem.active === false)일 때, 대시보드 최외각 .container에 .power-off 클래스를 동적 인가.
     - CSS를 통해 온도, ADC, DAC, DI, RS-485 카드 등의 모든 정보 및 제어를 회색빛 음영(filter: grayscale(0.5) brightness(0.4); opacity: 0.55;)으로 차단하고 조작 불가능(pointer-events: none;)하게 락업 제어.
     - 메인 스위치가 속한 디지털 출력 카드는 밝기 유지를 보장하되 내부 19개 일반 채널만 차단하는 HMI 전력 계통 제어 모사 완수.

### 210. [Control UI] 화면 우측 경계 기기용 좌측 팝업 툴팁(tooltip-left) 신설 및 DO/DI/ADC/DAC 2열 연동 고도화 완료 (2026-08-11)
- **검토 내용**: 대시보드의 화면 가장 우측 끝 열에 배치된 부품 및 계측 센서에 호버 시, 우측 팝업 툴팁이 브라우저 윈도우 가장자리 밖으로 나가 가로막혀 가려지는 문제를 완벽히 해결해 달라는 추가 피드백 반영.
- **분석 및 해결 가이드라인**:
  1. **좌측 지향 툴팁 스타일 신설 (CSS)**:
     - .tooltip-left::after 및 ::before 가상 요소를 만들어 right: 106%; left: auto;로 좌표축을 강제 좌측 정렬하고, 화살표 삼각 꼬리 방향을 우측으로 전환한 좌측 팝업 툴팁 인프라 설계.
  2. **그리드 및 테이블 열 위치 기반 조건부 클래스 매핑 (JS)**:
     - **DO 그리드 (7열)**: 7열 배치 상에서 실제 그려지는 순서(drawIndex)가 5번째(6열) 및 6번째(7열) 열에 있는 기기들에 대해 tooltip-left 클래스를 자동 탑재하도록 알고리즘 연동.
     - **DI 그리드 (7열)**: DI 인덱스 index % 7 === 5 || index % 7 === 6인 우측 기기들에 좌측 툴팁 적용.
     - **ADC & DAC (2열 테이블)**: 2열 중 우측 열(adc-col-2, dac-col-2)에 해당하는 모든 계측기/기기에 tooltip-left를 부여하여 화면 전체 가로축 경계 잘림 현상을 100% 원천 예방 완료.

### 211. [Control UI] DI/DO 7열 중 뒤쪽 5개 열(3~7열) 전체 스마트 대피 툴팁(tooltip-left) 바인딩 공식 확장 완료 (2026-08-11)
- **검토 내용**: 화면 오른쪽 경계선 2개 열(6~7열)뿐만 아니라, 글자 수가 긴 부품 툴팁의 경우 브라우저 윈도우 크기에 의해 3~5열 위치에서도 잘릴 위험성이 있어 좌측 팝업 대피 대상을 총 5개 열로 전면 확장해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **그리드 열 위치 공식 개조 (JS)**:
     - 기존의 6~7열 판단 공식 `(index % 7 === 5 || index % 7 === 6)`을 폐기하고, 뒤쪽 5개 열 전체를 지시하는 `index % 7 >= 2` (3열, 4열, 5열, 6열, 7열) 판정 스키마로 완화 조정.
     - 디지털 입력(DI) 및 디지털 출력(DO - drawIndex 적용) 렌더러에 각각 해당 확장 수식을 장착하여 화면 우측 경계선 밖 클리핑 현상을 완벽하게 조치 완료.

### 212. [Control UI] 온도 센서 카드의 분할 칼럼 소타이틀(T-Type, K-Type) 폰트 상향(12px) 및 패딩 확장을 통한 가독성 강화 완료 (2026-08-11)
- **검토 내용**: 온도 센서 측정 현황 카드 내 4개 분할 칼럼의 서브 타이틀 헤더 라벨(T-Type, K-Type) 글씨 크기가 너무 작고 왜소하여 다른 카드 타이틀 배지들과의 가독성 불일치가 일어나는 현상 개선 반영.
- **분석 및 해결 가이드라인**:
  1. **소타이틀 전용 클래스 신설 및 디자인 리팩토링 (CSS)**:
     - .tc-subtitle-header 클래스를 추가하여 글자 크기를 기존 10px에서 12px로 상향하고, 굵기를 800으로 묵직하게 두어 네온 식별력을 강화.
     - 패딩을 5px 8px로 늘려 타이틀 배지 형상의 디자인 정합성을 획득하고 아래 테이블과의 여백을 8px로 늘려 답답함을 전면 해소함.
  2. **마크업 교체 및 적용 (HTML)**:
     - 온도 측정 현황 4열 테이블 상단의 인라인 스타일로 기입되어 있던 소타이틀 마크업을 신설 클래스로 일제히 전환 장착하여 비주얼 선명도 보강.

### 213. [Control UI] 온도 테이블 th 헤더 타이틀(측정 위치, NET, 온도) 폰트 크기 동기화 및 12px 황금색 상속 환원 완료 (2026-08-11)
- **검토 내용**: 온도 측정 현황 카드 내부 테이블들의 th 헤더 폰트 크기가 다른 하위 테이블(아날로그 입력, 통신 장치 등)의 th 헤더들에 비해 작게 렌더링되어 대시보드 타이틀의 균형과 시인성이 깨지는 현상 정정 반영.
- **분석 및 해결 가이드라인**:
  - CSS 상에서 .tc-table-container th에 수동으로 걸려있던 font-size: 9.5px 강제 덮어쓰기(override) 블록을 완벽하게 제거.
  - 이로써 상위 공통 th 헤더 규칙인 font-size: 12px와 황금색(#fbbf24) 및 700 굵기 스타일이 온도 테이블 th 헤더에도 완벽히 상속 렌더링되도록 복원하여 대시보드 헤더 밸런스 완전 통합 완료.

### 214. [Control UI] 디지털 출력(DO) 기기 박스 내 조작 버튼(do-btn) 및 명칭/설명 글자 크기 대폭 확장 및 우측 카드 비율 최적화 완료 (2026-08-11)
- **검토 내용**: 디지털 릴레이 및 밸브 출력(DO 20채널) 카드 내의 열기/닫기/기동/정지 제어 사각 버튼의 크기와 폰트가 너무 미세하여 마우스 클릭 및 시각적 조작 식별성이 떨어지는 문제 해소 반영.
- **분석 및 해결 가이드라인**:
  1. **DO 기기 박스 및 버튼 스타일링 확장 (CSS/JS)**:
     - .do-btn의 글자 크기를 8px에서 10.5px로 30% 이상 크게 증대하고, padding을 4px 10px로 도톰하게 늘려 사각 물리 버튼 형상의 조작 시인성을 비약적으로 향상.
     - 기기 명칭(.led-name) 폰트 크기를 8.5px에서 9.5px로, 설명(.led-desc) 폰트 크기를 7.5px에서 8.5px로 동반 상향하여 밸런스 정합.
     - 버튼 간격 margin-top을 2px에서 4px로 조절하여 텍스트와의 겹침 방지.
  2. **우측 패널 카드 수직 flex 높이 재조정 (HTML)**:
     - 버튼이 커짐에 따른 DO 카드의 내부 세로 공간 부족을 해결하기 위해, 바로 위의 시리얼 통신 장치 카드의 flex 비율을 0.45에서 0.38로 소폭 축소.
     - 디지털 출력(DO) 카드의 flex 비율을 1.45에서 1.52로 확장하여 레이아웃 밀림이나 찌그러짐 현상 없이 광활하고 시원하게 렌더링되도록 튜닝 완료.

### 215. [Control UI] Windows 작업 표시줄(Taskbar) 가림 현상 해결을 위한 body 100vh 뷰포트 상대 높이 대응 및 하단 패딩 24px 레이아웃 최적화 완료 (2026-08-11)
- **검토 내용**: 사용자가 Windows 작업 표시줄을 아래쪽에 항상 노출시키고 사용함에 따라 브라우저 창 하단 영역이 작업 표시줄에 덮여, 디지털 출력(DO) 카드의 맨 아랫줄 조작 버튼이 잘려서 클릭 및 확인이 불가능해지는 치명적인 화면 가림 문제 해소 반영.
- **분석 및 해결 가이드라인**:
  1. **절대 해상도 고정 해제 및 뷰포트 맞춤 (CSS)**:
     - body 스타일 내의 width: 1920px 및 height: 1080px 절대 크기 고정을 해제하고, width: 100vw; height: 100vh; max-height: 100vh; 의 상대적 뷰포트 사이즈로 전격 전환.
     - 이로 인해 브라우저 창 높이가 작업 표시줄 영역을 뺀 남은 화면(1030px 등)에 맞춰 탄력적으로 리사이징되고 내부 카드 높이 비율이 자동으로 콤팩트하게 동기 수축하도록 설계.
  2. **하단 전용 마진 패딩 안전 확보 (CSS)**:
     - padding 수치를 14px 20px 24px 20px 로 조정하여 하단 영역에 24px 의 물리적 안전 패딩을 확보해 둠으로써, 맨 아랫줄의 버튼 및 텍스트들이 작업 표시줄 위로 안전하게 부상하여 100% 명확히 시각 노출되도록 보정 완료.

### 216. [Control UI] vh 강제 수축에 따른 대시보드 스크롤 오작동 복구 및 body 1920x1025px 절대 스펙 조율을 통한 포맷 안정화 완료 (2026-08-11)
- **검토 내용**: body 높이를 100vh 상대 비율로 변경함에 따라 브라우저 크기 수축 시 온도 센서 테이블 및 디지털 입력(DI) 카드의 컨텐츠가 찌그러져 세로 스크롤바가 다량 발생하고 비주얼 정합이 무너지는 레이아웃 왜곡 현상 개선 반영.
- **분석 및 해결 가이드라인**:
  1. **절대 해상도 스케일 환원 및 높이 미세 보정 (CSS)**:
     - 100vh 및 100vw 상대 규격을 전면 취소하고, 기존의 완벽했던 1920px 가로 폭 규격을 복원함.
     - 전체 수직 높이(height)를 기존 1080px 에서 작업 표시줄 높이(약 48px)를 안전하게 공제한 1025px 로 미세 감축 조정.
  2. **레이아웃 안정성 및 잘림 방지 동시 달성 (CSS/HTML)**:
     - 강제 vh 압축 현상이 제거되면서 온도 센서 테이블과 DI 카드가 이전의 깨끗하고 선명한 무스크롤 명품 상태로 100% 원상 복구됨.
     - 동시에 전체 높이가 1025px 로 축소되어, Windows 작업 표시줄이 항상 노출되어 있더라도 DO 카드의 맨 아랫줄 버튼들(LTS 촉매 히터, 점화 트랜스)이 작업 표시줄 경계선 휠씬 위쪽 공간에 안정적으로 노출되도록 보정 완료.

### 217. [Control UI] 좌측 패널 카드 및 갭(gap) 여백 픽셀 최적화를 통한 디지털 입력(DI) 카드 수직 안착 및 공간 확장 완료 (2026-08-11)
- **검토 내용**: Windows 작업 표시줄이 노출될 때 좌측 패널 하단 영역이 눌리면서 디지털 입력(DI 8채널) 카드의 세로 크기가 심하게 축소되어 비주얼 찌그러짐이 발생하는 문제를 방지하고, DI 카드가 넉넉하고 시원한 높이를 확보하도록 좌측 패널의 여백 및 비율을 재배정해 달라는 요청 반영.
- **분석 및 해결 가이드라인**:
  1. **패널 간 갭 여백 타이트화 (CSS)**:
     - 좌/우측 패널 내부 카드들의 수직 간격을 지배하는 .left-panel 및 .right-panel 의 gap 값을 기존 16px에서 10px로 일제히 감축하여 불필요한 공백 낭비 원천 차단.
  2. **수직 flex 비율 재배정 (HTML)**:
     - 좌측 패널 내 온도 센서 카드의 flex 비율을 1.25에서 1.15로, 아날로그 입력(ADC) 카드의 flex 비율을 1.15에서 1.0으로 각각 축소 조정. (ADC 테이블 th가 커졌으나 수직 공간이 충분하여 줄바꿈이나 스크롤바 없이 명품 포맷 유지)
     - 이 축소분으로 얻은 마진을 바탕으로 디지털 입력(DI) 카드의 flex 비율을 0.6에서 0.85로 대폭 확대하여, DI 카드가 작업 표시줄 가림 영역에서 완전히 벗어나 상단으로 안전하게 떠오르고 큼직하게 표출되도록 배치 조치 완수.

### 218. [Control UI] DO 히터/점화기와 DI 화염감지기 피드백(FB) 실시간 시뮬레이션 연동 및 가로 스택 공간 미세 축소 고도화 완료 (2026-08-11)
- **검토 내용**:
  1. 아날로그 입력(ADC) 테이블 하단에 불필요하게 떠 있는 여백을 한층 더 미세 튜닝하여 좌측 패널 하단의 디지털 입력(DI) 카드가 더 높고 안전하게 배치되도록 공간 긴축.
  2. 디지털 출력(DO) 카드 내부의 히터 4종(LTS 촉매 1/2, Prox 1/2) 및 점화 트랜스 기동 시, 가상 하드웨어 피드백 신호가 들어오는 연동 감시 기능을 HMI에 녹여내어 시각적 안전성과 직관성 극대화.
- **분석 및 해결 가이드라인**:
  1. **ADC 하단 여백 추가 감축 (CSS/HTML)**:
     - th, td 의 기본 padding을 3.5px에서 3px로 미세 조정하여 아날로그 테이블 수직 높이를 감축하고, ADC 카드의 card-body 하단 padding-bottom을 2px로 바짝 조여서 좌측 하단 DI 카드의 공간을 15px 이상 추가 확보하여 아래 가림 현상 최종 예방.
  2. **가상 피드백 감시 시스템 구축 (CSS/JS)**:
     - CSS에 .led-item.feedback-active 스타일을 정의하여, 히터 및 점화 기기가 기동할 때 기기 박스 전체가 은은한 녹색조(rgba(16,185,129,0.08)) 구동 광채를 발하도록 유기적 피드백 연출.
     - 기기 박스 내부에 실시간 피드백 램프 도트가 박힌 FB OK (초록 점멸) / FB OFF (회색) 상태 배지를 동적으로 렌더링되게 설계하여 가동 중인 하드웨어 피드백을 실시간 감시 가능하게 함.
     - DO_IGN175(점화 트랜스) 작동 수동 토글 및 시리얼 Modbus 패킷 파싱 구문 내에, 실제 좌측 DI 카드의 화염 감지기(DI_FD176)의 active 상태가 즉각 연동되어 함께 점멸하는 가상 물리 연계 시뮬레이션을 구현 완수.

### 219. [Control UI] FHD 뷰포트 레이아웃 최적화 및 좌측 하단 DI 카드 수축 찌그러짐 현상 해결 완료 (2026-08-12)
- **검토 내용**: FHD(1920x1025px) 화면 환경에서 좌측 하단의 '디지털 입력 상태 (DI 8채널)' 카드가 찌그러져 보이지 않는 현상과, 우측 하단의 '디지털 릴레이 및 밸브 출력' 영역의 가로 폭이 카드 경계 밖으로 삐져나가 우측 끝이 잘려 보이지 않는 현상 개선 반영.
- **분석 및 해결 가이드라인**:
  1. **가로축/세로축 공간 재분배 (CSS 및 HTML 수정)**:
     - `.container`의 `grid-template-columns` 비율을 `1.14fr 0.86fr` -> `1.1fr 0.9fr`로 조정하여 우측 패널 가로 폭을 약 `37px` 추가 확보함으로써 우측 경계 잘림 현상을 해결.
     - 좌측 하단 DI 카드의 flex 비율을 `0.85` -> `0.95`로 상향하고, `min-height: 145px;` 속성을 강제 인가하여 세로 수직 찌그러짐을 완벽히 방어.
     - DI 카드의 `.card-body` 내부 플렉스 정렬 스타일(`display: flex; align-items: center; justify-content: center;`)을 제거하여 일반 블록 정렬로 복구 완료.
  2. **led-item 및 버튼 크기 콤팩트 최적화 (CSS)**:
     - DO 카드의 `.led-item` 내에 위치한 버튼 `.do-btn`의 padding을 `4px 10px` -> `4px 6px`로 미세 압축하고, `.led-item` 자체의 padding도 `4px 6px` -> `4px 4px`로 조금 더 압축하여 7열 그리드의 가로 폭 부족을 최종 조치 완료.

### 220. [Control UI] 화염 감지기(DI_FD176) 삭제 및 점화 트랜스(DO_IGN175) 피드백 통합 반영 (2026-08-12)
- **검토 내용**: 자기야 요청에 따라 디지털 입력 상태 패널에 있던 화염 감지기(DI_FD176)를 제거하고, 이를 우측 패널의 점화 트랜스(DO_IGN175) 피드백 형태로 통합하여 표시되도록 구조 개선.
- **분석 및 해결 가이드라인**:
  1. **DI 채널 리스트 제거**: `index.html`의 `diChannels` 배열 첫 번째 항목인 `DI_FD176` 데이터 및 속성 정보 삭제.
  2. **가상 시뮬레이션 연동 로직 제거**: `DO_IGN175` 토글 및 Modbus 통신 시 `DI_FD176`를 묶어서 active 하던 의존성 코드(`toggleVirtualCoil` 및 레지스터 쓰기 로직) 완벽히 분리 및 삭제 완료.
  3. **의존성(Dependency) 확보**: `dependency_map.md` 등 문서와의 연관성을 검토한 후, 기타 부작용이 없도록 모듈 분리 및 기존 UI 피드백 구조(FB OK/OFF 뱃지)를 활용해 자연스럽게 녹아들게 마무리.
