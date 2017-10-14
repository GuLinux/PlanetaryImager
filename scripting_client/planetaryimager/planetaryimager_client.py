""" Planetary Imager Scripting client

This module allows you to control Planetary Imager with a simple Python API interface.
"""
from .network import Client, DriverProtocol, StatusProtocol
from .configuration import Configuration
from .capture import Capture


class PlanetaryImagerClient:
    def __init__(self, address='localhost', port=19232, autoconnect=True):
        """ Create a PlanetaryImager client.

        :param address: hostname of the machine running Planetary Imager (default: localhost).
        :param port: TCP port for Planetary Imager connection (default: 19232).
        :param autoconnect: set this to True to automatically connect on creation (default: True).
        """
        self.client = Client(address, port)
        self.imager_running = False
        self.configuration = Configuration(self.client)
        self.capture = Capture(self.client)
        if autoconnect:
            self.connect()

    def connect(self):
        """ Connect to PlanetaryImager instance."""
        self.client.connect()
        server_status = StatusProtocol.hello(self.client)
        self.imager_running = server_status.imager_running

    def disconnect(self):
        self.client.disconnect()

    @property
    def cameras(self):
        return DriverProtocol.camera_list(self.client)

    @property
    def status(self):
        return {
            'connected': self.client.connected,
            'imager_running': self.imager_running,
        }

    @property
    def imager(self):
        # TODO return instance of future Imager class
        return None

    @imager.setter
    def imager(self, camera):
        if not camera:
            DriverProtocol.close_camera(self.client)
        else:
            DriverProtocol.connect_camera(self.client, camera)
            self.imager_running = True

