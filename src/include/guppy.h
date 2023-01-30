#ifndef GUPPY_H_INCLUDED

#define NYFILL(t) {                      \
    if (!t.tp_new) {                     \
        t.tp_new = PyType_GenericNew;    \
    }                                    \
    if (PyType_Ready(&t) < 0) return -1; \
}

#if PY_VERSION_HEX >= 0x03080000
#  define Ny_TRASHCAN_BEGIN(op, dealloc) Py_TRASHCAN_BEGIN(op, dealloc)
#  define Ny_TRASHCAN_END(op) Py_TRASHCAN_END
#else
#  define Ny_TRASHCAN_BEGIN(op, dealloc) Py_TRASHCAN_SAFE_BEGIN(op)
#  define Ny_TRASHCAN_END(op) Py_TRASHCAN_SAFE_END(op)
#endif

#endif /* GUPPY_H_INCLUDED */
