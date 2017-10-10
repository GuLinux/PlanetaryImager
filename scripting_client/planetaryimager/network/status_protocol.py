from .protocol import Protocol

class StatusProtocol:
    NETWORK = 'Network'
    PACKET_HELLO = Protocol(NETWORK, 'Hello')
    REPLY_HELLO = Protocol(NETWORK, 'HelloReply')
    PACKET_PING = Protocol(NETWORK, 'ping')
    REPLY_PONG = Protocol(NETWORK, 'pong')
 
    @classmethod
    def hello(cls, client):
        packet = cls.PACKET_HELLO.packet().set_payload({
            'format': 2,
            'compression': False,
            'force8bit': False,
            'jpegQuality': 10
        })

        return Protocol.round_trip_tuple(client, packet, cls.REPLY_HELLO)

    @classmethod
    def ping(cls, client):
        Protocol.round_trip(client, cls.PACKET_PING.packet(), cls.REPLY_PONG)

