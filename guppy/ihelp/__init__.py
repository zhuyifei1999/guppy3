#._cv_part guppy.ihelp

from guppy.ihelp.PrettyPrintHTML import prettyPrintHTML

import os

thisdir = os.path.dirname(__file__)

print 'a'

class Help:
    def __init__(self, filename):

        self.filename=filename

    def __repr__(self):
        text = prettyPrintHTML(open(os.path.join(thisdir,self.filename)).read())
        return text
a=3

print 'hello!', a
