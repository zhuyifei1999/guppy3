#ifndef GUPPY_H_INCLUDED

#define NYFILL(t) {                      \
    if (PyType_Ready(&t) < 0) return -1; \
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 13
# define NYINTERPSTATE_PTR(is, attr, refattr, refdbgoff) ((void *)&(is)->attr)
# define NYINTERPSTATE_DEREF_PTR(is, attr, refattr, refdbgoff) ((is)->attr)
#else
# define NYINTERPSTATE_PTR(is, attr, refattr, refdbgoff) \
    ((void *)(&(is)->attr) \
        + _PyRuntime.debug_offsets.interpreter_state.refdbgoff \
        - offsetof(PyInterpreterState, refattr))

# if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#  define NYINTERPSTATE_DEREF_PTR(is, attr, refattr, refdbgoff) \
    (*(typeof((is)->attr) *)NYINTERPSTATE_PTR(is, attr, refattr, refdbgoff))
# elif defined(__GNUC__) || defined(__clang__)
#  define NYINTERPSTATE_DEREF_PTR(is, attr, refattr, refdbgoff) \
    (*(__typeof__((is)->attr) *)NYINTERPSTATE_PTR(is, attr, refattr, refdbgoff))
# else
#  define NYINTERPSTATE_DEREF_PTR(is, attr, refattr, refdbgoff) \
    ((PyInterpreterState){.attr = (*(void **)NYINTERPSTATE_PTR(is, attr, refattr, refdbgoff))}).attr
# endif
#endif

#endif /* GUPPY_H_INCLUDED */
