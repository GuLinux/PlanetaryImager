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
        return packet

    def named_tuple(self, packet):
        packet_dict = packet.variant
        classname = collections.namedtuple(self.name, list(packet_dict.keys()))
        return classname(**packet.variant)


    @classmethod
    def send(cls, client, packet_send):
        client.send(packet_send)

    @classmethod
    def round_trip_tuple(cls, client, packet_send, expected_reply):
        return expected_reply.named_tuple(cls.round_trip(client, packet_send, expected_reply))

    @classmethod
    def round_trip_variant(cls, client, packet_send, expected_reply):
        return cls.round_trip(client, packet_send, expected_reply).variant
   
    @classmethod
    def round_trip(cls, client, packet_send, expected_reply):
        reply = client.round_trip(packet_send, expected_reply)
        return expected_reply.check(reply)

    @classmethod
    def register_packet_handler(cls, client, expected, callback):
        client.add_handler(callback, packet=expected) 
