import pty
import os
import serial
import time
import random

master, slave = pty.openpty()

s_name = os.ttyname(slave)
print("Data streaming to:", s_name)

def bitstring_to_bytes(s):
    return int(s, 2).to_bytes((len(s) + 7) // 8, byteorder='big')

def binary_id(s):
    return bitstring_to_bytes((32 - len(s)) * '0' + s)

def hex_id(s):
    return bytes.fromhex((8 - len(s)) * '0' + s)

DELIM_BEGIN = b'\xf5'
DELIM_END = b'\xae'

while True:
    # send velo and accel 
    id = hex_id("f5ae")
    velo = random.randint(2000, 4000).to_bytes(4, 'big') 
    accel = random.randint(000, 2000).to_bytes(4, 'big') 
    os.write(master, DELIM_BEGIN + id + velo + accel + DELIM_END)

    # send voltage
    id = binary_id("11111111")
    volts = random.randint(12000, 24000).to_bytes(8, 'big') 
    os.write(master, DELIM_BEGIN + id + volts + DELIM_END)

os.close(master)
os.close(slave)
