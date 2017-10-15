from .protocol import Protocol
import functools


class SaveProtocol:
    AREA = 'SaveFile'
    PACKET_START_RECORDING = Protocol(AREA, 'StartRecording')
    PACKET_END_RECORDING = Protocol(AREA, 'EndRecording')
    SIGNAL_RECORDING = Protocol(AREA, 'signalRecording')
    SIGNAL_FINISHED = Protocol(AREA, 'signalFinished')
    PACKET_SET_PAUSED = Protocol(AREA, 'slotSetPaused')
    SIGNAL_SAVE_FPS = Protocol(AREA, 'signalSaveFPS')
    SIGNAL_MEAN_FPS = Protocol(AREA, 'signalMeanFPS')
    SIGNAL_SAVED_FRAMES = Protocol(AREA, 'signalSavedFrames')
    SIGNAL_DROPPED_FRAMES = Protocol(AREA, 'signalDroppedFrames')

    @classmethod
    def start_recording(cls, client):
        Protocol.send(client, cls.PACKET_START_RECORDING.packet())

    @classmethod
    def on_signal_recording(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_RECORDING, dispatch)

    @classmethod
    def on_signal_save_fps(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_SAVE_FPS, dispatch)

    @classmethod
    def on_signal_mean_fps(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_MEAN_FPS, dispatch)

    @classmethod
    def on_signal_saved_frames(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_SAVED_FRAMES, dispatch)

    @classmethod
    def on_signal_dropped_frames(cls, client, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(client, cls.SIGNAL_DROPPED_FRAMES, dispatch)

    @classmethod
    def end_recording(cls, client):
        Protocol.send(client, cls.PACKET_END_RECORDING.packet())

    @classmethod
    def on_signal_end_recording(cls, client, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(client, cls.SIGNAL_FINISHED, dispatch)

    @classmethod
    def set_paused(cls, client, paused):
        Protocol.send(client, cls.PACKET_SET_PAUSED.packet(variant=paused))

