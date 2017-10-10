from .protocol import Protocol


class ConfigurationProtocol:
    CONFIGURATION = 'Configuration'
    PACKET_LIST = Protocol(CONFIGURATION, 'List')
    REPLY_LIST = Protocol(CONFIGURATION, 'ListReply')
    PACKET_GET = Protocol(CONFIGURATION, 'Get')
    REPLY_GET = Protocol(CONFIGURATION, 'GetReply')
    PACKET_SET = Protocol(CONFIGURATION, 'Set')
    PACKET_RESET = Protocol(CONFIGURATION, 'Reset')

    @classmethod
    def list(cls, client):
        return Protocol.round_trip_variant(client, cls.PACKET_LIST.packet(), cls.REPLY_LIST)

    @classmethod
    def get(cls, client, name):
        return Protocol.round_trip_variant(client, cls.PACKET_GET.packet().set_payload(name), cls.REPLY_GET)

    @classmethod
    def set(cls, client, name, value):
        Protocol.send(client, cls.PACKET_SET.packet().set_payload({'name': name, 'value': value}))

    @classmethod
    def reset(cls, client, name):
        Protocol.send(client, cls.PACKET_RESET.packet().set_payload(name))

