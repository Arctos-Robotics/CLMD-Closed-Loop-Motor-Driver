@echo off
echo ========================================
echo Closed-Loop Stepper Driver - Build Script
echo ========================================
echo.

echo [1/3] Building firmware...
idf.py build
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo [2/3] Flashing to ESP32...
idf.py -p COM3 flash
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Flash failed! Check COM port.
    pause
    exit /b 1
)

echo.
echo [3/3] Opening serial monitor...
echo Press Ctrl+] to exit monitor
echo.
idf.py -p COM3 monitor
