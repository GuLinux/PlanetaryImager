from .protocol import Protocol
import collections

NETWORK = 'Network'

class Ping:
    PACKET=Protocol(NETWORK, 'ping')
    REPLY=Protocol(NETWORK, 'pong')
    
    @staticmethod
    def send():
        return Ping.PACKET.packet()

    @staticmethod
    def reply(packet):
        Ping.REPLY.check(packet)

