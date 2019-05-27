from linux import LinuxBase
    

class DebianBase(LinuxBase):
    def __init__(self, flavour, version, arch):
        super().__init__('debian', flavour, version, arch, ['libusb'])
 

class Debian(DebianBase):
    def __init__(self, version, arch):
        super().__init__('debian', version, arch)

class Ubuntu(DebianBase):
    def __init__(self, version, arch):
        super().__init__('ubuntu', version, arch)

