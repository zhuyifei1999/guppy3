# ._cv_part guppy.etc.Descriptor

import functools


class property_nondata:
    '''@property, but using non-data descriptor protocol'''
    def __init__(self, fget):
        self.fget = fget
        functools.update_wrapper(self, fget)

    def __get__(self, instance, owner=None):
        return self.fget(instance)
