from . import NetworkPacket

class Protocol:
    def __init__(self, area, name):
        self.area = area
        self.name = name

    def packet_name(self):
        return '{}_{}'.format(self.area, self.name)

    def packet(self):
        return NetworkPacket(self.packet_name())

