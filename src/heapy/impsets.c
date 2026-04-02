#define NyNodeSet_TYPE	(nodeset_exports->type)

#define NyNodeSet_Check(op) PyObject_TypeCheck(op, NyNodeSet_TYPE)

NyNodeSet_Exports *nodeset_exports;

/* Macro NODESET_EXPORTS where error (NULL) checking can be done */
#define NODESET_EXPORTS nodeset_exports

/* As of 3.14, PyCapsule_Import requires external synchronization if called
   from multiple threads, and I believe module_exec can be, if we are GIL-less
   eventually */
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
PyMutex nodeset_exports_mutex = {0};
#else
# define PyMutex_Lock(m) do {} while (0)
# define PyMutex_Unlock(m) do {} while (0)
#endif


NyNodeSetObject *
NyMutNodeSet_New(void) {
    return NODESET_EXPORTS->newMut();
}

NyNodeSetObject *
NyMutNodeSet_NewHiding(PyObject *tag) {
    return NODESET_EXPORTS->newMutHiding(tag);
}

NyNodeSetObject *
NyMutNodeSet_NewFlags(int flags) {
    return NODESET_EXPORTS->newMutFlags(flags);
}

int
NyNodeSet_setobj(NyNodeSetObject *v, PyObject *obj) {
    return NODESET_EXPORTS->setobj(v, obj);
}

int
NyNodeSet_clrobj(NyNodeSetObject *v, PyObject *obj) {
    return NODESET_EXPORTS->clrobj(v, obj);
}


int
NyNodeSet_hasobj(NyNodeSetObject *v, PyObject *obj) {
    return NODESET_EXPORTS->hasobj(v, obj);
}

int NyNodeSet_iterate(NyNodeSetObject *ns,
                      int (*visit)(PyObject *, void *),
                      void *arg) {
    return NODESET_EXPORTS->iterate(ns, visit, arg);;
}


NyNodeSetObject *
NyNodeSet_NewImmCopy(NyNodeSetObject *v) {
    return NODESET_EXPORTS->newImmCopy(v);
}

NyNodeSetObject *
NyImmNodeSet_NewSingleton(PyObject *element, PyObject *hiding_tag) {
    return NODESET_EXPORTS->newImmSingleton(element, hiding_tag);
}

int
NyNodeSet_be_immutable(NyNodeSetObject **nsp) {
    return NODESET_EXPORTS->be_immutable(nsp);
}

static int
import_sets(void)
{
    if (nodeset_exports)
        return 0;

    PyMutex_Lock(&nodeset_exports_mutex);

    if (nodeset_exports)
        goto out;
    nodeset_exports = PyCapsule_Import("guppy.sets.setsc.NyNodeSet_Exports", 0);
    if (!nodeset_exports)
        goto err;

out:
    PyMutex_Unlock(&nodeset_exports_mutex);
    return 0;
err:
    PyMutex_Unlock(&nodeset_exports_mutex);
    return -1;
}
