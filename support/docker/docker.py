#!/usr/bin/env python3
from debian import Debian, Ubuntu
from fedora import Fedora
from windows import Windows
from dockerfile import cleanup_logs

import sys
import argparse
import os

docker_path = os.path.abspath(os.path.dirname(__file__))
code_path = os.path.dirname(os.path.dirname(docker_path))
os.chdir(docker_path)

images = [
  Ubuntu('19.04', 'x86_64'),
  Ubuntu('19.04', 'arm32v7'),
  Ubuntu('18.04', 'x86_64'),
  Ubuntu('18.04', 'arm32v7'),
#  Ubuntu('16.04', 'x86_64'),
#  Ubuntu('16.04', 'arm32v7'),
  Debian('testing', 'x86_64'),
  Debian('testing', 'arm32v7'),
  Fedora('27', 'x86_64'),
  Windows('x86_64', 'static'),
]

def create_images():
  for image in images:
    image.write()
  print('Dockerfiles generated in images/\n')
  
def filter_images(filters):
  if not filters:
    return images
  def is_filtered_in(image):
    return any([filter in image.image_name for filter in filters])
  return [img for img in images if is_filtered_in(img)]
 
def build_images(args):
  if args.clean_logs:
    cleanup_logs()
  create_images()
  filtered_images = filter_images(args.images_filter)
  for image in filtered_images:
    image.build(stderr=args.stderr)
  print('\nImages build report:\n')
  build_success = True
  for image in filtered_images:
    build_success &= image.report_build()
  if not build_success:
    print('Some images could not be built. Please check the logs directory')
    sys.exit(1)

 
def list_images():
  for image in images:
    print(image.image_name)
    
    
def package(args):
  if args.clean_logs:
    cleanup_logs()
    
  destination_path = os.path.abspath(args.dest)
  filtered_images = filter_images(args.images_filter)
  cmake_defines = args.cmake_define
  cmake_defines.append('CMAKE_BUILD_TYPE=' + args.cmake_build_type)
  for image in filtered_images:
    image.package(code_path, destination_path, args.make_jobs, cmake_defines, stderr=args.stderr)
    
  print('\nPackages build report:\n')
  build_success = True
  for image in filtered_images:
    build_success &= image.report_build()
  if not build_success:
    print('Some packagescould not be built. Please check the logs directory')
    sys.exit(1)

  

parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers(help='Available actions')
parser_build = subparsers.add_parser('build', help='Build docker images')
parser_build.add_argument('-i', '--images-filter', action='append', default=[], help='Filter images by name (use multiple times if necessary)')
parser_build.add_argument('-c', '--clean-logs', action='store_true', default=False, help='Clean logs directory')
parser_build.add_argument('--stderr', action='store_true', default=False, help='Log to stderr instead of log file')
parser_build.set_defaults(action='build')
parser_list = subparsers.add_parser('list', help='List supported images')
parser_list.set_defaults(action='list')
parser_package = subparsers.add_parser('package', help='Run packager on built images')
parser_package.add_argument('-d', '--dest', required=True, help='Destination directory (i.e. where to put packages)')
parser_package.add_argument('--cmake-build-type', default='RelWithDebInfo', help='CMAKE_BUILD_TYPE')
parser_package.add_argument('-j', '--make-jobs', default=1, help='Make parallel jobs')
parser_package.add_argument('-D', '--cmake-define', action='append', default=[], help='CMake definitions to be passed to docker container (use multiple times if necessary)')
parser_package.add_argument('-i', '--images-filter', action='append', default=[], help='Filter images by name (use multiple times if necessary)')
parser_package.add_argument('-c', '--clean-logs', action='store_true', default=False, help='Clean logs directory')
parser_package.add_argument('--stderr', action='store_true', default=False, help='Log to stderr instead of log file')
parser_package.set_defaults(action='package')
args = parser.parse_args()

#print(args)

if not 'action' in args:
  parser.print_help()
  sys.exit(1)

if args.action == 'list':
  list_images()
elif args.action == 'build':
  build_images(args)
elif args.action == 'package':
  package(args)

