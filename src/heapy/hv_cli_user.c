/* Implementation of user defined classifiers. */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "heapy.h"
#include "classifier.h"
#include "hv.h"
#include "stoptheworld.h"

const char hv_cli_user_defined_doc[] = PyDoc_STR(
"\n"
);

typedef struct {
    /* Mimics a tuple */
    NYTUPLELIKE_HEAD
    NyObjectClassifierObject *cond_cli;
    PyObject *cond_kind;
    PyObject *classify;
    PyObject *memoized_kind;
} UserObject;
NYTUPLELIKE_ASSERT(UserObject, cond_cli);

static PyObject *
hv_cli_user_memoized_kind(struct HeapycState *ms, UserObject *self, PyObject *kind)
{
    NY_ASSERT_WORLD_RUNNING(); /* PyObject_CallFunctionObjArgs */

    if (!Py_IsNone(self->memoized_kind) && !Py_IsNone(kind)) {
        kind = PyObject_CallFunctionObjArgs(self->memoized_kind, kind, 0);
    } else {
        Py_INCREF(kind);
    }
    return kind;
}

static PyObject *
hv_cli_user_classify(struct HeapycState *ms, UserObject *self, PyObject *obj)
{
    PyObject *kind;

    NY_ASSERT_WORLD_RUNNING(); /* PyObject_CallFunctionObjArgs */
    kind = self->cond_cli->def->classify(ms, self->cond_cli->self, obj);
    if (!kind)
        return NULL;
    if (kind != self->cond_kind) {
        Py_CLEAR(kind);
        Py_RETURN_NONE;
    } else {
        Py_CLEAR(kind);
        return PyObject_CallFunctionObjArgs(self->classify, obj, 0);
    }
}

static NyObjectClassifierDef hv_cli_user_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "cli_user_defined",
    "user defined classifier",
    (modstatebinaryfunc)hv_cli_user_classify,
    (modstatebinaryfunc)hv_cli_user_memoized_kind,
};



PyObject *
hv_cli_user_defined(NyHeapViewObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cond_cli", "cond_kind", "classify", "memoized_kind", NULL};
    UserObject *s, tmp;
    PyObject *r;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!OOO:user_defined", kwlist,
                                     self->ms->ObjectClassifier_Type, &tmp.cond_cli,
                                     &tmp.cond_kind,
                                     &tmp.classify,
                                     &tmp.memoized_kind
                                     ))
        return NULL;

    s = NYTUPLELIKE_NEW(UserObject);
    if (!s)
        return NULL;

    s->cond_cli = Ny_NEWREF(tmp.cond_cli);
    s->cond_kind = Py_NewRef(tmp.cond_kind);
    s->classify = Py_NewRef(tmp.classify);
    s->memoized_kind = Py_NewRef(tmp.memoized_kind);
    r = NyObjectClassifier_New(self->ms, (PyObject *)s, &hv_cli_user_def);
    Py_CLEAR(s);
    return r;
}
