from .network import Client, Hello

class PlanetaryImagerClient:
    def __init__(self, address, port=19232):
        self.client = Client(address, port)
        self.connected = False

    def connect(self):
        self.client.connect()
        rec = self.client.send_receive(Hello.send_packet())
        if rec.name == 'Network_HelloReply':
            self.connected = True
            self.imager_running = rec.payload_variant()['imager_running']

    def disconnect(self):
        if not self.connected:
            return

        self.client.disconnect()
        self.connected = False

    
