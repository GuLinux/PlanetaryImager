from .protocol import Protocol

CONFIGURATION = 'Configuration'
    
class ListConfigurationProtocol:
    PACKET = Protocol(CONFIGURATION, 'List')
    REPLY = Protocol(CONFIGURATION, 'ListReply')

    @staticmethod
    def send():
        return ListConfigurationProtocol.PACKET.packet()

    @staticmethod
    def reply(packet):
        ListConfigurationProtocol.REPLY.check(packet)
        return packet.payload_variant()


class GetConfigurationProtocol:
    PACKET = Protocol(CONFIGURATION, 'Get')
    REPLY = Protocol(CONFIGURATION, 'GetReply')

    @staticmethod
    def send(name):
        return GetConfigurationProtocol.PACKET.packet().set_payload(name)

    @staticmethod
    def reply(packet):
        GetConfigurationProtocol.REPLY.check(packet)
        return packet.payload_variant()


class SetConfigurationProtocol:
    PACKET = Protocol(CONFIGURATION, 'Set')

    @staticmethod
    def send(name, value):
        return SetConfigurationProtocol.PACKET.packet().set_payload({'name': name, 'value': value})

class ResetConfigurationProtocol:
    PACKET = Protocol(CONFIGURATION, 'Reset')

    @staticmethod
    def send(name):
        return ResetConfigurationProtocol.PACKET.packet().set_payload(name)




