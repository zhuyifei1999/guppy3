#ifndef GUPPY_H_INCLUDED

#define NYFILL(t) {                      \
    if (PyType_Ready(&t) < 0) return -1; \
}

#endif /* GUPPY_H_INCLUDED */
