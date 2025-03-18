import serial
import time
import random

SERIAL_PORT = "COM10" 

# Open a serial connection
ser = serial.Serial(
    port=SERIAL_PORT,
    baudrate=9600,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=1
)

print(f"Simulating serial data on {SERIAL_PORT}...")

def bitstring_to_bytes(s):
    return int(s, 2).to_bytes((len(s) + 7) // 8, byteorder='big')

def binary_id(s):
    return bitstring_to_bytes((32 - len(s)) * '0' + s)

def hex_id(s):
    return bytes.fromhex((8 - len(s)) * '0' + s)

DELIM_BEGIN = b'\xf5'
DELIM_END = b'\xae'

try:
    while True:
        # send velocity and acceleration
        id = hex_id("f5ae")
        velo = random.randint(2000, 4000).to_bytes(4, 'big') 
        accel = random.randint(000, 2000).to_bytes(4, 'big') 
        ser.write(DELIM_BEGIN + id + velo + accel + DELIM_END)

        # send voltage
        id = binary_id("11111111")
        volts = random.randint(12000, 24000).to_bytes(8, 'big') 
        ser.write(DELIM_BEGIN + id + volts + DELIM_END)

        print(f"Sent data to {SERIAL_PORT}")

        time.sleep(1)  # Send data every second

except KeyboardInterrupt:
    print("\nSimulation stopped.")
    ser.close()
