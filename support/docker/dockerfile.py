from snippet import Snippet
import os
import shutil
import subprocess
from datetime import datetime

images_dir = 'images'
logs_dir = os.path.abspath('./logs')

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
                f.write(snippet.substitute(self.substitutions))
        for file in self.files:
            shutil.copy(file, self.image_dir)
        shutil.copy('files/entrypoint', self.image_dir)

    def build(self):
        os.makedirs(logs_dir, exist_ok=True)
        with open(os.path.join(logs_dir, '{}-{}.log'.format(datetime.now().strftime('%Y-%m-%dT%H-%M-%S'), self.name)), 'w') as logfile:
            args = ['docker', 'build', '-t', self.image_name, '.']
            logfile.write('running command line: `{}`\n'.format(' '.join(args)))
            logfile.flush()
            print('Building image `{}`: running command `{}`'.format(self.name, ' '.join(args)))
            self.result = subprocess.run(args, stdout=logfile, stderr=subprocess.STDOUT, cwd=self.image_dir)

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

    def __str__(self):
        return 'Dockerfile: ' + self.name

    def __repr__(self):
        return self.__str__()
