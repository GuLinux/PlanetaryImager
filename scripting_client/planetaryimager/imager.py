from .network import DriverProtocol

def check_connection(f):
    def wrap(*args, **kwargs):
        if not args[0].is_running:
            raise RuntimeError('Imager not running')
        return f(*args, **kwargs)
    return wrap


class Imager:
    """ Class representing a running camera

    This class exposes method to open a camera, manipulate controls, getting properties and setting callbacks.
    """
    def __init__(self, client):
        """ Create a new Imager object

        :param client: the PlanetaryImager remote client
        """
        self.is_running = False
        self.fps = None
        self.temperature = None
        self.driver_protocol = DriverProtocol(client)
        self.driver_protocol.on_signal_fps(self.__on_fps)
        self.driver_protocol.on_signal_temperature(self.__on_temperature)
        self.driver_protocol.on_control_changed(self.__on_control_changed)
        self.driver_protocol.on_camera_connected(self.__on_camera_connected)
        self.driver_protocol.on_camera_disconnected(self.__on_camera_disconnected)
        self.callbacks = {}

    def open(self, camera):
        """Open a camera, setting it as the current running camera in PlanetaryImager."""
        self.driver_protocol.connect_camera(camera)

    def close(self):
        """Closes the camera and clean up resources."""
        self.driver_protocol.close_camera()

    @check_connection
    def start_live(self):
        """Starts live streaming on the currently connected camera."""
        self.driver_protocol.start_live()

    @property
    @check_connection
    def name(self):
        """Current imager name (as in camera name)."""
        return self.driver_protocol.get_camera_name()

    @property
    @check_connection
    def controls(self):
        """Retrieve available controls from the camera (exposure, binning etc)."""
        return self.driver_protocol.get_controls()

    @property
    @check_connection
    def properties(self):
        """Retrieve camera properties, such as resolution, pixel size, etc."""
        return self.driver_protocol.get_properties()

    def __str__(self):
        try:
            return self.name
        except RuntimeError:
            return 'No imager connected'

    def __repr__(self):
        return self.__str__()

    def __on_fps(self, fps):
        self.fps = fps
        self.__run_callback('on_fps', fps)

    def __on_temperature(self, temperature):
        self.temperature = temperature
        self.__run_callback('on_temperature', temperature)

    def __on_control_changed(self, control):
        self.__run_callback('on_control_changed', control)

    def __on_camera_connected(self):
        self.is_running = True
        self.__run_callback('on_camera_connected')

    def __on_camera_disconnected(self):
        self.is_running = False
        self.fps = None
        self.temperature = None
        self.__run_callback('on_camera_disconnected')

    def __run_callback(self, name, *args, **kwargs):
        if name in self.callbacks:
            self.callbacks[name](*args, **kwargs)
