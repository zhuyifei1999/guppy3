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
    NyNodeGraphObject *rg;
    NyNodeSetObject *norefer;
    PyObject *dict;
} UserObject;
NYTUPLELIKE_ASSERT(UserObject, cond_cli);

static PyObject *
hv_cli_user_memoized_kind(struct HeapycState *ms, UserObject *self, PyObject *kind)
{
    NY_ASSERT_WORLD_RUNNING(); /* PyObject_CallFunctionObjArgs */

    if (self->memoized_kind != Py_None && kind != Py_None) {
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
      return 0;
    if (kind != self->cond_kind) {
        Py_DECREF(kind);
        kind = Py_None;
        Py_INCREF(kind);
        return kind;
    } else {
        Py_DECREF(kind);
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
    static char *kwlist[] = {"cond_cli", "cond_kind", "classify", "memoized_kind", 0};
    UserObject *s, tmp;
    PyObject *r;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!OOO:user_defined", kwlist,
                                     self->ms->ObjectClassifier_Type, &tmp.cond_cli,
                                     &tmp.cond_kind,
                                     &tmp.classify,
                                     &tmp.memoized_kind
                                     ))
      return 0;

    s = NYTUPLELIKE_NEW(UserObject);
    if (!s)
      return 0;

    s->cond_cli = tmp.cond_cli;
    Py_INCREF(s->cond_cli);
    s->cond_kind = tmp.cond_kind;
    Py_INCREF(s->cond_kind);
    s->classify = tmp.classify;
    Py_INCREF(s->classify);
    s->memoized_kind = tmp.memoized_kind;
    Py_INCREF(s->memoized_kind);
    r = NyObjectClassifier_New(self->ms, (PyObject *)s, &hv_cli_user_def);
    Py_DECREF(s);
    return r;
}
