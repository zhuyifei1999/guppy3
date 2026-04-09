#ifndef NY_UTILS_H
#define NY_UTILS_H

extern int
iterable_iterate(PyObject *v, int (*visit)(PyObject *, void *),
                 void *arg);

extern PyObject *gc_get_objects(void);

#endif
