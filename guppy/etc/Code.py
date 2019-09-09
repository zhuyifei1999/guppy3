def co_code_findloadednames(co):
    """Find in the code of a code object, all loaded names.
    (by LOAD_NAME, LOAD_GLOBAL or LOAD_FAST) """

    import dis
    from opcode import HAVE_ARGUMENT, opmap
    hasloadname = (opmap['LOAD_NAME'],
                   opmap['LOAD_GLOBAL'], opmap['LOAD_FAST'])
    insns = dis.get_instructions(co)
    len_co_names = len(co.co_names)
    indexset = {}
    for insn in insns:
        if insn.opcode >= HAVE_ARGUMENT:
            if insn.opcode in hasloadname:
                indexset[insn.argval] = 1
                if len(indexset) >= len_co_names:
                    break
    for name in co.co_varnames:
        try:
            del indexset[name]
        except KeyError:
            pass
    return indexset


def co_findloadednames(co):
    """Find all loaded names in a code object and all its consts of code type"""
    names = {}
    names.update(co_code_findloadednames(co))
    for c in co.co_consts:
        if isinstance(c, type(co)):
            names.update(co_findloadednames(c))
    return names
