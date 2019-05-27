from string import Template


class Snippet(Template):
    delimiter = '%'
    def __init__(self, filename):
        with open(filename) as f:
            super().__init__(f.read())


