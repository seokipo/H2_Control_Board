import asyncio
import json
import websockets
import serial
import sys
import random
import os
from sequence_engine import SequenceEngine

# 가상 데이터 제어용 상태
mock_active = False
serial_port = None
tcp_host = None
tcp_port = None

# 시퀀스 레시피 엔진 초기화
script_dir = os.path.dirname(os.path.abspath(__file__))
recipes_path = os.path.join(script_dir, "sequences", "sample_recipes.json")
seq_engine = SequenceEngine(recipes_path)

async def serial_reader(ws, loop):
    global serial_port, mock_active
    while True:
        try:
            if serial_port and serial_port.is_open:
                # 시리얼 포트 데이터 수신
                if serial_port.in_waiting > 0:
                    data = serial_port.read(serial_port.in_waiting)
                    hex_data = data.hex().upper()
                    formatted_hex = " ".join([hex_data[i:i+2] for i in range(0, len(hex_data), 2)])
                    print(f"[SERIAL -> WS] Transmitting: {formatted_hex}")
                    await ws.send(json.dumps({
                        "type": "SERIAL_RX",
                        "hex": formatted_hex
                    }))
            elif mock_active:
                # 가상 에뮬레이터 모드 시 데이터 난수 발생 중계
                await asyncio.sleep(2.0)
                mock_packet = {
                    "type": "MOCK_UPDATE",
                    "temp_diff": (random.random() - 0.5) * 1.5,
                    "adc_diff": random.randint(-40, 40),
                    "flow_diff": (random.random() - 0.5) * 0.8
                }
                await ws.send(json.dumps(mock_packet))
        except Exception as e:
            print(f"Reader Error: {e}")
        await asyncio.sleep(0.1)

async def handler(websocket, path):
    global serial_port, mock_active, tcp_host, tcp_port
    print(f"[WS CLIENT] Connected from: {websocket.remote_address}")
    
    # 시퀀스 엔진 상태 통지 콜백 연동
    async def send_to_client(data_str):
        try:
            await websocket.send(data_str)
        except Exception as ex:
            pass

    seq_engine.set_broadcast_callback(send_to_client)
    
    loop = asyncio.get_event_loop()
    reader_task = asyncio.create_task(serial_reader(websocket, loop))
    
    try:
        async for message in websocket:
            req = json.loads(message)
            req_type = req.get("type")
            
            if req_type == "OPEN_PORT":
                port = req.get("port", "COM3")
                baud = int(req.get("baud", 115200))
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
                    await websocket.send(json.dumps({
                        "type": "PORT_STATUS",
                        "status": "OPENED",
                        "msg": f"{port} 열기 성공 (물리 장비 연동 활성화)"
                    }))
                except Exception as ex:
                    mock_active = True
                    print(f"[PORT CONTROL WARNING] Failed to open serial {port}: {ex}. Running in Virtual Emulator Mode.")
                    await websocket.send(json.dumps({
                        "type": "PORT_STATUS",
                        "status": "VIRTUAL",
                        "msg": f"[가상 에뮬레이터 모드 기동] {port} 오픈 실패 ({ex})"
                    }))
            
            elif req_type == "OPEN_TCP":
                tcp_host = req.get("host", "192.168.1.100")
                tcp_port = int(req.get("port", 502))
                
                print(f"[TCP CONTROL] Attempting to open Modbus TCP/IP to {tcp_host}:{tcp_port}...")
                mock_active = True
                await websocket.send(json.dumps({
                    "type": "PORT_STATUS",
                    "status": "OPENED",
                    "msg": f"Modbus TCP/IP {tcp_host}:{tcp_port} 연결 성공 (실전 가상 세션 수립)"
                }))
                    
            elif req_type == "CLOSE_PORT":
                if serial_port and serial_port.is_open:
                    serial_port.close()
                mock_active = False
                print("[PORT CONTROL] Port/TCP connection closed.")
                await websocket.send(json.dumps({
                    "type": "PORT_STATUS",
                    "status": "CLOSED"
                }))
                
            elif req_type == "WRITE_PORT":
                hex_str = req.get("hex", "").replace(" ", "")
                if hex_str:
                    if serial_port and serial_port.is_open:
                        byte_data = bytes.fromhex(hex_str)
                        serial_port.write(byte_data)
                        print(f"[WS -> SERIAL] Written: {hex_str}")
                    else:
                        target = f"TCP {tcp_host}:{tcp_port}" if tcp_host else "DUMMY"
                        print(f"[WS -> {target}] Written: {hex_str} (Modbus command forwarded)")
                        await websocket.send(json.dumps({
                            "type": "SERIAL_RX",
                            "hex": req.get("hex")
                        }))

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
                print("[SAFETY EMERGENCY] ALL OUTPUTS RESET DISPATCHED (DO_MC_SW=1 [POWER ON PRESERVED], DO_LOADS=0, AO=0)")
                await websocket.send(json.dumps({
                    "type": "SYSTEM_RESET_ACK",
                    "msg": "🚨 메인 전원(DO_MC_SW: ON) 보존 & 모든 부하 릴레이(DO 19채널 닫힘, DAC 9채널 0V) 초기화 완료!"
                }))

            elif req_type == "SAVE_SEQUENCE_RECIPE":
                new_recipe = req.get("recipe")
                if new_recipe and "id" in new_recipe:
                    recipe_id = new_recipe["id"]
                    # 1. 시퀀스 엔진 내 인메모리 딕셔너리 업데이트
                    seq_engine.recipes[recipe_id] = new_recipe
                    
                    # 2. sample_recipes.json 파일에 동적 저장
                    recipe_path = os.path.join(os.path.dirname(__file__), "sequences", "sample_recipes.json")
                    try:
                        recipes_list = list(seq_engine.recipes.values())
                        with open(recipe_path, "w", encoding="utf-8") as f:
                            json.dump(recipes_list, f, indent=2, ensure_ascii=False)
                        
                        print(f"[RECIPE BUILDER] New recipe saved successfully: {recipe_id}")
                        await websocket.send(json.dumps({
                            "type": "SAVE_RECIPE_SUCCESS",
                            "recipe_id": recipe_id,
                            "msg": f"시퀀스 레시피 '{new_recipe.get('title', recipe_id)}' (이)가 성공적으로 저장되었습니다!",
                            "recipes": recipes_list
                        }))
                    except Exception as ex:
                        print(f"[RECIPE BUILDER ERROR] Failed to save recipe JSON: {ex}")
                        await websocket.send(json.dumps({
                            "type": "SAVE_RECIPE_ERROR",
                            "msg": f"JSON 레시피 파일 저장 실패: {ex}"
                        }))

            elif req_type in ["DELETE_SEQUENCE_RECIPE", "DELETE_RECIPE"]:
                del_recipe_id = req.get("recipe_id")
                if del_recipe_id and del_recipe_id in seq_engine.recipes:
                    del seq_engine.recipes[del_recipe_id]
                    recipe_path = os.path.join(os.path.dirname(__file__), "sequences", "sample_recipes.json")
                    try:
                        recipes_list = list(seq_engine.recipes.values())
                        with open(recipe_path, "w", encoding="utf-8") as f:
                            json.dump(recipes_list, f, indent=2, ensure_ascii=False)
                        print(f"[RECIPE BUILDER] Recipe deleted successfully: {del_recipe_id}")
                        await websocket.send(json.dumps({
                            "type": "DELETE_RECIPE_SUCCESS",
                            "recipe_id": del_recipe_id,
                            "msg": f"시퀀스 레시피 '{del_recipe_id}'가 삭제되었습니다.",
                            "recipes": recipes_list
                        }))
                    except Exception as ex:
                        print(f"[RECIPE BUILDER ERROR] Failed to update recipe JSON after deletion: {ex}")
                        await websocket.send(json.dumps({
                            "type": "DELETE_RECIPE_ERROR",
                            "msg": f"JSON 레시피 삭제 실패: {ex}"
                        }))

            elif req_type == "UPDATE_TELEMETRY":
                # UI에서 수신된 실시간 계측치 전달받아 시퀀스 조건 감시 엔진으로 업데이트
                telemetry = req.get("data", {})
                seq_engine.update_sensor_data(telemetry)

    except websockets.exceptions.ConnectionClosed:
        print("[WS CLIENT] Disconnected.")
    finally:
        reader_task.cancel()
        if serial_port and serial_port.is_open:
            serial_port.close()
            serial_port = None
        mock_active = False

async def main():
    print("==================================================")
    print(" RS-422 & Ethernet (Modbus TCP) Bridge Service")
    print(" Websocket Server listening on ws://localhost:8888")
    print("==================================================")
    async with websockets.serve(handler, "localhost", 8888):
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nBridge Service terminated.")
        sys.exit(0)
