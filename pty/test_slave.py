import pty, serial, os, sys

s_name = input("tty_name: ")

ser = serial.Serial(s_name, baudrate=9600, timeout=1)
while True:
    resp = ser.readline()
    print(resp)

ser.close()

