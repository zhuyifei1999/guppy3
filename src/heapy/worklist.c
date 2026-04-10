#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "impsets.h"
#include "roundupsize.h"
#include "worklist.h"

int
NySTWWorkList_Push(NySTWWorkList *w, PyObject *ob)
{
#ifndef Py_GIL_DISABLED
    if (NyNodeSet_setobj(&w->keepalive, ob) == -1)
        return -1;
#endif

    if (w->allo_size == w->used_size) {
        Py_ssize_t allo = roundupsize(w->allo_size + 1);
        PyObject **arr = w->arr;

        PyMem_RESIZE(arr, PyObject *, allo);
        if (!arr) {
            PyErr_NoMemory();
            return -1;
        }

        w->arr = arr;
        w->allo_size = allo;
    }

    w->arr[w->used_size++] = ob;
    return 0;
}

PyObject *
NySTWWorkList_Pop(NySTWWorkList *w)
{
    if (!w->used_size) {
        PyErr_SetString(PyExc_IndexError, "pop from empty list");
        return NULL;
    }
    return w->arr[--w->used_size];
}

PyObject *
NySTWWorkList_Peek(NySTWWorkList *w)
{
    if (!w->used_size) {
        PyErr_SetString(PyExc_IndexError, "peek in empty list");
        return NULL;
    }
    return w->arr[w->used_size - 1];
}

int
NySTWWorkList_ReplaceLast(NySTWWorkList *w, PyObject *ob)
{
    if (!w->used_size) {
        PyErr_SetString(PyExc_IndexError, "replace last in empty list");
        return -1;
    }
#ifndef Py_GIL_DISABLED
    if (NyNodeSet_setobj(&w->keepalive, ob) == -1)
        return -1;
#endif
    w->arr[w->used_size - 1] = ob;
    return 0;
}
