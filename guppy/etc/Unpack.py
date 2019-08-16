# ._cv_part guppy.etc.Unpack

import dis
from opcode import *
import sys

CALL_FUNCTION = opmap['CALL_FUNCTION']
UNPACK_SEQUENCE = opmap['UNPACK_SEQUENCE']
STORE_FAST = opmap['STORE_FAST']
STORE_NAME = opmap['STORE_NAME']
STORE_GLOBAL = opmap['STORE_GLOBAL']
STORE_ATTR = opmap['STORE_ATTR']
STORE_SUBSCR = opmap['STORE_SUBSCR']
# STORE_SLICE = opmap['STORE_SLICE']


def unpack(x):
    try:
        1/0
    except ZeroDivisionError:
        typ, value, traceback = sys.exc_info()

        f = traceback.tb_frame.f_back
        co = f.f_code
        i = f.f_lasti
        insns = dis.get_instructions(co)
        while True:
            insn = next(insns)
            if insn.offset == i:
                break

        if insn.opcode != CALL_FUNCTION:
            raise SyntaxError

        insn = next(insns)
        if insn.opcode != UNPACK_SEQUENCE:
            raise SyntaxError
        n = insn.argval

        names = []
        while len(names) < n:
            insn = next(insns)
            if insn.opcode in (STORE_FAST, STORE_NAME, STORE_ATTR, STORE_GLOBAL):
                names.append(insn.argval)
            if insn.opcode == STORE_SUBSCR:  # or STORE_SLICE <= op <= STORE_SLICE+3
                break

        if len(names) != n:
            raise SyntaxError

        r = []
        for name in names:
            try:
                v = getattr(x, name)
            except AttributeError:
                v = x[name]
            r.append(v)
        return r


def test_unpack():
    class C:
        a = 1
        b = 3
        c = 4
    y = C()
    a, b, c = unpack(y)
    x = [a, b, c]

    class D:
        pass
    D.a, c, D.b = unpack(y)
    x.extend([D.a, c, D.b])

    l = [None]
    try:
        l[0], c, b = unpack(y)
    except SyntaxError:
        pass
    else:
        raise RuntimeError
    l = [None]
    try:
        l[1:2], c, b = unpack(y)
    except SyntaxError:
        pass
    else:
        raise RuntimeError
    y = []

    y = {'a': 'A', 'b': 'B'}
    a, b = unpack(y)
    x.extend([a, b])

    global g
    y['g'] = 'G'
    g, b = unpack(y)
    x.extend([g, b])

    if x != [1, 3, 4, 1, 4, 3, 'A', 'B', 'G', 'B']:
        raise RuntimeError


__all__ = 'unpack'

if __name__ == '__main__':
    test_unpack()
