from .protocol import Protocol

class StatusProtocol:
    AREA = 'Network'
    PACKET_HELLO = Protocol(AREA, 'Hello')
    REPLY_HELLO = Protocol(AREA, 'HelloReply')
    PACKET_PING = Protocol(AREA, 'ping')
    REPLY_PONG = Protocol(AREA, 'pong')
 
    @classmethod
    def hello(cls, client):
        packet = cls.PACKET_HELLO.packet(variant={
            'format': 2,
            'compression': False,
            'force8bit': False,
            'jpegQuality': 10
        })

        return Protocol.round_trip_tuple(client, packet, cls.REPLY_HELLO)

    @classmethod
    def ping(cls, client):
        reply = Protocol.round_trip(client, cls.PACKET_PING.packet(), cls.REPLY_PONG)

