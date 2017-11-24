from PyQt5.QtCore import QBuffer, QIODevice, QDataStream, QByteArray, QVariant
import collections


class NetworkPacket:
    NAME_BYTES = 1
    PAYLOAD_LENGTH_BYTES = 4

    def __init__(self, name = None):
        self.name = name
        self.payload = bytearray()

    def send_to(self, sock):
        if self.name is None:
            raise RuntimeError('Name should be assigned before sending a packet')
        name_ba = self.name.encode('utf-8')
        sent = 0
        sent += sock.send(NetworkPacket.__num2hex(len(name_ba), NetworkPacket.NAME_BYTES))
        sent += sock.send(name_ba)
        sent += sock.send(NetworkPacket.__num2hex(len(self.payload), NetworkPacket.PAYLOAD_LENGTH_BYTES))
        sent += sock.send(self.payload)
        return sent

    def receive_from(self, sock):
        name_size = NetworkPacket.__hex2num(sock.recv(NetworkPacket.NAME_BYTES))
        self.name = sock.recv(name_size).decode()
        payload_size = NetworkPacket.__hex2num(sock.recv(NetworkPacket.PAYLOAD_LENGTH_BYTES))
        self.payload = bytearray()
        while len(self.payload) < payload_size:
            remaining_bytes = payload_size - len(self.payload)
            chunk_size = min(remaining_bytes, 1024)
            self.payload += sock.recv(chunk_size)

    @property
    def named_tuple(self):
        packet_dict = self.variant
        classname = collections.namedtuple(self.name, list(packet_dict.keys()))
        return classname(**self.variant)

    @property
    def variant(self):
        ba = QByteArray(self.payload)
        s = QDataStream(ba, QIODevice.ReadOnly)
        return s.readQVariant()

    @variant.setter
    def variant(self, variant):
        ba = QByteArray()
        s = QDataStream(ba, QIODevice.WriteOnly)
        s.writeQVariant(variant)
        self.payload = ba.data()

    @staticmethod
    def __num2hex(number, digits):
        out = bytearray(digits)
        for digit in range(0, digits): 
            out[digits-digit-1] = int(number) % 256
            number /= 256
        return out
    
    @staticmethod
    def __hex2num(hex_number):
        number = 0
        for digit in hex_number:
            number *= 256
            number += digit
        return number

