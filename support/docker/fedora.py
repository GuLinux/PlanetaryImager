from linux import LinuxBase


class Fedora(LinuxBase):
    def __init__(self,  version, arch):
        super().__init__('fedora', 'fedora', version, arch)
 

