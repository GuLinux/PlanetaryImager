from .protocol import *

@protocol(area='Network', packets=['Hello', 'HelloReply', 'ping', 'pong'])
class StatusProtocol:
    def hello(self):
        packet = self.packet_hello.packet(variant={
            'format': 2,
            'compression': False,
            'force8bit': False,
            'jpegQuality': 10
        })

        return self.client.round_trip(packet, self.packet_helloreply).named_tuple

    def ping(self):
        reply = self.client.round_trip(self.packet_ping.packet(), self.packet_pong)

