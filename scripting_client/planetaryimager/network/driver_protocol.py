from .protocol import *


class Camera:
    def __init__(self, camera_dict):
        self.name = camera_dict['n']
        self.address = camera_dict['a']

    def __str__(self):
        return '{} [{}]'.format(self.name, self.address)

    def __repr__(self):
        return self.__str__()


# TODO: ClearROI, SendFrame, SetControl, SetROI
@protocol(area='Driver', packets=['CameraList', 'CameraListReply', 'GetCameraName', 'GetCameraNameReply', 'ConnectCamera', 'ConnectCameraReply', \
                                  'CloseCamera', 'signalDisconnected', 'signalCameraConnected', 'signalFPS', 'signalTemperature', 'signalControlChanged', \
                                  'GetControls', 'GetControlsReply', 'GetProperties', 'GetPropertiesReply', 'StartLive', 'StartLiveReply'])
class DriverProtocol:

    def camera_list(self):
        return [Camera(x) for x in self.client.round_trip(self.packet_cameralist.packet(), self.packet_cameralistreply).variant]

    def connect_camera(self, camera):
        self.client.send(self.packet_connectcamera.packet(variant=camera.address))

    def close_camera(self):
        self.client.send(self.packet_closecamera.packet())

    def get_camera_name(self):
        return self.client.round_trip(self.packet_getcameraname.packet(), self.packet_getcameranamereply).variant

    def get_controls(self):
        return self.client.round_trip(self.packet_getcontrols.packet(), self.packet_getcontrolsreply).variant

    def get_properties(self):
        return self.client.round_trip(self.packet_getproperties.packet(), self.packet_getpropertiesreply).variant

    def start_live(self):
        return self.client.round_trip(self.packet_startlive.packet(), self.packet_startlivereply)

    def on_signal_fps(self, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(self.client, self.packet_signalfps, dispatch)

    def on_camera_connected(self, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(self.client, self.packet_signalcameraconnected, dispatch)

    def on_camera_disconnected(self, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(self.client, self.packet_signaldisconnected, dispatch)

    def on_signal_temperature(self, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(self.client, self.packet_signaltemperature, dispatch)

    def on_control_changed(self, callback):
        def dispatch(packet): callback(packet.variant)
        dir(self)
        Protocol.register_packet_handler(self.client, self.packet_signalcontrolchanged, dispatch)

