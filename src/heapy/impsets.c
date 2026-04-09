#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "impsets.h"

static NyNodeSet_Exports *nodeset_exports;

#define NODESET_EXPORTS \
    ((NyNodeSet_Exports *)_Py_atomic_load_ptr_relaxed(&nodeset_exports))

#define NyNodeSet_TYPE	(NODESET_EXPORTS->nodeset_type)
#define NyMutNodeSet_TYPE	(NODESET_EXPORTS->mutnodeset_type)
#define NyImmNodeSet_TYPE	(NODESET_EXPORTS->immnodeset_type)

/* As of 3.14, PyCapsule_Import requires external synchronization if called
   from multiple threads, and I believe module_exec can be, if we are GIL-less
   eventually */
#if NY_MASKED_VERSION_HEX >= Py_PACK_VERSION(3, 13)
static PyMutex nodeset_exports_mutex = {0};
#else
# define PyMutex_Lock(m) do {} while (0)
# define PyMutex_Unlock(m) do {} while (0)
#endif

bool NyNodeSet_Check(PyObject *op)
{
    return PyObject_TypeCheck(op, NyNodeSet_TYPE);
}

bool NyImmNodeSet_Check(PyObject *op)
{
    return PyObject_TypeCheck(op, NyImmNodeSet_TYPE);
}


NyNodeSetObject *
NyMutNodeSet_New(void)
{
    return NODESET_EXPORTS->newMut();
}

NyNodeSetObject *
NyMutNodeSet_NewHiding(PyObject *tag)
{
    return NODESET_EXPORTS->newMutHiding(tag);
}

NyNodeSetObject *
NyMutNodeSet_NewFlags(int flags)
{
    return NODESET_EXPORTS->newMutFlags(flags);
}

int
NyNodeSet_setobj(NyNodeSetObject *v, PyObject *obj)
{
    return NODESET_EXPORTS->setobj(v, obj);
}

int
NyNodeSet_clrobj(NyNodeSetObject *v, PyObject *obj)
{
    return NODESET_EXPORTS->clrobj(v, obj);
}


int
NyNodeSet_hasobj(NyNodeSetObject *v, PyObject *obj)
{
    return NODESET_EXPORTS->hasobj(v, obj);
}

int NyNodeSet_iterate(NyNodeSetObject *ns,
                      int (*visit)(PyObject *, void *),
                      void *arg)
{
    return NODESET_EXPORTS->iterate(ns, visit, arg);;
}


NyNodeSetObject *
NyImmNodeSet_NewCopy(NyNodeSetObject *v)
{
    return NODESET_EXPORTS->newImmCopy(v);
}

NyNodeSetObject *
NyImmNodeSet_NewSingleton(PyObject *element, PyObject *hiding_tag)
{
    return NODESET_EXPORTS->newImmSingleton(element, hiding_tag);
}

int
NyNodeSet_be_immutable(NyNodeSetObject **nsp)
{
    return NODESET_EXPORTS->be_immutable(nsp);
}

int
import_sets(void)
{
    NyNodeSet_Exports *local_nodeset_exports;

    if (_Py_atomic_load_ptr_relaxed(&nodeset_exports))
        return 0;

    PyMutex_Lock(&nodeset_exports_mutex);

    if (_Py_atomic_load_ptr_relaxed(&nodeset_exports))
        goto out;
    local_nodeset_exports = PyCapsule_Import("guppy.sets.setsc.NyNodeSet_Exports", 0);
    if (!local_nodeset_exports)
        goto err;
    _Py_atomic_store_ptr_relaxed(&nodeset_exports, local_nodeset_exports);

out:
    PyMutex_Unlock(&nodeset_exports_mutex);
    return 0;
err:
    PyMutex_Unlock(&nodeset_exports_mutex);
    return -1;
}
