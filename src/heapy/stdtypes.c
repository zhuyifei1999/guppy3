#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "structmember.h"
#include "compile.h"
#include "frameobject.h"
#include "unicodeobject.h"

#include "heapdef.h"
#include "stdtypes.h"


#define ALIGNMENT  sizeof(void *)
#define ALIGN_MASK (ALIGNMENT - 1)
#define ALIGN(z)   ((z + ALIGN_MASK) & ~ALIGN_MASK)

#define ATTR(name) if ((PyObject *)v->name == r->tgt &&        \
    (r->visit(NYHR_ATTRIBUTE, PyUnicode_FromString(#name), r))) \
        return 1;

#define RENAMEATTR(name, newname) if ((PyObject *)v->name == r->tgt && \
    (r->visit(NYHR_ATTRIBUTE, PyUnicode_FromString(#newname), r)))      \
        return 1;

#define INTERATTR(name) if ((PyObject *)v->name == r->tgt &&   \
    (r->visit(NYHR_INTERATTR, PyUnicode_FromString(#name), r))) \
        return 1;

extern PyObject *_hiding_tag__name;

int
dict_relate_kv(NyHeapRelate *r, PyObject *dict, int k, int v)
{
    PyObject *pk, *pv;
    Py_ssize_t i = 0;
    Py_ssize_t ix = 0;
    if (!dict)
        return 0;
    while (PyDict_Next(dict, &i, &pk, &pv)) {
        if (pk == r->tgt) {
            if (r->visit(k, PyLong_FromSsize_t(ix), r))
                return 0;
        }
        if (pv == r->tgt) {
            Py_INCREF(pk);
            if (r->visit(v, pk, r))
                return 0;
        }
        ix++;
    }
    return 0;
}

static int
dict_relate(NyHeapRelate *r)
{
    return dict_relate_kv(r, r->src, NYHR_INDEXKEY, NYHR_INDEXVAL);
}

static int
dict_traverse(NyHeapTraverse *ta)
{
    PyObject *v = (void *)ta->obj;
    if (PyDict_GetItem(v, _hiding_tag__name) == ta->_hiding_tag_)
        return 0;
    return Py_TYPE(v)->tp_traverse(ta->obj, ta->visit, ta->arg);
}


static int
dictproxy_relate(NyHeapRelate *r)
{
    mappingproxyobject *v = (void *)r->src;
    if (v->mapping == r->tgt) {
        if (r->visit(NYHR_INTERATTR, PyUnicode_FromString("mapping"), r))
            return 1;
    }
    return dict_relate_kv(r, v->mapping, NYHR_INDEXKEY, NYHR_INDEXVAL);
}


static int
list_relate(NyHeapRelate *r)
{
    Py_ssize_t len = PyList_Size(r->src);
    Py_ssize_t i;
    for (i = 0; i < len; i++) {
        PyObject *o = PyList_GET_ITEM(r->src, i);
        if (o == r->tgt) {
            PyObject *ix = PyLong_FromSsize_t(i);
            int x;
            if (!ix)
                return -1;
            x = r->visit(NYHR_INDEXVAL, ix, r);
            if (x)
                return 0;
        }
    }
    return 0;
}

static int
tuple_relate(NyHeapRelate *r)
{
    Py_ssize_t len = PyTuple_Size(r->src);
    Py_ssize_t i;
    for (i = 0; i < len; i++) {
        PyObject *o = PyTuple_GetItem(r->src, i);
        if (o == r->tgt) {
            PyObject *ix = PyLong_FromSsize_t(i);
            int x;
            if (!ix)
                return -1;
            x = r->visit(NYHR_INDEXVAL, ix, r);
            if (x)
                return 0;
        }
    }
    return 0;
}

static int
set_relate(NyHeapRelate *r)
{
    PyObject *it = PyObject_GetIter(r->src);
    PyObject *obj;

    if (it == NULL) {
        return -1;
    }

    Py_ssize_t i = 0;
    while ((obj = PyIter_Next(it))) {
        if (r->tgt == obj) {
            r->visit(NYHR_INSET, PyLong_FromSsize_t(i++), r);
            return 1;
        }
        Py_DECREF(obj);
    }

    Py_DECREF(it);

    if (PyErr_Occurred())
        return -1;
    return 0;
}

static int
function_relate(NyHeapRelate *r)
{
    PyFunctionObject *v = (void *)r->src;
    RENAMEATTR(func_code, __code__)
    RENAMEATTR(func_globals, __globals__)
    RENAMEATTR(func_module, __module__)
    RENAMEATTR(func_defaults, __defaults__)
    RENAMEATTR(func_kwdefaults, __kwdefaults__)
    RENAMEATTR(func_doc, __doc__)
    RENAMEATTR(func_name, __name__)
    RENAMEATTR(func_dict, __dict__)
    RENAMEATTR(func_closure, __closure__)
    RENAMEATTR(func_annotations, __annotations__)
    RENAMEATTR(func_qualname, __qualname__)
    return dict_relate_kv(r, v->func_dict, NYHR_HASATTR, NYHR_ATTRIBUTE);
}

static int
module_relate(NyHeapRelate *r)
{
    PyObject *v = (void *)r->src;
    PyObject *dct = PyModule_GetDict(v);
    if (dct == r->tgt &&
        (r->visit(NYHR_ATTRIBUTE, PyUnicode_FromString("__dict__"), r)))
        return 1;
    return dict_relate_kv(r, dct, NYHR_HASATTR, NYHR_ATTRIBUTE);
}

static int
frame_locals(NyHeapRelate *r, PyObject *map, Py_ssize_t start, Py_ssize_t n, int deref)
{
    PyFrameObject *v = (void *)r->src;
    Py_ssize_t i;
    for (i = start; i < start + n; i++) {
        if ((!deref && v->f_localsplus[i] == r->tgt) ||
            (deref && PyCell_GET(v->f_localsplus[i]) == r->tgt)) {
            PyObject *name;
            if (PyTuple_Check(map) && (i - start) < PyTuple_Size(map)) {
                name = PyTuple_GetItem(map, i - start);
                Py_INCREF(name);
            } else {
                name = PyUnicode_FromString("?");
            }
            if (r->visit(deref? NYHR_CELL : NYHR_LOCAL_VAR, name, r))
                return 1;
        }
    }
    return 0;
}

static int
frame_relate(NyHeapRelate *r)
{
    PyFrameObject *v = (void *)r->src;
    PyCodeObject *co = v->f_code;
    Py_ssize_t ncells = PyTuple_GET_SIZE(co->co_cellvars);
    Py_ssize_t nlocals = co->co_nlocals;
    Py_ssize_t nfreevars = PyTuple_GET_SIZE(co->co_freevars);
    ATTR(f_back)
    ATTR(f_code)
    ATTR(f_builtins)
    ATTR(f_globals)
    ATTR(f_locals)
    ATTR(f_trace)
    /*
    ATTR(f_exc_type)
    ATTR(f_exc_value)
    ATTR(f_exc_traceback)
    */

    /* locals */
    if (
        frame_locals(r, co->co_varnames, 0, nlocals, 0) ||
        frame_locals(r, co->co_cellvars, nlocals, ncells, 0) ||
        frame_locals(r, co->co_cellvars, nlocals, ncells, 1) ||
        frame_locals(r, co->co_freevars, nlocals + ncells, nfreevars, 0) ||
        frame_locals(r, co->co_freevars, nlocals + ncells, nfreevars, 1))
        return 1;

    /* stack */

    if (v->f_stacktop != NULL) {
        PyObject **p;
        for (p = v->f_valuestack; p < v->f_stacktop; p++) {
            if (*p == r->tgt) {
                if (r->visit(NYHR_STACK, PyLong_FromSsize_t(p-v->f_valuestack), r))
                    return 1;
            }
        }
    }
    return 0;
}

static int
frame_traverse(NyHeapTraverse *ta) {
    PyFrameObject *v = (void *)ta->obj;
    PyCodeObject *co = v->f_code;
    int nlocals = co->co_nlocals;
    if (PyTuple_Check(co->co_varnames)) {
        int i;
        for (i = 0; i < nlocals; i++) {
            PyObject *name = PyTuple_GET_ITEM(co->co_varnames, i);
            if (strcmp(PyUnicode_AsUTF8(name), "_hiding_tag_") == 0) {
                if (v->f_localsplus[i] == ta->_hiding_tag_)
                    return 0;
                else
                    break;
            }
        }
    }
    return Py_TYPE(v)->tp_traverse(ta->obj, ta->visit, ta->arg);
}


static int
traceback_relate(NyHeapRelate *r)
{
    PyTracebackObject *v = (void *)r->src;
    ATTR(tb_next)
    ATTR(tb_frame)
    return 0;
}

static int
cell_relate(NyHeapRelate *r)
{
    PyCellObject *v = (void *)r->src;
    if (v->ob_ref == r->tgt &&
        r->visit(NYHR_ATTRIBUTE, PyUnicode_FromString("cell_contents"), r))
        return 1;
    return 0;
}

static int
meth_relate(NyHeapRelate *r)
{
    PyCFunctionObject *v = (void *)r->src;
    RENAMEATTR(m_self, __self__);
    RENAMEATTR(m_module, __module__);
    return 0;
}

static int
code_traverse(NyHeapTraverse *ta) {
    PyCodeObject *co = (void *)ta->obj;
    visitproc visit = ta->visit;
    void *arg = ta->arg;
    Py_VISIT(co->co_code);
    Py_VISIT(co->co_consts);
    Py_VISIT(co->co_names);
    Py_VISIT(co->co_varnames);
    Py_VISIT(co->co_freevars);
    Py_VISIT(co->co_cellvars);
    Py_VISIT(co->co_filename);
    Py_VISIT(co->co_name);
    Py_VISIT(co->co_lnotab);
    return 0;
}

/* type_traverse adapted from typeobject.c from 2.4.2
   except:
   * I removed the check for heap type
   * I added visit of tp_subclasses and slots
 */

static int
type_traverse(NyHeapTraverse *ta)
{
    PyTypeObject *type=(void *)ta->obj;
    visitproc visit = ta->visit;
    void *arg = ta->arg;

    Py_VISIT(type->tp_dict);
    Py_VISIT(type->tp_cache);
    Py_VISIT(type->tp_mro);
    Py_VISIT(type->tp_bases);
    Py_VISIT(type->tp_base);
    Py_VISIT(type->tp_subclasses);

    if (!(type->tp_flags & Py_TPFLAGS_HEAPTYPE))
        return 0;
    Py_VISIT(((PyHeapTypeObject *)type)->ht_slots ) ;
    return 0;
}




static int
type_relate(NyHeapRelate *r)
{
    PyTypeObject *type = (void *)r->src;
    PyHeapTypeObject *et;
#define v type
    RENAMEATTR(tp_dict, __dict__);
    INTERATTR(tp_cache);
    RENAMEATTR(tp_mro, __mro__);
    RENAMEATTR(tp_bases, __bases__);
    RENAMEATTR(tp_base, __base__);
    INTERATTR(tp_subclasses);
#undef v
    if (!(type->tp_flags & Py_TPFLAGS_HEAPTYPE))
        return 0;
    et = (PyHeapTypeObject *)type;
#define v et
    RENAMEATTR(ht_slots, __slots__);
    return 0;
#undef v
}

NyHeapDef NyStdTypes_HeapDef[] = {
    {
        0,             /* flags */
        0,             /* type */
        0,             /* size */
        dict_traverse, /* traverse */
        dict_relate    /* relate */
    }, {
        0,          /* flags */
        0,          /* type */
        0,          /* size */
        0,          /* traverse */
        list_relate /* relate */
    }, {
        0,           /* flags */
        0,           /* type */
        0,           /* size */
        0,           /* traverse */
        tuple_relate /* relate */
    }, {
        0,         /* flags */
        0,         /* type */
        0,         /* size */
        0,         /* traverse */
        set_relate /* relate */
    }, {
        0,         /* flags */
        0,         /* type */
        0,         /* size */
        0,         /* traverse */
        set_relate /* relate */
    }, {
        0,              /* flags */
        0,              /* type */
        0,              /* size */
        0,              /* traverse */
        function_relate /* relate */
    }, {
        0,            /* flags */
        0,            /* type */
        0,            /* size */
        0,            /* traverse */
        module_relate /* relate */
    }, {
        0,              /* flags */
        0,              /* type */
        0,              /* size */
        frame_traverse, /* traverse */
        frame_relate    /* relate */
    }, {
        0,               /* flags */
        0,               /* type */
        0,               /* size */
        0,               /* traverse */
        traceback_relate /* relate */
    }, {
        0,          /* flags */
        0,          /* type */
        0,          /* size */
        0,          /* traverse */
        cell_relate /* relate */
    }, {
        0,          /* flags */
        0,          /* type */
        0,          /* size */
        0,          /* traverse */
        meth_relate /* relate */
    }, {
        0,             /* flags */
        0,             /* type */
        0,             /* size */
        code_traverse, /* traverse */
        0              /* relate */
    }, {
        0,             /* flags */
        0,             /* type */
        0,             /* size */
        type_traverse, /* traverse */
        type_relate    /* relate */
    }, {
        0,               /* flags */
        0,               /* type */ /* To be patched-in from a dictproxy ! */
        0,               /* size */
        0,               /* traverse */
        dictproxy_relate /* relate */
    },

/* End mark */
    {0}
};

void
NyStdTypes_init(void)
{
    /* Patch up the table for some types that were not directly accessible */
    int x = 0;

    NyStdTypes_HeapDef[x++].type = &PyDict_Type;
    NyStdTypes_HeapDef[x++].type = &PyList_Type;
    NyStdTypes_HeapDef[x++].type = &PyTuple_Type;
    NyStdTypes_HeapDef[x++].type = &PySet_Type;
    NyStdTypes_HeapDef[x++].type = &PyFrozenSet_Type;
    NyStdTypes_HeapDef[x++].type = &PyFunction_Type;
    NyStdTypes_HeapDef[x++].type = &PyModule_Type;
    NyStdTypes_HeapDef[x++].type = &PyFrame_Type;
    NyStdTypes_HeapDef[x++].type = &PyTraceBack_Type;
    NyStdTypes_HeapDef[x++].type = &PyCell_Type;
    NyStdTypes_HeapDef[x++].type = &PyCFunction_Type;
    NyStdTypes_HeapDef[x++].type = &PyCode_Type;
    NyStdTypes_HeapDef[x++].type = &PyType_Type;
    NyHeapDef *dictproxy_def = &NyStdTypes_HeapDef[x++];

    PyObject *d = PyDict_New();
    if (d) {
        PyObject *dp = PyDictProxy_New(d);
        if (dp) {
            dictproxy_def->type = (PyTypeObject *)Py_TYPE(dp);
            Py_DECREF(dp);
        }
        Py_DECREF(d);
    }
}
