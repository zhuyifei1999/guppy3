/* Module guppy.sets.setsc */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "../heapy/heapdef.h"
#include "../heapy/heapy.h"
#include "sets_internal.h"

PyDoc_STRVAR(sets_doc,
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
"    NyNodeSet_Exports   C-level exported function tables.\n"
);

static PyMethodDef module_methods[] = {
    {NULL, NULL}
};

static int
module_gc_traverse(PyObject *m, visitproc visit, void *arg)
{
    struct SetscState *ms = NyModule_AssertState(m);

    Py_VISIT(ms->BitSet_Type);
    Py_VISIT(ms->ImmBitSet_Type);
    Py_VISIT(ms->ImmBitSetIter_Type);
    Py_VISIT(ms->CplBitSet_Type);
    Py_VISIT(ms->MutBitSet_Type);
    Py_VISIT(ms->Union_Type);
    Py_VISIT(ms->NodeSet_Type);
    Py_VISIT(ms->MutNodeSet_Type);
    Py_VISIT(ms->ImmNodeSet_Type);
    Py_VISIT(ms->MutNodeSetIter_Type);
    Py_VISIT(ms->ImmNodeSetIter_Type);

    Py_VISIT(ms->ImmBitSet_Empty);
    Py_VISIT(ms->ImmBitSet_Omega);
    Py_VISIT(ms->BitSet_FormMethod);

    return 0;
}

static int
module_gc_clear(PyObject *m)
{
    struct SetscState *ms = NyModule_AssertState(m);

    Py_CLEAR(ms->BitSet_Type);
    Py_CLEAR(ms->ImmBitSet_Type);
    Py_CLEAR(ms->ImmBitSetIter_Type);
    Py_CLEAR(ms->CplBitSet_Type);
    Py_CLEAR(ms->MutBitSet_Type);
    Py_CLEAR(ms->Union_Type);
    Py_CLEAR(ms->NodeSet_Type);
    Py_CLEAR(ms->MutNodeSet_Type);
    Py_CLEAR(ms->ImmNodeSet_Type);
    Py_CLEAR(ms->MutNodeSetIter_Type);
    Py_CLEAR(ms->ImmNodeSetIter_Type);

    Py_CLEAR(ms->ImmBitSet_Empty);
    Py_CLEAR(ms->ImmBitSet_Omega);
    Py_CLEAR(ms->BitSet_FormMethod);

    return 0;
}

static void
module_free(void *mod)
{
    module_gc_clear(mod);
}

static int module_exec(PyObject *m)
{
    struct SetscState *ms = NyModule_AssertState(m);

    if (fsb_dx_nybitset_init(m) == -1)
        return -1;
    if (fsb_dx_nynodeset_init(m) == -1)
        return -1;

    ms->Sets_HeapDef[0] = (NyHeapDef){
        0,                   /* flags */
        ms->MutBitSet_Type,  /* type */
        (NyHeapDef_SizeGetter)mutbitset_indisize, /* size */
        NULL,                /* traverse */
        NULL                 /* relate */
    };
    ms->Sets_HeapDef[1] = (NyHeapDef){
        0,                   /* flags */
        ms->CplBitSet_Type,  /* type */
        NULL,                /* size */
        cplbitset_traverse,  /* traverse */
        NULL                 /* relate */
    };
    ms->Sets_HeapDef[2] = (NyHeapDef){
        0,                   /* flags */
        ms->NodeSet_Type,    /* type */
        nodeset_indisize,    /* size */
        nodeset_traverse,    /* traverse */
        nodeset_relate       /* relate */
    };
    /* End mark */
    ms->Sets_HeapDef[3] = (NyHeapDef){0};

    if (PyModule_Add(m, "_NyHeapDefs_",
            PyCapsule_New(&ms->Sets_HeapDef, "guppy.sets.setsc._NyHeapDefs_", 0)
    ) == -1)
        return -1;

    return 0;
}

static PyModuleDef_Slot module_slots[] = {
    {Py_mod_exec, module_exec},
#ifdef Py_mod_multiple_interpreters
    {Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED},
#endif
#ifdef Py_mod_gil
    {Py_mod_gil, Py_MOD_GIL_NOT_USED},
#endif
    {0, NULL}  /* Sentinel */
};

struct PyModuleDef setsc_def = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "setsc",
    .m_doc = PyDoc_STR(sets_doc),
    .m_size = sizeof(struct SetscState),
    .m_methods = module_methods,
    .m_slots = module_slots,
    .m_traverse = module_gc_traverse,
    .m_clear = module_gc_clear,
    .m_free = module_free,
};

/* -Wmissing-prototypes */
extern PyMODINIT_FUNC PyInit_setsc(void);

PyMODINIT_FUNC PyInit_setsc(void)
{
    return PyModuleDef_Init(&setsc_def);
}
