/* module heapyc */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "structmember.h"
#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "impsets.h"
#include "heapdef.h"
#include "heapy.h"
#include "classifier.h"
#include "hv.h"
#include "nodegraph.h"
#include "rootstate.h"
#include "stdtypes_internal.h"
#include "stoptheworld.h"
#include "utils.h"
#include "xmemstats.h"

PyDoc_STRVAR(heapyc_doc,
"This module contains low level functionality for the heapy system.\n"
"It is intended to be wrapped in higher level library classes.\n"
"\n"
"Summary of module content.\n"
"\n"
"Classes\n"
"    HeapView        Gives a parameterized view of the heap.\n"
"    Horizon         Limits the view back to some moment in time.\n"
"    NodeGraph           Graph of nodes (address-treated objects).\n"
"    ObjectClassifier    Classifies objects on various criteria.\n"
"    RootStateType       Root of heap traversal using Python internals.\n"
"\n"
"Functions\n"
"    has_deferred_refcount   Returns whether objects uses deferred\n"
"                            refcount (freethreading-only).\n"
"    interpreter         Start a new interpreter.\n"
"    set_async_exc       Raise an exception in another thread.\n"
"    xmemstats           Print system-dependent memory statistics.\n"
"\n"
"Object\n"
"    RootState           The single instance of RootStateType.\n"
);

#define Py_BUILD_CORE
# undef _PyGC_FINALIZED
/* PyInterpreterState */
# include <internal/pycore_interp.h>
#undef Py_BUILD_CORE

/* Extern decls - maybe put in .h file but not in heapy.h */


/* Forward decls */

/* Stop-the-world support data */

#ifdef Py_GIL_DISABLED
bool NY_IS_WORLD_STOPPED(void)
{
    return PyInterpreterState_Get()->stoptheworld.world_stopped;
}
#elif !defined(NDEBUG)
Ny_thread_local int _world_stopped = 0;
#endif

/* general utilities */

int
iterable_iterate(struct HeapycState *ms, PyObject *v,
                int (*visit)(PyObject *, void *), void *arg)
{
    if (PyObject_TypeCheck(v, ms->nodeset_exports->nodeset_type)) {
        return NyNodeSet_iterate((NyNodeSetObject *)v, visit, arg);
    } else if (PyObject_TypeCheck(v, ms->HeapView_Type)) {
        return NyHeapView_iterate((NyHeapViewObject *)v, visit, arg);
    } else if (PyList_Check(v)) { /* A bit faster than general iterator?? */
        Py_ssize_t i;
        int r;
        PyObject *item;
        for (i = 0; i < PyList_GET_SIZE(v); i++) {
            item = PyList_GET_ITEM(v, i);
            Py_INCREF(item);
            r = visit(item, arg);
            Py_DECREF(item);
            if (r == -1)
                return -1;
            if (r == 1)
                break;
        }
        return 0;
    } else { /* Do the general case. */
        PyObject *it = PyObject_GetIter(v);
        int r;
        if (it == NULL)
            goto Err;
        /* Run iterator to exhaustion. */
        for (;;) {
            PyObject *item = PyIter_Next(it);
            if (item == NULL) {
                if (PyErr_Occurred())
                    goto Err;
                break;
            }
            r = visit(item, arg);
            Py_DECREF(item);
            if (r == -1)
                goto Err;
            if (r == 1)
                break;
        }
        Py_DECREF(it);
        return 0;
Err:
        Py_XDECREF(it);
        return -1;
    }
}

PyObject *
gc_get_objects(void)
{
    PyObject *gc=0, *objects=0;

    NY_ASSERT_WORLD_RUNNING();
    gc = PyImport_ImportModule("gc");
    if (!gc)
        goto err;
    objects = PyObject_CallMethod(gc, "get_objects", "");
err:
    Py_XDECREF(gc);
    return objects;
}

/* objects */

#ifdef Py_GIL_DISABLED
# define Py_BUILD_CORE
/* PyObject_HasDeferredRefcount */
# if NY_MASKED_VERSION_HEX >= Py_PACK_VERSION(3, 14)
#  include <internal/pycore_object_deferred.h>
# else
#  include <internal/pycore_object.h>
# endif
# undef Py_BUILD_CORE
#endif

/* Other functions */
PyDoc_STRVAR(hp_has_deferred_refcount_doc,
"has_deferred_refcount(target:object)\n"
"\n"
"Returns whether the given object uses deferred refcount (freethreading-only).\n"
);

static PyObject *
hp_has_deferred_refcount(PyObject *self, PyObject *op)
{
#ifdef Py_GIL_DISABLED
    return PyBool_FromLong(_PyObject_HasDeferredRefcount(op));
#else
    return Py_NewRef(Py_False);
#endif
}

static PyMethodDef module_methods[] =
{
    /*
    {"interpreter", (PyCFunction)hp_interpreter, METH_VARARGS, hp_interpreter_doc},
    {"set_async_exc", (PyCFunction)hp_set_async_exc, METH_VARARGS, hp_set_async_exc_doc},
    */
    {"has_deferred_refcount", (PyCFunction)hp_has_deferred_refcount, METH_O, hp_has_deferred_refcount_doc},
    {"xmemstats", (PyCFunction)hp_xmemstats, METH_NOARGS, hp_xmemstats_doc},
    {0}
};


static int
module_gc_traverse(PyObject *m, visitproc visit, void *arg)
{
    struct HeapycState *ms = NyModule_AssertState(m);

    Py_VISIT(ms->HeapView_Type);
    Py_VISIT(ms->Horizon_Type);
    Py_VISIT(ms->NodeGraph_Type);
    Py_VISIT(ms->NodeGraphIter_Type);
    Py_VISIT(ms->NodeTuple_Type);
    Py_VISIT(ms->ObjectClassifier_Type);
    Py_VISIT(ms->Relation_Type);
    Py_VISIT(ms->RootState_Type);
    Py_VISIT(ms->RootState);

    return 0;
}

static int
module_gc_clear(PyObject *m)
{
    struct HeapycState *ms = NyModule_AssertState(m);

    Py_CLEAR(ms->HeapView_Type);
    Py_CLEAR(ms->Horizon_Type);
    Py_CLEAR(ms->NodeGraph_Type);
    Py_CLEAR(ms->NodeGraphIter_Type);
    Py_CLEAR(ms->NodeTuple_Type);
    Py_CLEAR(ms->ObjectClassifier_Type);
    Py_CLEAR(ms->Relation_Type);
    Py_CLEAR(ms->RootState_Type);
    Py_CLEAR(ms->RootState);

    return 0;
}

static void
module_free(void *mod)
{
    module_gc_clear(mod);
}

static int module_exec(PyObject *m)
{
    struct HeapycState *ms = NyModule_AssertState(m);
    PyObject *RootState;

    if (import_sets(ms) == -1)
        return -1;

    if (NyModule_AddTypeWithSpec(m, &NyHeapView_Spec, NULL, true,
                                 &ms->HeapView_Type) == -1)
        return -1;
    if (NyModule_AddTypeWithSpec(m, &NyHorizon_Spec, NULL, true,
                                 &ms->Horizon_Type) == -1)
        return -1;
    if (NyModule_AddTypeWithSpec(m, &NyNodeGraph_Spec, NULL, true,
                                 &ms->NodeGraph_Type) == -1)
        return -1;
    if (NyModule_AddTypeWithSpec(m, &NyNodeGraphIter_Spec, NULL, false,
                                 &ms->NodeGraphIter_Type) == -1)
        return -1;
    if (NyModule_AddTypeWithSpec(m, &NyNodeTuple_Spec, (PyObject *)&PyTuple_Type, false,
                                 &ms->NodeTuple_Type) == -1)
        return -1;
    if (NyModule_AddTypeWithSpec(m, &NyObjectClassifier_Spec, NULL, true,
                                 &ms->ObjectClassifier_Type) == -1)
        return -1;
    if (NyModule_AddTypeWithSpec(m, &NyRelation_Spec, NULL, true,
                                 &ms->Relation_Type) == -1)
        return -1;
    if (NyModule_AddTypeWithSpec(m, &NyRootState_Spec, NULL, true,
                                 &ms->RootState_Type) == -1)
        return -1;

    RootState = ms->RootState_Type->tp_alloc(ms->RootState_Type, 0);
    if (PyModule_AddObjectRef(m, "RootState", RootState) == -1)
        return -1;
    ms->RootState = RootState;

    ms->HvTypes_HeapDef[0] = (NyHeapDef){
        0,                           /* flags */
        ms->NodeGraph_Type,  /* type */
        nodegraph_size,              /* size */
        nodegraph_traverse,          /* traverse */
        nodegraph_relate             /* relate */
    };
    ms->HvTypes_HeapDef[1] = (NyHeapDef){
        0,                           /* flags */
        ms->RootState_Type,  /* type */
        0,                           /* size */
        rootstate_traverse,          /* traverse */
        rootstate_relate             /* relate */
    };
    ms->HvTypes_HeapDef[2] = (NyHeapDef){
        0,                           /* flags */
        ms->Horizon_Type,    /* type */
        0,                           /* size */
        0,                           /* traverse */
        0                            /* relate */
    };
    /* End mark */
    ms->HvTypes_HeapDef[3] = (NyHeapDef){0};

    NyStdTypes_init();
    xmemstats_init();

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

struct PyModuleDef heapyc_def = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "heapyc",
    .m_doc = PyDoc_STR(heapyc_doc),
    .m_size = sizeof(struct HeapycState),
    .m_methods = module_methods,
    .m_slots = module_slots,
    .m_traverse = module_gc_traverse,
    .m_clear = module_gc_clear,
    .m_free = module_free,
};

/* -Wmissing-prototypes */
extern PyMODINIT_FUNC PyInit_heapyc(void);

PyMODINIT_FUNC
PyInit_heapyc(void)
{
    return PyModuleDef_Init(&heapyc_def);
}
