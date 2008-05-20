#._cv_part guppy.help.Help

from guppy.help.PrettyPrintHTML import prettyPrintHTML

# Just a test comment added, worried about svn status ...

class HelpPrinter:
    def __init__(self, msg):
        self.msg = msg

class Help:
    def __init__(self, msg, attribs):
        self.msg = msg
        self.attribs = attribs

    def __getattr__(self, attr):
        return attr
        

    def __str__(self):
        return repr(self)

    def __repr__(self):
        return ''


    def getlongrepr(self):
        from StringIO import StringIO
        f=StringIO()

        return self.msg+"""\
<Type e.g. '_.<subject> for help subject'>"""
        

class _GLUECLAMP_:
    pass

def test():
    pass


print 'hello'

h=Help("""\
Attributes: a,b,c
""", {})
print h

f = open( '/home/sverker/guppy/doc/guppy.html' )
html=f.read()
text= prettyPrintHTML(html)

class Helper:
    pass

class Obj:
    class help(Helper):
        a='a attribute'

obj=Obj()
