import threading
import time

class Interval:
    def __init__(self, daemon=False):
        self.timer = None
        self.daemon = daemon

    def start(self, secs, f):
        self.f = f
        self.secs = secs
        self.timer = threading.Timer(secs, self.__on_hit)
        self.timer.daemon = self.daemon
        self.timer.start()

    def stop(self):
        if self.timer:
            self.timer.cancel()
        self.timer = None

    def __on_hit(self):
        self.f()
        self.start(self.secs, self.f)

