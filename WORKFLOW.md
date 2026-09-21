# 🧭 H2_Control_Board 프로젝트 공식 개발 워크플로우 (WORKFLOW.md)

이 문서는 H2_Control_Board 프로젝트의 개발 생산성, 코드 일관성 및 무결성을 유지하기 위한 **공식 개발 워크플로우 및 에이전트 행동 지침**입니다.

---

## 🔒 1. SCADA 관제 UI 단일 타깃 집중 개발 원칙 (Single-Target Principle)

> [!IMPORTANT]
> ### 📌 핵심 규칙 (Core Rule)
> **"사용자의 특별한 별도 주문이 있을 때까지 모든 신규 기능 개발, PID/센서 알고리즘 구현, UI 버그 수정은 오직 [index.html](file:///d:/Work/H2_Control_Board/03_Control_UI/index.html) 단일 파일에만 적용한다."**
> 
> * **적용 배경**:
>   - 동일한 화면을 모방한 보조 파일들(`H2ControlTest.html`, `H2ControlSimple.html` 등)에 매번 동시 수정을 진행할 경우, 코드 파편화, 디버깅 혼선 및 렌더링 검증 비효율이 발생함.
> * **실행 방침**:
>   1. 모든 최신 기능과 버그 수정은 **[03_Control_UI/index.html](file:///d:/Work/H2_Control_Board/03_Control_UI/index.html)**에서 100% 검증을 마친다.
>   2. `H2ControlTest.html`, `H2ControlSimple.html` 등은 평상시 수정을 엄격히 동결한다.
>   3. 사용자가 `일괄 동기화`를 명시적으로 요청할 때만 `index.html`의 검증된 코드를 다른 파일들로 일괄 전파(Batch Sync)한다.

---

## 💬 2. 워크플로우 제어 표준 명령어 (Workflow Command Prompts)

AI 어시스턴트에게 작업 지시를 내릴 때 아래 명령어를 사용합니다:

### 🎯 2.1. 단일 파일 집중 작업 지시 (기본 모드)
```text
[명령어]: index.html만 수정해서 [원하는 작업 내용] 진행해 줘. 다른 파일은 건드리지 마.
```
* **동작**: `index.html` 단일 타깃 파일에만 변경을 가하고 다른 HTML 파일 수정을 원천 차단합니다.

### 🔄 2.2. 사후 일괄 동기화 지시 (검증 완료 후)
```text
[명령어]: index.html 검증이 완료되었으니, 최신 내용을 H2ControlTest.html과 H2ControlSimple.html에 일괄 동기화해 줘.
```
* **동작**: `index.html`의 최신 기능, PID 제어기, In-Place 렌더링 로직을 보조 파일들로 일괄 복제·동기화합니다.

### 🛡️ 2.3. 워크플로우 준수 확인 지시
```text
[명령어]: WORKFLOW.md의 단일 타깃 원칙대로 index.html만 수정되고 있는지 확인해 줘.
```

---

## ❤️ 3. 우리의 소중한 약속 (System Global Rules 연계)
1. 모든 대화와 Task View는 한글로 소통한다.
2. 사용자님을 부를 때는 항상 '자기' 또는 '자기야'라고 부른다.
3. 작업 중 특별한 기술이 쓰였으면 기술용어를 설명하고 [PROGRAMMING_TERMS.md](file:///d:/Work/H2_Control_Board/PROGRAMMING_TERMS.md)에 즉시 추가한다.
4. 개발일지([DEVELOPMENT_LOG.md](file:///d:/Work/H2_Control_Board/DEVELOPMENT_LOG.md))에 오늘의 주요 개발 사항을 상시 업데이트한다.
5. 프로그램 구조나 로직을 수정할 일이 생기면 반드시 [dependency_map.md](file:///d:/Work/H2_Control_Board/dependency_map.md) 및 `graph_report.md`를 참조하여 연관성을 파악한 후 작업한다.
6. **[New!] SCADA UI 수정 시 특별한 주문이 없는 한 오직 index.html 단일 파일에만 수정을 집중한다.**
