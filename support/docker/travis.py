import sys
import os
import pyaml 

def b2i(bool_value):
    return 1 if bool_value else 0

class Travis:
  def __init__(self, images):
    self.images = images

  def generate(self, args):
    travis_obj = {
      'language': 'cpp',
      'if': 'branch = master OR type = pull_request',
      'matrix': {
      },
      'before_install': [
        'bash -x ${TRAVIS_BUILD_DIR}/scripts/travis/before_install',
      ],
      'script': 'bash -x ${TRAVIS_BUILD_DIR}/scripts/travis/ci-build && bash -x ${TRAVIS_BUILD_DIR}/scripts/travis/tests && bash -x ${TRAVIS_BUILD_DIR}/scripts/travis/package',
      'deploy': {
        'provider': 'script',
        'script': 'bash -x ${TRAVIS_BUILD_DIR}/scripts/travis/deploy',
        'skip_cleanup': True,
        'on': {
          'repo': 'GuLinux/PlanetaryImager',
          'branch': 'master',
        },
      },
    }
    includes = []
    if not 'osx' in args.exclude_images:
      brew_dependencies = ['python@3']
      with open(os.path.join(os.path.dirname(os.path.dirname(__file__)), 'osx', 'homebrew_deps')) as homebrew_deps:
        brew_dependencies.extend([x.strip() for x in homebrew_deps.readlines()])
      includes.append({
        'os': 'osx',
        'osx_image': 'xcode10.2',
        'env': 'SKIP_TESTS={} BUILD_OS_FAMILY=osx'.format(b2i('osx' in args.skip_tests)),
        'addons': {
          'homebrew': {
            'packages': brew_dependencies
          },
        },
      })
    for image in self.images:
      exclude_image = False
      skip_tests=False
      for t in args.skip_tests:
        skip_tests |= t in image.image_name
      for t in args.exclude_images:
        exclude_image |= t in image.image_name
      if not exclude_image:
        includes.append({
          'os': 'linux',
          'dist': 'xenial',
          'services': ['docker'],
          'env': 'SKIP_TESTS={} DOCKER_IMAGE={} IMAGE_ARCH={} BUILD_OS_FAMILY={} CMAKE_BINARY={}'.format(
            b2i(skip_tests), image.image_name, image.arch, image.os_family, image.cmake_binary
          )
        })
 
    travis_obj['matrix']['include'] = includes
    sys.stdout.write(pyaml.dump(travis_obj, width=200))

# vim: set tabstop=4 shiftwidth=4 expandtab
