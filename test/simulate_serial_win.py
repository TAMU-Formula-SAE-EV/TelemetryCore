import serial
import time
import random

def bitstring_to_bytes(s):
    return int(s, 2).to_bytes((len(s) + 7) // 8, byteorder='big')

def binary_id(s):
    return bitstring_to_bytes((32 - len(s)) * '0' + s)

def hex_id(s):
    return bytes.fromhex((8 - len(s)) * '0' + s)

DELIM_BEGIN = b'\xf5'
DELIM_END = b'\xae'

ser = serial.Serial('COM10', baudrate=3000000, timeout=1)
print("Data streaming to", ser.port)

counter = 0
ts = time.time()
K = 1000

while True:
    id = hex_id("f5ae")
    velo = random.randint(2000, 4000).to_bytes(4, 'big') 
    accel = random.randint(0, 2000).to_bytes(4, 'big') 
    ser.write(DELIM_BEGIN + id + velo + accel + DELIM_END)

    id = binary_id("11111111")
    volts = random.randint(12000, 24000).to_bytes(8, 'big') 
    ser.write(DELIM_BEGIN + id + volts + DELIM_END)
        
    counter += 1
    if counter % K == 0:
        elapsed = time.time() - ts
        ts = time.time()
        print("Sent", K, "messages in", elapsed, "seconds")
        print("@ a rate of ", K / elapsed, "messages per second")

ser.close() 

# is there data loss occuring? there is about 20-40% of the data not being read by the other comport. -- debug