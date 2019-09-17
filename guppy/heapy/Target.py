import os
import sys


class Target:
    def __init__(self, _hiding_tag_=None):
        self.wd = os.getcwd()
        self.pid = os.getpid()
        self.sys = sys
        self._hiding_tag_ = _hiding_tag_
