from .network import DriverProtocol


class Imager:
    def __init__(self, client):
        self.client = client
        self.is_running = False

    def open(self, camera):
        DriverProtocol.connect_camera(self.client, camera)
        self.is_running = True

    def close(self):
        DriverProtocol.close_camera(self.client)
        self.is_running = False


