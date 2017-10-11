from .protocol import Protocol

class SaveProtocol:
    AREA = 'SaveFile'
    PACKET_START_RECORDING = Protocol(AREA, 'StartRecording')
    PACKET_END_RECORDING = Protocol(AREA, 'EndRecording')
    SIGNAL_RECORDING = Protocol(AREA, 'signalRecording')
    SIGNAL_FINISHED = Protocol(AREA, 'signalFinished')

    @classmethod
    def start_recording(cls, client):
        Protocol.send(client, cls.PACKET_START_RECORDING.packet())

    @classmethod
    def on_signal_recording(cls, client, callback):
        def dispatch(packet):
            callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_RECORDING, dispatch)

    @classmethod
    def end_recording(cls, client):
        Protocol.send(client,  cls.PACKET_END_RECORDING.packet())


    @classmethod
    def on_signal_end_recording(cls, client, callback):
        def dispatch(_):
            callback()
        Protocol.register_packet_handler(client, cls.SIGNAL_FINISHED, dispatch)
