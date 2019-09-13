class _GLUECLAMP_:
    uniset_imports = ('UniSet', 'View', 'Path', 'RefPat')

    def _get_fa(self):
        us = self.UniSet
        us.out_reach_module_names = self.uniset_imports
        return us.fromargs
