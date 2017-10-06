from .network import Client, Hello

class PlanetaryImagerClient:
    def __init__(self, address, port=19232):
        self.client = Client(address, port)

    def connect(self):
        self.client.connect()
        rec = self.client.send_receive(Hello.send_packet())
        return rec

    
