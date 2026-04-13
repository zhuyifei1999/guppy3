/* Classify by 'relation', incoming (perhaps outcoming)

    inrel
    outrel
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "structmember.h"
#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "heapy.h"
#include "impsets.h"
#include "heapdef.h"
#include "classifier.h"
#include "hv.h"
#include "nodegraph.h"
#include "relation.h"
#include "stoptheworld.h"
#include "utils.h"

const char hv_cli_inrel_doc[] = PyDoc_STR(
"HV.cli_inrel(referrers, memo) -> ObjectClassifier\n"
"\n"
"Return a classifier that classifes by \"incoming relations\".\n"
"\n"
"The classification of an object is the set of incoming relations.\n"
"\n"
"    referrers   A NodeGraph object used to\n"
"                map each object to its referrers.\n"
"\n"
"    memo        A dict object used to\n"
"                memoize the classification sets.\n"
);

PyDoc_STRVAR(rel_doc,
"");

static void
rel_dealloc(NyRelationObject *op)
{
    PyTypeObject *tp = Py_TYPE(op);
    PyObject_GC_UnTrack(op);
    Py_TRASHCAN_BEGIN(op, rel_dealloc)
    Py_XDECREF(op->relator);
    tp->tp_free(op);
    Py_CLEAR(tp);
    Py_TRASHCAN_END
}

static PyObject *
NyRelation_SubTypeNew(PyTypeObject *type, int kind, PyObject *relator)
{
    NyRelationObject *rel = (NyRelationObject *)type->tp_alloc(type, 1);
    if (!rel)
        return 0;
    rel->kind = kind;
    if (!relator) {
        relator = Py_None;
    }
    rel->relator = relator;
    Py_INCREF(relator);
    return (PyObject *)rel;
}

static NyRelationObject *
NyRelation_New(struct HeapycState *ms, int kind, PyObject *relator)
{
    return (NyRelationObject *)NyRelation_SubTypeNew(ms->Relation_Type, kind, relator);
}

static PyObject *
rel_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    int kind;
    PyObject *relator;
    static char *kwlist[] = {"kind", "relator", 0};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "iO:rel_new",kwlist,
                                     &kind,
                                     &relator))
        return NULL;
    if (! (0 < kind && kind < NYHR_LIMIT) ) {
        PyErr_Format(PyExc_ValueError,
                     "rel_new: Invalid relation kind: %d, must be > 0 and < %d.",
                     kind,
                     NYHR_LIMIT);
        return 0;
    }
    return NyRelation_SubTypeNew(type, kind, relator);
}


static int
rel_traverse(NyRelationObject *op, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(op));
    Py_VISIT(op->relator);
    return 0;
}

static int
rel_clear(NyRelationObject *op)
{
    Py_XDECREF(op->relator);
    op->relator = NULL;
    return 0;
}

static Py_hash_t
rel_hash(NyRelationObject *op)
{
    Py_hash_t x = PyObject_Hash(op->relator);
    if (x == -1)
        return -1;
    x ^= op->kind;
    if (x == -1)
        x = -2;
    return x;
}

static PyObject *
rel_richcompare(PyObject *v, PyObject *w, int op)
{
    struct HeapycState *ms = NyType_AssertModuleState(Py_TYPE(v), &heapyc_def);
    NyRelationObject *vr, *wr;
    int vkind, wkind;
    if (!PyObject_TypeCheck(v, ms->Relation_Type) ||
        !PyObject_TypeCheck(w, ms->Relation_Type)
    )
        return Py_NewRef(Py_NotImplemented);

    vr = (NyRelationObject *)v;
    wr = (NyRelationObject *)w;
    vkind = vr->kind;
    wkind = wr->kind;
    if (vkind != wkind) {
        PyObject *result;
        int cmp;
        switch (op) {
        case Py_LT: cmp = vkind <  wkind; break;
        case Py_LE: cmp = vkind <= wkind; break;
        case Py_EQ: cmp = vkind == wkind; break;
        case Py_NE: cmp = vkind != wkind; break;
        case Py_GT: cmp = vkind >  wkind; break;
        case Py_GE: cmp = vkind >= wkind; break;
        default: return NULL; /* cannot happen */
        }
        result = cmp? Py_True:Py_False;
        Py_INCREF(result);
        return result;
    }
    return PyObject_RichCompare(vr->relator, wr->relator, op);
}

static PyMethodDef rel_methods[] = {
    {0} /* sentinel */
};

#define OFF(x) offsetof(NyRelationObject, x)

static PyMemberDef rel_members[] = {
    {"kind", T_INT, OFF(kind), READONLY},
    {"relator", T_OBJECT, OFF(relator), READONLY},
    {0} /* Sentinel */
};

#undef OFF

static PyType_Slot rel_slots[] = {
    {Py_tp_dealloc,     rel_dealloc},
    {Py_tp_hash,        rel_hash},
    {Py_tp_getattro,    PyObject_GenericGetAttr},
    {Py_tp_doc,         (void *)rel_doc},
    {Py_tp_traverse,    rel_traverse},
    {Py_tp_clear,       rel_clear},
    {Py_tp_richcompare, rel_richcompare},
    {Py_tp_methods,     rel_methods},
    {Py_tp_members,     rel_members},
    {Py_tp_alloc,       PyType_GenericAlloc},
    {Py_tp_new,         rel_new},
    {Py_tp_free,        PyObject_GC_Del},
    {0, NULL},
};

PyType_Spec NyRelation_Spec = {
    .name      = "guppy.heapy.heapyc.Relation",
    .basicsize = sizeof(NyRelationObject),
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IMMUTABLETYPE | Py_TPFLAGS_HAVE_GC,
    .slots     = rel_slots,
};

typedef struct {
    /* Mimics a tuple - xxx should perhaps make a proper object/use tuple macros?! */
    NYTUPLELIKE_HEAD
    NyHeapViewObject *hv;
    NyNodeGraphObject *rg;
    NyRelationObject *rel;
    PyObject *memokind, *memorel;
} InRelObject;
NYTUPLELIKE_ASSERT(InRelObject, hv);

typedef struct {
    struct HeapycState *ms;
    PyObject *memorel;
    NyNodeSetObject *ns;
} MemoRelArg;

static int
inrel_visit_memoize_relation(PyObject *obj, MemoRelArg *arg)
{
    PyObject *mrel;
    if (!PyObject_TypeCheck(obj, arg->ms->Relation_Type)) {
        PyErr_Format(PyExc_TypeError,
               "inrel_visit_memoize_relation: can only memoize relation (not \"%.200s\")",
               Py_TYPE(obj)->tp_name);
        return -1;
    }

    NY_ASSERT_IMMUTABLE_BUILTIN(arg->ms, obj);
    mrel = PyDict_GetItemWithError(arg->memorel, obj);
    if (!mrel) {
        if (PyErr_Occurred())
            return -1;
        if (PyDict_SetItem(arg->memorel, obj, obj) == -1)
            return -1;
        mrel = obj;
    }
    if (NyNodeSet_setobj(arg->ns, mrel) == -1)
        return -1;
    return 0;
}

static PyObject *
inrel_fast_memoized_kind(struct HeapycState *ms, InRelObject *self, PyObject *kind)
     /* When the elements are already memoized */
{
    PyObject *result;
    int r;

    NY_ASSERT_IMMUTABLE_BUILTIN(ms, kind);
    r = PyDict_GetItemRef(self->memokind, kind, &result);
    if (r == -1)
        return NULL;
    if (result)
        return result;

    if (PyDict_SetItem(self->memokind, kind, kind) == -1)
        return NULL;
    /* Caller assumes it owns both kind and the return value */
    Py_INCREF(kind);
    return kind;
}


static PyObject *
hv_cli_inrel_memoized_kind(struct HeapycState *ms, InRelObject *self, PyObject *kind)
{
    MemoRelArg arg;
    PyObject *result;
    arg.ms = ms;
    arg.memorel = self->memorel;
    Ny_BEGIN_CRITICAL_SECTION(self->hv);
    arg.ns = hv_mutnodeset_new(self->hv);
    Ny_END_CRITICAL_SECTION();
    if (!arg.ns)
        return 0;
    if (iterable_iterate(ms, kind, (visitproc)inrel_visit_memoize_relation, &arg) == -1)
        goto Err;
    if (NyNodeSet_be_immutable(&arg.ns) == -1)
        goto Err;
    result = inrel_fast_memoized_kind(ms, self, (PyObject *)arg.ns);
Ret:
    Py_DECREF(arg.ns);
    return result;
Err:
    result = 0;
    goto Ret;
}

typedef struct {
    NyHeapRelate hr;
    int err;
    struct HeapycState *ms;
    NyNodeSetObject *relset;
    NyRelationObject *rel;
    PyObject *memorel;
} hv_cli_inrel_visit_arg;

static int
hv_cli_inrel_visit(unsigned int kind, PyObject *relator, NyHeapRelate *arg_)
{
    hv_cli_inrel_visit_arg *arg = (void *)arg_;
    PyObject *rel = NULL;
    int r;

    arg->err = -1;

    if (!relator) {
        if (PyErr_Occurred())
            return -1;
        relator = Py_None;
        Py_INCREF(relator);
    }

    arg->rel->kind = kind;
    arg->rel->relator = relator;

    NY_ASSERT_IMMUTABLE_BUILTIN(arg->ms, (PyObject *)arg->rel);
    r = PyDict_GetItemRef(arg->memorel, (PyObject *)arg->rel, &rel);
    if (r == -1)
        goto ret;
    if (!rel) {
        rel = (PyObject *)NyRelation_New(arg->ms, kind, relator);
        if (!rel)
            goto ret;
        NY_ASSERT_IMMUTABLE_BUILTIN(arg->ms, rel);
        if (PyDict_SetItem(arg->memorel, rel, rel) == -1)
            goto ret;
    }
    if (NyNodeSet_setobj(arg->relset, rel) != -1)
        arg->err = 0;
ret:
    Py_XDECREF(rel);
    Py_DECREF(relator);
    /* NyNodeSet_be_immutable might call into GC, causing arg->rel to be traversed
       this no longer happens after Py 3.12:
       https://github.com/python/cpython/commit/83eb827247dd */
    arg->rel->relator = Py_None;
    return arg->err;
}



static PyObject *
hv_cli_inrel_classify(struct HeapycState *ms, InRelObject *self, PyObject *obj)
{
    NyNodeGraphEdge *lo, *hi, *cur;
    PyObject *result = NULL;
    ExtraType *xt;
    NyNodeSetObject relset;
    hv_cli_inrel_visit_arg crva;
    crva.hr.flags = 0;
    crva.hr.hv = (PyObject *)self->hv;
    crva.hr.tgt = obj;
    crva.hr.visit = hv_cli_inrel_visit;
    crva.err = 0;
    crva.ms = ms;
    crva.memorel = self->memorel;
    assert(self->rel->relator == Py_None); /* This will be restored, w/o incref, at return. */
    crva.rel = self->rel;
    crva.relset = &relset;

    NY_STOP_WORLD();
    if (NySTWMutNodeSet_InitOnStack(ms->nodeset_exports->ms, &relset) == -1)
        goto err_start;
    if (NyNodeGraph_Region(self->rg, obj, &lo, &hi) == -1)
        goto err_start;
    for (cur = lo; cur < hi; cur++) {
        if (cur->tgt == Py_None)
            continue;
        crva.hr.src = cur->tgt;
        xt = hv_extra_type(self->hv, Py_TYPE(crva.hr.src));
        assert(xt->xt_hv == self->hv);
        assert(self->hv == (void *)crva.hr.hv);

        if (xt->xt_relate(xt, &crva.hr) == -1 || crva.err) {
            /* fprintf(stderr, "xt 0x%x\n", xt); */
            goto err_start;
        }
    }

    if (NyNodeSet_be_immutable(&crva.relset) == -1)
        goto err_start;
    result = inrel_fast_memoized_kind(ms, self, (PyObject *)crva.relset);

err_start:
    NySTWMutNodeSet_Destroy(&relset);
    NY_START_WORLD();
    Py_XDECREF(crva.relset);
    assert(self->rel->relator == Py_None);
    return result;

}


static int
hv_cli_inrel_le(PyObject *self, PyObject *a, PyObject *b)
{
    return PyObject_RichCompareBool(a, b, Py_LE);
}


static NyObjectClassifierDef hv_cli_inrel_def = {
    0,
    sizeof(NyObjectClassifierDef),
    "hv_cli_rcs",
    "classifier returning ...",
    (modstatebinaryfunc)hv_cli_inrel_classify,
    (modstatebinaryfunc)hv_cli_inrel_memoized_kind,
    hv_cli_inrel_le
};


PyObject *
hv_cli_inrel(NyHeapViewObject *hv, PyObject *args)
{
    PyObject *r;
    InRelObject *s, tmp;
    if (!PyArg_ParseTuple(args, "O!O!O!:cli_inrel",
                          hv->ms->NodeGraph_Type, &tmp.rg,
                          &PyDict_Type, &tmp.memokind,
                          &PyDict_Type, &tmp.memorel
                          )) {
        return NULL;
    }
    s = NYTUPLELIKE_NEW(InRelObject);
    if (!s)
        return 0;
    s->hv = hv;
    Py_INCREF(s->hv);
    s->rg = tmp.rg;
    Py_INCREF(s->rg);
    s->memokind = tmp.memokind;
    Py_INCREF(s->memokind);
    s->memorel = tmp.memorel;
    Py_INCREF(s->memorel);
    /* Init a relation object used for lookup, to save an allocation per relation. */
    s->rel = NyRelation_New(hv->ms, 1, Py_None); /* kind & relator will be changed  */
    if (!s->rel) {
        Py_DECREF(s);
        return 0;
    }
    r = NyObjectClassifier_New(hv->ms, (PyObject *)s, &hv_cli_inrel_def);
    Py_DECREF(s);
    return r;
}
