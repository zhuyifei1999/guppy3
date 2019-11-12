/* Implementation of the 'prod' classifier */

PyDoc_STRVAR(hv_cli_prod_doc,
"HV.cli_prod(memo) -> ObjectClassifier\n"
"\n"
"Return a classifier that classifes by \"producer\".\n"
"\n"
"The classification of an object is the fine name and line number\n"
"in which the object is produced (allocated).\n"
"\n"
"    memo        A dict object used to\n"
"                memoize the classification sets.\n"
);

// The sizeof of PyGC_Head is not to be trusted upon even across Python minor
// released. Eg: python/cpython@8766cb7
static Py_ssize_t sizeof_PyGC_Head;

static void lazy_init_hv_cli_prod()
{
    if (sizeof_PyGC_Head)
        return;

    if (PyLong_AsLong(PySys_GetObject("hexversion")) == PY_VERSION_HEX) {
        sizeof_PyGC_Head = sizeof(PyGC_Head);
        return;
    }

    PyObject *_testcapimodule, *_testcapi_SIZEOF_PYGC_HEAD = NULL;

    _testcapimodule = PyImport_ImportModule("_testcapi");
    if (!_testcapimodule)
        goto Err;

    _testcapi_SIZEOF_PYGC_HEAD = PyObject_GetAttrString(
        _testcapimodule, "SIZEOF_PYGC_HEAD");
    if (!_testcapi_SIZEOF_PYGC_HEAD)
        goto Err;

    sizeof_PyGC_Head = PyLong_AsSsize_t(_testcapi_SIZEOF_PYGC_HEAD);
    if (sizeof_PyGC_Head < 0)
        goto Err;

    Py_DECREF(_testcapimodule);
    Py_DECREF(_testcapi_SIZEOF_PYGC_HEAD);
    return;

Err:
    Py_XDECREF(_testcapimodule);
    Py_XDECREF(_testcapi_SIZEOF_PYGC_HEAD);

    PyErr_Clear();
    sizeof_PyGC_Head = sizeof(PyGC_Head);
    PyErr_WarnFormat(PyExc_UserWarning, 1,
                     "Unable to determine sizeof(PyGC_Head) from "
                     "_testcapi.SIZEOF_PYGC_HEAD, assuming %zd",
                     sizeof_PyGC_Head);
}

typedef struct {
    PyObject_VAR_HEAD
    NyHeapViewObject *hv;
    PyObject *memo;
} ProdObject;

static PyObject *
hv_cli_prod_memoized_kind(ProdObject * self, PyObject *kind)
{
    PyObject *result = PyDict_GetItem(self->memo, kind);
    if (!result) {
        if (PyErr_Occurred())
            goto Err;
        if (PyDict_SetItem(self->memo, kind, kind) == -1)
            goto Err;
        result = kind;
    }
    Py_INCREF(result);
    return result;
Err:
    return 0;
}

static PyObject *
hv_cli_prod_classify(ProdObject *self, PyObject *obj)
{
    Py_uintptr_t ptr;
    PyTypeObject *type;
    PyObject *kind = NULL;
    PyObject *tb, *result;

    // Refer to _tracemalloc.c:_tracemalloc__get_object_traceback
    type = Py_TYPE(obj);
    if (PyType_IS_GC(type)) {
        ptr = (Py_uintptr_t)((char *)obj - sizeof_PyGC_Head);
    } else {
        ptr = (Py_uintptr_t)obj;
    }

    tb = _PyTraceMalloc_GetTraceback(0, (Py_uintptr_t)ptr);

    if (!tb)
        goto Err;

    if (PySequence_Check(tb) && PySequence_Length(tb)) {
        kind = PySequence_GetItem(tb, 0);
    } else {
        kind = Py_None;
        Py_INCREF(Py_None);
    }

    result = hv_cli_prod_memoized_kind(self, kind);
    Py_DECREF(tb);
    Py_DECREF(kind);
    return result;

Err:
    Py_XDECREF(tb);
    Py_XDECREF(kind);
    return 0;
}

static int
hv_cli_prod_le(PyObject * self, PyObject *a, PyObject *b)
{
    return PyObject_RichCompareBool(a, b, Py_LE);
}

static NyObjectClassifierDef hv_cli_prod_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "hv_cli_prod",
    "classifier returning object producer",
    (binaryfunc)hv_cli_prod_classify,
    (binaryfunc)hv_cli_prod_memoized_kind,
    hv_cli_prod_le
};

static PyObject *
hv_cli_prod(NyHeapViewObject *self, PyObject *args)
{
    PyObject *r, *memo;
    ProdObject *s;
    if (!PyArg_ParseTuple(args, "O!:cli_prod",
                          &PyDict_Type, &memo))
        return NULL;

    lazy_init_hv_cli_prod();

    s = NYTUPLELIKE_NEW(ProdObject);
    if (!s)
        return 0;
    s->hv = self;
    Py_INCREF(s->hv);
    s->memo = memo;
    Py_INCREF(memo);
    r = NyObjectClassifier_New((PyObject *)s, &hv_cli_prod_def);
    Py_DECREF(s);
    return r;
}
