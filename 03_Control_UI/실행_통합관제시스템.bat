@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo   [H2 Control Board] Integrated Control System Launcher
echo ========================================================
echo.
echo [1/2] Starting Python Serial Bridge in Background (Silent Mode)...
wscript.exe "%~dp0run_silent_bridge.vbs"

timeout /t 2 /nobreak > nul

echo [2/2] Launching SCADA Web Dashboard...
start "" "index.html"

echo.
echo ========================================================
echo   Launch Complete! (Bridge running in Background)
echo ========================================================

