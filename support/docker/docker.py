#!/usr/bin/env python3
from debian import Debian, Ubuntu
from fedora import Fedora
from windows import Windows
from dockerfile import cleanup_logs
from travis import Travis

import sys
import argparse
import os

docker_path = os.path.abspath(os.path.dirname(__file__))
code_path = os.path.dirname(os.path.dirname(docker_path))

images = [
  Ubuntu('19.04', 'x86_64'),
  Ubuntu('19.04', 'arm32v7'),
  Ubuntu('18.04', 'x86_64'),
  Ubuntu('18.04', 'arm32v7'),
  Ubuntu('16.04', 'x86_64'),
  Ubuntu('16.04', 'arm32v7'),
  Debian('stretch', 'x86_64'),
  Debian('stretch', 'arm32v7'),
  Debian('buster', 'x86_64'),
  Debian('buster', 'arm32v7'),
  Fedora('29', 'x86_64'),
  Fedora('30', 'x86_64'),
  Fedora('31', 'x86_64'),
  Windows('x86_64', 'static'),
]

travis = Travis(images)

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
  destination_path = os.path.abspath(args.dest) if args.dest and args.dest != 'none' else None
  filtered_images = filter_images(args.images_filter)
  cmake_defines = args.cmake_define
  cmake_defines.append('CMAKE_BUILD_TYPE=' + args.cmake_build_type)
  for image in filtered_images:
    image.package(code_path, destination_path, args.make_jobs, cmake_defines, stderr=args.stderr, build_directory=args.build_directory, privileged=args.privileged)
    
  print('\nPackages build report:\n')
  build_success = True
  for image in filtered_images:
    build_success &= image.report_build()
  if not build_success:
    print('Some packagescould not be built. Please check the logs directory')
    sys.exit(1)

def push(args):
  for image in images:
    image.push()
  print('\nImages push report:\n')
  push_success = True
  for image in images:
    push_success &= image.report_build()
  if not push_success:
    print('Some images could not be pushed. Please check the logs')
    sys.exit(1)

def generate_travis(args):
  travis.generate(args)
  

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
parser_package.add_argument('-d', '--dest', required=True, help='Destination directory (i.e. where to put packages). Use `none` if you want to skip exporting packages.')
parser_package.add_argument('-b', '--build-directory', required=False, help='Bind build directory on docker')
parser_package.add_argument('--cmake-build-type', default='RelWithDebInfo', help='CMAKE_BUILD_TYPE')
parser_package.add_argument('-D', '--cmake-define', action='append', default=[], help='CMake definitions to be passed to docker container (use multiple times if necessary)')
parser_package.add_argument('-i', '--images-filter', action='append', default=[], help='Filter images by name (use multiple times if necessary)')
parser_package.add_argument('-c', '--clean-logs', action='store_true', default=False, help='Clean logs directory')
parser_package.add_argument('--stderr', action='store_true', default=False, help='Log to stderr instead of log file')
parser_package.add_argument('--privileged', action='store_true', default=False, help='Run docker container in privileged mode')
parser_package.set_defaults(action='package')

parser_push = subparsers.add_parser('push', help='Push images to docker hub')
parser_push.set_defaults(action='push')

parser_gen_travis = subparsers.add_parser('generate-travis-yml', help='Generate travis yml file')
parser_gen_travis.add_argument('-s', '--skip-tests', action='append', default=[], help='Specify filter for images to be excluded from testing (can be used multiple times)')
parser_gen_travis.add_argument('-e', '--exclude-images', action='append', default=[], help='Exclude images by name (can be used multiple times)')
parser_gen_travis.set_defaults(action='generate-travis-yml')

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
elif args.action == 'push':
  push(args)
elif args.action == 'generate-travis-yml':
  generate_travis(args)

