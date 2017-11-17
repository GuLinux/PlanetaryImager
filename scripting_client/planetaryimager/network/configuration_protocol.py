from .protocol import *


@protocol(area='Configuration', packets=['List', 'ListReply', 'Get', 'GetReply', 'Set', 'Reset', 'signalSettingsChanged'])
class ConfigurationProtocol:

    def list(self):
        return self.client.round_trip(self.packet_list.packet(), self.packet_listreply).variant

    def get(self, name):
        return self.client.round_trip(self.packet_get.packet(variant=name), self.packet_getreply).variant

    def set(self, name, value):
        self.client.send(self.packet_set.packet(variant={'name': name, 'value': value}))

    def reset(self, name):
        self.client.send(self.packet_reset.packet(variant=name))

    def on_signal_settings_changed(self, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(self.client, self.signalsettingschanged, dispatch)
