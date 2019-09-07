/* Implementation of ImmNodeSet */

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

static PyObject *nodeset_ior(NyNodeSetObject *v, PyObject *w);

typedef struct {
    PyObject_HEAD
    int i;
    NyNodeSetObject *nodeset; /* Need to hold on to this 'cause it shouldn't decref
                                 objects in set*/
} NyImmNodeSetIterObject;


static void
immnsiter_dealloc(NyImmNodeSetIterObject *it)
{
    PyObject_GC_UnTrack(it);
    Py_TRASHCAN_SAFE_BEGIN(it)
        Py_XDECREF(it->nodeset);
        PyObject_GC_Del(it);
    Py_TRASHCAN_SAFE_END(it)
}

static PyObject *
immnsiter_getiter(PyObject *it)
{
    Py_INCREF(it);
    return it;
}

static int
immnsiter_traverse(NyImmNodeSetIterObject *it, visitproc visit, void *arg)
{
    if (it->nodeset == NULL)
        return 0;
    return visit((PyObject *)it->nodeset, arg);
}



static PyObject *
immnsiter_iternext(NyImmNodeSetIterObject *it)
{
    if (it->nodeset && it->i < Py_SIZE(it->nodeset)) {
        PyObject *ret = it->nodeset->u.nodes[it->i];
        it->i += 1;
        Py_INCREF(ret);
        return ret;
    } else {
        Py_XDECREF(it->nodeset);
        it->nodeset = NULL;
        return NULL;
    }
}

PyTypeObject NyImmNodeSetIter_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "immnodeset-iterator",
    .tp_basicsize = sizeof(NyImmNodeSetIterObject),
    .tp_dealloc   = (destructor)immnsiter_dealloc,
    .tp_getattro  = PyObject_GenericGetAttr,
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
    .tp_traverse  = (traverseproc)immnsiter_traverse,
    .tp_iter      = (getiterfunc)immnsiter_getiter,
    .tp_iternext  = (iternextfunc)immnsiter_iternext,
};




/* immnodeset specific methods */


NyNodeSetObject *
NyImmNodeSet_SubtypeNew(PyTypeObject *type, NyBit size, PyObject *hiding_tag)
{
    NyNodeSetObject *v = (void *)type->tp_alloc(type, size);
    if (!v)
        return NULL;
    v->flags = NS_HOLDOBJECTS;
    v->_hiding_tag_ = hiding_tag;
    Py_XINCREF(hiding_tag);
    memset(v->u.nodes, 0, sizeof(*v->u.nodes) * size);
    return v;
}

NyNodeSetObject *
NyImmNodeSet_New(NyBit size, PyObject *hiding_tag)
{
    return NyImmNodeSet_SubtypeNew(&NyImmNodeSet_Type, size, hiding_tag);
}


NyNodeSetObject *
NyImmNodeSet_NewSingleton(PyObject *element, PyObject *hiding_tag)
{
    NyNodeSetObject *s = NyImmNodeSet_New(1, hiding_tag);
    if (!s)
        return 0;
    s->u.nodes[0] = element;
    Py_INCREF(element);
    return s;
}


typedef struct {
    NyNodeSetObject *ns;
    int i;
} NSISetArg;

static int
as_immutable_visit(PyObject *obj, NSISetArg *v)
{
    v->ns->u.nodes[v->i] = obj;
    Py_INCREF(obj);
    v->i += 1;
    return 0;
}

NyNodeSetObject *
NyImmNodeSet_SubtypeNewCopy(PyTypeObject *type, NyNodeSetObject *v)
{
    NSISetArg sa;
    sa.i = 0;
    sa.ns = NyImmNodeSet_SubtypeNew(type, Py_SIZE(v), v->_hiding_tag_);
    if (!sa.ns)
        return 0;
    NyNodeSet_iterate(v, (visitproc)as_immutable_visit, &sa);
    return sa.ns;
}

NyNodeSetObject *
NyImmNodeSet_NewCopy(NyNodeSetObject *v)
{
    return NyImmNodeSet_SubtypeNewCopy(&NyImmNodeSet_Type, v);
}


NyNodeSetObject *
NyImmNodeSet_SubtypeNewIterable(PyTypeObject *type, PyObject *iterable, PyObject *hiding_tag)
{
    NyNodeSetObject *imms, *muts;
    muts = NyMutNodeSet_SubtypeNewIterable(&NyMutNodeSet_Type, iterable, hiding_tag);
    if (!muts)
        return 0;
    imms = NyImmNodeSet_SubtypeNewCopy(type, muts);
    Py_DECREF(muts);
    return imms;
}

int
NyNodeSet_be_immutable(NyNodeSetObject **nsp) {
    NyNodeSetObject *cp = NyImmNodeSet_NewCopy(*nsp);
    if (!cp)
        return -1;
    Py_DECREF(*nsp);
    *nsp = cp;
    return 0;
}


static PyObject *
immnodeset_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{

    PyObject *iterable = NULL;
    PyObject *hiding_tag = NULL;
    static char *kwlist[] = {"iterable", "hiding_tag", 0};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO:ImmNodeSet.__new__",kwlist,
                                     &iterable,
                                     &hiding_tag
                                     ))
        return 0;
    if (type == &NyImmNodeSet_Type &&
        iterable &&
        Py_TYPE(iterable) == &NyImmNodeSet_Type &&
        ((NyNodeSetObject *)iterable)->_hiding_tag_ == hiding_tag) {
        Py_INCREF(iterable);
        return iterable;
    }
    return (PyObject *)NyImmNodeSet_SubtypeNewIterable(type, iterable, hiding_tag);
}


static int
immnodeset_gc_clear(NyNodeSetObject *v)
{
    if (v->_hiding_tag_) {
        PyObject *x = v->_hiding_tag_;
        v->_hiding_tag_ = 0;
        Py_DECREF(x);
    }
    if (v->flags & NS_HOLDOBJECTS) {
        NyBit i;
        for (i = 0; i < Py_SIZE(v); i++) {
            PyObject *x = v->u.nodes[i];
            if (x) {
                v->u.nodes[i] = 0;
                Py_DECREF(x);
            }
        }
    }
    return 0;
}

static void
immnodeset_dealloc(NyNodeSetObject *v)
{
    PyObject_GC_UnTrack(v);
    Py_TRASHCAN_SAFE_BEGIN(v)
    immnodeset_gc_clear(v);
    Py_TYPE(v)->tp_free((PyObject *)v);
    Py_TRASHCAN_SAFE_END(v)
}



static int
immnodeset_gc_traverse(NyNodeSetObject *v, visitproc visit, void *arg)
{
    NyBit i;
    int err;
    err = 0;
    if (v->flags & NS_HOLDOBJECTS) {
        for (i = 0; i < Py_SIZE(v); i++) {
            PyObject *x = v->u.nodes[i];
            if (x) {
                err = visit(x, arg);
                if (err)
                    return err;
            }
        }
    }
    if (v->_hiding_tag_) {
        err = visit(v->_hiding_tag_, arg);
    }
    return err;
}

static Py_hash_t
immnodeset_hash(NyNodeSetObject *v)
{
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
    NyImmNodeSetIterObject *it = PyObject_GC_New(NyImmNodeSetIterObject, &NyImmNodeSetIter_Type);
    if (!it)
        return 0;
    it->i = 0;
    it->nodeset = ns;
    Py_INCREF(ns);
    PyObject_GC_Track(it);
    return (PyObject *)it;
}

static NyNodeSetObject *
immnodeset_op(NyNodeSetObject *v, NyNodeSetObject *w, int op)
{
    int z;
    PyObject *pos;
    int bits, a, b;
    NyNodeSetObject *dst = 0;
    PyObject **zf, **vf, **wf, **ve, **we;
    ve = &v->u.nodes[Py_SIZE(v)];
    we = &w->u.nodes[Py_SIZE(w)];
    for (z = 0, zf = 0; ;) {
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
                    *zf = pos;
                    Py_INCREF(pos);
                    zf++;
                } else {
                    z++;
                }
            }
        }
        if (zf) {
            return dst;
        } else {
            dst = NyImmNodeSet_New(z, v->_hiding_tag_);
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
    PyObject **lo;
    PyObject **hi;

    Py_uintptr_t addr =
#if SIZEOF_VOID_P <= SIZEOF_LONG
        PyLong_AsUnsignedLongMask(obj);
#else
        PyLong_AsUnsignedLongLongMask(obj);
#endif
    if (addr == (Py_uintptr_t) -1 && PyErr_Occurred())
        return 0;

    lo = &v->u.nodes[0];
    hi = &v->u.nodes[Py_SIZE(v)];
    while (lo < hi) {
        PyObject **cur = lo + (hi - lo) / 2;
        if ((Py_uintptr_t)(*cur) == addr) {
            Py_INCREF(*cur);
            return *cur;
        }
        else if ((Py_uintptr_t)*cur < addr)
            lo = cur + 1;
        else
            hi = cur;
    }
    PyErr_Format(PyExc_ValueError, "No object found at address %p\n",(void *)addr);
    return 0;
}


static PyMethodDef immnodeset_methods[] = {
    {"obj_at",    (PyCFunction)immnodeset_obj_at, METH_O, immnodeset_obj_at_doc},
    {0} /* sentinel */
};



PyTypeObject NyImmNodeSet_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name        = "guppy.sets.setsc.ImmNodeSet",
    .tp_basicsize   = sizeof(NyNodeSetObject)-sizeof(PyObject *),
    .tp_itemsize    = sizeof(PyObject *),
    .tp_dealloc     = (destructor)immnodeset_dealloc,
    .tp_hash        = (hashfunc)immnodeset_hash,
    .tp_getattro    = PyObject_GenericGetAttr,
    .tp_flags       = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE,
    .tp_doc         = immnodeset_doc,
    .tp_traverse    = (traverseproc)immnodeset_gc_traverse,
    .tp_clear       = (inquiry)immnodeset_gc_clear,
    .tp_richcompare = (richcmpfunc)nodeset_richcompare,
    .tp_iter        = (getiterfunc)immnodeset_iter,
    .tp_methods     = immnodeset_methods,
    .tp_members     = immnodeset_members,
    .tp_getset      = immnodeset_getset,
    .tp_base        = &NyNodeSet_Type,
    .tp_alloc       = PyType_GenericAlloc,
    .tp_new         = immnodeset_new,
    .tp_free        = PyObject_GC_Del,
};
