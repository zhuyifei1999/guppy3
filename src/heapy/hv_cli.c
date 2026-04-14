/* Classifier implementations */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "impsets.h"
#include "heapy.h"
#include "classifier.h"
#include "hv.h"

static PyObject *
hv_cli_none_classify(struct HeapycState *ms, NyHeapViewObject *self, PyObject *arg)
{
    Py_INCREF(Py_None);
    return Py_None;
}

static int
hv_cli_none_le(PyObject *self, PyObject *a, PyObject *b)
{
    return 1;
}

static NyObjectClassifierDef hv_cli_none_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "cli_none",
    "classifier returning None",
    (modstatebinaryfunc)hv_cli_none_classify,
    (modstatebinaryfunc)NULL,
    hv_cli_none_le,
};

const char hv_cli_none_doc[] = PyDoc_STR(
"HV.cli_none() -> ObjectClassifier\n\
\n\
Return a classifier that classifies all objects the same.\n\
\n\
The classification of each object is None.");

PyObject *
hv_cli_none(NyHeapViewObject *self, PyObject *args)
{
    return NyObjectClassifier_New(self->ms, (PyObject *)self, &hv_cli_none_def);
}

static PyObject *
hv_cli_type_classify(struct HeapycState *ms, NyHeapViewObject *hv, PyObject *obj)
{
    Py_INCREF(Py_TYPE(obj));
    return (PyObject *)Py_TYPE(obj);
}

static int
hv_cli_type_le(PyObject *self, PyObject *a, PyObject *b)
{
    return (a == b) || PyType_IsSubtype((PyTypeObject *)a, (PyTypeObject *)b);

}

static NyObjectClassifierDef hv_cli_type_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "cli_type",
    "classifier returning object type",
    (modstatebinaryfunc)hv_cli_type_classify,
    (modstatebinaryfunc)NULL,
    hv_cli_type_le,
};

const char hv_cli_type_doc[] = PyDoc_STR(
"HV.cli_type() -> ObjectClassifier\n\
\n\
Return a classifier that classifies by type.\n\
\n\
The classification of each object is the type, as given by its\n\
C-level member 'ob_type'. (This is the same as the type returned\n\
by the Python-level builtin 'type'.)");

PyObject *
hv_cli_type(NyHeapViewObject *self, PyObject *args)
{
    return NyObjectClassifier_New(self->ms, (PyObject *)self, &hv_cli_type_def);
}
