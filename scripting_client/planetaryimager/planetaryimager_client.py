from .network import *

class PlanetaryImagerClient:
    def __init__(self, address, port=19232):
        self.client = Client(address, port)
        self.connected = False

    def connect(self):
        self.client.connect()
        rec = Hello.reply(self.client.send_receive(Hello.send()))
        self.connected = True
        self.imager_running = rec.imager_running

    def disconnect(self):
        if not self.__check_connection():
            return

        self.client.disconnect()
        self.connected = False

    def cameras(self):
        self.__check_connection()
        return CameraListProtocol.reply(self.client.send_receive(CameraListProtocol.send()))

    def ping(self):
        self.__check_connection()
        return self.client.send_receive(Ping.send())

    def status(self):
        return {
            'connected': self.connected,
            'imager_running': self.imager_running,
        }

    def connect_camera(self, camera):
        self.__check_connection()
        ConnectCameraProtocol.reply(self.client.send_receive(ConnectCameraProtocol.send(camera)))
        self.imager_running = True
        return True


    def __check_connection(self, exception=True):
        if not self.connected and exception:
            raise RuntimeError('Not connected')
        return self.connected
