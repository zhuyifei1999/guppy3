/* Implementation of the "findex" classifier (for lack of a better name)
   a generalization of biper (bipartitioner)
   as discussed in Notes Sep 21 2005.

*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "heapy.h"
#include "classifier.h"
#include "hv.h"

const char hv_cli_findex_doc[] = PyDoc_STR(
"HV.cli_findex(tuple, memo) -> ObjectClassifier\n\
");


typedef struct {
    NYTUPLELIKE_HEAD
    PyObject *alts;
    PyObject *memo;
    PyObject *kinds;
    PyObject *cmps;
} FindexObject;
NYTUPLELIKE_ASSERT(FindexObject, alts);

static PyObject *
hv_cli_findex_memoized_kind(struct HeapycState *ms, FindexObject *self, PyObject *kind)
{
    PyObject *result;
    int r;

    NY_ASSERT_IMMUTABLE_BUILTIN(ms, kind);
    r = PyDict_GetItemRef(self->memo, kind, &result);
    if (r == -1)
        return NULL;
    if (result)
        return result;

    if (PyDict_SetItem(self->memo, kind, kind) == -1)
        return NULL;
    /* Caller assumes it owns both kind and the return value */
    return Py_NewRef(kind);
}


static PyObject *
hv_cli_findex_classify(struct HeapycState *ms, FindexObject *self, PyObject *obj)
{
    Py_ssize_t i, numalts;
    PyObject *kind, *ret, *index;
    numalts = PyTuple_GET_SIZE(self->alts);
    for (i = 0; i < numalts; i++) {
        PyObject *ckc = PyTuple_GET_ITEM(self->alts, i);
        NyObjectClassifierObject *cli = (void *)PyTuple_GET_ITEM(ckc, 0);
        PyObject *cmpkind = PyTuple_GET_ITEM(self->kinds, i);
        long cmp = PyLong_AsLong(PyTuple_GET_ITEM(self->cmps, i));
        if (cmp == -1 && PyErr_Occurred())
            return NULL;
        kind = cli->def->classify(ms, cli->self, obj);
        if (!kind)
            return NULL;
        cmp = NyObjectClassifier_Compare(cli, kind, cmpkind, cmp);
        Py_CLEAR(kind);
        if (cmp == -1)
            return NULL;
        if (cmp)
            break;
    }
    index = PyLong_FromSsize_t(i);
    if (!index)
        return NULL;
    ret = hv_cli_findex_memoized_kind(ms, self, index);
    Py_CLEAR(index);
    return ret;
}

static int
hv_cli_findex_le(PyObject *self, PyObject *a, PyObject *b)
{
    return PyObject_RichCompareBool(a, b, Py_LE);
}

static NyObjectClassifierDef hv_cli_findex_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "cli_findex",
    "classifier returning index of matching kind",
    (modstatebinaryfunc)hv_cli_findex_classify,
    (modstatebinaryfunc)hv_cli_findex_memoized_kind,
    hv_cli_findex_le,
};

PyObject *
hv_cli_findex(NyHeapViewObject *hv, PyObject *args)
{
    PyObject *r = NULL;
    FindexObject *s, tmp;
    Py_ssize_t numalts;
    Py_ssize_t i;
    if (!PyArg_ParseTuple(args, "O!O!:cli_findex",
                          &PyTuple_Type, &tmp.alts,
                          &PyDict_Type, &tmp.memo)) {
        return NULL;
    }
    numalts = PyTuple_GET_SIZE(tmp.alts);
    for (i = 0; i < numalts; i++) {
        PyObject *ckc = PyTuple_GET_ITEM(tmp.alts, i);
        if (!PyTuple_Check(ckc)) {
            PyErr_SetString(PyExc_TypeError, "Tuple of TUPLES expected.");
            return NULL;
        }
        if (PyTuple_GET_SIZE(ckc) != 3) {
            PyErr_SetString(PyExc_TypeError, "Tuple of TRIPLES expected.");
            return NULL;
        }
        if (!PyObject_TypeCheck(PyTuple_GET_ITEM(ckc, 0), hv->ms->ObjectClassifier_Type)) {
            PyErr_SetString(PyExc_TypeError, "Tuple of triples with [0] a CLASSIFIER expected.");
            return NULL;
        }
        if (!PyUnicode_Check(PyTuple_GET_ITEM(ckc, 2))) {
            PyErr_SetString(PyExc_TypeError, "Tuple of triples with [2] a STRING expected.");
            return NULL;
        }
        if (cli_cmp_as_int(PyTuple_GET_ITEM(ckc, 2)) == -1) {
            return NULL;
        }
    }
    s = NYTUPLELIKE_NEW(FindexObject);
    if (!s)
        return NULL;
    s->alts = Py_NewRef(tmp.alts);
    s->memo = Py_NewRef(tmp.memo);
    s->kinds = PyTuple_New(numalts);
    s->cmps = PyTuple_New(numalts);
    if (!s->kinds || !s->cmps)
        goto err;
    for (i = 0; i < numalts; i++) {
        PyObject *ckc = PyTuple_GET_ITEM(tmp.alts, i);
        NyObjectClassifierObject *cli = (void *)PyTuple_GET_ITEM(ckc, 0);
        PyObject *mk = PyTuple_GET_ITEM(ckc, 1);
        if (cli->def->memoized_kind) {
            mk = cli->def->memoized_kind(hv->ms, cli->self, mk);
            if (!mk)
                goto err;
        } else {
            Py_INCREF(mk);
        }
        PyTuple_SET_ITEM(s->kinds, i, mk);
        mk = PyLong_FromLong(cli_cmp_as_int(PyTuple_GET_ITEM(ckc, 2)));
        if (!mk)
            goto err;
        PyTuple_SET_ITEM(s->cmps, i, mk);

    }
    r = NyObjectClassifier_New(hv->ms, (PyObject *)s, &hv_cli_findex_def);
err:
    Py_CLEAR(s);
    return r;
}
