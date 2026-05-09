#ifndef GUPPY_H_INCLUDED

#define NYFILL(t) {                      \
    if (PyType_Ready(&t) < 0) return -1; \
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 13
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
