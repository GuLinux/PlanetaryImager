from .protocol import Protocol

class Hello:
    def __init__(self):
       self.send_protocol = Protocol('Network', 'Hello') 
       self.reply_protocol = Protocol('Network', 'HelloReply') 

    def send_packet(self):
        packet = self.send_protocol.packet()
        packet.set_payload({
            'format': 2,
            'compression': False,
            'force8bit': False,
            'jpegQuality': 10
        })
        return packet

    def reply_packet(self):
        return self.reply_protocol.packet()

