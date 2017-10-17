from .network import DriverProtocol


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
    def name(self):
        return DriverProtocol.get_camera_name(self.client) if self.is_running else None

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
