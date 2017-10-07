import time
from . import NetworkPacket
import socket

class Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = None

    def connect(self):
        self.sock = socket.create_connection((self.host, self.port))

    def send_receive(self, packet):
        packet.send_to(self.sock)
        received = NetworkPacket()
        received.receive_from(self.sock)
        return received


    def disconnect(self):
        if not self.sock:
            return
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()
        self.sock = None

