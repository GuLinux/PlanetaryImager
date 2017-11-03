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
        self.client = client

        self.__on_recording_started = None
        self.__on_recording_finished = None
        self.__on_saved_frames = None
        self.__on_dropped_frames = None
        self.__on_save_mean_fps = None
        self.__on_save_fps = None

        self.__saveprotocol = SaveProtocol(self.client)

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

    def on_recording_started(self, callback):
        """Sets a callback function to be invoked when recording starts.

        :param callback: function receiving `filename` as parameter. Set to None to remove the current callback
        Callback signature: function(filename)
        """
        self.__on_recording_started = callback

    def on_recording_finished(self, callback):
        """Sets a callback function to be invoked when recording ends.

        :param callback: function with no parameters. Set to None to remove the current callback
        Callback signature: function()
        """
        self.__on_recording_finished = callback

    def on_save_mean_fps(self, callback):
        """Sets a callback function to be invoked when receiving mean save fps info.

        :param callback: function receiving mean fps (float) as parameter. Set to None to remove the current callback
        """
        self.__on_save_mean_fps = callback

    def on_save_fps(self, callback):
        """Sets a callback function to be invoked when receiving save fps info.

        :param callback: function receiving save fps (float) as parameter. Set to None to remove the current callback
        """
        self.__on_save_fps = callback

    def on_saved_frames(self, callback):
        """Sets a callback function to be invoked when receiving saved frames info.

        :param callback: function receiving saved frames (int) as parameter. Set to None to remove the current callback
        """
        self.__on_saved_frames = callback

    def on_dropped_frames(self, callback):
        """Sets a callback function to be invoked when receiving dropped frames info.

        :param callback: function receiving dropped frames (int) as parameter. Set to None to remove the current callback
        """
        self.__on_dropped_frames = callback

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
        if callable(self.__on_recording_started):
            self.__on_recording_started(filename)

    def __handle_signal_end_recording(self):
        self.is_recording = False
        self.__recording_filename = None
        if callable(self.__on_recording_finished):
            self.__on_recording_finished()

    def __handle_mean_fps(self, fps):
        self.mean_save_fps = fps
        if callable(self.__on_save_mean_fps):
            self.__on_save_mean_fps(fps)

    def __handle_save_fps(self, fps):
        self.save_fps = fps
        if callable(self.__on_save_fps):
            self.__on_save_fps(fps)

    def __handle_saved_frames(self, frames):
        self.saved_frames = frames
        if callable(self.__on_saved_frames):
            self.__on_saved_frames(frames)

    def __handle_dropped_frames(self, frames):
        self.dropped_frames = frames
        if callable(self.__on_dropped_frames):
            self.__on_dropped_frames(frames)
