import functools
import inspect


class property_nondata:
    '''@property, but using non-data descriptor protocol'''
    def __init__(self, fget):
        self.fget = fget
        functools.update_wrapper(self, fget)

    def __get__(self, instance, owner=None):
        return self.fget(instance)


class property_exp(property):
    '''@property, but blacklist tab completers like rlcompleter from getattr'''
    def __init__(self, fget, *, doc=None):
        super().__init__(fget)
        self.__doc__ = doc

    def __get__(self, instance, owner=None):
        try:
            frame = inspect.currentframe()
            try:
                frame = frame.f_back
                if frame.f_globals['__name__'] == 'rlcompleter':
                    return None
            finally:
                del frame
        except Exception:
            pass
        return super().__get__(instance, owner)
