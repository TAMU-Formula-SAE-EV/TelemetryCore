import pty
import os
import serial
import time

master, slave = pty.openpty()

s_name = os.ttyname(slave)
print("Data streaming to:", s_name)

while True:
    # Write data to the pseudo-terminal
    os.write(master, b"Hello from master!\n")
    time.sleep(.1)

os.close(master)
os.close(slave)
