from .network import SaveProtocol

class Capture:
    def __init__(self, client):
        self.client = client

        self.on_recording_started = None
        """Optional callback function invoked when recording starts."""
        self.on_recording_finished = None
        """Optional callback function invoked when recording ends."""

        SaveProtocol.on_signal_recording(client, self.__handle_signal_start_recording)
        SaveProtocol.on_signal_end_recording(client, self.__handle_signal_end_recording)

        self.__reset()

    def start_recording(self):
        SaveProtocol.start_recording(self.client)

    def end_recording(self):
        SaveProtocol.end_recording(self.client)

    def pause(self):
        SaveProtocol.set_paused(self.client, True)

    def resume(self):
        SaveProtocol.set_paused(self.client, False)

    @property
    def recording_filename(self):
        return self.__recording_filename

    def __handle_signal_start_recording(self, filename):
        self.is_recording = True
        self.__recording_filename = filename
        if callable(self.on_recording_started):
            self.on_recording_started()

    def __handle_signal_end_recording(self):
        self.__recording_filename = None
        self.is_recording = False
        if callable(self.on_recording_finished):
            self.on_recording_finished()

    def __reset(self):
        self.is_recording = False
        self.__recording_filename = None

