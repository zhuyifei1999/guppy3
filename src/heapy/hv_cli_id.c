/* Implementation of the identity classifier */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "heapy.h"
#include "classifier.h"
#include "hv.h"

const char hv_cli_id_doc[] = PyDoc_STR(
"HV.cli_id() -> ObjectClassifier\n\
\n\
Return a classifier that classifies by identity.\n\
\n\
The classification of an object is the object itself.");

static PyObject *
hv_cli_id_classify(struct HeapycState *ms, NyHeapViewObject *self, PyObject *arg)
{
    return Py_NewRef(arg);
}

static int
hv_cli_id_le(PyObject *self, PyObject *a, PyObject *b)
{
    return a <= b;
}


static NyObjectClassifierDef hv_cli_id_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "cli_id",
    "classifier returning the object itself",
    (modstatebinaryfunc)hv_cli_id_classify,
    (modstatebinaryfunc)NULL,
    hv_cli_id_le,
};

PyObject *
hv_cli_id(NyHeapViewObject *self, PyObject *args)
{
    return NyObjectClassifier_New(self->ms, (PyObject *)self, &hv_cli_id_def);
}
