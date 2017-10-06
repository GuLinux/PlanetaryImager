from PyQt5.QtNetwork import QTcpSocket
import time
from . import NetworkPacket

class Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.socket = None

    def connect(self):
        self.socket = QTcpSocket()
        self.socket.connectToHost(self.host, self.port)
        self.__wait_for(lambda: self.socket.state() == QTcpSocket.ConnectedState, 'connecting')

    def send_receive(self, packet):
        packet.send_to(self.socket)
        self.__wait_for(lambda: self.socket.bytesAvailable() > 0, 'data')
        received = NetworkPacket()
        received.receive_from(self.socket)
        return received

    def __wait_for(self, condition, operation_description, timeout=30):
        start = time.time()
        while not condition() and time.time() - start < timeout:
            time.sleep(0.1)
        if not condition():
            raise RuntimeError('Error while waiting for {}: {}'.format(operation_description, self.socket.errorString()))

