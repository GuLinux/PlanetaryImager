from .network import *
from .configuration import Configuration

class PlanetaryImagerClient:
    def __init__(self, address, port=19232):
        self.client = Client(address, port)
        self.imager_running = False
        self.configuration = Configuration(self.client)

    def connect(self):
        self.client.connect()
        rec = Hello.reply(self.client.round_trip(Hello.send(), Hello.REPLY))
        self.imager_running = rec.imager_running

    def disconnect(self):
        self.client.disconnect()

    def cameras(self):
        return CameraListProtocol.reply(self.client.round_trip(CameraListProtocol.send(), CameraListProtocol.REPLY))

    def ping(self):
        return self.client.round_trip(Ping.send(), Ping.REPLY)

    def status(self):
        return {
            'connected': self.client.connected(),
            'imager_running': self.imager_running,
        }

    def connect_camera(self, camera):
        ConnectCameraProtocol.reply(self.client.round_trip(ConnectCameraProtocol.send(camera), ConnectCameraProtocol.REPLY))
        self.imager_running = True
        return True


