#!/usr/bin/python3
import struct
import serial
import argparse
from pathlib import Path


parser = argparse.ArgumentParser(description='Send kernel image to bootloader')
parser.add_argument('kernel', type=str, default="build/kernel.img", help='kernel image path')
parser.add_argument('--port', type=str, default="/dev/ttyUSB0", help='serial port path')
parser.add_argument('--baud', type=int, default=115200, help='baud rate')

args = parser.parse_args()


with open(args.kernel, "rb") as f:
    kernel_data = f.read()

checksum = sum(kernel_data) & 0xFFFFFFFF
header = struct.pack('<III', 0x544F4F42, len(kernel_data), checksum)

with serial.Serial(args.port, args.baud) as ser:
    ser.write(header)
    ser.write(kernel_data)
    print("send kernel")
