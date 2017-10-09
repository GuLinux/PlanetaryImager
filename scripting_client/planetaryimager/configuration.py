from .network import *

class Configuration:
    def __init__(self, client):
        self.client = client

    def list(self):
        return ListConfigurationProtocol.reply(self.client.round_trip(ListConfigurationProtocol.send(), ListConfigurationProtocol.REPLY))

    def get(self, name):
        return GetConfigurationProtocol.reply(self.client.round_trip(GetConfigurationProtocol.send(name), GetConfigurationProtocol.REPLY))

    def set(self, name, value):
        self.client.send(SetConfigurationProtocol.send(name, value))

    def reset(self, name, value):
        self.client.send(ResetConfigurationProtocol.send(name))

