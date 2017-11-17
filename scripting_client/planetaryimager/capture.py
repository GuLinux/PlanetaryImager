from .network import SaveProtocol

class Capture:
    def __init__(self, client):
        """PlanetaryImager capture manager.

        Handles start, stop and pause recording, and gets information about fps and saved/dropped frames.
        Can also be configured with callbacks to be executed when these events occur.

        :param client: network client to communicate with PlanetaryImager.
        """

        """Mean fps on saving frames."""
        self.mean_save_fps = 0
        """Current fps on saving frames."""
        self.save_fps = 0
        """Total frames saved."""
        self.saved_frames = 0
        """Total frames dropped."""
        self.dropped_frames = 0
        """Boolean flag to indicate if Planetary Imager is currently recording."""
        self.is_recording = False
        self.__recording_filename = None

        """Callbacks to be called when capturing events occur.

        Key: string (name of callback).
        Value: function.

        Supported callbacks:

         - on_recording_started: Sets a callback function to be invoked when recording starts.
           - Callback signature: function(filename)

        - on_recording_finished: Sets a callback function to be invoked when recording ends.
           - Callback signature: function()


        - on_save_fps: Sets a callback function to be invoked when receiving save fps info.
           - Callback signature: function(float)

        - on_save_mean_fps: Sets a callback function to be invoked when receiving mean save fps info.
           - Callback signature: function(float)

        - on_saved_frames: Sets a callback function to be invoked when receiving saved frames info.
           - Callbacks signature: function(int)

        - on_dropped_frames: Sets a callback function to be invoked when receiving dropped frames info.
           - Callbacks signature: function(int)
        """
        self.callbacks = {}

        self.__saveprotocol = SaveProtocol(client)

        self.__saveprotocol.on_signal_recording(self.__handle_signal_start_recording)
        self.__saveprotocol.on_signal_end_recording(self.__handle_signal_end_recording)
        self.__saveprotocol.on_signal_mean_fps(self.__handle_mean_fps)
        self.__saveprotocol.on_signal_save_fps(self.__handle_save_fps)
        self.__saveprotocol.on_signal_saved_frames(self.__handle_saved_frames)
        self.__saveprotocol.on_signal_dropped_frames(self.__handle_dropped_frames)

    def start_recording(self):
        self.__saveprotocol.start_recording()

    def end_recording(self):
        self.__saveprotocol.end_recording()

    def pause(self):
        self.__saveprotocol.set_paused(True)

    def resume(self):
        self.__saveprotocol.set_paused(False)

    @property
    def recording_filename(self):
        return self.__recording_filename

    def __handle_signal_start_recording(self, filename):
        self.is_recording = True
        self.mean_save_fps = 0
        self.save_fps = 0
        self.saved_frames = 0
        self.dropped_frames = 0

        self.__recording_filename = filename
        self.__invoke_callback('on_recording_started', filename)

    def __handle_signal_end_recording(self):
        self.is_recording = False
        self.__recording_filename = None
        self.__invoke_callback('on_recording_finished')

    def __handle_mean_fps(self, fps):
        self.mean_save_fps = fps
        self.__invoke_callback('on_save_mean_fps', fps)

    def __handle_save_fps(self, fps):
        self.save_fps = fps
        self.__invoke_callback('on_save_fps', fps)

    def __handle_saved_frames(self, frames):
        self.saved_frames = frames
        self.__invoke_callback('on_saved_frames', frames)

    def __handle_dropped_frames(self, frames):
        self.dropped_frames = frames
        self.__invoke_callback('on_dropped_frames', frames)

    def __invoke_callback(self, name, *args, **kwargs):
        if name in self.callbacks:
            self.callbacks[name](*args, **kwargs)

