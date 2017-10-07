from .network import Client, Hello, CameraList

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
        return self.client.send_receive(CameraList.send())


    def __check_connection(self, exception=True):
        if not self.connected and exception:
            raise RuntimeError('Not connected')
        return self.connected
