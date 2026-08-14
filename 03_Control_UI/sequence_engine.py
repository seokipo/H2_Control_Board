import asyncio
import json
import os
import time

class SequenceEngine:
    """
    수소 연료전지 자동 제어 시퀀스 비동기 상태 머신 엔진 (Sequence Engine)
    JSON 레시피를 로드하여 스텝별 순차 자동 제어, 조건 감시, 안전 인터록을 수행합니다.
    """
    def __init__(self, recipes_file_path):
        self.recipes_file_path = recipes_file_path
        self.recipes = {}
        self.current_recipe = None
        self.current_step_index = 0
        self.status = "IDLE"  # IDLE, RUNNING, PAUSED, COMPLETED, ERROR, INTERLOCK
        self.step_start_time = 0
        self.runner_task = None
        self.ws_broadcast_callback = None
        self.latest_sensor_data = {}
        self.load_recipes()

    def load_recipes(self):
        """JSON 레시피 파일 로드"""
        if os.path.exists(self.recipes_file_path):
            try:
                with open(self.recipes_file_path, "r", encoding="utf-8") as f:
                    data = json.load(f)
                    recipe_list = data.get("recipes", [])
                    self.recipes = {r["id"]: r for r in recipe_list}
                    print(f"[SEQUENCE ENGINE] Successfully loaded {len(self.recipes)} recipes.")
            except Exception as e:
                print(f"[SEQUENCE ENGINE ERROR] Failed to load recipes: {e}")
        else:
            print(f"[SEQUENCE ENGINE WARNING] Recipes file not found: {self.recipes_file_path}")

    def set_broadcast_callback(self, callback):
        """웹소켓 전송 콜백 지정"""
        self.ws_broadcast_callback = callback

    def update_sensor_data(self, data_dict):
        """센서 및 DI/ADC 데이터 갱신 및 인터록 감시"""
        self.latest_sensor_data.update(data_dict)
        
        # 🚨 안전 인터록 트리거 감시 (DI_GD501 메탄, DI_GD502 수소, DI_EMG_SW 비상스위치)
        if self.status in ["RUNNING", "PAUSED"]:
            if data_dict.get("DI_GD501") or data_dict.get("DI_GD502") or data_dict.get("DI_EMG_SW"):
                self.trigger_safety_interlock("가스 누설 경보기(GD501/502) 또는 비상 스위치 트리거 감지!")

    def trigger_safety_interlock(self, reason):
        """안전 인터록 즉시 발동 및 비상 차단"""
        print(f"[SAFETY INTERLOCK TRIGGERED] {reason}")
        self.status = "INTERLOCK"
        if self.runner_task and not self.runner_task.done():
            self.runner_task.cancel()
        
        # 비상 락다운 방송
        asyncio.create_task(self.notify_status(
            msg=f"🚨 [안전 인터록 비상 발동] {reason} - 모든 가스 밸브 긴급 차단!",
            interlock=True
        ))

    async def notify_status(self, msg="", interlock=False):
        """실시간 상태 웹소켓 통지"""
        if not self.ws_broadcast_callback:
            return

        total_steps = len(self.current_recipe["steps"]) if self.current_recipe else 0
        progress_pct = 0.0
        if total_steps > 0:
            if self.status == "COMPLETED":
                progress_pct = 100.0
            else:
                progress_pct = round((self.current_step_index / total_steps) * 100.0, 1)

        cur_step_info = None
        if self.current_recipe and 0 <= self.current_step_index < total_steps:
            cur_step_info = self.current_recipe["steps"][self.current_step_index]

        payload = {
            "type": "SEQUENCE_STATUS",
            "status": self.status,
            "recipe_id": self.current_recipe["id"] if self.current_recipe else None,
            "recipe_name": self.current_recipe["name"] if self.current_recipe else None,
            "current_step_index": self.current_step_index,
            "total_steps": total_steps,
            "progress": progress_pct,
            "current_step": cur_step_info,
            "msg": msg,
            "interlock": interlock
        }
        await self.ws_broadcast_callback(json.dumps(payload))

    def start_recipe(self, recipe_id):
        """시퀀스 레시피 시작"""
        if recipe_id not in self.recipes:
            print(f"[SEQUENCE ENGINE ERROR] Unknown recipe ID: {recipe_id}")
            return False

        if self.status in ["RUNNING", "PAUSED"]:
            self.stop_recipe("신규 시퀀스 기동에 따른 기존 작업 정지")

        self.current_recipe = self.recipes[recipe_id]
        self.current_step_index = 0
        self.status = "RUNNING"
        self.step_start_time = time.time()
        print(f"[SEQUENCE ENGINE] Starting recipe: {self.current_recipe['name']}")

        self.runner_task = asyncio.create_task(self._runner_loop())
        return True

    def pause_recipe(self):
        """일시 정지"""
        if self.status == "RUNNING":
            self.status = "PAUSED"
            print(f"[SEQUENCE ENGINE] Recipe paused.")
            asyncio.create_task(self.notify_status("시퀀스가 일시 정지되었습니다."))

    def resume_recipe(self):
        """재개"""
        if self.status == "PAUSED":
            self.status = "RUNNING"
            self.step_start_time = time.time()
            print(f"[SEQUENCE ENGINE] Recipe resumed.")
            asyncio.create_task(self.notify_status("시퀀스가 재개되었습니다."))

    def stop_recipe(self, reason="사용자 강제 정지"):
        """정지"""
        print(f"[SEQUENCE ENGINE] Stopping recipe: {reason}")
        self.status = "IDLE"
        if self.runner_task and not self.runner_task.done():
            self.runner_task.cancel()
        asyncio.create_task(self.notify_status(f"시퀀스가 정지되었습니다: {reason}"))

    async def _runner_loop(self):
        """스텝 수행 루프"""
        try:
            steps = self.current_recipe.get("steps", [])
            total_steps = len(steps)

            while self.current_step_index < total_steps:
                if self.status == "PAUSED":
                    await asyncio.sleep(0.5)
                    continue

                if self.status != "RUNNING":
                    break

                step = steps[self.current_step_index]
                step_num = step.get("step", self.current_step_index + 1)
                step_title = step.get("title", f"Step {step_num}")
                step_type = step.get("type") or step.get("action_type") or "NORMAL"

                await self.notify_status(f"[스텝 {step_num}/{total_steps}] '{step_title}' 실행 진입")
                
                # 🔀 0. IF 조건 분기 (IF_BRANCH) 처리
                if step_type == "IF_BRANCH":
                    cond = step.get("condition") or {}
                    sensor_key = cond.get("sensor") or step.get("sensor_id") or step.get("target")
                    operator = cond.get("operator") or step.get("operator", ">=")
                    target_val = cond.get("target_val") if cond.get("target_val") is not None else step.get("target_val", 0.0)
                    
                    jump_true = step.get("jump_true") or step.get("jump_true_step")
                    jump_false = step.get("jump_false") or step.get("jump_false_step")

                    cur_val = self.latest_sensor_data.get(sensor_key, 0.0)
                    is_true = False

                    if operator == ">=" and cur_val >= target_val: is_true = True
                    elif operator == "<=" and cur_val <= target_val: is_true = True
                    elif operator == "==" and cur_val == target_val: is_true = True
                    elif operator == "!=" and cur_val != target_val: is_true = True
                    elif operator == ">" and cur_val > target_val: is_true = True
                    elif operator == "<" and cur_val < target_val: is_true = True

                    target_step_num = jump_true if is_true else jump_false

                    eval_str = "참(TRUE)" if is_true else "거짓(FALSE)"
                    await self.notify_status(f"🔀 [IF 조건판단] 센서({sensor_key}={cur_val}) {operator} {target_val} ➔ {eval_str}")

                    if target_step_num is not None:
                        # 지정한 스텝 번호로 점프 인덱스 계산 (1-based step 번호 또는 index)
                        next_idx = -1
                        for idx, s in enumerate(steps):
                            if s.get("step") == target_step_num or (idx + 1) == target_step_num:
                                next_idx = idx
                                break

                        if next_idx != -1:
                            await self.notify_status(f"🔀 [경로 분기 점프] Step {step_num} ➔ Step {target_step_num} (인덱스 {next_idx})로 이동")
                            self.current_step_index = next_idx
                            await asyncio.sleep(0.3)
                            continue
                        else:
                            await self.notify_status(f"⚠️ [분기 오류] 이동할 스텝 Step {target_step_num}을 찾을 수 없어 다음 스텝으로 진행합니다.")

                # 1. 스텝 액션(Actions) 수행 (DO 릴레이 / DAC 출력 설정 명령 발행)
                actions = step.get("actions", {})
                if actions and self.ws_broadcast_callback:
                    action_payload = {
                        "type": "SEQUENCE_ACTION",
                        "actions": actions
                    }
                    await self.ws_broadcast_callback(json.dumps(action_payload))

                # 2. 대기 조건(Wait Seconds 또는 Sensor Condition) 처리
                wait_sec = step.get("wait_seconds", 0)
                condition = step.get("condition")
                timeout_sec = step.get("timeout_seconds", 30)

                start_t = time.time()

                if wait_sec > 0:
                    # 타이머 대기
                    elapsed = 0
                    while elapsed < wait_sec:
                        if self.status != "RUNNING":
                            break
                        await asyncio.sleep(0.2)
                        elapsed = time.time() - start_t

                elif condition and step_type != "IF_BRANCH":
                    # 조건 대기 (센서 목표치 도달 감시)
                    sensor_key = condition.get("sensor")
                    operator = condition.get("operator", ">=")
                    target_val = condition.get("target_val", 0.0)

                    cond_met = False
                    while not cond_met:
                        if self.status != "RUNNING":
                            break

                        # 센서 값 체크
                        cur_val = self.latest_sensor_data.get(sensor_key, 0.0)
                        if operator == ">=" and cur_val >= target_val:
                            cond_met = True
                        elif operator == "<=" and cur_val <= target_val:
                            cond_met = True
                        elif operator == "==" and cur_val == target_val:
                            cond_met = True
                        elif operator == "!=" and cur_val != target_val:
                            cond_met = True

                        if cond_met:
                            break

                        # 타임아웃 검사
                        if time.time() - start_t > timeout_sec:
                            await self.notify_status(f"⚠️ [스텝 {step_num} 타임아웃] 센서({sensor_key}={cur_val})가 목표치({target_val})에 미달했습니다.")
                            break

                        await asyncio.sleep(0.3)

                # 일반 스텝 완료 후 다음 스텝 순차 이동
                self.current_step_index += 1
                await asyncio.sleep(0.3)

            if self.status == "RUNNING" and self.current_step_index >= total_steps:
                self.status = "COMPLETED"
                await self.notify_status("🎉 모든 시퀀스 스텝이 성공적으로 완료되었습니다!")

        except asyncio.CancelledError:
            print("[SEQUENCE ENGINE] Task cancelled.")
        except Exception as e:
            print(f"[SEQUENCE ENGINE ERROR] Error in runner loop: {e}")
            self.status = "ERROR"
            await self.notify_status(f"시퀀스 실행 중 오류 발생: {e}")
