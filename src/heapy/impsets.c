#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "impsets.h"
#include "heapy.h"
#include "stoptheworld.h"

/* For function pointers only */
static NyNodeSet_Exports nodeset_exports;

#define NODESET_EXPORTED_FUNC(attr) \
    ((typeof(nodeset_exports.attr))_Py_atomic_load_ptr_relaxed(&nodeset_exports.attr))


NyNodeSetObject *
NyMutNodeSet_New(struct SetscState *ms)
{
    return NODESET_EXPORTED_FUNC(newMut)(ms);
}

NyNodeSetObject *
NyMutNodeSet_NewHiding(struct SetscState *ms, PyObject *tag)
{
    return NODESET_EXPORTED_FUNC(newMutHiding)(ms, tag);
}

NyNodeSetObject *
NyMutNodeSet_NewFlags(struct SetscState *ms, int flags)
{
    return NODESET_EXPORTED_FUNC(newMutFlags)(ms, flags);
}

int
NyNodeSet_setobj(NyNodeSetObject *v, PyObject *obj)
{
    return NODESET_EXPORTED_FUNC(setobj)(v, obj);
}

int
NyNodeSet_clrobj(NyNodeSetObject *v, PyObject *obj)
{
    return NODESET_EXPORTED_FUNC(clrobj)(v, obj);
}


int
NyNodeSet_hasobj(NyNodeSetObject *v, PyObject *obj)
{
    return NODESET_EXPORTED_FUNC(hasobj)(v, obj);
}

int NyNodeSet_iterate(NyNodeSetObject *ns,
                      int (*visit)(PyObject *, void *),
                      void *arg)
{
    return NODESET_EXPORTED_FUNC(iterate)(ns, visit, arg);;
}


NyNodeSetObject *
NyImmNodeSet_NewCopy(NyNodeSetObject *v)
{
    return NODESET_EXPORTED_FUNC(newImmCopy)(v);
}

NyNodeSetObject *
NyImmNodeSet_NewSingleton(struct SetscState *ms, PyObject *element, PyObject *hiding_tag)
{
    return NODESET_EXPORTED_FUNC(newImmSingleton)(ms, element, hiding_tag);
}

int
NyNodeSet_be_immutable(NyNodeSetObject **nsp)
{
    return NODESET_EXPORTED_FUNC(be_immutable)(nsp);
}

int
NySTWMutNodeSet_InitOnStack(struct SetscState *ms, NyNodeSetObject *v)
{
    NY_ASSERT_WORLD_STOPPED();
    return NODESET_EXPORTED_FUNC(initStw)(ms, v);
}

void
NySTWMutNodeSet_Destroy(NyNodeSetObject *v)
{
    NY_ASSERT_WORLD_STOPPED();
    NODESET_EXPORTED_FUNC(destroyStw)(v);
}

int
import_sets(struct HeapycState *ms)
{
    ms->nodeset_exports = PyCapsule_Import("guppy.sets.setsc.NyNodeSet_Exports", 0);
    if (!ms->nodeset_exports)
        return -1;

    _Py_atomic_store_ptr_relaxed(&nodeset_exports.newMut,
                                 ms->nodeset_exports->newMut);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.newMutHiding,
                                 ms->nodeset_exports->newMutHiding);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.newMutFlags,
                                 ms->nodeset_exports->newMutFlags);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.setobj,
                                 ms->nodeset_exports->setobj);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.clrobj,
                                 ms->nodeset_exports->clrobj);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.hasobj,
                                 ms->nodeset_exports->hasobj);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.iterate,
                                 ms->nodeset_exports->iterate);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.newImmCopy,
                                 ms->nodeset_exports->newImmCopy);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.newImmSingleton,
                                 ms->nodeset_exports->newImmSingleton);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.be_immutable,
                                 ms->nodeset_exports->be_immutable);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.initStw,
                                 ms->nodeset_exports->initStw);
    _Py_atomic_store_ptr_relaxed(&nodeset_exports.destroyStw,
                                 ms->nodeset_exports->destroyStw);
    return 0;
}
