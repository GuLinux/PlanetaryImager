from .network import *

class PlanetaryImagerClient:
    def __init__(self, address, port=19232):
        self.client = Client(address, port)
        self.connected = False
        self.imager_running = False

    def connect(self):
        self.client.connect()
        rec = Hello.reply(self.client.round_trip(Hello.send(), Hello.REPLY))
        self.connected = True
        self.imager_running = rec.imager_running

    def disconnect(self):
        if not self.__check_connection():
            return

        self.client.disconnect()
        self.connected = False

    def cameras(self):
        self.__check_connection()
        return CameraListProtocol.reply(self.client.round_trip(CameraListProtocol.send(), CameraListProtocol.REPLY))

    def ping(self):
        self.__check_connection()
        return self.client.round_trip(Ping.send(), Ping.REPLY)

    def status(self):
        return {
            'connected': self.connected,
            'imager_running': self.imager_running,
        }

    def connect_camera(self, camera):
        self.__check_connection()
        ConnectCameraProtocol.reply(self.client.round_trip(ConnectCameraProtocol.send(camera), ConnectCameraProtocol.REPLY))
        self.imager_running = True
        return True


    def __check_connection(self, exception=True):
        if not self.connected and exception:
            raise RuntimeError('Not connected')
        return self.connected
