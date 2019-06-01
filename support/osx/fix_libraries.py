#!/usr/bin/env python3
import os
import sys
import subprocess
import argparse
import shutil

parser = argparse.ArgumentParser(description='Fix libraries path in an application/library')
parser.add_argument('--otool', default='otool')
parser.add_argument('--install-name-tool', default='install_name_tool')
parser.add_argument('files', nargs='*')

args = parser.parse_args()

brew_prefix = subprocess.check_output(['brew', '--prefix']).decode().strip()

def is_brew_dependency(library):
    if os.path.basename(library).startswith('Qt'):
        return False
    if library.startswith(brew_prefix):
        return True
    return False

def dependencies(file):
    deps = [
        x.strip().split(' (')[0] for x in
        subprocess.check_output([args.otool, '-L', file]).decode().split('\n')[1:]
        if x
    ]
    deps = [d for d in deps if is_brew_dependency(d)]
    return deps

def fix_dependencies(file):
    for library in dependencies(file):
        libname = os.path.basename(library)
        print('fixing library {}: add rpath for dependency {}'.format(file, libname))
        result = subprocess.call([args.install_name_tool, '-change', library, '@rpath/{0}'.format(libname), file])

files = args.files
if not files:
    print('Looking for libraries in {}'.format(os.getcwd()))
    files = [os.path.join(os.getcwd(), x) for x in os.listdir('.') if x.endswith('.dylib')]

for file in files:
    fix_dependencies(file)
