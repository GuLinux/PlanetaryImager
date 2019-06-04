import shutil
from dockerfile import Dockerfile

class Windows(Dockerfile):
    def __init__(self, arch, linktype='static'):
        self.arch = arch
        name = 'windows-{}'.format(arch)
        mxe_prefix = '{}-w64-mingw32.{}'.format(arch, linktype)
        configuration_file = 'configuration-windows-{}-{}.cmake'.format(arch, linktype)
        self.cmake_binary = mxe_prefix + '-cmake'
        self.os_family = 'windows'

        substitutions = {
            'CMAKE_BIN': self.cmake_binary,
            'MXE_PREFIX': mxe_prefix,
            'ARCH': arch,
            'LINKTYPE': linktype,
            'CMAKE_CACHE_INIT': configuration_file,
            'PACKAGE_SYSTEM_NAME': 'Windows-' + arch,
        }
        super().__init__(name, ['windows'], [], substitutions)

