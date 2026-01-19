#!/bin/bash
# Build and flash STM32F7 MNIST firmware

set -e

echo "========================================"
echo "STM32F7 MNIST Firmware Build & Flash"
echo "========================================"

cd firmware/stm32f7_mnist

echo "[*] Cleaning..."
make clean

echo "[*] Building..."
make -j4

echo "[*] Flashing to STM32F746G Discovery..."
make flash

echo "[+] Complete!"
echo "[*] Monitor output with: python scripts/serial_monitor.py /dev/ttyUSB0"
