from .protocol import Protocol


class Camera:
    def __init__(self, camera_dict):
        self.name = camera_dict['n']
        self.address = camera_dict['a']

    def __str__(self):
        return '{} [{}]'.format(self.name, self.address)

    def __repr__(self):
        return self.__str__()



class DriverProtocol:
    AREA = 'Driver'
    PACKET_CAMERA_LIST = Protocol(AREA, 'CameraList')
    REPLY_CAMERA_LIST = Protocol(AREA, 'CameraListReply')
    PACKET_CONNECT_CAMERA = Protocol(AREA, 'ConnectCamera')
    REPLY_CONNECT_CAMERA = Protocol(AREA, 'ConnectCameraReply')
    PACKET_CLOSE_CAMERA = Protocol(AREA, 'CloseCamera')
    SIGNAL_DISCONNECTED = Protocol(AREA, 'signalDisconnected')

    @classmethod
    def camera_list(cls, client):
        return [Camera(x) for x in Protocol.round_trip_variant(client, cls.PACKET_CAMERA_LIST.packet(), cls.REPLY_CAMERA_LIST)]

    @classmethod
    def connect_camera(cls, client, camera):
        Protocol.round_trip(client, cls.PACKET_CONNECT_CAMERA.packet(variant=camera.address), cls.REPLY_CONNECT_CAMERA)
    
    @classmethod
    def close_camera(cls, client):
        Protocol.round_trip(client, cls.PACKET_CLOSE_CAMERA.packet(), cls.SIGNAL_DISCONNECTED)
