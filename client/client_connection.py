from time import time 
import socket
import select
import json
from threading import Thread

class ClientConnection():
    
    def __init__(self, on_receive, on_connect=None, timeout=-1):
        self.sock = None
        self.connecting = False
        self.running = True
        self.connected = False
        self.last_heartbeat_timestamp = 0
        self.receive_thread = None
        self.on_connect = on_connect
        self.on_receive = on_receive 
        self.timeout = timeout

    def async_connect(self, host: str, port: int):
        if (not self.connecting):
            self.connecting = True
            self.connect_thread = Thread(target=self.connect, args=(host, port))
            self.connect_thread.start()

    def connect(self, host: str, port: int):
            print(f"Connecting to {host}:{port}")
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(3)
            res = self.sock.connect_ex((host, port))
            self.sock.settimeout(0)

            if (res == 0):
                print(f"Connected")
                self.receive_thread = Thread(target=self.receive)
                self.receive_thread.start()
            else:
                print(f"Failed to connect, Error {res}")

            self.connecting = False
            self.connected = res == 0
            if self.on_connect:
                self.on_connect(self.connected)

    def receive(self):
        while self.running:
            readable, writable, errors = select.select([self.sock], [], [], 1)
            if len(readable) > 0 and readable[0] is self.sock and self.running:
                data = self.sock.recv(2**20)

                if len(data) > 0:
                    self.last_heartbeat_timestamp = int(time() * 1000) # ms
                    if (not self.connected):
                        self.connected = True
                        self.on_connect(True)

                if self.on_receive:
                    seg = data[0:-1].split(b']')
                    self.on_receive(seg[0], seg[1])
                
            if (self.timeout != -1 and int(time() * 1000) - self.last_heartbeat_timestamp > self.timeout and self.connected):
                print("Disconnected")
                self.connected = False
                self.on_connect(False)
    
    def send_packet(self, header, content):
        if not self.connected: return
        self.sock.sendall(f'[{header}]{content};'.encode())

    def close(self):
        print("Closing Connection")
        self.running = False
        if self.sock:
            self.sock.close()
        if self.receive_thread and self.receive_thread.ident:
            self.receive_thread.join()