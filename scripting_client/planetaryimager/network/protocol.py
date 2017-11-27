from . import NetworkPacket
from functools import wraps


class Protocol:
    def __init__(self, area, name):
        self.area = area
        self.name = name

    @property
    def packet_name(self):
        return '{}_{}'.format(self.area, self.name)

    def packet(self, payload=None, variant=None):
        packet = NetworkPacket(self.packet_name)
        if payload is not None:
            packet.payload = payload
        if variant is not None:
            packet.variant = variant
        return packet

    def check(self, packet):
        if not packet.name == self.packet_name:
            raise RuntimeError('Unknown packet: expecting {}, got {}'.format(self.packet_name, packet.name))
        return packet

    @classmethod
    def send(cls, client, packet_send):
        client.send(packet_send)

    @classmethod
    def register_packet_handler(cls, client, expected, callback):
        client.add_handler(callback, packet=expected) 


def protocol(area, packets=[]):
    def protocol_decorator(f):
        @wraps(f)
        def wrap(client):
            instance = f()
            instance.client = client
            instance.area = area
            for packet in packets:
                setattr(instance, 'packet_' + packet.lower(), Protocol(area, packet))
            return instance
        return wrap
    return protocol_decorator

