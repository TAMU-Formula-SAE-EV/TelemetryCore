import pty
import os
import serial
import time

master, slave = pty.openpty()

s_name = os.ttyname(slave)
print("Data streaming to:", s_name)

def bitstring_to_bytes(s):
    return int(s, 2).to_bytes((len(s) + 7) // 8, byteorder='big')

def binary_id(s):
    return bitstring_to_bytes((32 - len(s)) * '0' + s)

def hex_id(s):
    return bytes.fromhex((8 - len(s)) * '0' + s)
    

ids = [
    binary_id("11111111"),
    hex_id("f5ae")
]

DELIM_BEGIN = b'\xf5'
DELIM_END = b'\xae'

while True:
    # Write data to the pseudo-terminal
    for id in ids:
        data = os.urandom(8)
        frame = DELIM_BEGIN + id + data + DELIM_END
        # print(len(frame), frame)
        os.write(master, frame)
    time.sleep(.0001) 

os.close(master)
os.close(slave)
