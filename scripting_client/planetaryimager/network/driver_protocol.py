from .protocol import Protocol


class Camera:
    def __init__(self, camera_dict):
        self.name = camera_dict['n']
        self.address = camera_dict['a']

    def __str__(self):
        return '{} [{}]'.format(self.name, self.address)

    def __repr__(self):
        return self.__str__()


# ADD_PROTOCOL_PACKET_NAME(StartLive)
# ADD_PROTOCOL_PACKET_NAME(StartLiveReply)
# ADD_PROTOCOL_PACKET_NAME(ClearROI)
# ADD_PROTOCOL_PACKET_NAME(SendFrame)
# ADD_PROTOCOL_PACKET_NAME(SetControl)
# ADD_PROTOCOL_PACKET_NAME(SetROI)

class DriverProtocol:
    AREA = 'Driver'
    PACKET_CAMERA_LIST = Protocol(AREA, 'CameraList')
    REPLY_CAMERA_LIST = Protocol(AREA, 'CameraListReply')
    PACKET_CAMERA_NAME = Protocol(AREA, 'GetCameraName')
    REPLY_CAMERA_NAME = Protocol(AREA, 'GetCameraNameReply')
    PACKET_CONNECT_CAMERA = Protocol(AREA, 'ConnectCamera')
    REPLY_CONNECT_CAMERA = Protocol(AREA, 'ConnectCameraReply')
    PACKET_CLOSE_CAMERA = Protocol(AREA, 'CloseCamera')
    SIGNAL_DISCONNECTED = Protocol(AREA, 'signalDisconnected')
    SIGNAL_CONNECTED = Protocol(AREA, 'signalCameraConnected')
    SIGNAL_FPS = Protocol(AREA, 'signalFPS')
    SIGNAL_TEMPERATURE = Protocol(AREA, 'signalTemperature')
    SIGNAL_CONTROL_CHANGED = Protocol(AREA, 'signalControlChanged')
    PACKET_CONTROLS = Protocol(AREA, 'GetControls')
    PACKET_CONTROLS_REPLY = Protocol(AREA, 'GetControlsReply')
    PACKET_PROPERTIES = Protocol(AREA, 'GetProperties')
    PACKET_PROPERTIES_REPLY = Protocol(AREA, 'GetPropertiesReply')

    @classmethod
    def camera_list(cls, client):
        return [Camera(x) for x in Protocol.round_trip_variant(client, cls.PACKET_CAMERA_LIST.packet(), cls.REPLY_CAMERA_LIST)]

    @classmethod
    def connect_camera(cls, client, camera):
        Protocol.send(client, cls.PACKET_CONNECT_CAMERA.packet(variant=camera.address))

    @classmethod
    def close_camera(cls, client):
        Protocol.send(client, cls.PACKET_CLOSE_CAMERA.packet())

    @classmethod
    def get_camera_name(cls, client):
        return Protocol.round_trip_variant(client, cls.PACKET_CAMERA_NAME.packet(), cls.REPLY_CAMERA_NAME)

    @classmethod
    def get_controls(cls, client):
        return Protocol.round_trip_variant(client, cls.PACKET_CONTROLS.packet(), cls.PACKET_CONTROLS_REPLY)

    @classmethod
    def get_properties(cls, client):
        return Protocol.round_trip_variant(client, cls.PACKET_PROPERTIES.packet(), cls.PACKET_PROPERTIES_REPLY)

    @classmethod
    def on_signal_fps(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_FPS, dispatch)

    @classmethod
    def on_camera_connected(cls, client, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(client, cls.SIGNAL_CONNECTED, dispatch)

    @classmethod
    def on_camera_disconnected(cls, client, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(client, cls.SIGNAL_DISCONNECTED, dispatch)

    @classmethod
    def on_signal_temperature(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_TEMPERATURE, dispatch)

    @classmethod
    def on_control_changed(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_CONTROL_CHANGED, dispatch)
