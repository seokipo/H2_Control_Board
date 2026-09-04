@echo off
chcp 65001 > nul
echo ========================================================
echo  [H2_Control_Board] dsPIC33CK MCU 펌웨어 컴파일 및 빌드
echo ========================================================
echo.

cd /d "%~dp0H2_Firmware.X"
"D:\Program files\MPLABX\gnuBins\GnuWin32\bin\make.exe" -f nbproject/Makefile-default.mk

if %ERRORLEVEL% equ 0 (
    copy /y "dist\default\production\H2_Firmware.X.production.hex" "..\H2_Control_Board.hex" > nul
    copy /y "dist\default\production\H2_Firmware.X.production.elf" "..\H2_Control_Board.elf" > nul
    echo.
    echo ========================================================
    echo  🎉 펌웨어 빌드 성공! (H2_Control_Board.hex 생성 완료)
    echo ========================================================
) else (
    echo.
    echo [ERROR] 펌웨어 컴파일 중 오류가 발생했습니다.
)

pause
