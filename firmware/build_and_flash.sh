#!/bin/bash

echo "========================================"
echo "Closed-Loop Stepper Driver - Build Script"
echo "========================================"
echo ""

echo "[1/3] Building firmware..."
idf.py build
if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "[2/3] Flashing to ESP32..."
idf.py -p /dev/ttyUSB0 flash
if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Flash failed! Check port."
    exit 1
fi

echo ""
echo "[3/3] Opening serial monitor..."
echo "Press Ctrl+] to exit monitor"
echo ""
idf.py -p /dev/ttyUSB0 monitor
