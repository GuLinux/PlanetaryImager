from PyQt5.QtCore import QBuffer, QIODevice, QDataStream, QByteArray, QVariant
import time

class NetworkPacket:
    def __init__(self, name = None):
        self.name = name
        self.payload = QByteArray()

    def send_to(self, device):
        if self.name is None:
            raise RuntimeError('Name should be assigned before sending a packet')
        buffer = QBuffer()
        buffer.open(QIODevice.WriteOnly)
        s = QDataStream(buffer)
        s.writeString(self.name.encode('utf-8'))
        s.writeInt(self.payload.size())
        wrote = device.write(buffer.data())
        wrote += device.write(self.payload)
        if wrote == -1:
            raise RuntimeError('Error writing to device: ' + device.errorString())
        return wrote

    def receive_from(self, device):
        s = QDataStream(device)
        self.name = s.readString().decode('utf-8')
        data_size = s.readInt()
        while(device.bytesAvailable() < data_size):
            time.sleep(0.1)
        self.payload = device.read(data_size)

    def set_payload(self, variant):
        buffer = QBuffer(self.payload)
        buffer.open(QIODevice.WriteOnly)
        s = QDataStream(buffer)
        s.writeQVariant(variant)

    def payload_variant(self):
        buffer = QBuffer()
        buffer.setData(self.payload)
        buffer.open(QIODevice.ReadOnly)
        s = QDataStream(buffer)
        return s.readQVariant()

