#ifndef NY_UTILS_H
#define NY_UTILS_H

struct HeapycState;

extern int
iterable_iterate(struct HeapycState *ms, PyObject *v,
                 int (*visit)(PyObject *, void *), void *arg);

extern PyObject *gc_get_objects(void);

#endif
