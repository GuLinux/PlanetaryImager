from .protocol import Protocol
import collections

NETWORK = 'Network'

class Hello:
    PACKET=Protocol(NETWORK, 'Hello')
    REPLY=Protocol(NETWORK, 'HelloReply')
    
    @staticmethod
    def send():
        return Hello.PACKET.packet().set_payload({
            'format': 2,
            'compression': False,
            'force8bit': False,
            'jpegQuality': 10
        })

    @staticmethod
    def reply(packet):
        Hello.REPLY.check(packet)
        return Hello.REPLY.named_tuple(packet)

