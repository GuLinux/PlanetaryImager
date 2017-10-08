import threading
import time

class Interval:
    def __init__(self):
        self.timer = None

    def start(self, secs, f):
        self.f = f
        self.secs = secs
        self.timer = threading.Timer(secs, self.__on_hit)
        self.timer.start()

    def stop(self):
        if self.timer:
            self.timer.cancel()
        self.timer = None

    def __on_hit(self):
        self.f()
        self.start(self.secs, self.f)

