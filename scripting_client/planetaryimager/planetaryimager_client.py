""" Planetary Imager Scripting client

This module allows you to control Planetary Imager with a simple Python API interface.
"""
from .network import Client, DriverProtocol, StatusProtocol
from .configuration import Configuration
from .capture import Capture


class PlanetaryImagerClient:
    """Main entrypoint for PlanetaryImager scripting.

    This class allows you to interact with a running PlanetaryImager session.

    Main methods and properties:

     * connect: connect to the configured PlanetaryImager instance.
     * cameras: get a list of connected cameras.
     * imager: sets the current camera, and interact with it.
     * configuration: manage PlanetaryImager configuration.
     * capture: manage capturing to file.
     * status: get current status.
     * disconnect: detach from PlanetaryImager instance.
    """
    def __init__(self, address='localhost', port=19232, autoconnect=True):
        """Create a PlanetaryImager client.

        :param address: hostname of the machine running Planetary Imager (default: localhost).
        :param port: TCP port for Planetary Imager connection (default: 19232).
        :param autoconnect: set this to True to automatically connect on creation (default: True).
        """
        self.client = Client(address, port)
        self.__imager_running = False
        """PlanetaryImager configuration manager."""
        self.configuration = Configuration(self.client)
        """Start and stops capture, monitoring capture status"""
        self.capture = Capture(self.client)
        if autoconnect:
            self.connect()

    def connect(self):
        """Connect to PlanetaryImager instance."""
        self.client.connect()
        server_status = StatusProtocol.hello(self.client)
        self.__imager_running = server_status.imager_running

    def disconnect(self):
        """Disonnect from PlanetaryImager instance."""
        self.client.disconnect()

    @property
    def cameras(self):
        """List of cameras detected. They can be assigned to the `imager` property to set the current imager."""
        return DriverProtocol.camera_list(self.client)

    @property
    def status(self):
        """Report of current PlanetaryImager status"""
        return {
            'connected': self.client.connected,
            'imager_running': self.__imager_running,
        }

    @property
    def imager(self):
        # TODO return instance of future Imager class
        return None

    @imager.setter
    def imager(self, camera):
        """Set the current imager and start live framing.

        :param camera: camera instance from the `cameras` attribute.
        """
        if not camera:
            DriverProtocol.close_camera(self.client)
        else:
            DriverProtocol.connect_camera(self.client, camera)
            self.__imager_running = True

