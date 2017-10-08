import time
from . import NetworkPacket
import socket

class Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = None
        self.handlers = []

    def connect(self):
        self.sock = socket.create_connection((self.host, self.port))

    def send_receive(self, packet):
        self.send(packet)
        return self.receive()

    def round_trip(self, packet, expected, timeout=30):
        self.send(packet)
        sent = time.time()
        while time.time() - sent < timeout:
            received = self.receive()
            if received.name == expected.packet_name():
                return received
            self.__handle(received)
        raise RuntimeError('Expected packet {} not received'.format(expected.packet_name()))

    def send(self, packet):
        if not self.sock:
            raise RuntimeError('not connected')
        packet.send_to(self.sock)

    def receive(self):
        if not self.sock:
            raise RuntimeError('not connected')
        received = NetworkPacket()
        received.receive_from(self.sock)
        return received

    def disconnect(self):
        if not self.sock:
            return
        self.sock.shutdown(socket.SHUT_RDWR)
        self.sock.close()
        self.sock = None

    def add_handler(self, expected, callback):
        self.handlers.append((expected.packet_name(), callback))

    def remove_handler(self, expected):
        self.handlers = [x for x in self.handlers if x[0] != expected.packet_name()]

    def __handle(self, packet):
        handled = False
        for handler in self.handlers:
            if handler[0] == packet.name:
                handler[1](packet)
                handled = True
        if not handled:
            # TODO: add some kind of logging class
            print('Received unexpected packet {}'.format(packet.name))

