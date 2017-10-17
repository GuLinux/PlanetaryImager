from .protocol import Protocol


class ConfigurationProtocol:
    AREA = 'Configuration'
    PACKET_LIST = Protocol(AREA, 'List')
    REPLY_LIST = Protocol(AREA, 'ListReply')
    PACKET_GET = Protocol(AREA, 'Get')
    REPLY_GET = Protocol(AREA, 'GetReply')
    PACKET_SET = Protocol(AREA, 'Set')
    PACKET_RESET = Protocol(AREA, 'Reset')
    SIGNAL_SETTINGS_CHANGED = Protocol(AREA, 'signalSettingsChanged')

    @classmethod
    def list(cls, client):
        return Protocol.round_trip_variant(client, cls.PACKET_LIST.packet(), cls.REPLY_LIST)

    @classmethod
    def get(cls, client, name):
        return Protocol.round_trip_variant(client, cls.PACKET_GET.packet(variant=name), cls.REPLY_GET)

    @classmethod
    def set(cls, client, name, value):
        Protocol.send(client, cls.PACKET_SET.packet(variant={'name': name, 'value': value}))

    @classmethod
    def reset(cls, client, name):
        Protocol.send(client, cls.PACKET_RESET.packet(variant=name))

    @classmethod
    def on_signal_settings_changed(cls, client, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(client, cls.SIGNAL_SETTINGS_CHANGED, dispatch)
