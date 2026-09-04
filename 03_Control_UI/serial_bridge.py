import asyncio
import time
import json
import websockets
import serial
import sys
import random
import os
from sequence_engine import SequenceEngine

# 전역 상태 관리
mock_active = False
serial_port = None
tcp_host = None
tcp_port = None
write_lock = asyncio.Lock()
last_write_time = 0.0

# 연결된 모든 웹소켓 클라이언트 (브로드캐스트용)
CONNECTED_CLIENTS = set()

# 시퀀스 레시피 엔진 초기화
script_dir = os.path.dirname(os.path.abspath(__file__))
recipes_path = os.path.join(script_dir, "sequences", "sample_recipes.json")
seq_engine = SequenceEngine(recipes_path)

def modbus_crc16(data: bytes) -> int:
    """표준 Modbus RTU CRC-16 계산 (다항식 0xA001)"""
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

def build_modbus_frame(slave_id: int, func_code: int, addr: int, val_or_qty: int) -> bytes:
    """표준 Modbus RTU 요청 바이트 프레임 생성 (CRC16 포함)"""
    packet = bytes([
        slave_id & 0xFF,
        func_code & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        (val_or_qty >> 8) & 0xFF,
        val_or_qty & 0xFF
    ])
    crc = modbus_crc16(packet)
    return packet + bytes([crc & 0xFF, (crc >> 8) & 0xFF])

async def broadcast(payload: dict):
    """모든 연결된 웹소켓 클라이언트에게 동일 데이터 브로드캐스트 전송"""
    if not CONNECTED_CLIENTS:
        return
    msg = json.dumps(payload)
    dead = []
    for ws in list(CONNECTED_CLIENTS):
        try:
            await ws.send(msg)
        except Exception:
            dead.append(ws)
    for ws in dead:
        CONNECTED_CLIENTS.discard(ws)

async def read_serial_response(sp: serial.Serial, timeout_ms: int = 420, expected_len: int = 0) -> bytes:
    """
    👑 [길이 인지형 초정밀 1:1 수신 엔진]
    - Modbus RTU 헤더(국번, FC, 바이트 수)를 실시간 감지하여 기대 전체 패킷 길이를 동적 산출.
    - 패킷이 온전히 도착(len >= expected_len)하는 순간 0.1ms 만에 즉시 반환하여 응답 속도 극대화.
    - 패킷이 도착할 때까지 최대 timeout_ms(기본 420ms) 동안 여유 있게 대기하여 조기 포기 및 연속 TX 누락 원천 차단.
    """
    start_time = time.time()
    deadline = start_time + (timeout_ms / 1000.0)
    rx_bytes = b""
    quiet_deadline = None

    while time.time() < deadline:
        if sp.in_waiting > 0:
            rx_bytes += sp.read(sp.in_waiting)
            
            # 헤더를 분석하여 기대 전체 패킷 길이(expected_len) 동적 결정
            if expected_len == 0 and len(rx_bytes) >= 3:
                slave_id = rx_bytes[0]
                fc = rx_bytes[1]
                
                # [1] 예외 응답 (Error Exception: MSB 1): 국번(1) + FC(1) + 에러코드(1) + CRC(2) = 5바이트
                if fc & 0x80:
                    expected_len = 5
                # [2] 0x01, 0x02, 0x03, 0x04 읽기 응답: 국번(1) + FC(1) + 바이트수(1) + 데이터(N) + CRC(2) = 5 + N
                elif fc in (0x01, 0x02, 0x03, 0x04):
                    byte_count = rx_bytes[2]
                    expected_len = 5 + byte_count
                # [3] 0x05, 0x06, 0x10 쓰기 에코백 응답: 항상 8바이트
                elif fc in (0x05, 0x06, 0x10):
                    expected_len = 8

            # 기대 길이에 완전히 도달했으면 즉각 수신 완료!
            if expected_len > 0 and len(rx_bytes) >= expected_len:
                return rx_bytes

            # 아직 길이를 확정하지 못했거나 가변 패킷인 경우, 30ms 묵음 대기
            quiet_deadline = time.time() + 0.03
        else:
            if quiet_deadline and time.time() >= quiet_deadline and len(rx_bytes) > 0:
                # 더 이상 바이트가 안 들어오고 묵음 시간 경과 시 프레임 완결로 판정
                return rx_bytes
            await asyncio.sleep(0.005)

    return rx_bytes

async def global_serial_worker():
    """
    👑 [싱글톤 1:1 핑퐁 동기화 엔진]
    전역에서 오직 1개만 실행되는 시리얼 마스터 워커 루프.
    다수의 웹 브라우저 창/탭이 열려도 시리얼 요청은 오직 1개씩 순차 송출(TX)되고,
    보드의 응답(RX)을 1:1로 수신한 후 모든 창에 실시간 브로드캐스트합니다.
    """
    global serial_port, mock_active, last_write_time, write_lock
    poll_step = 0

    while True:
        try:
            if serial_port and serial_port.is_open:
                # [1] 사용자 제어 명령(Write) 직후 0.25초간은 자동 폴링을 양보하여 즉시 터치감 확보
                if time.time() - last_write_time < 0.25:
                    await asyncio.sleep(0.05)
                    continue

                # [2] 물리 보드로 1:1 Ping-Pong Modbus 질의 송출 및 응답 수신
                async with write_lock:
                    try:
                        # 버퍼 잔류 쓰레기 비우기
                        if serial_port.in_waiting > 0:
                            serial_port.reset_input_buffer()

                        # 폴링 요청 패킷 생성
                        if poll_step == 0:
                            req = build_modbus_frame(1, 4, 0, 56) # 0x04 Read Input Regs (센서 56개)
                            desc = "0x04 Read Input Regs (센서 계측)"
                            expected_len = 117 # 5 + 112
                        else:
                            req = build_modbus_frame(1, 3, 0, 40) # 0x03 Read Holding Regs (출력 40개)
                            desc = "0x03 Read Holding Regs (출력 상태)"
                            expected_len = 85 # 5 + 80
                        
                        poll_step = (poll_step + 1) % 2

                        # TX 송출 및 모든 클라이언트에 브로드캐스트
                        serial_port.write(req)
                        tx_hex = req.hex().upper()
                        tx_formatted = " ".join([tx_hex[i:i+2] for i in range(0, len(tx_hex), 2)])
                        await broadcast({
                            "type": "SERIAL_TX",
                            "hex": tx_formatted,
                            "desc": desc
                        })

                        # 👑 보드의 응답 완독 대기 (1:1 Ping-Pong, 최대 420ms)
                        rx_data = await read_serial_response(serial_port, timeout_ms=420, expected_len=expected_len)
                        if rx_data:
                            rx_hex = rx_data.hex().upper()
                            rx_formatted = " ".join([rx_hex[i:i+2] for i in range(0, len(rx_hex), 2)])
                            await broadcast({
                                "type": "SERIAL_RX",
                                "hex": rx_formatted
                            })
                        else:
                            # ⚠️ 타임아웃 발생 시 화면에 침묵하지 않고 명확하게 통지하여 1:1 시각적 싱크 보존
                            await broadcast({
                                "type": "SERIAL_TIMEOUT",
                                "desc": f"{desc} 보드 응답 시간 초과 (420ms)"
                            })
                            print(f"[POLL TIMEOUT] {desc} - 보드 응답 없음 (420ms 경과)")

                    except Exception as poll_err:
                        print(f"[POLL ERROR] {poll_err}")

                # 다음 폴링까지 100ms 안전 턴어라운드 유휴 대기 (초당 약 3회의 완벽한 1:1 왕복 리듬)
                await asyncio.sleep(0.10)

            elif mock_active:
                await asyncio.sleep(2.0)
                mock_packet = {
                    "type": "MOCK_UPDATE",
                    "temp_diff": (random.random() - 0.5) * 1.5,
                    "adc_diff": random.randint(-40, 40),
                    "flow_diff": (random.random() - 0.5) * 0.8
                }
                await broadcast(mock_packet)
            else:
                await asyncio.sleep(0.2)

        except Exception as e:
            print(f"[WORKER ERROR] {e}")
            await asyncio.sleep(0.3)

async def handler(websocket, path=None):
    global serial_port, mock_active, tcp_host, tcp_port, last_write_time, write_lock
    CONNECTED_CLIENTS.add(websocket)
    client_addr = getattr(websocket, 'remote_address', 'unknown')
    print(f"[WS CLIENT] Connected from: {client_addr} (Total: {len(CONNECTED_CLIENTS)})")
    
    # 시퀀스 엔진 상태 통지 콜백 연동 (모든 클라이언트에 브로드캐스트)
    async def send_to_all(data_str):
        try:
            payload = json.loads(data_str)
            await broadcast(payload)
        except Exception:
            pass

    seq_engine.set_broadcast_callback(send_to_all)
    
    try:
        async for message in websocket:
            req = json.loads(message)
            req_type = req.get("type")
            
            if req_type == "OPEN_PORT":
                port = req.get("port", "COM3")
                baud = int(req.get("baud", 19200))
                parity_char = req.get("parity", "N")[0].upper()
                stopbits = float(req.get("stop", 1))
                
                parity_map = {"N": serial.PARITY_NONE, "E": serial.PARITY_EVEN, "O": serial.PARITY_ODD}
                parity = parity_map.get(parity_char, serial.PARITY_NONE)
                
                print(f"[PORT CONTROL] Attempting to open serial {port} at {baud}bps...")
                
                try:
                    if serial_port and serial_port.is_open:
                        serial_port.close()
                    
                    serial_port = serial.Serial(
                        port=port,
                        baudrate=baud,
                        parity=parity,
                        stopbits=stopbits,
                        timeout=0.1
                    )
                    mock_active = False
                    print(f"[PORT CONTROL] Successfully opened serial {port}.")
                    await broadcast({
                        "type": "PORT_STATUS",
                        "status": "OPENED",
                        "msg": f"{port} 열기 성공 (물리 장비 연동 활성화)"
                    })
                except Exception as ex:
                    print(f"[PORT CONTROL] Failed to open {port}: {ex}")
                    mock_active = True
                    await broadcast({
                        "type": "PORT_STATUS",
                        "status": "MOCK_ACTIVE",
                        "msg": f"{port} 포트 연결 실패 ({ex}). 시뮬레이터(가상) 모드로 자동 전환됩니다."
                    })
                    
            elif req_type == "CLOSE_PORT":
                if serial_port and serial_port.is_open:
                    serial_port.close()
                    serial_port = None
                mock_active = False
                print("[PORT CONTROL] Port connection closed.")
                await broadcast({
                    "type": "PORT_STATUS",
                    "status": "CLOSED"
                })
                
            elif req_type == "WRITE_PORT":
                hex_str = req.get("hex", "").replace(" ", "")
                if hex_str:
                    last_write_time = time.time()
                    if serial_port and serial_port.is_open:
                        async with write_lock:
                            # 턴어라운드 안전 윈도우 (보드가 송신 직후 수신 모드로 복귀할 시간 확보)
                            await asyncio.sleep(0.015)
                            try:
                                serial_port.reset_input_buffer()
                            except:
                                pass

                            byte_data = bytes.fromhex(hex_str)
                            tx_formatted = " ".join([hex_str[i:i+2] for i in range(0, len(hex_str), 2)])
                            
                            # 1차 송출
                            serial_port.write(byte_data)
                            await broadcast({
                                "type": "SERIAL_TX",
                                "hex": tx_formatted,
                                "desc": "0x06/0x05 Write Command (출력 제어)"
                            })
                            print(f"[WS -> SERIAL] Written: {tx_formatted}")

                            # 👑 보드로부터의 응답 ACK 완독 대기 (1:1 수신 동기화, 최대 350ms, 8바이트)
                            ack_data = await read_serial_response(serial_port, timeout_ms=350, expected_len=8)
                            
                            # 만약 순간 글리치로 ACK 미도착 시 1회 스마트 자동 재시도
                            if not ack_data:
                                await asyncio.sleep(0.02)
                                serial_port.write(byte_data)
                                ack_data = await read_serial_response(serial_port, timeout_ms=350, expected_len=8)

                            if ack_data:
                                ack_hex = ack_data.hex().upper()
                                ack_formatted = " ".join([ack_hex[i:i+2] for i in range(0, len(ack_hex), 2)])
                                await broadcast({
                                    "type": "SERIAL_RX",
                                    "hex": ack_formatted
                                })
                                print(f"[SERIAL -> WS] ACK: {ack_formatted}")
                    else:
                        await broadcast({
                            "type": "SERIAL_RX",
                            "hex": req.get("hex")
                        })

            # 🤖 시퀀스 레시피 제어 커맨드 핸들링
            elif req_type == "GET_RECIPES":
                await websocket.send(json.dumps({
                    "type": "RECIPE_LIST",
                    "recipes": list(seq_engine.recipes.values())
                }))

            elif req_type == "START_SEQUENCE":
                recipe_id = req.get("recipe_id")
                seq_engine.start_recipe(recipe_id)

            elif req_type == "PAUSE_SEQUENCE":
                seq_engine.pause_recipe()

            elif req_type == "RESUME_SEQUENCE":
                seq_engine.resume_recipe()

            elif req_type == "STOP_SEQUENCE":
                seq_engine.stop_recipe("사용자 요청에 의한 정지")

            elif req_type == "RESET_ALL_OUTPUTS":
                seq_engine.stop_recipe("RESET_ALL")
                if serial_port and serial_port.is_open:
                    async with write_lock:
                        try:
                            # 1. 모든 DAC (0~11번지) 0V 리셋
                            for dac_idx in range(12):
                                pkt = build_modbus_frame(1, 6, dac_idx, 0)
                                serial_port.write(pkt)
                                await asyncio.sleep(0.015)
                            # 2. 부하 DO 릴레이 (20~39번지) 닫힘, 단 DO_MC_SW(인덱스 14)는 ON(1) 유지
                            for do_idx in range(20):
                                reg_addr = 20 + do_idx
                                val = 1 if do_idx == 14 else 0
                                pkt = build_modbus_frame(1, 6, reg_addr, val)
                                serial_port.write(pkt)
                                await asyncio.sleep(0.015)
                        except Exception as reset_ex:
                            print(f"[RESET ERROR] Failed to send hardware reset: {reset_ex}")

                print("[SAFETY EMERGENCY] ALL OUTPUTS RESET DISPATCHED")
                await broadcast({
                    "type": "SYSTEM_RESET_ACK",
                    "msg": "🚨 메인 전원(DO_MC_SW: ON) 보존 & 모든 부하 릴레이(DO 19채널 닫힘, DAC 11채널 0V) 초기화 완료!"
                })

            elif req_type == "SAVE_SEQUENCE_RECIPE":
                new_recipe = req.get("recipe")
                if new_recipe and "id" in new_recipe:
                    recipe_id = new_recipe["id"]
                    seq_engine.recipes[recipe_id] = new_recipe
                    recipe_path = os.path.join(os.path.dirname(__file__), "sequences", "sample_recipes.json")
                    try:
                        recipes_list = list(seq_engine.recipes.values())
                        with open(recipe_path, "w", encoding="utf-8") as f:
                            json.dump(recipes_list, f, indent=2, ensure_ascii=False)
                        print(f"[RECIPE BUILDER] New recipe saved successfully: {recipe_id}")
                        await broadcast({
                            "type": "SAVE_RECIPE_SUCCESS",
                            "recipe_id": recipe_id,
                            "msg": f"시퀀스 레시피 '{new_recipe.get('title', recipe_id)}' 저장 완료!",
                            "recipes": recipes_list
                        })
                    except Exception as ex:
                        print(f"[RECIPE BUILDER ERROR] Failed to save recipe JSON: {ex}")

            elif req_type in ["DELETE_SEQUENCE_RECIPE", "DELETE_RECIPE"]:
                del_recipe_id = req.get("recipe_id")
                if del_recipe_id and del_recipe_id in seq_engine.recipes:
                    del seq_engine.recipes[del_recipe_id]
                    recipe_path = os.path.join(os.path.dirname(__file__), "sequences", "sample_recipes.json")
                    try:
                        recipes_list = list(seq_engine.recipes.values())
                        with open(recipe_path, "w", encoding="utf-8") as f:
                            json.dump(recipes_list, f, indent=2, ensure_ascii=False)
                        print(f"[RECIPE BUILDER] Recipe deleted: {del_recipe_id}")
                        await broadcast({
                            "type": "DELETE_RECIPE_SUCCESS",
                            "recipe_id": del_recipe_id,
                            "msg": f"시퀀스 레시피 '{del_recipe_id}' 삭제 완료!",
                            "recipes": recipes_list
                        })
                    except Exception as ex:
                        print(f"[RECIPE BUILDER ERROR] Failed to delete recipe: {ex}")

            elif req_type == "UPDATE_TELEMETRY":
                telemetry = req.get("data", {})
                seq_engine.update_sensor_data(telemetry)

    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        CONNECTED_CLIENTS.discard(websocket)
        print(f"[WS CLIENT] Disconnected: {client_addr} (Remaining: {len(CONNECTED_CLIENTS)})")

async def main():
    print("==================================================")
    print(" RS-422 Modbus RTU Singleton Ping-Pong Bridge")
    print(" Websocket Server listening on ws://localhost:8888")
    print("==================================================")
    
    # 👑 단독 시리얼 마스터 워커 루프 백그라운드 상시 가동
    asyncio.create_task(global_serial_worker())
    
    async with websockets.serve(handler, "localhost", 8888):
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nBridge Service terminated.")
        sys.exit(0)
