from .network import *

class Configuration:
    def __init__(self, client):
        self.client = client

    def list(self):
        return ConfigurationProtocol.list(self.client)

    def get(self, name):
        return ConfigurationProtocol.get(self.client, name)

    def set(self, name, value):
        ConfigurationProtocol.set(self.client, name, value)

    def reset(self, name):
        ConfigurationProtocol.reset(self.client, name)

