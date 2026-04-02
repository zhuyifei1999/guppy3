/* Module guppy.sets.setsc */

char sets_doc[] =
"This module implements two specialized kinds of sets, 'bitsets' and\n"
"'nodesets'. Bitsets are sets of 'bits' -- here meaning integers in a\n"
"particular range -- and designed to be efficient with dense as well as\n"
"sparse distributions.  Nodesets are sets of 'nodes', i.e. objects with\n"
"equality based on their address; this makes inclusion test work with\n"
"any combination of objects independently from how equality or hashing\n"
"has been defined for the objects involved.\n"
"\n"
"Summary of module content.\n"
"\n"
"Classes\n"
"    BitSet              Abstract bitset base class.\n"
"        CplBitSet       Complemented immutable bitset.\n"
"        ImmBitSet       Immutable bitset, non-complemented.\n"
"        MutBitSet       Mutable bitset, complemented or not.\n"
"    NodeSet             Abstract nodeset base class.\n"
"        ImmNodeSet      Immutable nodeset.\n"
"        MutNodeSet      Mutable nodeset.\n"
"    \n"
"Functions\n"
"    immbit              Immutable bitset singleton constructor.\n"
"    immbitrange         Immutable bitset range constructor.\n"
"    immbitset           Immutable bitset constructor.\n"
"\n"
"Data\n"
"    NyBitSet_Exports,\n"
"    NyNodeSet_Exports   C-level exported function tables.\n";

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/pythoncapi_compat.h"
#include "../heapy/heapdef.h"
#include "../heapy/heapy.h"
#include "sets_internal.h"

extern int fsb_dx_nybitset_init(PyObject *m);
extern int fsb_dx_nynodeset_init(PyObject *m);

static PyMethodDef module_methods[] = {
    {NULL, NULL}
};

static NyHeapDef nysets_heapdefs[] = {
    {0, 0, (NyHeapDef_SizeGetter) mutbitset_indisize},
    {0, 0, 0, cplbitset_traverse},
    {0, 0, nodeset_indisize,  nodeset_traverse, nodeset_relate},
    {0}
};

static int module_exec(PyObject *m);

static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, module_exec},
#ifdef Py_mod_multiple_interpreters
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_SUPPORTED},
#endif
#ifdef Py_mod_gil
    {Py_mod_gil, Py_MOD_GIL_USED},
#endif
    {0, NULL}  /* Sentinel */
};

static struct PyModuleDef moduledef = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "setsc",
    .m_doc = PyDoc_STR(sets_doc),
    .m_size = 0,
    .m_methods = module_methods,
    .m_slots = module_slots,
};

static int module_exec(PyObject *m)
{
    nysets_heapdefs[0].type = &NyMutBitSet_Type;
    nysets_heapdefs[1].type = &NyCplBitSet_Type;
    nysets_heapdefs[2].type = &NyNodeSet_Type;

    if (fsb_dx_nybitset_init(m) == -1)
        return -1;
    if (fsb_dx_nynodeset_init(m) == -1)
        return -1;
    if (PyModule_Add(m, "_NyHeapDefs_",
            PyCapsule_New(&nysets_heapdefs, "guppy.sets.setsc._NyHeapDefs_", 0)
    ) == -1)
        return -1;

    return 0;
}

PyMODINIT_FUNC
PyInit_setsc (void)
{
    return PyModuleDef_Init(&moduledef);
}
