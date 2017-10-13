from .network import Client, DriverProtocol, StatusProtocol
from .configuration import Configuration
from .capture import Capture

class PlanetaryImagerClient:
    def __init__(self, address, port=19232):
        self.client = Client(address, port)
        self.imager_running = False
        self.configuration = Configuration(self.client)
        self.capture = Capture(self.client)

    def connect(self):
        self.client.connect()
        server_status = StatusProtocol.hello(self.client)
        self.imager_running = server_status.imager_running

    def disconnect(self):
        self.client.disconnect()

    @property
    def cameras(self):
        return DriverProtocol.camera_list(self.client)

    def ping(self):
        return self.client.round_trip(Ping.send(), Ping.REPLY)

    @property
    def status(self):
        return {
            'connected': self.client.connected,
            'imager_running': self.imager_running,
        }

    @property
    def imager(self):
        # TODO return instance of future Imager class
        return None

    @imager.setter
    def imager(self, camera):
        if not camera:
            DriverProtocol.close_camera(self.client)
        else:
            DriverProtocol.connect_camera(self.client, camera)
            self.imager_running = True

