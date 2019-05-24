import shutil
from dockerfile import Dockerfile


class LinuxBase(Dockerfile):
    def __init__(self, snippet, flavour, version, arch, extra_files=[], extra_snippets = [], extra_substitutions={}):
        self.flavour = flavour
        self.version = version
        self.arch = arch
        name = '{}-{}-{}'.format(flavour, version, arch)
        base_image = '{}:{}'.format(flavour, version)
        config_name = '{}-{}'.format(flavour, version)

        if arch != 'x86_64':
            base_image = arch + '/' + base_image

        configuration_file = 'configuration-{}.cmake'.format(config_name)

        substitutions = {
            'CONFIG_NAME': config_name,
            'BASE_IMAGE': base_image,
            'CMAKE_BIN': 'cmake',
            'CMAKE_CACHE_INIT': configuration_file,
        }
        substitutions.update(extra_substitutions)

        snippets = [snippet]
        snippets.extend(extra_snippets)

        qemu_arm_static = shutil.which('qemu-arm-static')
        if qemu_arm_static is None:
            raise RuntimeError('qemu-arm-static not found in PATH')
 

        files = [
            'files/' + configuration_file,
            'files/configuration-linux.cmake',
            qemu_arm_static,
        ]
        files.extend(extra_files)
        super().__init__(name, snippets, files, substitutions)
 

