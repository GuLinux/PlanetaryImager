from .protocol import *


@protocol(area='SaveFile', packets=['StartRecording', 'EndRecording', 'signalRecording', 'signalFinished',
                                    'slotSetPaused', 'signalSaveFPS', 'signalMeanFPS', 'signalSavedFrames',
                                    'signalDroppedFrames'])
class SaveProtocol:

    def start_recording(self):
        self.client.send(self.packet_startrecording.packet())

    def on_signal_recording(self, callback):
        def dispatch(packet):
            callback(packet.variant)
        Protocol.register_packet_handler(self.client, self.packet_signalrecording, dispatch)

    def on_signal_save_fps(self, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(self.client, self.packet_signalsavefps, dispatch)

    def on_signal_mean_fps(self, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(self.client, self.packet_signalmeanfps, dispatch)

    def on_signal_saved_frames(self, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(self.client, self.packet_signalsavedframes, dispatch)

    def on_signal_dropped_frames(self, callback):
        def dispatch(packet): callback(packet.variant)
        Protocol.register_packet_handler(self.client, self.packet_signaldroppedframes, dispatch)

    def end_recording(self):
        self.client.send(self.packet_endrecording.packet())

    def on_signal_end_recording(self, callback):
        def dispatch(_): callback()
        Protocol.register_packet_handler(self.client, self.packet_signalfinished, dispatch)

    def set_paused(self, paused):
        self.client.send(self.packet_slotsetpaused.packet(variant=paused))

