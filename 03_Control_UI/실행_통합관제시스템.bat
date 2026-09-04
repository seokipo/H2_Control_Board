@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo   [H2 Control Board] Integrated Control System Launcher
echo ========================================================
echo.
echo [1/2] Starting Python Serial Bridge Server...
start "H2_Serial_Bridge" cmd /k "python serial_bridge.py"

timeout /t 2 /nobreak > nul

echo [2/2] Launching SCADA Web Dashboard...
start "" "index.html"

echo.
echo ========================================================
echo   Launch Complete!
echo ========================================================
