#._cv_part guppy.etc.Compat

#
# This module resolves some differences
# between different Python versions
# and is to be used when it would help compatibility
# to use the objects defined herein.
# (There may of course be other compatibility issues.)

# Importing this module will write missing names
# into __builtin__ so that they will be generally available.

# In newer versions of Python (from 2.3 I think ) there is
# no effect on builtins.

try:
    str
except NameError:
    str = str

try:
    bool
except NameError:
    def bool(x):
        return not not x

try:
    True
except NameError:
    False = 0
    True = 1

try:
    enumerate
except NameError:
    def enumerate(lt):
        return map(None, range(len(lt)), lt)

def _make_system_compatible():
    import builtins
    for name, value in list(globals().items()):
        if name[:1] != '_':
            setattr(__builtin__, name, value)

_make_system_compatible()
