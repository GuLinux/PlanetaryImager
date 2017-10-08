from . import NetworkPacket
import collections

class Protocol:
    def __init__(self, area, name):
        self.area = area
        self.name = name

    def packet_name(self):
        return '{}_{}'.format(self.area, self.name)

    def packet(self):
        return NetworkPacket(self.packet_name())

    def check(self, packet):
        if not packet.name == self.packet_name():
            raise RuntimeError('Unknown packet: expecting {}, got {}'.format(self.packet_name(), packet.name))

    def named_tuple(self, packet):
        packet_dict = packet.payload_variant()
        classname = collections.namedtuple(self.name, list(packet_dict.keys()))
        return classname(**packet.payload_variant())
