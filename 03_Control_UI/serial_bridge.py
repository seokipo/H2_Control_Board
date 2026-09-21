import asyncio
import time
import json
import websockets
import serial
import serial.tools.list_ports
import sys
import random
import os
import datetime
import argparse
from sequence_engine import SequenceEngine

def get_available_ports():
    """시스템에 연결된 실제 물리 시리얼 COM 포트 목록 동적 스캔 (장치명 및 하드웨어 ID 포함)"""
    ports = []
    try:
        for p in serial.tools.list_ports.comports():
            ports.append({
                "port": p.device,
                "description": p.description or p.device,
                "hwid": p.hwid or ""
            })
    except Exception as e:
        print(f"[PORT SCAN ERROR] {e}")

    def port_sort_key(item):
        port_str = item.get("port", "")
        if port_str.upper().startswith("COM"):
            num_part = port_str[3:]
            if num_part.isdigit():
                return int(num_part)
        return 9999

    ports.sort(key=port_sort_key)
    return ports


# ==============================================================================
# 전역 통신 상태 및 파라미터 관리
# ==============================================================================
COMM_MODE = "TCP" # "TCP" (이더넷) 또는 "SERIAL" (RS-422)
tcp_host = "192.168.0.100"
tcp_port = 502
tcp_reader = None
tcp_writer = None
tcp_connected = False

mock_active = False
serial_port = None
write_lock = asyncio.Lock()
last_write_time = 0.0
force_holding_read = False # 제어 쓰기 직후 0x03 즉각 우선 폴링 플래그
trans_id_counter = 0

# 연결된 모든 웹소켓 클라이언트 (브로드캐스트용)
CONNECTED_CLIENTS = set()
shutdown_timer_task = None
has_client_connected_ever = False

async def check_auto_shutdown():
    """모든 관제 화면(클라이언트)이 닫혔을 때 백그라운드 프로세스 자동 자가 종료"""
    global shutdown_timer_task
    try:
        await asyncio.sleep(4.0) # 새로고침(F5) 및 팝업 화면 전환 유예 시간
        if len(CONNECTED_CLIENTS) == 0:
            print("\n[AUTO SHUTDOWN] 모든 관제 화면이 닫혔습니다. 백그라운드 브릿지를 안전하게 자동 종료합니다.")
            if serial_port and serial_port.is_open:
                try:
                    serial_port.close()
                except Exception:
                    pass
            if tcp_writer:
                try:
                    tcp_writer.close()
                except Exception:
                    pass
            os._exit(0)
    except asyncio.CancelledError:
        pass

async def initial_idle_watchdog():
    """부팅 후 45초간 한 번도 클라이언트가 연결되지 않으면 유휴 프로세스 자동 종료"""
    await asyncio.sleep(45.0)
    if not has_client_connected_ever and len(CONNECTED_CLIENTS) == 0:
        print("[IDLE WATCHDOG] 45초간 관제 클라이언트 연결이 없어 백그라운드 브릿지를 자동 종료합니다.")
        os._exit(0)

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

def build_modbus_tcp_frame(t_id: int, unit_id: int, func_code: int, addr: int, val_or_qty: int) -> bytes:
    """표준 Modbus TCP 요청 프레임 생성 (MBAP 7B + PDU 5B = 12B)"""
    mbap = bytes([
        (t_id >> 8) & 0xFF,
        t_id & 0xFF,
        0x00, 0x00,       # Protocol ID: 0 (Modbus)
        0x00, 0x06,       # Length: 6 bytes (Unit ID 1B + PDU 5B)
        unit_id & 0xFF    # Unit ID (1)
    ])
    pdu = bytes([
        func_code & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF,
        (val_or_qty >> 8) & 0xFF,
        val_or_qty & 0xFF
    ])
    return mbap + pdu

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
    """길이 인지형 초정밀 시리얼 1:1 수신 엔진"""
    start_time = time.time()
    deadline = start_time + (timeout_ms / 1000.0)
    rx_bytes = b""
    quiet_deadline = None

    while time.time() < deadline:
        if sp.in_waiting > 0:
            rx_bytes += sp.read(sp.in_waiting)
            if expected_len == 0 and len(rx_bytes) >= 3:
                fc = rx_bytes[1]
                if fc & 0x80:
                    expected_len = 5
                elif fc in (0x01, 0x02, 0x03, 0x04):
                    byte_count = rx_bytes[2]
                    expected_len = 5 + byte_count
                elif fc in (0x05, 0x06, 0x10):
                    expected_len = 8

            if expected_len > 0 and len(rx_bytes) >= expected_len:
                return rx_bytes

            quiet_deadline = time.time() + 0.03
        else:
            if quiet_deadline and time.time() >= quiet_deadline and len(rx_bytes) > 0:
                return rx_bytes
            await asyncio.sleep(0.005)

    return rx_bytes

# ==============================================================================
# 🌐 이더넷(Modbus TCP) 전용 워커 엔진
# ==============================================================================
async def global_tcp_worker():
    """
    🌐 [W5500 Modbus TCP 초고속 1:1 핑퐁 워커]
    - W5500 제어보드(192.168.0.100:502)와 소켓 연결 유지
    - 0x04(센서 64개)와 0x03(출력 40개)을 초고속 100~150ms 주기로 폴링
    - 수신된 Modbus TCP PDU를 UI 호환 RTU 바이트 스트림으로 변환 브로드캐스트
    """
    global tcp_reader, tcp_writer, tcp_connected, mock_active, last_write_time, write_lock, force_holding_read, trans_id_counter
    poll_cycle = 0

    while True:
        try:
            if COMM_MODE != "TCP":
                await asyncio.sleep(0.5)
                continue

            # [1] TCP 소켓 연결 수립
            if tcp_writer is None or tcp_writer.is_closing():
                try:
                    print(f"[TCP] Connecting to H2 Control Board ({tcp_host}:{tcp_port})...")
                    tcp_reader, tcp_writer = await asyncio.wait_for(
                        asyncio.open_connection(tcp_host, tcp_port),
                        timeout=2.0
                    )
                    tcp_connected = True
                    print(f"[TCP] 🎉 Connected to H2 Control Board ({tcp_host}:{tcp_port})!")
                    await broadcast({
                        "type": "PORT_STATUS",
                        "status": "OPENED",
                        "msg": f"🌐 이더넷 연결 성공 ({tcp_host}:{tcp_port})",
                        "port": f"ETH ({tcp_host})"
                    })
                except Exception as conn_err:
                    tcp_connected = False
                    tcp_writer = None
                    tcp_reader = None
                    # 연결 실패 시 1.5초 후 재시도
                    await asyncio.sleep(1.5)
                    continue

            # [2] 사용자 제어 명령 직후에는 폴링 일시 대기
            if time.time() - last_write_time < 0.1:
                await asyncio.sleep(0.02)
                continue

            # [3] Modbus TCP 질의 송수신
            async with write_lock:
                try:
                    trans_id_counter = (trans_id_counter + 1) & 0xFFFF
                    t_id = trans_id_counter

                    if force_holding_read or (poll_cycle % 4 == 3):
                        req = build_modbus_tcp_frame(t_id, 1, 3, 0, 40) # 0x03 Read Holding Regs (40개)
                        desc = "0x03 Read Holding Regs (출력 상태)"
                        force_holding_read = False
                    else:
                        req = build_modbus_tcp_frame(t_id, 1, 4, 0, 64) # 0x04 Read Input Regs (64개)
                        desc = "0x04 Read Input Regs (센서 계측 + M701)"

                    poll_cycle = (poll_cycle + 1) % 4

                    # TCP 프레임 전송
                    tcp_writer.write(req)
                    await tcp_writer.drain()

                    # [4] Modbus TCP 응답 수신 (MBAP 헤더 6바이트 + 바디)
                    hdr = await asyncio.wait_for(tcp_reader.readexactly(6), timeout=0.6)
                    mbap_len = (hdr[4] << 8) | hdr[5]
                    body = await asyncio.wait_for(tcp_reader.readexactly(mbap_len), timeout=0.6)

                    # unit_id = body[0], pdu = body[1:]
                    pdu = body[1:]
                    # UI 호환을 위해 RTU 포맷으로 변환: [Slave 1] + [PDU] + [CRC16]
                    rtu_payload = bytes([1]) + pdu
                    crc = modbus_crc16(rtu_payload)
                    rtu_full = rtu_payload + bytes([crc & 0xFF, (crc >> 8) & 0xFF])

                    # UI 및 패킷 스트리머로 브로드캐스트
                    tx_hex = rtu_payload[:6].hex().upper()
                    tx_formatted = " ".join([tx_hex[i:i+2] for i in range(0, len(tx_hex), 2)])
                    await broadcast({
                        "type": "SERIAL_TX",
                        "hex": tx_formatted,
                        "desc": desc
                    })

                    rx_hex = rtu_full.hex().upper()
                    rx_formatted = " ".join([rx_hex[i:i+2] for i in range(0, len(rx_hex), 2)])
                    await broadcast({
                        "type": "SERIAL_RX",
                        "hex": rx_formatted
                    })

                except (asyncio.TimeoutError, ConnectionResetError, BrokenPipeError) as net_err:
                    print(f"[TCP NET ERROR] {net_err}")
                    if tcp_writer:
                        try:
                            tcp_writer.close()
                        except:
                            pass
                    tcp_writer = None
                    tcp_reader = None
                    tcp_connected = False
                    await asyncio.sleep(1.0)
                    continue

            # 이더넷 초고속 주기 (약 80~100ms)
            await asyncio.sleep(0.08)

        except Exception as e:
            print(f"[TCP WORKER ERROR] {e}")
            await asyncio.sleep(0.2)


# ==============================================================================
# 🔌 시리얼(RS-422) 전용 워커 엔진
# ==============================================================================
async def global_serial_worker():
    """시리얼 모드 전용 1:1 Ping-Pong 워커"""
    global serial_port, mock_active, last_write_time, write_lock, force_holding_read
    poll_cycle = 0

    while True:
        try:
            if COMM_MODE != "SERIAL":
                await asyncio.sleep(0.5)
                continue

            if serial_port and serial_port.is_open:
                if time.time() - last_write_time < 0.25:
                    await asyncio.sleep(0.05)
                    continue

                async with write_lock:
                    try:
                        if serial_port.in_waiting > 0:
                            serial_port.reset_input_buffer()

                        if force_holding_read or (poll_cycle % 4 == 3):
                            req = build_modbus_frame(1, 3, 0, 40)
                            desc = "0x03 Read Holding Regs (출력 상태)"
                            expected_len = 85
                            force_holding_read = False
                        else:
                            req = build_modbus_frame(1, 4, 0, 64)
                            desc = "0x04 Read Input Regs (센서 계측 + M701)"
                            expected_len = 133

                        poll_cycle = (poll_cycle + 1) % 4

                        serial_port.write(req)
                        tx_hex = req.hex().upper()
                        tx_formatted = " ".join([tx_hex[i:i+2] for i in range(0, len(tx_hex), 2)])
                        await broadcast({
                            "type": "SERIAL_TX",
                            "hex": tx_formatted,
                            "desc": desc
                        })

                        rx_data = await read_serial_response(serial_port, timeout_ms=500, expected_len=expected_len)
                        if rx_data:
                            rx_hex = rx_data.hex().upper()
                            rx_formatted = " ".join([rx_hex[i:i+2] for i in range(0, len(rx_hex), 2)])
                            await broadcast({
                                "type": "SERIAL_RX",
                                "hex": rx_formatted
                            })
                            turnaround_delay = 0.22 if expected_len > 100 else 0.20
                        else:
                            await broadcast({
                                "type": "SERIAL_TIMEOUT",
                                "desc": f"{desc} 보드 응답 시간 초과 (500ms)"
                            })
                            try:
                                serial_port.reset_input_buffer()
                                serial_port.reset_output_buffer()
                            except:
                                pass
                            turnaround_delay = 0.35

                    except Exception as poll_err:
                        print(f"[POLL ERROR] {poll_err}")
                        turnaround_delay = 0.25

                await asyncio.sleep(turnaround_delay)

            elif mock_active:
                await asyncio.sleep(2.0)
                now_dt = datetime.datetime.now()
                mock_packet = {
                    "type": "MOCK_UPDATE",
                    "temp_diff": (random.random() - 0.5) * 1.5,
                    "adc_diff": random.randint(-40, 40),
                    "flow_diff": (random.random() - 0.5) * 0.8,
                    "rtc": {
                        "year": now_dt.year,
                        "month": now_dt.month,
                        "date": now_dt.day,
                        "hour": now_dt.hour,
                        "minute": now_dt.minute,
                        "second": now_dt.second
                    },
                    "m701": {
                        "eco2": random.randint(420, 680),
                        "ech2o": random.randint(10, 35),
                        "tvoc": random.randint(45, 120),
                        "pm25": random.randint(12, 38),
                        "pm10": random.randint(18, 55),
                        "temperature": round(23.5 + (random.random() - 0.5) * 2.0, 1),
                        "humidity": round(48.0 + (random.random() - 0.5) * 5.0, 1),
                        "valid": True
                    }
                }
                await broadcast(mock_packet)
            else:
                await asyncio.sleep(0.2)

        except Exception as e:
            print(f"[SERIAL WORKER ERROR] {e}")
            await asyncio.sleep(0.3)


# ==============================================================================
# 🌐 WebSocket 클라이언트 요청 핸들러
# ==============================================================================
async def handler(websocket, path=None):
    global serial_port, mock_active, tcp_host, tcp_port, tcp_writer, tcp_reader, tcp_connected, COMM_MODE
    global last_write_time, write_lock, force_holding_read, shutdown_timer_task, has_client_connected_ever, trans_id_counter
    has_client_connected_ever = True
    if shutdown_timer_task and not shutdown_timer_task.done():
        shutdown_timer_task.cancel()
        shutdown_timer_task = None
    CONNECTED_CLIENTS.add(websocket)
    client_addr = getattr(websocket, 'remote_address', 'unknown')
    print(f"[WS CLIENT] Connected from: {client_addr} (Total: {len(CONNECTED_CLIENTS)})")
    
    async def send_to_all(data_str):
        try:
            payload = json.loads(data_str)
            await broadcast(payload)
        except Exception:
            pass

    seq_engine.set_broadcast_callback(send_to_all)
    
    # 클라이언트 초기 상태 전송
    try:
        cur_port = f"ETH ({tcp_host}:{tcp_port})" if COMM_MODE == "TCP" else (serial_port.port if (serial_port and serial_port.is_open) else None)
        is_open = tcp_connected if COMM_MODE == "TCP" else bool(serial_port and serial_port.is_open)
        await websocket.send(json.dumps({
            "type": "PORTS_LIST",
            "ports": get_available_ports(),
            "current_port": cur_port,
            "is_open": is_open,
            "comm_mode": COMM_MODE
        }))
    except Exception as ex:
        print(f"[WS CLIENT INIT] Init push failed: {ex}")

    try:
        async for message in websocket:
            req = json.loads(message)
            req_type = req.get("type")
            
            if req_type in ["GET_PORTS", "REFRESH_PORTS", "SCAN_PORTS"]:
                cur_port = f"ETH ({tcp_host}:{tcp_port})" if COMM_MODE == "TCP" else (serial_port.port if (serial_port and serial_port.is_open) else None)
                is_open = tcp_connected if COMM_MODE == "TCP" else bool(serial_port and serial_port.is_open)
                await websocket.send(json.dumps({
                    "type": "PORTS_LIST",
                    "ports": get_available_ports(),
                    "current_port": cur_port,
                    "is_open": is_open,
                    "comm_mode": COMM_MODE
                }))

            elif req_type == "OPEN_PORT":
                port = req.get("port", "COM3")
                baud = int(req.get("baud", 19200))
                COMM_MODE = "SERIAL"
                print(f"[PORT CONTROL] Attempting to open serial {port} at {baud}bps...")
                
                try:
                    if serial_port and serial_port.is_open:
                        serial_port.close()
                    
                    serial_port = serial.Serial(port=port, baudrate=baud, timeout=0.1)
                    mock_active = False
                    print(f"[PORT CONTROL] Successfully opened serial {port}.")
                    await broadcast({
                        "type": "PORT_STATUS",
                        "status": "OPENED",
                        "msg": f"{port} 열기 성공 (시리얼 연동 활성화)",
                        "port": port
                    })
                except Exception as ex:
                    print(f"[PORT CONTROL] Failed to open {port}: {ex}")
                    mock_active = True
                    await broadcast({
                        "type": "PORT_STATUS",
                        "status": "MOCK_ACTIVE",
                        "msg": f"{port} 포트 연결 실패 ({ex}). 가상 모드로 전환됩니다."
                    })

            elif req_type == "CLOSE_PORT":
                if COMM_MODE == "SERIAL" and serial_port and serial_port.is_open:
                    serial_port.close()
                    serial_port = None
                mock_active = False
                await broadcast({"type": "PORT_STATUS", "status": "CLOSED"})

            elif req_type == "WRITE_PORT":
                # UI에서 내려온 0x05 / 0x06 제어 명령
                hex_str = req.get("hex", "").replace(" ", "")
                if hex_str:
                    last_write_time = time.time()
                    force_holding_read = True

                    if COMM_MODE == "TCP" and tcp_writer and not tcp_writer.is_closing():
                        async with write_lock:
                            try:
                                rtu_bytes = bytes.fromhex(hex_str)
                                # RTU: [Slave 1B][FC 1B][Addr 2B][Val 2B][CRC 2B] -> TCP PDU: [FC][Addr][Val]
                                if len(rtu_bytes) >= 6:
                                    fc = rtu_bytes[1]
                                    addr = (rtu_bytes[2] << 8) | rtu_bytes[3]
                                    val = (rtu_bytes[4] << 8) | rtu_bytes[5]
                                    trans_id_counter = (trans_id_counter + 1) & 0xFFFF
                                    tcp_pkt = build_modbus_tcp_frame(trans_id_counter, 1, fc, addr, val)
                                    tcp_writer.write(tcp_pkt)
                                    await tcp_writer.drain()

                                    # ACK 응답 수신
                                    hdr = await asyncio.wait_for(tcp_reader.readexactly(6), timeout=0.5)
                                    mbap_len = (hdr[4] << 8) | hdr[5]
                                    body = await asyncio.wait_for(tcp_reader.readexactly(mbap_len), timeout=0.5)
                                    
                                    pdu = body[1:]
                                    rtu_ack = bytes([1]) + pdu
                                    crc = modbus_crc16(rtu_ack)
                                    rtu_ack_full = rtu_ack + bytes([crc & 0xFF, (crc >> 8) & 0xFF])

                                    ack_hex = rtu_ack_full.hex().upper()
                                    ack_formatted = " ".join([ack_hex[i:i+2] for i in range(0, len(ack_hex), 2)])
                                    await broadcast({
                                        "type": "SERIAL_RX",
                                        "hex": ack_formatted
                                    })
                                    print(f"[TCP -> WS] ACK: {ack_formatted}")
                            except Exception as tcp_wr_err:
                                print(f"[TCP WRITE ERROR] {tcp_wr_err}")

                    elif COMM_MODE == "SERIAL" and serial_port and serial_port.is_open:
                        async with write_lock:
                            await asyncio.sleep(0.015)
                            byte_data = bytes.fromhex(hex_str)
                            serial_port.write(byte_data)
                            ack_data = await read_serial_response(serial_port, timeout_ms=350, expected_len=8)
                            if ack_data:
                                ack_hex = ack_data.hex().upper()
                                ack_formatted = " ".join([ack_hex[i:i+2] for i in range(0, len(ack_hex), 2)])
                                await broadcast({
                                    "type": "SERIAL_RX",
                                    "hex": ack_formatted
                                })

            elif req_type == "RTC_SYNC":
                # RTC 시간 동기화 명령
                year = int(req.get("year", datetime.datetime.now().year))
                month = int(req.get("month", datetime.datetime.now().month))
                date = int(req.get("date", datetime.datetime.now().day))
                hour = int(req.get("hour", datetime.datetime.now().hour))
                minute = int(req.get("minute", datetime.datetime.now().minute))
                second = int(req.get("second", datetime.datetime.now().second))

                rtc_regs = [
                    (40, year), (41, month), (42, date),
                    (43, hour), (44, minute), (45, second),
                    (46, 1) # Trigger
                ]

                async with write_lock:
                    for reg_addr, val in rtc_regs:
                        if COMM_MODE == "TCP" and tcp_writer:
                            trans_id_counter = (trans_id_counter + 1) & 0xFFFF
                            tcp_pkt = build_modbus_tcp_frame(trans_id_counter, 1, 6, reg_addr, val)
                            tcp_writer.write(tcp_pkt)
                            await tcp_writer.drain()
                            hdr = await asyncio.wait_for(tcp_reader.readexactly(6), timeout=0.3)
                            mlen = (hdr[4] << 8) | hdr[5]
                            await tcp_reader.readexactly(mlen)
                        elif COMM_MODE == "SERIAL" and serial_port and serial_port.is_open:
                            pkt = build_modbus_frame(1, 6, reg_addr, val)
                            serial_port.write(pkt)
                            await read_serial_response(serial_port, timeout_ms=250, expected_len=8)
                        await asyncio.sleep(0.015)

                await broadcast({
                    "type": "RTC_SYNC_ACK",
                    "status": "SUCCESS",
                    "time": f"{year:04d}-{month:02d}-{date:02d} {hour:02d}:{minute:02d}:{second:02d}"
                })

            elif req_type == "GET_RECIPES":
                await websocket.send(json.dumps({
                    "type": "RECIPE_LIST",
                    "recipes": list(seq_engine.recipes.values())
                }))

            elif req_type == "START_SEQUENCE":
                seq_engine.start_recipe(req.get("recipe_id"))

            elif req_type == "PAUSE_SEQUENCE":
                seq_engine.pause_recipe()

            elif req_type == "RESUME_SEQUENCE":
                seq_engine.resume_recipe()

            elif req_type == "STOP_SEQUENCE":
                seq_engine.stop_recipe("사용자 요청에 의한 정지")

            elif req_type == "RESET_ALL_OUTPUTS":
                seq_engine.stop_recipe("RESET_ALL")
                async with write_lock:
                    try:
                        for dac_idx in range(12):
                            if COMM_MODE == "TCP" and tcp_writer:
                                trans_id_counter = (trans_id_counter + 1) & 0xFFFF
                                pkt = build_modbus_tcp_frame(trans_id_counter, 1, 6, dac_idx, 0)
                                tcp_writer.write(pkt)
                                await tcp_writer.drain()
                                hdr = await tcp_reader.readexactly(6)
                                mlen = (hdr[4] << 8) | hdr[5]
                                await tcp_reader.readexactly(mlen)
                            elif COMM_MODE == "SERIAL" and serial_port and serial_port.is_open:
                                pkt = build_modbus_frame(1, 6, dac_idx, 0)
                                serial_port.write(pkt)
                                await asyncio.sleep(0.015)
                        
                        for do_idx in range(20):
                            reg_addr = 20 + do_idx
                            val = 1 if do_idx == 14 else 0
                            if COMM_MODE == "TCP" and tcp_writer:
                                trans_id_counter = (trans_id_counter + 1) & 0xFFFF
                                pkt = build_modbus_tcp_frame(trans_id_counter, 1, 6, reg_addr, val)
                                tcp_writer.write(pkt)
                                await tcp_writer.drain()
                                hdr = await tcp_reader.readexactly(6)
                                mlen = (hdr[4] << 8) | hdr[5]
                                await tcp_reader.readexactly(mlen)
                            elif COMM_MODE == "SERIAL" and serial_port and serial_port.is_open:
                                pkt = build_modbus_frame(1, 6, reg_addr, val)
                                serial_port.write(pkt)
                                await asyncio.sleep(0.015)
                    except Exception as reset_ex:
                        print(f"[RESET ERROR] {reset_ex}")

                await broadcast({
                    "type": "SYSTEM_RESET_ACK",
                    "msg": "🚨 메인 전원(DO_MC_SW: ON) 보존 & 모든 부하 릴레이 초기화 완료!"
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
                        await broadcast({
                            "type": "DELETE_RECIPE_SUCCESS",
                            "recipe_id": del_recipe_id,
                            "msg": f"시퀀스 레시피 '{del_recipe_id}' 삭제 완료!",
                            "recipes": recipes_list
                        })
                    except Exception as ex:
                        print(f"[RECIPE BUILDER ERROR] Failed to delete recipe: {ex}")

            elif req_type == "UPDATE_TELEMETRY":
                seq_engine.update_sensor_data(req.get("data", {}))

            elif req_type == "SHUTDOWN_BRIDGE":
                print("[SHUTDOWN] 관제 UI 창 닫힘 감지. 백그라운드 브릿지를 안전하게 즉각 종료합니다.")
                if serial_port and serial_port.is_open:
                    try:
                        serial_port.close()
                    except:
                        pass
                if tcp_writer:
                    try:
                        tcp_writer.close()
                    except:
                        pass
                os._exit(0)

    except websockets.exceptions.ConnectionClosed:
        pass
    finally:
        CONNECTED_CLIENTS.discard(websocket)
        print(f"[WS CLIENT] Disconnected: {client_addr} (Remaining: {len(CONNECTED_CLIENTS)})")
        if len(CONNECTED_CLIENTS) == 0 and has_client_connected_ever:
            if shutdown_timer_task is None or shutdown_timer_task.done():
                shutdown_timer_task = asyncio.create_task(check_auto_shutdown())

async def main():
    global COMM_MODE, tcp_host, tcp_port

    parser = argparse.ArgumentParser(description="H2 Control Board Modbus Bridge")
    parser.add_argument("--mode", choices=["TCP", "SERIAL"], default="TCP", help="Communication Mode (TCP or SERIAL)")
    parser.add_argument("--ip", default="192.168.0.100", help="W5500 Board IP address")
    parser.add_argument("--port", type=int, default=502, help="Modbus TCP Port")
    args, unknown = parser.parse_known_args()

    COMM_MODE = args.mode
    tcp_host = args.ip
    tcp_port = args.port

    print("==================================================")
    print(f" H2 Control Board Bridge [MODE: {COMM_MODE}]")
    if COMM_MODE == "TCP":
        print(f" Target Board IP: {tcp_host}:{tcp_port} (W5500 ETH1)")
    else:
        print(" Mode: RS-422 Serial RTU")
    print(" Websocket Server listening on ws://localhost:8888")
    print("==================================================")
    
    # 🌐 이더넷 워커 및 시리얼 워커 백그라운드 등록
    asyncio.create_task(global_tcp_worker())
    asyncio.create_task(global_serial_worker())
    asyncio.create_task(initial_idle_watchdog())
    
    async with websockets.serve(handler, "localhost", 8888):
        await asyncio.Future()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nBridge Service terminated.")
        sys.exit(0)
