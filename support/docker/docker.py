#!/usr/bin/env python3
from debian import Debian, Ubuntu
from fedora import Fedora
from windows import Windows
import sys


images = [
    Ubuntu('19.04', 'x86_64'),
    Ubuntu('19.04', 'arm32v7'),
    Ubuntu('18.04', 'x86_64'),
    Ubuntu('18.04', 'arm32v7'),
    Ubuntu('16.04', 'x86_64'),
    Ubuntu('16.04', 'arm32v7'),
    Debian('testing', 'x86_64'),
    Debian('testing', 'arm32v7'),
    Fedora('27', 'x86_64'),
    Windows('x86_64', 'static'),
]


for image in images:
    image.write()
print('Dockerfiles generated in images/\n')

if len(sys.argv) > 1:
    if sys.argv[1] == 'build':
        for image in images:
            image.build()
        print('\nImages build report:\n')
        build_success = True
        for image in images:
            build_success &= image.report_build()
        if not build_success:
            print('Some images could not be built. Please check the logs directory')
            sys.exit(1)
    elif sys.argv[1] == 'list':
        for image in images:
            print(image.image_name)
    else:
        print('''
Usage: {} [<command>]
By default, without commands, it only builds docker images.

Available commands:

    - build: also build docker images
    - list: list docker image names
'''.format(sys.argv[0]))
