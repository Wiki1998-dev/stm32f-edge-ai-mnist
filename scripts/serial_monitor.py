#!/usr/bin/env python3
"""
Serial Monitor for STM32F7 MNIST
Display debug output from microcontroller
"""

import serial
import sys
import argparse
from datetime import datetime

def monitor_serial(port, baudrate=115200):
    """
    Monitor serial port output
    """
    print(f"[*] Connecting to {port} @ {baudrate} baud...")
    
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"[+] Connected!\n")
        print("=" * 60)
        print(f"Starting serial monitor - Press Ctrl+C to exit")
        print("=" * 60 + "\n")
        
        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').rstrip()
                if line:
                    print(f"[{datetime.now().strftime('%H:%M:%S')}] {line}")
    
    except serial.SerialException as e:
        print(f"[ERROR] Serial error: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n[*] Exiting...")
        ser.close()
        sys.exit(0)

def main():
    parser = argparse.ArgumentParser(description='STM32F7 Serial Monitor')
    parser.add_argument('port', help='Serial port (e.g., /dev/ttyUSB0)')
    parser.add_argument('-b', '--baud', type=int, default=115200,
                       help='Baud rate (default: 115200)')
    
    args = parser.parse_args()
    monitor_serial(args.port, args.baud)

if __name__ == '__main__':
    main()
