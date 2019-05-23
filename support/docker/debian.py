import shutil
from dockerfile import Dockerfile

class DebianBase(Dockerfile):
    def __init__(self, flavour, version, arch):
        self.version = version
        self.arch = arch
        name = '{}-{}-{}'.format(flavour, version, arch)
        base_image = '{}:{}'.format(flavour, version)
        if arch != 'x86_64':
            base_image = arch + '/' + base_image
        substitutions = {
            'DEB_CONFIG_NAME': '{}-{}'.format(flavour, version),
            'DEB_BASE_IMAGE': base_image,
            'CMAKE_BIN': 'cmake',
        }
        qemu_arm_static = shutil.which('qemu-arm-static')
        if qemu_arm_static is None:
            raise RuntimeError('qemu-arm-static not found in PATH')
        snippets = ['Debian-Based', 'libusb', 'workdir']
        files = [
            'files/debian-package-checker.sh',
            'files/configuration-{}-{}.cmake'.format(flavour, version),
            qemu_arm_static,
        ]
        super().__init__(name, snippets, files, substitutions)
 

class Debian(DebianBase):
    def __init__(self, version, arch):
        super().__init__('debian', version, arch)

class Ubuntu(DebianBase):
    def __init__(self, version, arch):
        super().__init__('ubuntu', version, arch)

