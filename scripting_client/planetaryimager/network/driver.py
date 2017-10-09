from .protocol import Protocol

DRIVER = 'Driver'

class Camera:
    def __init__(self, camera_dict):
        self.name = camera_dict['n']
        self.address = camera_dict['a']

    def __str__(self):
        return '{} [{}]'.format(self.name, self.address)

    def __repr__(self):
        return self.__str__()


class CameraListProtocol:
    PACKET = Protocol(DRIVER, 'CameraList')
    REPLY = Protocol(DRIVER, 'CameraListReply')

    @staticmethod
    def send():
        return CameraListProtocol.PACKET.packet()

    @staticmethod
    def reply(packet):
        CameraListProtocol.REPLY.check(packet)
        return [Camera(x) for x in packet.payload_variant()]

        
class ConnectCameraProtocol:
    PACKET = Protocol(DRIVER, 'ConnectCamera')
    REPLY = Protocol(DRIVER, 'ConnectCameraReply')

    @staticmethod
    def send(camera):
        return ConnectCameraProtocol.PACKET.packet().set_payload(camera.address)

    def reply(packet):
        ConnectCameraProtocol.REPLY.check(packet)

