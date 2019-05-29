from snippet import Snippet
import os
import sys
import shutil
import subprocess
from datetime import datetime

images_dir = 'images'
logs_dir = os.path.abspath('./logs')

def cleanup_logs():
    shutil.rmtree(logs_dir, ignore_errors=True)
  

class Dockerfile:
    def __init__(self, name, snippets, files, substitutions):
        self.name = name
        self.snippets = [Snippet(os.path.join('snippets', snippet + '.in')) for snippet in snippets]
        self.substitutions = substitutions
        self.files = files
        self.status = None
        self.image_dir = os.path.join(images_dir, self.name)
        self.image_name = 'gulinux/planetaryimager_build:{}'.format(self.name)

    def write(self):

        dockerfile = os.path.join(self.image_dir, 'Dockerfile')
        os.makedirs(self.image_dir, exist_ok=True)
        if os.path.exists(dockerfile):
            os.remove(dockerfile)
        with open(dockerfile, 'x'):
            pass
        snippets = []

        snippets.extend(self.snippets)
        snippets.append(Snippet('snippets/workdir.in'))
        snippets.append(Snippet('snippets/entrypoint.in'))

        for snippet in snippets:
            with open(dockerfile, 'a') as f:
                try:
                    f.write(snippet.substitute(self.substitutions))
                except Exception as e:
                    raise RuntimeError('Error creating Dockerfile for {}: {}'.format(self.name, e))
        for file in self.files:
            shutil.copy(file, self.image_dir)

    def build(self, stderr=False):
      self.__create_logsdir()
      args = ['docker', 'build', '-t', self.image_name, '.']
      self.__run_command(args, 'build', stderr, run_directory=self.image_dir)
           
    def package(self, code_path, destination_path, make_jobs, cmake_defines, stderr=False, build_directory=None):
      self.__create_logsdir()
      cmdline = [
        'docker',
        'run',
#        '-it',
        '--rm',
        '-v',
        '{}:/code'.format(os.path.abspath(code_path)),
        '-v',
        '{}:/dest'.format(os.path.abspath(destination_path)),
        '-e',
        'MAKE_OPTS=-j{}'.format(make_jobs),
      ]
      if build_directory:
        cmdline.extend(['-v', '{}:/build'.format(os.path.abspath(build_directory))])
      cmdline.append(self.image_name)
      cmdline.extend(['-D' + x for x in cmake_defines])
      self.__run_command(cmdline, 'package', stderr)

    def report_build(self):
        is_error = False
        if self.result is None:
            status = '[SKIPPED]'
        elif self.result.returncode == 0:
            status = '[OK]'
        else:
            status = '[ERROR]'
            is_error = True
        print('{: <40}: {}'.format(self.name, status))
        return not is_error
      
    def __run_command(self, cmdline, command_name, stderr=False, run_directory=None):
      def run(output):
        output.write('{}: running command line: `{}`\n'.format(command_name, ' '.join(cmdline)))
        output.flush()
        self.result = subprocess.run(cmdline, stdout=output, stderr=subprocess.STDOUT, cwd=run_directory)

      if stderr:
        run(sys.stderr)
      else:
        sys.stdout.write('{} for {} (`{}`)...'.format(command_name, self.name, ' '.join(cmdline)))
        sys.stdout.flush()
        logfile_path = os.path.join(logs_dir, '{}-{}-{}.log'.format(self.name, command_name, datetime.now().strftime('%Y-%m-%dT%H-%M-%S')))
        with open(logfile_path, 'w') as logfile:
          run(logfile)
        if self.result.returncode == 0:
          sys.stdout.write('[OK]\n')
        else:
          with open(logfile_path, 'r') as logfile:
            sys.stdout.write('[ERROR]\n')
            sys.stderr.write(''.join(logfile.readlines()[-10:]))
            sys.stderr.write('\n')
            sys.stderr.flush()
            
      
    def __create_logsdir(self):
       os.makedirs(logs_dir, exist_ok=True)
 

    def __str__(self):
        return 'Dockerfile: ' + self.name

    def __repr__(self):
        return self.__str__()
