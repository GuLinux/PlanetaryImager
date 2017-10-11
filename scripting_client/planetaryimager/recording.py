from .network import SaveProtocol

class Recording:
    def __init__(self, client):
        self.client = client
        self.recording = False
        self.__recording_filename = None
        SaveProtocol.on_signal_recording(client, self.__handle_signal_start_recording)
        SaveProtocol.on_signal_end_recording(client, self.__handle_signal_end_recording)

    def start_recording(self):
        SaveProtocol.start_recording(self.client)

    def end_recording(self):
        SaveProtocol.end_recording(self.client)

    @property
    def recording_filename(self):
        return self.__recording_filename

    def __handle_signal_start_recording(self, filename):
        self.recording = True
        self.__recording_filename = filename

    def __handle_signal_end_recording(self):
        self.__recording_filename = None
        self.recording = False
