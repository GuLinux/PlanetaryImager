from .network import DriverProtocol

def check_connection(f):
    def wrap(*args, **kwargs):
        if not args[0].is_running:
            raise RuntimeError('Imager not running')
        return f(*args, **kwargs)
    return wrap


class Imager:
    def __init__(self, client):
        self.client = client
        self.is_running = False
        self.fps = None
        self.temperature = None
        DriverProtocol.on_signal_fps(self.client, self.__on_fps)
        DriverProtocol.on_signal_temperature(self.client, self.__on_temperature)
        DriverProtocol.on_control_changed(self.client, self.__on_control_changed)
        DriverProtocol.on_camera_connected(self.client, self.__on_camera_connected)
        DriverProtocol.on_camera_disconnected(self.client, self.__on_camera_disconnected)
        self.callbacks = {}

    def open(self, camera):
        DriverProtocol.connect_camera(self.client, camera)

    def close(self):
        DriverProtocol.close_camera(self.client)

    @property
    @check_connection
    def name(self):
        return DriverProtocol.get_camera_name(self.client)

    @property
    @check_connection
    def controls(self):
        return DriverProtocol.get_controls(self.client)

    @property
    @check_connection
    def properties(self):
        return DriverProtocol.get_properties(self.client)

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
