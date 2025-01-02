#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
# define Py_BUILD_CORE
#  undef _PyGC_FINALIZED
/* static_builtin_state */
#  include <internal/pycore_typeobject.h>
/* PyInterpreterState */
#  include <internal/pycore_interp.h>
# undef Py_BUILD_CORE
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
# define Py_BUILD_CORE
/* PyFrameObject */
#  include <internal/pycore_frame.h>
/* _PyLocals_GetKind */
#  include <internal/pycore_code.h>
# undef Py_BUILD_CORE
#endif

#include "structmember.h"
#include "compile.h"
#include "frameobject.h"
#include "unicodeobject.h"

#include "heapdef.h"
#include "stdtypes.h"


#define GATTR(obj, name, rel) do {                           \
    if ((PyObject *)(obj) == r->tgt &&                       \
            (r->visit(rel, PyUnicode_FromString(#name), r))) \
        return 1;                                            \
} while (0)

#define ATTR(name) GATTR(v->name, name, NYHR_ATTRIBUTE);
#define RENAMEATTR(name, newname) GATTR(v->name, newname, NYHR_ATTRIBUTE);
#define INTERATTR(name) GATTR(v->name, name, NYHR_INTERATTR);

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

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    // Python 3.13 no longer traverses object .__dict__ values.
    visitproc visit = ta->visit;
    void *arg = ta->arg;
    Py_ssize_t i = 0;
    PyObject *pv;

    while (PyDict_Next(v, &i, NULL, &pv))
        Py_VISIT(pv);
#endif

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
    RENAMEATTR(func_code, __code__);
    RENAMEATTR(func_globals, __globals__);
    RENAMEATTR(func_module, __module__);
    RENAMEATTR(func_defaults, __defaults__);
    RENAMEATTR(func_kwdefaults, __kwdefaults__);
    RENAMEATTR(func_doc, __doc__);
    RENAMEATTR(func_name, __name__);
    RENAMEATTR(func_dict, __dict__);
    RENAMEATTR(func_closure, __closure__);
    RENAMEATTR(func_annotations, __annotations__);
    RENAMEATTR(func_qualname, __qualname__);
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

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 11
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
#endif

static int
frame_relate(NyHeapRelate *r)
{
    PyFrameObject *v = (void *)r->src;
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    _PyInterpreterFrame *iv = v->f_frame;
#else
    PyFrameObject *iv = v;
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    PyCodeObject *co = iv->f_executable && PyCode_Check(iv->f_executable) ?
                       (PyCodeObject *)iv->f_executable : NULL;
#else
    PyCodeObject *co = iv->f_code;
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 11
    Py_ssize_t ncells = PyTuple_GET_SIZE(co->co_cellvars);
    Py_ssize_t nlocals = co->co_nlocals;;
    Py_ssize_t nfreevars = PyTuple_GET_SIZE(co->co_freevars);
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    // Py 3.11 only holds f_back when FRAME_OWNED_BY_FRAME_OBJECT
    PyFrameObject *next_frame = PyFrame_GetBack(v);
    if ((PyObject *)next_frame == r->tgt && r->visit(
            NYHR_ATTRIBUTE, PyUnicode_FromString("f_back"), r)) {
        Py_XDECREF(next_frame);
        return 1;
    }
    Py_XDECREF(next_frame);
#endif
    ATTR(f_back)
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
    GATTR(iv->f_funcobj, f_funcobj, NYHR_INTERATTR);
#elif PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    GATTR(iv->f_func, f_func, NYHR_INTERATTR);
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    if (iv->f_executable && PyCode_Check(iv->f_executable))
        GATTR(iv->f_executable, f_code, NYHR_ATTRIBUTE);
    else
        GATTR(iv->f_executable, f_executable, NYHR_INTERATTR);
#else
    GATTR(iv->f_code, f_code, NYHR_ATTRIBUTE);
#endif
    GATTR(iv->f_builtins, f_builtins, NYHR_ATTRIBUTE);
    GATTR(iv->f_globals, f_globals, NYHR_ATTRIBUTE);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    GATTR(iv->f_locals, f_locals, NYHR_INTERATTR);
#else
    GATTR(iv->f_locals, f_locals, NYHR_ATTRIBUTE);
#endif
    ATTR(f_trace)

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    ATTR(f_extra_locals)
    ATTR(f_locals_cache)
#endif

    // FIXME: Not sure if there's anything one can do about optimized frames,
    // need testing.
    if (!co)
        return 0;

    /* locals */
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    Py_ssize_t i;
    for (i = 0; i < co->co_nlocalsplus; i++) {
        _PyLocals_Kind kind = _PyLocals_GetKind(co->co_localspluskinds, i);
        PyObject *name = PyTuple_GET_ITEM(co->co_localsplusnames, i);

        if (iv->localsplus[i] == r->tgt) {
            Py_INCREF(name);
            if (r->visit(NYHR_LOCAL_VAR, name, r))
                return 1;
        }

        if (!(kind & CO_FAST_CELL) && !(kind & CO_FAST_FREE))
            continue;

        if (PyCell_GET(iv->localsplus[i]) == r->tgt) {
            Py_INCREF(name);
            if (r->visit(NYHR_CELL, name, r))
                return 1;
        }
    }
#else
    if (
        frame_locals(r, co->co_varnames, 0, nlocals, 0) ||
        frame_locals(r, co->co_cellvars, nlocals, ncells, 0) ||
        frame_locals(r, co->co_cellvars, nlocals, ncells, 1) ||
        frame_locals(r, co->co_freevars, nlocals + ncells, nfreevars, 0) ||
        frame_locals(r, co->co_freevars, nlocals + ncells, nfreevars, 1))
        return 1;
#endif

    /* stack */
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    PyObject **p;
    PyObject **s = iv->localsplus + co->co_nlocalsplus;
    PyObject **e = iv->localsplus + iv->stacktop;
    for (p = s; p < e; p++) {
        if (*p == r->tgt) {
            if (r->visit(NYHR_STACK, PyLong_FromSsize_t(p-s), r))
                return 1;
        }
    }
#elif PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 10
    PyObject **p;
    PyObject **l = v->f_valuestack + v->f_stackdepth;
    for (p = v->f_valuestack; p < l; p++) {
        if (*p == r->tgt) {
            if (r->visit(NYHR_STACK, PyLong_FromSsize_t(p-v->f_valuestack), r))
                return 1;
        }
    }
#else
    if (v->f_stacktop != NULL) {
        PyObject **p;
        for (p = v->f_valuestack; p < v->f_stacktop; p++) {
            if (*p == r->tgt) {
                if (r->visit(NYHR_STACK, PyLong_FromSsize_t(p-v->f_valuestack), r))
                    return 1;
            }
        }
    }
#endif
    return 0;
}

static int
frame_traverse(NyHeapTraverse *ta) {
    PyFrameObject *v = (void *)ta->obj;
    visitproc visit = ta->visit;
    void *arg = ta->arg;
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    _PyInterpreterFrame *iv = v->f_frame;
#else
    PyFrameObject *iv = v;
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    PyCodeObject *co = iv->f_executable && PyCode_Check(iv->f_executable) ?
                       (PyCodeObject *)iv->f_executable : NULL;
#else
    PyCodeObject *co = iv->f_code;
#endif
    int i;
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    // FIXME: What happens if JIT?
    for (i = 0; co && i < co->co_nlocalsplus; i++) {
        _PyLocals_Kind kind = _PyLocals_GetKind(co->co_localspluskinds, i);
        PyObject *name = PyTuple_GET_ITEM(co->co_localsplusnames, i);
        if (kind & CO_FAST_LOCAL && strcmp(PyUnicode_AsUTF8(name), "_hiding_tag_") == 0) {
            if (iv->localsplus[i] == ta->_hiding_tag_)
                return 0;
            else
                break;
        }
    }
#else
    int nlocals = co->co_nlocals;
    if (PyTuple_Check(co->co_varnames)) {
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
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    /* _PyFrame_Traverse is not exposed and CPython's frame_traverse only
      calls it when FRAME_OWNED_BY_FRAME_OBJECT :( */
    PyFrameObject *next_frame = PyFrame_GetBack(v);
    Py_VISIT(next_frame);
    Py_XDECREF(next_frame);

    Py_VISIT(v->f_trace);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
    Py_VISIT(iv->f_funcobj);
#else
    Py_VISIT(iv->f_func);
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    Py_VISIT(iv->f_executable);
#else
    Py_VISIT(iv->f_code);
#endif
    Py_VISIT(iv->f_builtins);
    Py_VISIT(iv->f_globals);
    Py_VISIT(iv->f_locals);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    Py_VISIT(v->f_extra_locals);
    Py_VISIT(v->f_locals_cache);
#endif

    /* locals */
    if (!co) {
        // FIXME: is it okay to assume stacktop is always valid when !co?
        for (i = 0; i < iv->stacktop; i++)
            Py_VISIT(iv->localsplus[i]);
    } else {
        for (i = 0; i < co->co_nlocalsplus; i++)
            Py_VISIT(iv->localsplus[i]);
    }

    return 0;
#else
    return Py_TYPE(v)->tp_traverse(ta->obj, visit, arg);
#endif
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
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
    if (co->_co_cached) {
        Py_VISIT(co->_co_cached->_co_code);
        Py_VISIT(co->_co_cached->_co_cellvars);
        Py_VISIT(co->_co_cached->_co_freevars);
        Py_VISIT(co->_co_cached->_co_varnames);
    }
#elif PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    Py_VISIT(co->_co_code);
#else
    Py_VISIT(co->co_code);
#endif
    Py_VISIT(co->co_consts);
    Py_VISIT(co->co_names);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    Py_VISIT(co->co_exceptiontable);
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    Py_VISIT(co->co_localsplusnames);
    Py_VISIT(co->co_localspluskinds);
#else
    Py_VISIT(co->co_varnames);
    Py_VISIT(co->co_freevars);
    Py_VISIT(co->co_cellvars);
#endif
    Py_VISIT(co->co_filename);
    Py_VISIT(co->co_name);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    Py_VISIT(co->co_qualname);
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 10
    Py_VISIT(co->co_linetable);
#else
    Py_VISIT(co->co_lnotab);
#endif
    Py_VISIT(co->co_weakreflist);
    return 0;
}

static int
code_relate(NyHeapRelate *r)
{
    PyCodeObject *v = (void *)r->src;
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
    if (v->_co_cached) {
        RENAMEATTR(_co_cached->_co_code, co_code);
        RENAMEATTR(_co_cached->_co_cellvars, co_cellvars);
        RENAMEATTR(_co_cached->_co_freevars, co_freevars);
        RENAMEATTR(_co_cached->_co_varnames, co_varnames);
    }
#elif PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    RENAMEATTR(_co_code, co_code);
#else
    ATTR(co_code);
#endif
    ATTR(co_consts);
    ATTR(co_names);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    ATTR(co_exceptiontable);
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    INTERATTR(co_localsplusnames);
    INTERATTR(co_localspluskinds);
#else
    ATTR(co_varnames);
    ATTR(co_freevars);
    ATTR(co_cellvars);
#endif
    ATTR(co_filename);
    ATTR(co_name);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 11
    ATTR(co_qualname);
#endif
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 10
    ATTR(co_linetable);
#else
    ATTR(co_lnotab);
#endif
    ATTR(co_weakreflist);
    return 0;
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
# if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
typedef managed_static_type_state ny_static_type_state;
# else
typedef static_builtin_state ny_static_type_state;
# endif

static ny_static_type_state *NyStaticType_GetState(PyTypeObject *self)
{
    // FIXME: interpreter probably should be a argument,
    // but with per-interp GIL, it's only safe to traverse
    // current interpreter anyways.
    PyInterpreterState *is = PyInterpreterState_Get();

    assert(self->tp_flags & _Py_TPFLAGS_STATIC_BUILTIN);

# if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    managed_static_type_state *state;
    size_t index;

    index = (size_t)self->tp_subclasses - 1;

    // FIXME: These constants may be subject to change within a Python version
    if (index <= _Py_MAX_MANAGED_STATIC_BUILTIN_TYPES) {
        state = &is->types.builtins.initialized[index];
        if (state->type == self)
            return state;
    }
    if (index <= _Py_MAX_MANAGED_STATIC_EXT_TYPES) {
        state = &is->types.for_extensions.initialized[index];
        if (state->type == self)
            return state;
    }

    PyErr_Format(PyExc_RuntimeError,
        "Unable to find managed_static_type_state for %R", self);
    return NULL;
# else
    size_t index;

    index = (size_t)self->tp_subclasses - 1;
    return &is->types.builtins[index];
# endif
}
#endif

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

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
    if (type->tp_flags & _Py_TPFLAGS_STATIC_BUILTIN) {
        ny_static_type_state *state = NyStaticType_GetState(type);
        if (!state)
            return -1;

        Py_VISIT(state->tp_dict);
        Py_VISIT(state->tp_subclasses);
    } else
#endif
    {
       Py_VISIT(type->tp_dict);
       Py_VISIT(type->tp_subclasses);
    }

    Py_VISIT(type->tp_mro);
    Py_VISIT(type->tp_bases);
    Py_VISIT(type->tp_cache);
    Py_VISIT(type->tp_base);

    if (!(type->tp_flags & Py_TPFLAGS_HEAPTYPE))
        return 0;
    Py_VISIT(((PyHeapTypeObject *)type)->ht_name);
    Py_VISIT(((PyHeapTypeObject *)type)->ht_slots);
    Py_VISIT(((PyHeapTypeObject *)type)->ht_qualname);

    Py_VISIT(((PyHeapTypeObject *)type)->ht_module);
    return 0;
}


static int
type_relate(NyHeapRelate *r)
{
    PyTypeObject *type = (void *)r->src;
    PyHeapTypeObject *et;

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
    if (type->tp_flags & _Py_TPFLAGS_STATIC_BUILTIN) {
        ny_static_type_state *state = NyStaticType_GetState(type);
        if (!state)
            return -1;

#define v state
        RENAMEATTR(tp_dict, __dict__);
        INTERATTR(tp_subclasses);
#undef v
    } else
#endif
#define v type
    {
        RENAMEATTR(tp_dict, __dict__);
        INTERATTR(tp_subclasses);
    }

    RENAMEATTR(tp_mro, __mro__);
    RENAMEATTR(tp_bases, __bases__);
    INTERATTR(tp_cache);
    RENAMEATTR(tp_base, __base__);
#undef v
    if (!(type->tp_flags & Py_TPFLAGS_HEAPTYPE))
        return 0;
    et = (PyHeapTypeObject *)type;
#define v et
    RENAMEATTR(ht_name, __name__);
    RENAMEATTR(ht_slots, __slots__);
    RENAMEATTR(ht_qualname, __qualname__);

    INTERATTR(ht_module);
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
        code_relate    /* relate */
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
