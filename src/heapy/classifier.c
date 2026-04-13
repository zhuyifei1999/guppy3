/* ObjectClassifier object implementation */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "structmember.h"
#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "heapy.h"
#include "classifier.h"
#include "stoptheworld.h"
#include "nodegraph.h"
#include "utils.h"

PyDoc_STRVAR(cli_doc,
"This is the type of objects created by the hv_cli_* factory methods of\n"
"HeapView objects. See HeapView.__doc__ and the factory methods for the\n"
"different kinds of classifiers that are supported.\n"
);

int
NyObjectClassifier_Compare(NyObjectClassifierObject *cli, PyObject *a, PyObject *b, int cmp)
{
    /* This may in principle support more comparisions than the Python ones,
       hence the use of separate code definitions.
    */
    switch (cmp) {
    case CLI_LT:
        if (a == b)
            return 0;
        /* Fallin LE */
    case CLI_LE:
        return cli->def->cmp_le(cli->self, a, b);
    case CLI_EQ:
        return (a == b);
    case CLI_NE:
        return (a != b);
    case CLI_GT:
        if (a == b)
            return 0;
        /* Fallin GE */
    case CLI_GE:
        return cli->def->cmp_le(cli->self, b, a);
    default:
        PyErr_SetString(PyExc_ValueError, "Invalid cmp argument to NyNyObjectClassifier_Compare");
        return -1;
    }
}

static void
cli_dealloc(NyObjectClassifierObject *op)
{
    PyTypeObject *tp = Py_TYPE(op);
    PyObject_GC_UnTrack(op);
    Py_TRASHCAN_BEGIN(op, cli_dealloc)
    Py_CLEAR(op->self);
    PyObject_GC_Del(op);
    Py_CLEAR(tp);
    Py_TRASHCAN_END
}

static int
cli_traverse(NyObjectClassifierObject *op, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(op));
    Py_VISIT(op->self);
    return 0;
}


static int
cli_clear(NyObjectClassifierObject *op)
{
    Py_CLEAR(op->self);
    return 0;
}

PyDoc_STRVAR(cli_classify_doc,
"C.classify(object) -> object\n"
"\n"
"Return the kind of an object.\n"
"\n"
"The object is classified by C, to get its kind which is then returned."
);

static PyObject *
cli_classify(NyObjectClassifierObject *self, PyObject *object)
{
    struct HeapycState *ms = NyType_AssertModuleState(Py_TYPE(self), &heapyc_def);
    return self->def->classify(ms, self->self, object);
}


typedef struct {
    NyObjectClassifierObject *self;
    struct HeapycState *ms;
    PyObject *map;
    NyNodeGraphObject *emap;
} PATravArg;

PyDoc_STRVAR(cli_partition_doc,
"C.partition(X:iterable) -> dict\n"
"\n"
"Return a partition of a set of objects.\n"
"\n"
"Each object in X is classified by C to get its kind. The partition\n"
"returned is a mapping from each different kind to a list containing\n"
"the objects of that kind."
);

static int
cli_partition_iter(PyObject *obj, PATravArg *ta)
{
    PyObject *sp = NULL;
    PyObject *kind = ta->self->def->classify(ta->ms, ta->self->self, obj);
    int r;

    if (!kind)
        return -1;

    /* This does not qualify for NY_ASSERT_IMMUTABLE_BUILTIN

       hv_cli_type_classify may return a metaclass, which may have a custom
       __hash__, in which case the kind will have a tp_hash that calls into
       Python code. */
    NY_ASSERT_WORLD_RUNNING();
    r = PyDict_GetItemRef(ta->map, kind, &sp);
    if (r == -1)
        goto Err;
    if (!sp) {
        sp = PyList_New(0);
        if (!sp)
            goto Err;
        if (PyObject_SetItem(ta->map, kind, sp) == -1)
            goto Err;
    }
    if (PyList_Append(sp, obj) == -1)
        goto Err;
    Py_DECREF(sp);
    Py_DECREF(kind);
    return 0;
Err:
    Py_XDECREF(sp);
    Py_XDECREF(kind);
    return -1;
}

static PyObject *
cli_partition(NyObjectClassifierObject *self, PyObject *args)
{
    struct HeapycState *ms = NyType_AssertModuleState(Py_TYPE(self), &heapyc_def);
    PATravArg ta;
    PyObject *iterable;
    if (!PyArg_ParseTuple(args, "O:partition",
                          &iterable))
        return NULL;
    ta.self = self;
    ta.ms = ms;
    ta.map = PyDict_New();
    if (!ta.map)
        goto Err;
    if (iterable_iterate(ms, iterable, (visitproc)cli_partition_iter, &ta) == -1)
        goto Err;
    return ta.map;

Err:
    Py_XDECREF(ta.map);
    return NULL;
}

static int
cli_epartition_iter(PyObject *obj, PATravArg *ta)
{
    PyObject *kind = ta->self->def->classify(ta->ms, ta->self->self, obj);
    if (!kind)
        return -1;

    if (NyNodeGraph_AddEdge(ta->emap, kind, obj) == -1) {
        Py_DECREF(kind);
        return -1;
    }
    Py_DECREF(kind);
    return 0;
}

static PyObject *
cli_epartition(NyObjectClassifierObject *self, PyObject *iterable)
{
    struct HeapycState *ms = NyType_AssertModuleState(Py_TYPE(self), &heapyc_def);
    PATravArg ta;
    ta.self = self;
    ta.ms = ms;
    ta.emap = NyNodeGraph_New(ms);
    if (!ta.emap)
        goto Err;
    if (iterable_iterate(ms, iterable, (visitproc)cli_epartition_iter, &ta) == -1)
        goto Err;
    return (PyObject *)ta.emap;

Err:
    Py_XDECREF(ta.emap);
    return NULL;
}

PyDoc_STRVAR(cli_select_doc,
"C.select(X:iterable, kind:object, cmp:string) -> list\n"
"\n"
"Select objects of a particular kind.\n"
"\n"
"Each object in X is classified by C to get its kind and this is\n"
"compared with the kind argument to determine if the object will\n"
"be returned in the return list. The cmp argumt tells how the\n"
"kind of the object OK is compared to the kind argument AK \n"
"and can be one of < <= == != > >= .\n"
"\n"
"The comparison '<=' used depends on the classifier. Not all\n"
"classifier may have such a comparison available. The common\n"
"cases where it is available are:\n"
"\n"
"For the type classifier:\n"
"\n"
"    A <= B means A is a subtype of B.\n"
"\n"
"For the size classifier:\n"
"\n"
"    A <= B means that the size A is less or equal than B.\n"
"\n"
"For the referenced-by classifier:\n"
"\n"
"    A <= B means that A is a subset of B.\n"
);

typedef struct {
    NyObjectClassifierObject *cli;
    struct HeapycState *ms;
    PyObject *kind, *ret;
    int cmp;
} SELTravArg;

static int
cli_select_kind(PyObject *obj, SELTravArg *ta)
{
    PyObject *kind = ta->cli->def->classify(ta->ms, ta->cli->self, obj);
    int cmp;
    if (!kind)
        return -1;

    cmp = NyObjectClassifier_Compare(ta->cli, kind, ta->kind, ta->cmp);
    if (cmp == -1)
        goto Err;
    if (cmp) {
        if ( PyList_Append(ta->ret, obj) == -1) {
            goto Err;
        }
    }
    Py_DECREF(kind);
    return 0;
Err:
    Py_DECREF(kind);
    return -1;
}

static const char *cmp_strings[] = {
    "<",
    "<=",
    "==",
    "!=",
    ">",
    ">=",
    0
};

int
cli_cmp_as_int(PyObject *cmp)
{
    const char *s, *c;
    int i;
    if (!PyUnicode_Check(cmp)) {
        PyErr_SetString(PyExc_TypeError, "Compare argument must be a string.");
        return -1;
    }
    s = PyUnicode_AsUTF8(cmp);
    for (i = 0; (c = cmp_strings[i]); i++) {
        if (strcmp(c, s) == 0)
            return i;
    }
    PyErr_SetString(PyExc_ValueError, "Compare argument must be one of < <= == != > >=");
        return -1;
}

static PyObject *
cli_select(NyObjectClassifierObject *self, PyObject *args)
{
    struct HeapycState *ms = NyType_AssertModuleState(Py_TYPE(self), &heapyc_def);
    SELTravArg ta;
    PyObject *X, *cmp;

    int r;
    if (!PyArg_ParseTuple(args, "OOO:select",
                          &X, &ta.kind, &cmp)) {
        return NULL;
    }
    ta.cmp = cli_cmp_as_int(cmp);
    if (ta.cmp == -1)
        return 0;
    if (!(0 <= ta.cmp && ta.cmp <= CLI_MAX)) {
        PyErr_SetString(PyExc_ValueError, "Invalid value of cmp argument.");
        return 0;
    }
    if (!(ta.cmp == CLI_EQ || ta.cmp == CLI_NE || self->def->cmp_le)) {
        PyErr_SetString(PyExc_ValueError, "This classifier supports only equality selection.");
        return 0;
    }
    if (self->def->memoized_kind) {
        if (!(ta.kind = self->def->memoized_kind(ms, self->self, ta.kind)))
            return 0;
    } else {
        Py_INCREF(ta.kind);
    }
    ta.ret = PyList_New(0);
    if (!ta.ret)
        goto err;
    ta.cli = self;
    ta.ms = ms;
    r = iterable_iterate(ms, X, (visitproc)cli_select_kind, &ta);
    if (r == -1) {
        Py_DECREF(ta.ret);
        ta.ret = 0;
    }
err:
    Py_DECREF(ta.kind);
    return ta.ret;
}



static PyMethodDef cli_methods[] = {
    {"classify",(PyCFunction)cli_classify, METH_O, cli_classify_doc},
    {"partition",(PyCFunction)cli_partition, METH_VARARGS, cli_partition_doc},
    {"epartition",(PyCFunction)cli_epartition, METH_O, cli_partition_doc},
    {"select",(PyCFunction)cli_select, METH_VARARGS, cli_select_doc},
    {0} /* sentinel */
};

#define OFF(x) offsetof(NyObjectClassifierObject, x)

static PyMemberDef cli_members[] = {
    {"self",  T_OBJECT,     OFF(self), READONLY},
    {0} /* Sentinel */
};

#undef OFF

static PyType_Slot cli_slots[] = {
    {Py_tp_dealloc,  cli_dealloc},
    {Py_tp_getattro, PyObject_GenericGetAttr},
    {Py_tp_doc,      (void *)cli_doc},
    {Py_tp_traverse, cli_traverse},
    {Py_tp_clear,    cli_clear},
    {Py_tp_methods,  cli_methods},
    {Py_tp_members,  cli_members},
    {Py_tp_alloc,    PyType_GenericAlloc},
    {Py_tp_free,     PyObject_GC_Del},
    {0, NULL},
};

PyType_Spec NyObjectClassifier_Spec = {
    .name      = "guppy.heapy.heapyc.ObjectClassifier",
    .basicsize = sizeof(NyObjectClassifierObject),
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IMMUTABLETYPE |
                 Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots     = cli_slots,
};

PyObject *
NyObjectClassifier_New(struct HeapycState *ms, PyObject *self, NyObjectClassifierDef *def)
{
    NyObjectClassifierObject *op;
    op = PyObject_GC_New(NyObjectClassifierObject, ms->ObjectClassifier_Type);
    if (!op)
        return 0;
    Py_INCREF(self);
    op->self = self;
    op->def = def;
    PyObject_GC_Track(op);
    return (PyObject *)op;
}
