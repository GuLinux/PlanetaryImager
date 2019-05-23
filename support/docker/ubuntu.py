import shutil
from dockerfile import Dockerfile


class Ubuntu(Dockerfile):
    def __init__(self, version, arch):
        self.version = version
        self.arch = arch
        name = 'ubuntu-{}-{}'.format(version, arch)
        base_image = 'ubuntu:' + version
        if arch != 'x86_64':
            base_image = arch + '/' + base_image
        substitutions = {
            'DEB_CONFIG_NAME': 'ubuntu-' + version,
            'DEB_BASE_IMAGE': base_image,
            'CMAKE_BIN': 'cmake',
        }
        qemu_arm_static = shutil.which('qemu-arm-static')
        if qemu_arm_static is None:
            raise RuntimeError('qemu-arm-static not found in PATH')
        snippets = ['Debian-Based', 'libusb', 'workdir']
        files = [
            'files/debian-package-checker.sh',
            'files/configuration-ubuntu-{}.cmake'.format(version),
            qemu_arm_static,
        ]
        super().__init__(name, snippets, files, substitutions)
 


