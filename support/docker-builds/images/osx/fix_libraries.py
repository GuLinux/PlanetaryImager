import os
import sys
import subprocess
import argparse
import shutil

parser = argparse.ArgumentParser(description='Fix libraries path in an application/library')
parser.add_argument('--otool', default='otool')
parser.add_argument('--install-name-tool', default='install_name_tool')
parser.add_argument('--libs-dest', required=True)
#parser.add_argument('--libs-prefix', action='append')
parser.add_argument('file')

args = parser.parse_args()

fixed = []


#library_dirs = args.libs_prefix
#library_dirs.append('')
to_be_fixed = [args.file]




def dependencies(file):
    deps = subprocess.check_output([args.otool, '-L', file]).split('\n')[1:]
    deps = [x.split(' (')[0].lstrip() for x in deps]
    deps = [x for x in deps if not x.startswith('/System') and not x.startswith('/usr') and not x.startswith('@') and x]
    return deps

def handle_dependency(file):
    print('** fixing dependencies in {0}'.format(file))
    to_be_fixed.remove(file)
    if file in fixed:
        return

    deps = dependencies(file)
    for dep in deps:
        dep_filename = dep.split('/')[-1]
        print('*** changing {0} in {1}'.format(dep_filename, file))
        result = subprocess.call([args.install_name_tool, '-change', dep, '@executable_path/../lib/{0}'.format(dep_filename), file])
        dest = '{0}/{1}'.format(args.libs_dest, dep_filename)
        if os.path.isfile(dep) and not os.path.isfile(dest):
            print('**** copying {0} to {1}'.format(dep, dest))
            shutil.copyfile(dep, dest)
            to_be_fixed.append(dest)
        elif not os.path.isfile(dep):
            print('**** WARNING: dependency not found: {0}'.format(dep))

        fixed.append(file)
#                to_be_fixed.append('{0}/{1}'.format(library_dirs, dep))

while not len(to_be_fixed) == 0:
    for file in to_be_fixed:
        handle_dependency(file)

