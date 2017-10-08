import time
from . import NetworkPacket, Ping
import socket
import threading
from ..utils import Interval

class Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.__sock = None
        self.__sock_lock = threading.RLock()
        self.handlers = []
        self.interval = Interval()

    def connect(self):
        with self.__sock_lock:
            self.__sock = socket.create_connection((self.host, self.port))
        self.interval.start(2, self.__ping)


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
        with self.__sock_lock:
            if not self.__sock:
                raise RuntimeError('not connected')
            packet.send_to(self.__sock)

    def receive(self):
        with self.__sock_lock:
            if not self.__sock:
                raise RuntimeError('not connected')
            received = NetworkPacket()
            received.receive_from(self.__sock)
            return received

    def disconnect(self):
        if not self.__sock:
            return

        self.interval.stop()
        with self.__sock_lock:
            self.__sock.shutdown(socket.SHUT_RDWR)
            self.__sock.close()
            self.__sock = None

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

    def __ping(self):
        self.round_trip(Ping.send(), Ping.REPLY)
