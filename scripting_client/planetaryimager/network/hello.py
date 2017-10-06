from .protocol import Protocol

class Hello:

    @staticmethod
    def send_packet():
        send_protocol = Protocol('Network', 'Hello') 
        packet = send_protocol.packet()
        packet.set_payload({
            'format': 2,
            'compression': False,
            'force8bit': False,
            'jpegQuality': 10
        })
        return packet

    @staticmethod
    def reply_packet():
       reply_protocol = Protocol('Network', 'HelloReply') 
       return reply_protocol.packet()

