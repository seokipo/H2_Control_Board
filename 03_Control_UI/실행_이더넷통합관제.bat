@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo   [H2 Control Board] W5500 Ethernet SCADA Launcher
echo ========================================================
echo.
echo [1/2] Starting W5500 Modbus TCP Bridge (192.168.0.100:502)...
wscript.exe "%~dp0run_silent_bridge.vbs"

timeout /t 2 /nobreak > nul

echo [2/2] Launching SCADA Web Dashboard...
start "" "index.html"

echo.
echo ========================================================
echo   Launch Complete! (Ethernet Bridge running in Background)
echo ========================================================
