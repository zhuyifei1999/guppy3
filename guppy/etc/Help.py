#._cv_part guppy.etc.Help

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


class Helper:
    pass

class Obj:
    class help(Helper):
        a='a attribute'

obj=Obj()
