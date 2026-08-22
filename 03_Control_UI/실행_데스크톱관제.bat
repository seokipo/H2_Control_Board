@echo off
setlocal
cd /d "%~dp0"
echo ===================================================
echo   H2 Control Board Desktop SCADA (Frameless)
echo ===================================================
call npm start
pause
