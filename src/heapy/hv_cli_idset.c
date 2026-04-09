/* Implementation of the identity-set classifier */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "impsets.h"
#include "heapy.h"
#include "classifier.h"
#include "hv.h"

const char hv_cli_idset_doc[] = PyDoc_STR(
"HV.cli_idset() -> ObjectClassifier\n\
\n\
Return a classifier that classifies by set of identity.\n\
\n\
The classification of an object is a singleton immnodeset containing the object itself.");


static PyObject *
hv_cli_idset_classify(NyHeapViewObject *self, PyObject *arg)
{
    return (PyObject *)NyImmNodeSet_NewSingleton(arg, self->_hiding_tag_);
}

static int
hv_cli_idset_le(PyObject * self, PyObject *a, PyObject *b)
{
    return PyObject_RichCompareBool(a, b, Py_LE);
}


static NyObjectClassifierDef hv_cli_idset_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "cli_idset",
    "classifier returning singleton set containing object itself",
    (binaryfunc)hv_cli_idset_classify,
    (binaryfunc)0,
    hv_cli_idset_le,
};

PyObject *
hv_cli_idset(NyHeapViewObject *self, PyObject *args)
{
    return NyObjectClassifier_New((PyObject *)self, &hv_cli_idset_def);
}
