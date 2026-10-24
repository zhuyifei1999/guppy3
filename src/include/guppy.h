#ifndef GUPPY_H_INCLUDED
#define GUPPY_H_INCLUDED

#include <stdbool.h>

#define NY_MASKED_VERSION_HEX (PY_VERSION_HEX & 0xffff0000)

#if NY_MASKED_VERSION_HEX < 0x030e0000
# define Py_PACK_FULL_VERSION(X, Y, Z, LEVEL, SERIAL) ( \
    (((X) & 0xff) << 24) |                              \
    (((Y) & 0xff) << 16) |                              \
    (((Z) & 0xff) << 8) |                               \
    (((LEVEL) & 0xf) << 4) |                            \
    (((SERIAL) & 0xf) << 0))

# define Py_PACK_VERSION(major, minor) \
   Py_PACK_FULL_VERSION(major, minor, 0, 0, 0)
#endif

#if NY_MASKED_VERSION_HEX < Py_PACK_VERSION(3, 12)
# include "structmember.h"  // PyMemberDef
#endif

/* From pyport.h */
# ifdef thread_local
#   define Ny_thread_local thread_local
# elif __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
#   define Ny_thread_local _Thread_local
# elif defined(_MSC_VER)  /* AKA NT_THREADS */
#   define Ny_thread_local __declspec(thread)
# elif defined(__GNUC__)  /* includes clang */
#   define Ny_thread_local __thread
# endif

#ifdef Py_GIL_DISABLED
# define Ny_BEGIN_CRITICAL_SECTION(op) Py_BEGIN_CRITICAL_SECTION(op)
# define Ny_END_CRITICAL_SECTION() Py_END_CRITICAL_SECTION()
# define Ny_BEGIN_CRITICAL_SECTION2(a, b) Py_BEGIN_CRITICAL_SECTION2(a, b)
# define Ny_END_CRITICAL_SECTION2() Py_END_CRITICAL_SECTION2()
#else
/* We can't goto a normal Py_END_CRITICAL_SECTION() without using,
   C23 extensions, and certain compilers don't like that. Other
   compilers don't like empty statement at the beginning of
   a block... */
# define Ny_BEGIN_CRITICAL_SECTION(op) { (void)0
# define Ny_END_CRITICAL_SECTION() (void)0; } (void)0
# define Ny_BEGIN_CRITICAL_SECTION2(a, b) { (void)0
# define Ny_END_CRITICAL_SECTION2() (void)0; } (void)0
#endif

#if !defined(Py_GIL_DISABLED) && NY_MASKED_VERSION_HEX < Py_PACK_VERSION(3, 13)
/* For non-freethreading builds, a datarace requires per-interpreter GIL, which
   I'm not too concerned about */
#define _Py_atomic_load_int(ptr) (*(ptr))
#define _Py_atomic_load_int_relaxed(ptr) (*(ptr))
#define _Py_atomic_load_int_acquire(ptr) (*(ptr))
#define _Py_atomic_store_int(ptr, val) (*(ptr) = (val))
#define _Py_atomic_store_int_relaxed(ptr, val) (*(ptr) = (val))
#define _Py_atomic_store_int_release(ptr, val) (*(ptr) = (val))

#define _Py_atomic_load_ptr_relaxed(ptr) (*(ptr))
#define _Py_atomic_store_ptr_relaxed(ptr, val) (*(ptr) = (val))

#define _Py_atomic_load_ssize_relaxed(ptr) (*(ptr))
#define _Py_atomic_store_ssize_relaxed(ptr, val) (*(ptr) = (val))

#define _Py_atomic_fence_acquire() ((void)0)
#define _Py_atomic_fence_release() ((void)0)
#endif

static inline int
NyModule_AddTypeWithSpec(PyObject *m, PyType_Spec *spec, PyObject *bases,
                         bool addtomod, PyTypeObject **state)
{
    PyObject *t = PyType_FromModuleAndSpec(m, spec, bases);
    if (!t)
        return -1;

    assert(PyType_Check(t));
    if (addtomod) {
        if (PyModule_AddType(m, (PyTypeObject *)t) == -1) {
            Py_CLEAR(t);
            return -1;
        }
    }
    if (state)
        *state = (PyTypeObject *)t;
    else
        Py_CLEAR(t);

    return 0;
}

/* Py_NewRef, but cast to the original C type */
#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
# define Ny_NEWREF(obj) ((typeof(obj))Py_NewRef(obj))
#elif defined(__GNUC__) || defined(__clang__)
# define Ny_NEWREF(obj) ((__typeof__(obj))Py_NewRef(obj))
#else
# define Ny_NEWREF(obj) ((void *)Py_NewRef(obj))
#endif

static inline void *
NyModule_AssertState(PyObject *module)
{
    void *state = PyModule_GetState(module);
    assert(state);
    return state;
}

static inline void *
NyType_AssertModuleState(PyTypeObject *type, struct PyModuleDef *def)
{
    PyObject *module = PyType_GetModuleByDef(type, def);
    assert(module);
    return NyModule_AssertState(module);
}

static inline void *
NyType_AssertModuleState2(PyTypeObject *type1, PyTypeObject *type2,
                          struct PyModuleDef *def)
{
    /* Annoyingly, binaryfuncs of the number protocol may invoke our functions
       with no way to tell which of the two arguments has a type we are looking for */

    if (type1 == type2)
        return NyType_AssertModuleState(type1, def);

    PyObject *module = PyType_GetModuleByDef(type1, def);
    if (module)
        return NyModule_AssertState(module);

    assert(PyErr_ExceptionMatches(PyExc_TypeError));
    PyErr_Clear();

    return NyType_AssertModuleState(type2, def);
}

#if NY_MASKED_VERSION_HEX >= Py_PACK_VERSION(3, 13)
struct NyOnce {
   int done; /* atomic bool */
   PyMutex mutex;
};

static inline void
NyOnce_CallOnce(struct NyOnce *once, void (*fn)(void))
{
    if (_Py_atomic_load_int_acquire(&once->done))
        return;

    PyMutex_Lock(&once->mutex);
    if (once->done)
        goto out;

    fn();

    _Py_atomic_store_int_release(&once->done, 1);
out:
    PyMutex_Unlock(&once->mutex);
}
#else
struct NyOnce {
   bool done;
};

static inline void
NyOnce_CallOnce(struct NyOnce *once, void (*fn)(void))
{
    if (once->done)
        return;

    fn();

    once->done = true;
}
#endif

#if NY_MASKED_VERSION_HEX < Py_PACK_VERSION(3, 13)
# define NYPTR_ADJUSTED(is, attr, adj) ((void *)&(is)->attr)
# define NYPTR_ADJUSTED_DEREF(is, attr, adj) ((is)->attr)
# define NYPTR_ADJUST_FROM_INTEPSTATE(attr, dbgoff) 0
#else
# define NYPTR_ADJUSTED(is, attr, adj) ((void *)((uintptr_t)(&(is)->attr) + adj))

# if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#  define NYPTR_ADJUSTED_DEREF(is, attr, adj) \
    (*(typeof((is)->attr) *)NYPTR_ADJUSTED(is, attr, adj))
# elif defined(__GNUC__) || defined(__clang__)
#  define NYPTR_ADJUSTED_DEREF(is, attr, adj) \
    (*(__typeof__((is)->attr) *)NYPTR_ADJUSTED(is, attr, adj))
# else
#  define NYPTR_ADJUSTED_DEREF(is, attr, adj) \
    (*(void **)NYPTR_ADJUSTED(is, attr, adj))
# endif

# define NYPTR_ADJ_INTEPSTATE(attr, dbgoff) \
    ((ptrdiff_t)_PyRuntime.debug_offsets.interpreter_state.dbgoff - offsetof(PyInterpreterState, attr))
#endif

#endif /* GUPPY_H_INCLUDED */
