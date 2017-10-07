import time
from . import NetworkPacket
import socket

class Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = None

    def connect(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))

    def send_receive(self, packet):
        packet.send_to(self.sock)
        received = NetworkPacket()
        received.receive_from(self.sock)
        return received


