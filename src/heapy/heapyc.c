/* module heapyc */

char heapyc_doc[] =

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
;


#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "structmember.h"
#include "compile.h"
#include "frameobject.h"
#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"
#include "../sets/nodeset.h"
#include "heapdef.h"
#include "heapy.h"
#include "classifier.h"
#include "nodegraph.h"
#include "relation.h"
#include "stoptheworld.h"

/* Extern decls - maybe put in .h file but not in heapy.h */

extern NyHeapDef NyStdTypes_HeapDef[];
extern NyHeapDef NyStdTypes_HeapDef[];
extern NyHeapDef NyObjectWithHeapType_HeapDef;
extern void NyStdTypes_init(void);
extern int dict_relate_kv(NyHeapRelate *r, PyObject *dict, int k, int v);

/* Forward decls */

PyTypeObject NyObjectClassifier_Type;
PyTypeObject NyNodeSet_Type;
PyTypeObject NyHeapView_Type;
PyTypeObject NyHorizon_Type;
PyTypeObject NyNodeGraph_Type;
PyTypeObject NyRootState_Type;
PyTypeObject NyRelation_Type;
PyTypeObject NyNodeTuple_Type;

NyHeapDef NyHvTypes_HeapDef[];

PyObject * NyObjectClassifier_New(PyObject *self, NyObjectClassifierDef *def);
int NyHeapView_iterate(NyHeapViewObject *hv, int (*visit)(PyObject *, void *),
                       void *arg);

static Py_ssize_t roundupsize(Py_ssize_t n);

/* Thread-local data */

#if !defined(Py_GIL_DISABLED) && !defined(NDEBUG)
thread_local int _world_stopped = 0;
#endif

/* general utilities */

#include "impsets.c"

static int
iterable_iterate(PyObject *v, int (*visit)(PyObject *, void *),
                void *arg)
{
    if (NyNodeSet_Check(v)) {
        return NyNodeSet_iterate((NyNodeSetObject *)v, visit, arg);
    } else if (NyHeapView_Check(v)) {
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
    gc = PyImport_ImportModule("gc");
    if (!gc)
        goto err;
    objects = PyObject_CallMethod(gc, "get_objects", "");
err:
    Py_XDECREF(gc);
    return objects;
}

#include "roundupsize.c"

/* objects */

#include "hv.c"
#include "classifier.c"
#include "horizon.c"
#include "nodegraph.c"
#include "rootstate.c"

#ifdef Py_GIL_DISABLED
# define Py_BUILD_CORE
/* PyObject_HasDeferredRefcount */
# if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 14
#  include <internal/pycore_object_deferred.h>
# else
#  include <internal/pycore_object.h>
# endif
# undef Py_BUILD_CORE
#endif

/* Other functions */
static char hp_has_deferred_refcount_doc[] =
"has_deferred_refcount(target:object)\n"
"\n"
"Returns whether the given object uses deferred refcount (freethreading-only).\n"
;

static PyObject *
hp_has_deferred_refcount(PyObject *self, PyObject *op)
{
#ifdef Py_GIL_DISABLED
    return PyBool_FromLong(_PyObject_HasDeferredRefcount(op));
#else
    return Py_NewRef(Py_False);
#endif
}

/* #include "interpreter.c" */
#include "xmemstats.c"

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


NyHeapDef NyHvTypes_HeapDef[] = {
    {
        0,                  /* flags */
        &NyNodeGraph_Type,  /* type */
        nodegraph_size,     /* size */
        nodegraph_traverse, /* traverse */
        nodegraph_relate    /* relate */
    },
    {
        0,                  /* flags */
        &NyRootState_Type,  /* type */
        0,                  /* size */
        rootstate_traverse, /* traverse */
        rootstate_relate    /* relate */
    },
    {
        0,                  /* flags */
        &NyHorizon_Type,    /* type */
        0,                  /* size */
        0,                  /* traverse */
        0                   /* relate */
    },
    /* End mark */
    {0}
};

static int module_exec(PyObject *m);

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

static struct PyModuleDef moduledef = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "heapyc",
    .m_doc = PyDoc_STR(heapyc_doc),
    .m_size = 0,
    .m_methods = module_methods,
    .m_slots = module_slots,
};

static int module_exec(PyObject *m)
{
    Py_SET_TYPE(&_Ny_RootStateStruct, &NyRootState_Type);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    PyUnstable_SetImmortal(&_Ny_RootStateStruct);
#endif

    // This has to be here because of 'initializer is not a constant'
    // build error on Windows.
    NyNodeTuple_Type.tp_base = &PyTuple_Type;

    NYFILL(NyNodeTuple_Type);
    NYFILL(NyRelation_Type);
    NYFILL(NyHeapView_Type);
    NYFILL(NyObjectClassifier_Type);
    NYFILL(NyHorizon_Type);
    NYFILL(NyNodeGraph_Type);
    NYFILL(NyNodeGraphIter_Type);
    NYFILL(NyRootState_Type);

    if (import_sets() == -1)
        return -1;

    if (PyModule_AddType(m, &NyHeapView_Type) == -1)
        return -1;
    if (PyModule_AddType(m, &NyHorizon_Type) == -1)
        return -1;
    if (PyModule_AddType(m, &NyObjectClassifier_Type) == -1)
        return -1;
    if (PyModule_AddType(m, &NyNodeGraph_Type) == -1)
        return -1;
    if (PyModule_AddType(m, &NyRelation_Type) == -1)
        return -1;
    if (PyModule_AddObjectRef(m, "RootState", Ny_RootState) == -1)
        return -1;
    if (PyModule_AddType(m, &NyRootState_Type) == -1)
        return -1;

    NyStdTypes_init();
    xmemstats_init();

    return 0;
}

PyMODINIT_FUNC
PyInit_heapyc (void)
{
    return PyModuleDef_Init(&moduledef);
}
