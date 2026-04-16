/* Implementation of ImmNodeSet */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "structmember.h"
#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "../heapy/heapdef.h"
#include "sets_internal.h"

PyDoc_STRVAR(immnodeset_doc,
"ImmNodeSet([iterable])\n"
"\n"
"Return a new immutable nodeset with elements from iterable.\n"
"\n"
"An immutable nodeset inherits the operations defined for NodeSet.\n"
"It also supports the following operation:\n"
"\n"
"hash(x)    -> int\n"
"\n"
"Return a hash value based on the addresses of the elements."
);


/* NyImmNodeSetIter methods */

typedef struct {
    PyObject_HEAD
    int i;
    NyNodeSetObject *nodeset; /* Need to hold on to this 'cause it shouldn't decref
                                 objects in set*/
} NyImmNodeSetIterObject;


static int
immnsiter_clear(NyImmNodeSetIterObject *it)
{
    Py_CLEAR(it->nodeset);
    return 0;
}

static void
immnsiter_dealloc(NyImmNodeSetIterObject *it)
{
    PyTypeObject *tp = Py_TYPE(it);
    PyObject_GC_UnTrack(it);
    Py_TRASHCAN_BEGIN(it, immnsiter_dealloc)
    immnsiter_clear(it);
    PyObject_GC_Del(it);
    Py_CLEAR(tp);
    Py_TRASHCAN_END
}


static int
immnsiter_traverse(NyImmNodeSetIterObject *it, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(it));
    Py_VISIT(it->nodeset);
    return 0;
}


static PyObject *
immnsiter_iternext(NyImmNodeSetIterObject *it)
{
    PyObject *ret = NULL;

    Ny_BEGIN_CRITICAL_SECTION(it);
    if (it->nodeset && it->i < Py_SIZE(it->nodeset))
        ret = Py_NewRef(it->nodeset->u.nodes[it->i++]);
    else
        Py_CLEAR(it->nodeset);
    Ny_END_CRITICAL_SECTION();
    return ret;
}

static PyType_Slot immnsiter_slots[] = {
    {Py_tp_dealloc,  immnsiter_dealloc},
    {Py_tp_getattro, PyObject_GenericGetAttr},
    {Py_tp_traverse, immnsiter_traverse},
    {Py_tp_clear,    immnsiter_clear},
    {Py_tp_iter,     PyObject_SelfIter},
    {Py_tp_iternext, immnsiter_iternext},
    {0, NULL}
};

PyType_Spec NyImmNodeSetIter_Spec = {
    .name      = "immnodeset-iterator",
    .basicsize = sizeof(NyImmNodeSetIterObject),
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IMMUTABLETYPE |
                 Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots     = immnsiter_slots,
};


/* immnodeset specific methods */


static NyNodeSetObject *
NyImmNodeSet_SubtypeNew(PyTypeObject *type, NyBit size, PyObject *hiding_tag)
{
    struct SetscState *ms = NyType_AssertModuleState(type, &setsc_def);
    NyNodeSetObject *v = (void *)type->tp_alloc(type, size);
    if (!v)
        return NULL;
    v->flags = NS_HOLDOBJECTS;
    v->ms = ms;
    v->_hiding_tag_ = Py_XNewRef(hiding_tag);
    memset(v->u.nodes, 0, sizeof(*v->u.nodes) * size);
    return v;
}

NyNodeSetObject *
NyImmNodeSet_New(struct SetscState *ms, NyBit size, PyObject *hiding_tag)
{
    return NyImmNodeSet_SubtypeNew(ms->ImmNodeSet_Type, size, hiding_tag);
}


NyNodeSetObject *
NyImmNodeSet_NewSingleton(struct SetscState *ms, PyObject *element, PyObject *hiding_tag)
{
    NyNodeSetObject *s = NyImmNodeSet_New(ms, 1, hiding_tag);
    if (!s)
        return NULL;
    s->u.nodes[0] = Py_NewRef(element);
    return s;
}


typedef struct {
    NyNodeSetObject *ns;
    int i;
} NSISetArg;

static int
as_immutable_visit(PyObject *obj, NSISetArg *v)
{
    v->ns->u.nodes[v->i] = Py_NewRef(obj);
    v->i += 1;
    return 0;
}

static NyNodeSetObject *
NyImmNodeSet_SubtypeNewCopy(PyTypeObject *type, NyNodeSetObject *v)
{
    PyObject *v_hiding_tag;
    NSISetArg sa;

    Ny_BEGIN_CRITICAL_SECTION(v);
    v_hiding_tag = Py_XNewRef(v->_hiding_tag_);
    Ny_END_CRITICAL_SECTION();

    sa.i = 0;
    sa.ns = NyImmNodeSet_SubtypeNew(type, Py_SIZE(v), v_hiding_tag);
    Py_CLEAR(v_hiding_tag);
    if (!sa.ns)
        return NULL;
    NyNodeSet_iterate(v, (visitproc)as_immutable_visit, &sa);
    return sa.ns;
}

NyNodeSetObject *
NyImmNodeSet_NewCopy(NyNodeSetObject *v)
{
    return NyImmNodeSet_SubtypeNewCopy(v->ms->ImmNodeSet_Type, v);
}


static NyNodeSetObject *
NyImmNodeSet_SubtypeNewIterable(PyTypeObject *type, PyObject *iterable, PyObject *hiding_tag)
{
    struct SetscState *ms = NyType_AssertModuleState(type, &setsc_def);
    NyNodeSetObject *imms, *muts;
    muts = NyMutNodeSet_SubtypeNewIterable(ms->MutNodeSet_Type, iterable, hiding_tag);
    if (!muts)
        return NULL;
    imms = NyImmNodeSet_SubtypeNewCopy(type, muts);
    Py_CLEAR(muts);
    return imms;
}

int
NyNodeSet_be_immutable(NyNodeSetObject **nsp) {
    NyNodeSetObject *cp = NyImmNodeSet_NewCopy(*nsp);
    if (!cp)
        return -1;
    if (!((*nsp)->flags & _NS_STW))
        Py_CLEAR(*nsp);
    *nsp = cp;
    return 0;
}


static PyObject *
immnodeset_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    struct SetscState *ms = NyType_AssertModuleState(type, &setsc_def);
    PyObject *iterable = NULL;
    PyObject *hiding_tag = NULL;
    bool hiding_tag_diff;

    static char *kwlist[] = {"iterable", "hiding_tag", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO:ImmNodeSet.__new__",kwlist,
                                     &iterable,
                                     &hiding_tag
                                     ))
        return NULL;
    if (type != ms->ImmNodeSet_Type)
        goto new;
    if (!iterable || Py_TYPE(iterable) != ms->ImmNodeSet_Type)
        goto new;

    Ny_BEGIN_CRITICAL_SECTION(iterable);
    hiding_tag_diff = ((NyNodeSetObject *)iterable)->_hiding_tag_ != hiding_tag;
    Ny_END_CRITICAL_SECTION();
    if (hiding_tag_diff)
        goto new;

    return Py_NewRef(iterable);

new:
    return (PyObject *)NyImmNodeSet_SubtypeNewIterable(type, iterable, hiding_tag);
}


static int
immnodeset_gc_clear(NyNodeSetObject *v)
{
    /* NOT LOCKED: Object is dying */
    assert(!(v->flags & _NS_STW));

    Py_CLEAR(v->_hiding_tag_);
    if (v->flags & NS_HOLDOBJECTS) {
        NyBit i;
        for (i = 0; i < Py_SIZE(v); i++)
            Py_CLEAR(v->u.nodes[i]);
    }
    return 0;
}

static void
immnodeset_dealloc(NyNodeSetObject *v)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)v))
        return;  /* resurrected */
    PyTypeObject *tp = Py_TYPE(v);
    PyObject_GC_UnTrack(v);
    Py_TRASHCAN_BEGIN(v, immnodeset_dealloc)
    immnodeset_gc_clear(v);
    tp->tp_free((PyObject *)v);
    Py_CLEAR(tp);
    Py_TRASHCAN_END
}



static int
immnodeset_gc_traverse(NyNodeSetObject *v, visitproc visit, void *arg)
{
    /* NOT LOCKED: Stop the world from GC */
    assert(!(v->flags & _NS_STW));

    NyBit i;
    Py_VISIT(Py_TYPE(v));
    if (v->flags & NS_HOLDOBJECTS)
        for (i = 0; i < Py_SIZE(v); i++)
            Py_VISIT(v->u.nodes[i]);
    Py_VISIT(v->_hiding_tag_);
    return 0;
}

static Py_hash_t
immnodeset_hash(NyNodeSetObject *v)
{
    /* NOT LOCKED: Immutable */
    NyBit i;
    Py_hash_t x = 0x983714;
    for (i = 0; i < Py_SIZE(v); i++)
        x ^= (Py_hash_t)v->u.nodes[i];
    if (x == -1)
        x = -2;
    return x;
}

#define OFF(x) offsetof(NyNodeSetObject, x)

static PyMemberDef immnodeset_members[] = {
    {"_hiding_tag_",     T_OBJECT_EX, OFF(_hiding_tag_), READONLY},
    {0} /* Sentinel */
};

#undef OFF

static  PyGetSetDef immnodeset_getset[] = {
    {"is_immutable", (getter)nodeset_get_is_immutable, (setter)0,
"S.is_immutable == True\n"
"\n"
"True since S is immutable."},
    {0}
};

static PyObject *
immnodeset_iter(NyNodeSetObject *ns)
{
    NyImmNodeSetIterObject *it = PyObject_GC_New(NyImmNodeSetIterObject, ns->ms->ImmNodeSetIter_Type);
    if (!it)
        return NULL;
    it->i = 0;
    it->nodeset = Ny_NEWREF(ns);
    PyObject_GC_Track(it);
    return (PyObject *)it;
}

NyNodeSetObject *
immnodeset_op(NyNodeSetObject *v, NyNodeSetObject *w, int op)
{
    /* NOT LOCKED: Immutable */
    int z;
    PyObject *pos;
    int bits, a, b;
    NyNodeSetObject *dst = NULL;
    PyObject **zf, **vf, **wf, **ve, **we;
    ve = &v->u.nodes[Py_SIZE(v)];
    we = &w->u.nodes[Py_SIZE(w)];
    for (z = 0, zf = NULL; ;) {
        for (vf = &v->u.nodes[0], wf = &w->u.nodes[0];;) {
            if (vf < ve) {
                if (wf < we) {
                    if (*vf <= *wf) {
                        pos = *vf;
                        a = 1;
                        if (*vf == *wf) {
                            b = 1;
                            wf++;
                        } else {
                            b = 0;
                        }
                        vf++;
                    } else {        /* (*vf > *wf) { */
                        pos = *wf;
                        a = 0;
                        b = 1;
                        wf++;
                    }
                } else {
                    pos = *vf;
                    a = 1;
                    vf++;
                    b = 0;
                }
            } else if (wf < we) {
                pos = *wf;
                a = 0;
                b = 1;
                wf++;
            } else
                break;
            switch(op) {
            case NyBits_AND: bits = a & b; break;
            case NyBits_OR : bits = a | b; break;
            case NyBits_XOR: bits = a ^ b; break;
            case NyBits_SUB: bits = a & ~b; break;
            default        : bits = 0; /* slicence undefined-warning */
                             assert(0);
            }
            if (bits) {
                if (zf) {
                    *zf = Py_NewRef(pos);
                    zf++;
                } else {
                    z++;
                }
            }
        }
        if (zf) {
            return dst;
        } else {
            dst = NyImmNodeSet_New(v->ms, z, v->_hiding_tag_);
            if (!dst)
                return dst;
            zf = &dst->u.nodes[0];
        }
    }
}

PyDoc_STRVAR(immnodeset_obj_at_doc,
"x.obj_at(address)\n"
"Return the object in x that is at a specified address, if any,\n"
"otherwise raise ValueError"
);


static PyObject *
immnodeset_obj_at(NyNodeSetObject *v, PyObject *obj)
{
    /* NOT LOCKED: Immutable */
    PyObject **lo;
    PyObject **hi;

    Py_uintptr_t addr =
#if SIZEOF_VOID_P <= SIZEOF_LONG
        PyLong_AsUnsignedLongMask(obj);
#else
        PyLong_AsUnsignedLongLongMask(obj);
#endif
    if (addr == (Py_uintptr_t) -1 && PyErr_Occurred())
        return NULL;

    lo = &v->u.nodes[0];
    hi = &v->u.nodes[Py_SIZE(v)];
    while (lo < hi) {
        PyObject **cur = lo + (hi - lo) / 2;
        if ((Py_uintptr_t)(*cur) == addr)
            return Py_NewRef(*cur);
        else if ((Py_uintptr_t)*cur < addr)
            lo = cur + 1;
        else
            hi = cur;
    }
    PyErr_Format(PyExc_ValueError, "No object found at address %p\n",(void *)addr);
    return NULL;
}


static PyMethodDef immnodeset_methods[] = {
    {"obj_at",    (PyCFunction)immnodeset_obj_at, METH_O, immnodeset_obj_at_doc},
    {0} /* sentinel */
};


static PyType_Slot immnodeset_slots[] = {
    {Py_tp_dealloc,     immnodeset_dealloc},
    {Py_tp_hash,        immnodeset_hash},
    {Py_tp_getattro,    PyObject_GenericGetAttr},
    {Py_tp_doc,         (void *)immnodeset_doc},
    {Py_tp_traverse,    immnodeset_gc_traverse},
    {Py_tp_clear,       immnodeset_gc_clear},
    {Py_tp_richcompare, nodeset_richcompare},
    {Py_tp_iter,        immnodeset_iter},
    {Py_tp_methods,     immnodeset_methods},
    {Py_tp_members,     immnodeset_members},
    {Py_tp_getset,      immnodeset_getset},
    {Py_tp_alloc,       PyType_GenericAlloc},
    {Py_tp_new,         immnodeset_new},
    {Py_tp_free,        PyObject_GC_Del},
    {0, NULL}
};

PyType_Spec NyImmNodeSet_Spec = {
    .name      = "guppy.sets.setsc.ImmNodeSet",
    .basicsize = offsetof(NyNodeSetObject, u.nodes),
    .itemsize  = sizeof(PyObject *),
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IMMUTABLETYPE |
                 Py_TPFLAGS_HAVE_GC | Ny_TPFLAGS_BASETYPE_ON_PY3_11,
    .slots     = immnodeset_slots,
};
