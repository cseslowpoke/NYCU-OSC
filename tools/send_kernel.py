import struct
import serial
port_path = "/dev/ttyUSB4"
# port_path = "/dev/pts/14"
baud_rate = 115200

with open("build/kernel.img", "rb") as f:
    kernel_data = f.read()

checksum = sum(kernel_data) & 0xFFFFFFFF
header = struct.pack('<III', 0x544F4F42, len(kernel_data), checksum)

with serial.Serial(port_path, baud_rate) as ser:
    ser.write(header)
    ser.write(kernel_data)
    print("send kernel")
