import shutil
from dockerfile import Dockerfile

class Windows(Dockerfile):
    def __init__(self, arch):
        self.arch = arch
        name = 'windows-{}'.format(arch)
        mxe_prefix = 'x86_64-w64-mingw32.shared' if arch == 'x86_64' else  'i686-w64-mingw32.shared'
        substitutions = {
            'CMAKE_BIN': mxe_prefix + 'cmake',
            'MXE_PREFIX': mxe_prefix,
            'ARCH': arch,
        }
        snippets = ['windows']
        files = [
            'files/windows-deps-{}.cmake'.format(arch),
        ]
        super().__init__(name, snippets, files, substitutions)

