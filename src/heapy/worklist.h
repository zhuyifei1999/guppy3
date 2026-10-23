#ifndef NY_WORKLIST_H
#define NY_WORKLIST_H

#include "impsets.h"
#include "heapy.h"
#include "stoptheworld.h"

/* Stop-the-world worklist running on borrowed refs */

/* Non-freethreading builds are interruptable by signal, and the risk of
   fully running on borrowed refs is that object might be freed while in the
   worklist. Workaround this by adding an additional NodeSet as a
   keepalive mechanism */
typedef struct NySTWWorkList {
    Py_ssize_t allo_size;
    Py_ssize_t used_size;
    PyObject **arr;
#ifndef Py_GIL_DISABLED
    NyNodeSetObject keepalive;
#endif
} NySTWWorkList;

static inline int
NySTWWorkList_InitOnStack(struct HeapycState *ms, NySTWWorkList *w)
{
    NY_ASSERT_WORLD_STOPPED();
    w->allo_size = 0;
    w->used_size = 0;
    w->arr = NULL;
#ifndef Py_GIL_DISABLED
    /* In !Py_GIL_DISABLED, NySTWMutNodeSet holds objects */
    return NySTWMutNodeSet_InitOnStack(ms->nodeset_exports->ms, &w->keepalive);
#endif
    return 0;
}

static inline void
NySTWWorkList_Destroy(NySTWWorkList *w)
{
    NY_ASSERT_WORLD_STOPPED();
    PyMem_Free(w->arr);
    w->allo_size = 0;
    w->used_size = 0;
    w->arr = NULL;
#ifndef Py_GIL_DISABLED
    NySTWMutNodeSet_Destroy(&w->keepalive);
#endif
}

static inline Py_ssize_t
NySTWWorkList_Size(NySTWWorkList *w)
{
    return w->used_size;
}

extern int NySTWWorkList_Push(NySTWWorkList *w, PyObject *ob);
extern PyObject *NySTWWorkList_Pop(NySTWWorkList *w);
extern PyObject *NySTWWorkList_Peek(NySTWWorkList *w);
extern int NySTWWorkList_ReplaceLast(NySTWWorkList *w, PyObject *ob);

#endif
