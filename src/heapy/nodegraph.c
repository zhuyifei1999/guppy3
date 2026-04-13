/* NodeGraph object implementation */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "structmember.h"
#include "../include/guppy.h"
#include "../include/pythoncapi_compat.h"

#include "heapdef.h"
#include "heapy.h"
#include "impsets.h"
#include "nodegraph.h"
#include "roundupsize.h"
#include "stoptheworld.h"
#include "utils.h"

#define Py_BUILD_CORE
/* PyGC_Head */
# if NY_MASKED_VERSION_HEX >= Py_PACK_VERSION(3, 14)
#  include <internal/pycore_interp_structs.h>
# else
#  undef _PyGC_FINALIZED
#  include <internal/pycore_gc.h>
# endif
#undef Py_BUILD_CORE

/* Pointer comparison macros
   Used for comparison of pointers that are not pointing
   into the same array. It would be formally undefined to
   compare pointers directly according to standard C definition.
   This should get around it.

*/

#define PTR_LT(a, b)    ((Py_uintptr_t)(a) < (Py_uintptr_t)(b))
#define PTR_EQ(a, b)    ((Py_uintptr_t)(a) == (Py_uintptr_t)(b))

#define PTR_CMP(a,b)    (PTR_LT(a, b) ? -1 : (PTR_EQ(a, b) ? 0: 1))

/* NodeGraphIter objects */

typedef struct {
    PyObject_HEAD
    NyNodeGraphObject *nodegraph;
    Py_ssize_t i;
    Py_ssize_t oldsize;
} NyNodeGraphIterObject;

/* NodeGraphIter methods */

static int
ngiter_clear(NyNodeGraphIterObject *it)
{
    Py_CLEAR(it->nodegraph);
    return 0;
}

static void
ngiter_dealloc(NyNodeGraphIterObject *it)
{
    PyTypeObject *tp = Py_TYPE(it);
    PyObject_GC_UnTrack(it);
    Py_TRASHCAN_BEGIN(it, ngiter_dealloc)
    ngiter_clear(it);
    PyObject_GC_Del(it);
    Py_CLEAR(tp);
    Py_TRASHCAN_END
}

static int
ngiter_traverse(NyNodeGraphIterObject *it, visitproc visit, void *arg)
{
    Py_VISIT(Py_TYPE(it));
    Py_VISIT(it->nodegraph);
    return 0;
}

static PyObject *
ngiter_iternext(NyNodeGraphIterObject *ngi)
{
    PyObject *ret = NULL;
    NyNodeGraphEdge *e;

    Ny_BEGIN_CRITICAL_SECTION(ngi->nodegraph);
    if (ngi->i >= ngi->nodegraph->used_size)
        goto err;
    ret = PyTuple_New(2);
    if (!ret)
        goto err;
    if (ngi->nodegraph->used_size != ngi->oldsize ||
        !ngi->nodegraph->is_sorted) {
        PyErr_SetString(PyExc_RuntimeError, "nodegraph changed size during iteration");
        goto err;
    }
    e = &ngi->nodegraph->edges[ngi->i];
    PyTuple_SET_ITEM(ret, 0, Py_NewRef(e->src));
    PyTuple_SET_ITEM(ret, 1, Py_NewRef(e->tgt));
    ngi->i++;
    goto out;

err:
    Py_CLEAR(ret);
out:
    Ny_END_CRITICAL_SECTION();
    return ret;
}

/* NodeGraphIter type */

static PyType_Slot ngiter_slots[] = {
    {Py_tp_dealloc,  ngiter_dealloc},
    {Py_tp_getattro, PyObject_GenericGetAttr},
    {Py_tp_traverse, ngiter_traverse},
    {Py_tp_clear,    ngiter_clear},
    {Py_tp_iter,     PyObject_SelfIter},
    {Py_tp_iternext, ngiter_iternext},
    {0, NULL}
};

PyType_Spec NyNodeGraphIter_Spec = {
    .name      = "nodegraph-iterator",
    .basicsize = sizeof(NyNodeGraphIterObject),
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IMMUTABLETYPE |
                 Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_DISALLOW_INSTANTIATION,
    .slots     = ngiter_slots,
};

/* NodeGraph methods */

void
NyNodeGraph_Clear(NyNodeGraphObject *ng)
{
    NyNodeGraphEdge *edges;
    Py_ssize_t N;
    Py_ssize_t i;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    edges = ng->edges;
    N = ng->used_size;
    ng->edges = NULL;
    ng->used_size = ng->allo_size = 0;
    for (i = 0; i < N; i++) {
        Py_CLEAR(edges[i].src);
        Py_CLEAR(edges[i].tgt);
    }
    PyMem_Free(edges);
    Ny_END_CRITICAL_SECTION();
}

static int
ng_gc_clear(NyNodeGraphObject *ng)
{
    /* NOT LOCKED: Object is dying */
    Py_CLEAR(ng->_hiding_tag_);
    NyNodeGraph_Clear(ng);
    return 0;
}

static void
ng_dealloc(PyObject *v)
{
    /* NOT LOCKED: Object is dying */
    if (PyObject_CallFinalizerFromDealloc(v))
        return;  /* resurrected */
    PyTypeObject *tp = Py_TYPE(v);
    NyNodeGraphObject *ng = (void *)v;
    Py_ssize_t i;
    PyObject_GC_UnTrack(v);
    Py_TRASHCAN_BEGIN(v, ng_dealloc)
    ng_gc_clear(ng);
    for (i = 0; i < ng->used_size; i++) {
        Py_CLEAR(ng->edges[i].src);
        Py_CLEAR(ng->edges[i].tgt);
    }
    PyMem_Free(ng->edges);
    tp->tp_free(v);
    Py_CLEAR(tp);
    Py_TRASHCAN_END
}

static int
ng_gc_traverse(NyNodeGraphObject *ng, visitproc visit, void *arg)
{
    /* NOT LOCKED: Stop the world from GC */
    Py_ssize_t i;

    Py_VISIT(Py_TYPE(ng));
    for (i = 0; i < ng->used_size; i++) {
        Py_VISIT(ng->edges[i].src) ;
        Py_VISIT(ng->edges[i].tgt) ;
    }
    Py_VISIT(ng->_hiding_tag_);
    return 0;
}

int
NyNodeGraph_AddEdge(NyNodeGraphObject *ng, PyObject *src, PyObject *tgt)
{
    int r = -1;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    if (!ng->is_preserving_duplicates &&
        ng->used_size &&
        ng->edges[ng->used_size-1].src == src &&
        ng->edges[ng->used_size-1].tgt == tgt
    ) {
        r = 0;
        goto out;
    }

#if Py_GIL_DISABLED
    /* Do nothing */
#elif NY_MASKED_VERSION_HEX >= Py_PACK_VERSION(3, 12)
    assert((Py_uintptr_t)Py_TYPE(src) > 0x1000 &&
            (Py_REFCNT(src) < 0xa000000 || _Py_IsImmortal(src)));
    assert((Py_uintptr_t)Py_TYPE(tgt) > 0x1000 &&
            (Py_REFCNT(tgt) < 0xa000000 || _Py_IsImmortal(tgt)));
#elif NY_MASKED_VERSION_HEX >= Py_PACK_VERSION(3, 11)
    /* Py >= 3.11 _PyObject_IMMORTAL_INIT sets initial refcount of 999999999 */
    assert((Py_uintptr_t)Py_TYPE(src) > 0x1000 &&
            (Py_REFCNT(src) < 0xa000000 ||
             (Py_REFCNT(src) >= 999999999 && Py_REFCNT(src) < 999999999 + 0xa000000)));
    assert((Py_uintptr_t)Py_TYPE(tgt) > 0x1000 &&
            (Py_REFCNT(tgt) < 0xa000000 ||
             (Py_REFCNT(tgt) >= 999999999 && Py_REFCNT(tgt) < 999999999 + 0xa000000)));
#else
    assert((Py_uintptr_t)Py_TYPE(src) > 0x1000 && Py_REFCNT(src) < 0xa000000);
    assert((Py_uintptr_t)Py_TYPE(tgt) > 0x1000 && Py_REFCNT(tgt) < 0xa000000);
#endif

    if (ng->used_size >= ng->allo_size) {
        Py_ssize_t allo = roundupsize(ng->used_size + 1);
        NyNodeGraphEdge *edges = ng->edges;

        PyMem_RESIZE(edges, NyNodeGraphEdge, allo);
        if (!edges) {
            PyErr_NoMemory();
            goto out;
        }
        ng->edges = edges;
        ng->allo_size = allo;
    }
    ng->edges[ng->used_size].src = Py_NewRef(src);
    ng->edges[ng->used_size].tgt = Py_NewRef(tgt);
    ng->used_size ++;
    ng->is_sorted = 0;
    r = 0;

out:
    Ny_END_CRITICAL_SECTION();
    return r;
}

static int
ng_compare(const void *x, const void *y)
{
    int c = PTR_CMP(((NyNodeGraphEdge *)x)->src, ((NyNodeGraphEdge *)y)->src);
    if (!c)
        c = PTR_CMP(((NyNodeGraphEdge *)x)->tgt, ((NyNodeGraphEdge *)y)->tgt);
    return c;
}

static int
ng_compare_src_only(const void *x, const void *y)
{
    int c = PTR_CMP(((NyNodeGraphEdge *)x)->src, ((NyNodeGraphEdge *)y)->src);
    return c;
}

static void
ng_sort(NyNodeGraphObject *ng)
{
    NY_ASSERT_OBJ_LOCKED_OR_STW(ng);

    qsort(ng->edges, ng->used_size, sizeof(NyNodeGraphEdge),
          ng->is_preserving_duplicates ? ng_compare_src_only : ng_compare);
}

static void
ng_remove_dups(NyNodeGraphObject *ng)
{
    NyNodeGraphEdge *dst, *src, *hi;

    NY_ASSERT_OBJ_LOCKED_OR_STW(ng);

    if (ng->used_size <= 1)
        return;
    hi = ng->edges + ng->used_size;
    dst = ng->edges + 1;
    src = ng->edges + 1;
    while( src < hi )  {
        if (src[0].src == dst[-1].src && src[0].tgt == dst[-1].tgt) {
            Py_CLEAR(src[0].src);
            Py_CLEAR(src[0].tgt);
            src++;
        } else {
            if (src != dst)
                dst[0] = src[0];
            dst++;
            src++;
        }
    }
    ng->used_size = dst - ng->edges;
}

static void
ng_trim(NyNodeGraphObject *ng)
{
    NyNodeGraphEdge *edges = ng->edges;

    NY_ASSERT_OBJ_LOCKED_OR_STW(ng);

    PyMem_RESIZE(edges, NyNodeGraphEdge, ng->used_size);
    if (!edges)
        return;

    ng->edges = edges;
    ng->allo_size = ng->used_size;
}

static void
ng_sortetc(NyNodeGraphObject *ng)
{
    NY_ASSERT_OBJ_LOCKED_OR_STW(ng);

    ng_sort(ng);
    if (!ng->is_preserving_duplicates)
        ng_remove_dups(ng);
    ng_trim(ng);
    ng->is_sorted = 1;
}

static void
ng_maybesortetc(NyNodeGraphObject *ng)
{
    NY_ASSERT_OBJ_LOCKED_OR_STW(ng);

    if (!ng->is_sorted)
        ng_sortetc(ng);
}

PyDoc_STRVAR(ng_add_edge_doc,
"NG.add_edge(source, target)\n\
\n\
Add to NG, an edge from source to target."
);


static PyObject *
ng_add_edge(NyNodeGraphObject *ng, PyObject *args)
{
    PyObject *src, *tgt;
    if (!PyArg_ParseTuple(args, "OO:",  &src, &tgt))
        return NULL;
    if (NyNodeGraph_AddEdge(ng, src, tgt) == -1)
        return NULL;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(ng_add_edges_n1_doc,
"NG.add_edges_n1(srcs:iterable, tgt)\n\
\n\
Add to NG, for each src in srcs, an edge from src to tgt."
);


typedef struct {
    NyNodeGraphObject *ng;
    PyObject *tgt;
} AETravArg;

static int
ng_add_edges_n1_trav(PyObject *obj, AETravArg *ta)
{
    if (NyNodeGraph_AddEdge(ta->ng, obj, ta->tgt) == -1)
        return -1;
    return 0;
}

static PyObject *
ng_add_edges_n1(NyNodeGraphObject *ng, PyObject *args)
{
    AETravArg ta;
    PyObject *it;
    ta.ng = ng;
    if (!PyArg_ParseTuple(args, "OO:",  &it, &ta.tgt))
        return NULL;
    if (iterable_iterate(ng->ms, it, (visitproc)ng_add_edges_n1_trav, &ta) == -1)
        return NULL;
    Py_RETURN_NONE;
}


PyDoc_STRVAR(ng_as_flat_list_doc,
"NG.as_flat_list() -> list\n\
\n\
Return the edges of NG in the form [src0, tgt0, src1, tgt1 ...]."
);

static PyObject *
ng_as_flat_list(NyNodeGraphObject *ng, PyObject *arg)
{
    PyObject *r = PyList_New(0);
    Py_ssize_t i;
    if (!r)
        return NULL;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    for (i = 0; i < ng->used_size; i++) {
        if ((PyList_Append(r, ng->edges[i].src) == -1) ||
            (PyList_Append(r, ng->edges[i].tgt) == -1)) {
            Py_CLEAR(r);
            goto out;
        }
    }

out:
    Ny_END_CRITICAL_SECTION();
    return r;
}

PyDoc_STRVAR(ng_clear_doc,
"NG.clear()\n\
\n\
Remove all items from NG."
);

static PyObject *
ng_clear_method(NyNodeGraphObject *ng, PyObject *arg_notused)
{
    NyNodeGraph_Clear(ng);
    Py_RETURN_NONE;
}

static NyNodeGraphObject *
NyNodeGraph_SubtypeNew(PyTypeObject *type)
{
    struct HeapycState *ms = NyType_AssertModuleState(type, &heapyc_def);
    NyNodeGraphObject *ng = (NyNodeGraphObject *)type->tp_alloc(type, 1);
    if (!ng)
        return NULL;
    ng->ms = ms;
    ng->_hiding_tag_ = NULL;
    ng->allo_size = ng->used_size = 0;
    ng->is_sorted = 0;
    ng->is_mapping = 0;
    ng->is_preserving_duplicates = 0;
    ng->edges = NULL;
    return ng;
}

static NyNodeGraphObject *
NyNodeGraph_SiblingNew(NyNodeGraphObject *ng)
{
    NyNodeGraphObject *cp = NyNodeGraph_SubtypeNew(Py_TYPE(ng));
    PyObject *he;
    if (!cp)
        return NULL;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    he = cp->_hiding_tag_;
    cp->_hiding_tag_ = Py_XNewRef(ng->_hiding_tag_);
    Py_CLEAR(he);
    cp->is_mapping = ng->is_mapping;
    Ny_END_CRITICAL_SECTION();

    return cp;
}

NyNodeGraphObject *
NyNodeGraph_Copy(NyNodeGraphObject *ng)
{
    NyNodeGraphObject *cp = NyNodeGraph_SiblingNew(ng);
    if (!cp)
        return NULL;
    if (NyNodeGraph_Update(cp, (PyObject *)ng) == -1)
        Py_CLEAR(cp);
    return cp;
}

PyDoc_STRVAR(ng_copy_doc,
"NG.copy() -> NodeGraph\n\
\n\
Return a copy of NG."
);

static PyObject *
ng_copy(NyNodeGraphObject *ng, PyObject *notused)
{
    return (PyObject *)NyNodeGraph_Copy(ng);
}


typedef struct {
    NyNodeGraphObject *ng;
    int ret;
} DCTravArg;

static int
ng_dc_trav(PyObject *obj, DCTravArg *ta)
{
    NyNodeGraphEdge *lo, *hi;
    if (NyNodeGraph_Region(ta->ng, obj, &lo, &hi) == -1) {
        return -1;
    }
    if (lo == hi) {
        ta->ret = 0;
        return 1;
    }
    return 0;
}


PyDoc_STRVAR(ng_domain_covers_doc,
"NG.domain_covers(X:iterable) -> bool\n\
\n\
Return True if each node in X is the source of some edge in NG, False otherwise."
);

static PyObject *
ng_domain_covers(NyNodeGraphObject *ng, PyObject *X)
{
    DCTravArg ta;
    PyObject *result = NULL;
    ta.ng = ng;
    ta.ret = 1;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    if (iterable_iterate(ng->ms, X, (visitproc)ng_dc_trav, &ta) == -1)
        goto out;

    result = Py_NewRef(ta.ret ? Py_True : Py_False);
out:
    Ny_END_CRITICAL_SECTION();
    return result;
}


typedef struct {
    NyNodeGraphObject *ng, *ret;
} DRTravArg;

static int
ng_dr_trav(PyObject *obj, DRTravArg *ta)
{
    NyNodeGraphEdge *lo, *hi, *cur;
    if (NyNodeGraph_Region(ta->ng, obj, &lo, &hi) == -1) {
        return -1;
    }
    for (cur = lo; cur < hi; cur++) {
        if (NyNodeGraph_AddEdge(ta->ret, obj, cur->tgt) == -1)
            return -1;
    }
    return 0;
}



PyDoc_STRVAR(ng_domain_restricted_doc,
"NG.domain_restricted(X:iterable) -> NodeGraph\n\
\n\
Return a new NodeGraph, containing those edges in NG that have source in X."
);

static PyObject *
ng_domain_restricted(NyNodeGraphObject *ng, PyObject *X)
{
    DRTravArg ta;
    ta.ng = ng;
    ta.ret = NyNodeGraph_SiblingNew(ng);
    if (!ta.ret)
        return NULL;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    if (iterable_iterate(ng->ms, X, (visitproc)ng_dr_trav, &ta) == -1)
        Py_CLEAR(ta.ret);
    Ny_END_CRITICAL_SECTION();
    return (PyObject *)ta.ret;
}


PyDoc_STRVAR(ng_get_domain_doc,
"NG.get_domain() -> NodeSet\n\
\n\
Return the set of nodes that are the source of some edge in NG."
);

static PyObject *
ng_get_domain(NyNodeGraphObject *ng, void *closure)
{
    NyNodeSetObject *ns;
    Py_ssize_t i;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    ns = NyMutNodeSet_NewHiding(ng->ms->nodeset_exports->ms, ng->_hiding_tag_);
    if (!ns)
        goto out;
    for (i = 0; i < ng->used_size; i++) {
        if (NyNodeSet_setobj(ns, ng->edges[i].src) == -1) {
            Py_CLEAR(ns);
            goto out;
        }
    }
out:
    Ny_END_CRITICAL_SECTION();
    return (PyObject *)ns;
}

PyDoc_STRVAR(ng_get_range_doc,
"NG.get_range() -> NodeSet\n\
\n\
Return the set of nodes that are the target of some edge in NG."
);

static PyObject *
ng_get_range(NyNodeGraphObject *ng, void *closure)
{
    NyNodeSetObject *ns;
    Py_ssize_t i;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    ns = NyMutNodeSet_NewHiding(ng->ms->nodeset_exports->ms, ng->_hiding_tag_);
    if (!ns)
        goto out;
    for (i = 0; i < ng->used_size; i++) {
        if (NyNodeSet_setobj(ns, ng->edges[i].tgt) == -1) {
            Py_CLEAR(ns);
            goto out;
        }
    }
out:
    Ny_END_CRITICAL_SECTION();
    return (PyObject *)ns;
}

int
NyNodeGraph_Invert(NyNodeGraphObject *ng) {
    NyNodeGraphEdge *edge;
    Py_ssize_t i;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    edge = ng->edges;
    for (i = 0; i < ng->used_size; i++, edge++) {
        PyObject *t = edge->src;
        edge->src = edge->tgt;
        edge->tgt = t;
    }
    ng->is_sorted = 0;
    Ny_END_CRITICAL_SECTION();
    return 0;
}

PyDoc_STRVAR(ng_invert_doc,
"NG.invert()\n\
\n\
Invert the edges of NG."
);

static PyObject *
ng_invert(NyNodeGraphObject *ng, void *notused)
{
    if (NyNodeGraph_Invert(ng) == -1)
        return NULL;
    Py_RETURN_NONE;
}

NyNodeGraphObject *
NyNodeGraph_Inverted(NyNodeGraphObject *ng)
{
    NyNodeGraphObject *ob;
    ob = NyNodeGraph_Copy(ng);
    if (!ob)
        return NULL;
    if (NyNodeGraph_Invert(ob) == -1)
        Py_CLEAR(ob);
    return ob;

}

PyDoc_STRVAR(ng_inverted_doc,
"NG.inverted() -> NodeGraph\n\
\n\
Return a copy of NG with the edges inverted."
);

static PyObject *
ng_inverted(NyNodeGraphObject *ng, void *notused)
{
    return (PyObject *)NyNodeGraph_Inverted(ng);
}

static PyObject *
ng_iter(NyNodeGraphObject *v)
{
    NyNodeGraphIterObject *iter = PyObject_GC_New(NyNodeGraphIterObject, v->ms->NodeGraphIter_Type);
    if (!iter)
        return NULL;

    iter->nodegraph = Ny_NEWREF(v);
    iter->i = 0;
    Ny_BEGIN_CRITICAL_SECTION(v);
    ng_maybesortetc(v);
    iter->oldsize = v->used_size;
    Ny_END_CRITICAL_SECTION();
    PyObject_GC_Track(iter);
    return (PyObject *)iter;
}

int
NyNodeGraph_Region(NyNodeGraphObject *ng, PyObject *key,
                   NyNodeGraphEdge **lop, NyNodeGraphEdge **hip)
{
    NY_ASSERT_OBJ_LOCKED_OR_STW(ng);

    NyNodeGraphEdge *lo, *hi, *cur;
    ng_maybesortetc(ng);
    lo = ng->edges;
    hi = ng->edges + ng->used_size;
    if (lo >=  hi) {
        *lop = *hip = lo;
        return 0;
    }
    for (;;) {
        cur = lo + (hi - lo) / 2;
        if (cur->src == key) {
            for (lo = cur; lo > ng->edges && (lo-1)->src == key; lo--);
            for (hi = cur + 1; hi < ng->edges + ng->used_size && hi->src == key;
                hi++);
            *lop = lo;
            *hip = hi;
            return 0;
        } else if (cur == lo) {
            *lop = *hip = lo;
            return 0;
        } else if (PTR_LT(cur->src, key)) /* Make sure use same lt as in sort */
            lo = cur;
        else
            hi = cur;
    }
}

typedef struct {
    NyNodeGraphObject *ng;
    NyNodeSetObject *hs;
} RITravArg;

static int
ng_relimg_trav(PyObject *obj, RITravArg *ta)
{
    NyNodeGraphEdge *lo, *hi, *cur;
    if (NyNodeGraph_Region(ta->ng, obj, &lo, &hi) == -1) {
        return -1;
    }
    for (cur = lo; cur < hi; cur++) {
        if (NyNodeSet_setobj(ta->hs, cur->tgt) == -1)
            return -1;
    }
    return 0;
}

PyDoc_STRVAR(ng_relimg_doc,
"NG.relimg(X:iterable) -> NodeSet\n\
\n\
Return the relational image of NG wrt X. That is, the set of nodes\n\
that are the target of some edge that have its source in X."
);

static NyNodeSetObject *
ng_relimg(NyNodeGraphObject *ng, PyObject *S)
{
    RITravArg ta;
    ta.ng = ng;
    ta.hs = NULL;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    ta.hs = NyMutNodeSet_NewHiding(ng->ms->nodeset_exports->ms, ng->_hiding_tag_);
    if (!ta.hs)
        return NULL;
    ng_maybesortetc(ng);
    if (iterable_iterate(ng->ms, S, (visitproc)ng_relimg_trav, &ta) == -1)
        goto err;
    goto out;

err:
    Py_CLEAR(ta.hs);
out:
    Ny_END_CRITICAL_SECTION();
    return ta.hs;
}

static int
ng_update_visit(PyObject *obj, NyNodeGraphObject *ng)
{
    if (!PyTuple_Check(obj) || PyTuple_GET_SIZE(obj) != 2) {
        PyErr_SetString(PyExc_TypeError, "update: right argument must be sequence of 2-tuples");
        return -1;
    }
    if (NyNodeGraph_AddEdge(ng,
                            PyTuple_GET_ITEM(obj, 0),
                            PyTuple_GET_ITEM(obj, 1)) == -1)
        return -1;
    return 0;
}


int
NyNodeGraph_Update(NyNodeGraphObject *a, PyObject *u)
{
    return iterable_iterate(a->ms, u, (visitproc)ng_update_visit, a);
}


PyDoc_STRVAR(ng_update_doc,
"NG.update(X:iterable)\n\
\n\
Update NG with the edges from X,\n\
specified as pairs of the form (source, target)."
);


static PyObject *
ng_update(NyNodeGraphObject *ng, PyObject *arg)
{
    if (NyNodeGraph_Update(ng, arg) == -1)
        return NULL;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(ng_updated_doc,
"NG.updated(X:iterable) -> NodeGraph\n\
\n\
Return a copy of NG updated with the edges from X,\n\
specified as pairs of the form (source, target)."
);

static PyObject *
ng_updated(NyNodeGraphObject *ng, PyObject *arg)
{
    ng = NyNodeGraph_Copy(ng);
    if (!ng)
        return NULL;
    if (NyNodeGraph_Update(ng, arg) == -1)
        Py_CLEAR(ng);
    return (PyObject *)ng;
}

static PyMethodDef ng_methods[] = {
    {"add_edge", (PyCFunction)ng_add_edge, METH_VARARGS, ng_add_edge_doc},
    {"add_edges_n1", (PyCFunction)ng_add_edges_n1, METH_VARARGS, ng_add_edges_n1_doc},
    {"as_flat_list", (PyCFunction)ng_as_flat_list, METH_NOARGS, ng_as_flat_list_doc},
    {"clear", (PyCFunction)ng_clear_method, METH_NOARGS, ng_clear_doc},
    {"copy", (PyCFunction)ng_copy, METH_NOARGS, ng_copy_doc},
    {"domain_covers", (PyCFunction)ng_domain_covers, METH_O, ng_domain_covers_doc},
    {"domain_restricted", (PyCFunction)ng_domain_restricted, METH_O, ng_domain_restricted_doc},
    {"get_domain", (PyCFunction)ng_get_domain, METH_NOARGS, ng_get_domain_doc},
    {"get_range", (PyCFunction)ng_get_range, METH_NOARGS, ng_get_range_doc},
    {"invert", (PyCFunction)ng_invert, METH_NOARGS, ng_invert_doc},
    {"inverted", (PyCFunction)ng_inverted, METH_NOARGS, ng_inverted_doc},
    {"relimg", (PyCFunction)ng_relimg, METH_O, ng_relimg_doc},
    {"update", (PyCFunction)ng_update, METH_O, ng_update_doc},
    {"updated", (PyCFunction)ng_updated, METH_O, ng_updated_doc},
    {0} /* sentinel */
};

size_t
nodegraph_size(PyObject *obj)
{
    Py_ssize_t z;

    Ny_BEGIN_CRITICAL_SECTION(obj);
    z = Py_TYPE(obj)->tp_basicsize +
        ((NyNodeGraphObject *)obj)->allo_size * sizeof(NyNodeGraphEdge);
    Ny_END_CRITICAL_SECTION();

#ifndef Py_GIL_DISABLED
    if (PyObject_IS_GC(obj))
        z += sizeof(PyGC_Head);
#endif
    return z;
}

int
nodegraph_traverse(NyHeapTraverse *t)
{
    NyNodeGraphObject *ng = (void *)t->obj;

    NY_ASSERT_WORLD_STOPPED();
    if (t->_hiding_tag_ != ng->_hiding_tag_)
        return Py_TYPE(ng)->tp_traverse((PyObject *)ng, t->visit, t->arg);
    return 0;
}

int
nodegraph_relate(NyHeapRelate *r)
{
    NyNodeGraphObject *ng = (void *)r->src;
    Py_ssize_t i;

    NY_ASSERT_WORLD_STOPPED();
    for (i = 0; i < ng->used_size; i++) {
        if (r->tgt == ng->edges[i].src) {
            if (r->visit(NYHR_INTERATTR, PyUnicode_FromFormat("edges[%d].src", i), r))
                return 0;
        }
        if (r->tgt == ng->edges[i].tgt) {
            if (r->visit(NYHR_INTERATTR, PyUnicode_FromFormat("edges[%d].tgt", i), r))
                return 0;
        }
    }
    return 0;
}


PyDoc_STRVAR(ng_doc,
"NodeGraph([iterable [,is_mapping]])\n\
\n\
Construct a new NodeGraph object. The arguments are:\n\
\n\
    iterable         An iterable object that will be used to\n\
                     initialize the new nodegraph. It should yield a\n\
                     sequence of edges of the form (source, target).\n\
\n\
    is_mapping       A boolean which, if True, will cause the nodegraph\n\
                     to be treated like a 'mapping'. It will then, for the\n\
                     purpose of indexing, be expected to contain a single\n\
                     target for each source node.\n\
\n\
A NodeGraph object contains pairs of nodes (edges) and can be indexed\n\
on the first node of the pair (the source of an edge) to find all\n\
second nodes of such pairs (the targets of those edges).\n\
\n\
NodeGraph objects are used internally in the heapy system, for example\n\
to record dict ownership and shortest-path graphs.\n\
\n\
They may be used generally for mapping and dict-like purposes, but\n\
differ in the following:\n\
\n\
o The mapping is based on object identity - no equality or hashing is\n\
  assumed, so any object can be used as a key. Only the address is used.\n\
  To distinguish this usage from that of ordinary dicts and sets, such\n\
  objects are called 'nodes'.\n\
\n\
o There may be any number of targets associated with each source.\n\
\n\
o Performance characteristics differ from dicts, in somewhat subtle ways.\n\
");


static PyGetSetDef ng_getset[] = {
    {0}
};

#define OFF(x) offsetof(NyNodeGraphObject, x)

static PyMemberDef ng_members[] = {
    {"_hiding_tag_",     T_OBJECT_EX, OFF(_hiding_tag_), 0,
"The hiding tag: if it is the the same object as the hiding tag\n\
of a HeapView object, the nodegraph will be hidden from that view."},
    {"is_mapping", T_UBYTE, OFF(is_mapping), READONLY,
"NG.is_mapping : boolean kind, read only\n\
\n\
True if NG is a 'mapping'. Then, only one edge is allowed for each\n\
source; indexing returns the actual target object instead of a tuple\n\
of targets."},
    {"is_sorted", T_UBYTE, OFF(is_sorted), READONLY,
"NG.is_sorted : boolean kind, read only\n\
\n\
True if NG is sorted. It will become unsorted after any update. It\n\
will need to be sorted to make it possible to find edges\n\
(implementation uses binary search). Any indexing operation will\n\
automatically sort it if it was not already sorted.  The flag is\n\
currently used from Python to see if the nodegraph has been used at\n\
least once after update, so that it will not be cleared too early."},
    {0} /* Sentinel */
};

#undef OFF


static Py_ssize_t
ng_length(PyObject *_ng)
{
    NyNodeGraphObject *ng=(void*)_ng;
    Py_ssize_t r;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    ng_maybesortetc(ng);
    r = ng->used_size;
    Ny_END_CRITICAL_SECTION();
    return r;
}


static PyObject *
ng_subscript(NyNodeGraphObject *ng, PyObject *obj)
{
    NyNodeGraphEdge *lo, *hi;
    PyObject *ret = NULL;
    Py_ssize_t i, size;

    Ny_BEGIN_CRITICAL_SECTION(ng);
    ng_maybesortetc(ng);
    if (NyNodeGraph_Region(ng, obj, &lo, &hi) == -1)
        goto out;
    size = hi - lo;
    if (ng->is_mapping) {
        if (size == 0) {
            PyErr_SetObject(PyExc_KeyError, obj);
            goto out;
        } else if (size > 1) {
            PyErr_SetString(PyExc_ValueError, "Ambiguos mapping");
            goto out;
        }
        ret = Py_NewRef(lo->tgt);
    } else {
        ret = PyTuple_New(size);
        if (!ret)
            goto out;
        for (i = 0; i < size; i++, lo++)
            PyTuple_SET_ITEM(ret, i, Py_NewRef(lo->tgt));
    }

out:
    Ny_END_CRITICAL_SECTION();
    return ret;
}


static int
ng_ass_sub(NyNodeGraphObject *ng, PyObject *v, PyObject *w)
{
    NyNodeGraphEdge *lo, *hi;
    Py_ssize_t i, regsize, tupsize;
    int r = -1;

    if (!w) {
        PyErr_SetString(PyExc_NotImplementedError,
                        "Item deletion is not implemented for nodegraphs.");
        return -1;
    }

    Ny_BEGIN_CRITICAL_SECTION(ng);
    ng_maybesortetc(ng);
    if (NyNodeGraph_Region(ng, v, &lo, &hi) == -1)
        goto out;
    regsize = hi - lo;
    if (ng->is_mapping) {
        if (regsize != 1) {
            PyErr_SetString(PyExc_ValueError,
"ng_ass_sub: can not change number of edges (wants to always be fast);\n"
"consider using .add_edge() etc. instead.");
            goto out;
        } else {
            PyObject *old = lo->tgt;
            lo->tgt = Py_NewRef(w);
            Py_CLEAR(old);
        }
    } else {
        if (!PyTuple_Check(w)) {
            PyErr_SetString(PyExc_TypeError, "ng_ass_sub: value to assign must be a tuple");
            goto out;
        }
        tupsize = PyTuple_GET_SIZE(w);
        if (tupsize != regsize) {
            PyErr_SetString(PyExc_ValueError,
"ng_ass_sub: can not change number of edges (wants to always be fast);\n"
"consider using .add_edge() etc. instead.");
            goto out;
        }
        for (i = 0; i < regsize; i++) {
            PyObject *old = lo[i].tgt;
            lo[i].tgt = Py_NewRef(PyTuple_GET_ITEM(w, i));
            Py_CLEAR(old);
        }
    }
    r = 0;

out:
    Ny_END_CRITICAL_SECTION();
    return r;
}


static PyObject *
ng_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *iterable = 0;
    PyObject *is_mapping = 0;
    static char *kwlist[] = {"iterable", "is_mapping", NULL};
    NyNodeGraphObject *ng;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO:NodeGraph.__new__",
                                     kwlist,
                                     &iterable,
                                     &is_mapping))
        return NULL;
    ng = NyNodeGraph_SubtypeNew(type);
    if (!ng)
        return NULL;
    if (is_mapping && PyObject_IsTrue(is_mapping)) {
        ng->is_mapping = 1;
    }
    if (iterable && !Py_IsNone(iterable) && NyNodeGraph_Update(ng, iterable) == -1)
        Py_CLEAR(ng);
    return (PyObject *)ng;
}


static PyType_Slot ng_slots[] = {
    {Py_tp_dealloc,       ng_dealloc},
    {Py_mp_length,        ng_length},
    {Py_mp_subscript,     ng_subscript},
    {Py_mp_ass_subscript, ng_ass_sub},
    {Py_tp_getattro,      PyObject_GenericGetAttr},
    {Py_tp_doc,           (void *)ng_doc},
    {Py_tp_traverse,      ng_gc_traverse},
    {Py_tp_clear,         ng_gc_clear},
    {Py_tp_iter,          ng_iter},
    {Py_tp_methods,       ng_methods},
    {Py_tp_members,       ng_members},
    {Py_tp_getset,        ng_getset},
    {Py_tp_alloc,         PyType_GenericAlloc},
    {Py_tp_new,           ng_new},
    {Py_tp_free,          PyObject_GC_Del},
    {0, NULL}
};

PyType_Spec NyNodeGraph_Spec = {
    .name      = "guppy.heapy.heapyc.NodeGraph",
    .basicsize = sizeof(NyNodeGraphObject),
    .flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_IMMUTABLETYPE |
                 Py_TPFLAGS_HAVE_GC | Ny_TPFLAGS_BASETYPE_ON_PY3_11,
    .slots     = ng_slots,
};

NyNodeGraphObject *
NyNodeGraph_New(struct HeapycState *ms)
{
    return NyNodeGraph_SubtypeNew(ms->NodeGraph_Type);
}
