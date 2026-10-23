#ifndef Ny_NODESETOBJECT_H
#define Ny_NODESETOBJECT_H

/* Flags for NyNodeSetObject */

#define NS_HOLDOBJECTS 1 /* Only to be cleared in special case with mutable nodeset. */
#define _NS_STW        2 /* Internal flag, only used for stop-the-world where objects are not refcounted. */

typedef struct NyNodeSetObject {
    PyObject_VAR_HEAD
    int flags;
    struct SetscState *ms;
    PyObject *_hiding_tag_;
    union {
        PyObject *bitset;   /* If mutable type, a mutable bitset with addresses (divided). */
        PyObject *nodes[1]; /* If immutable type, the start of node array, in address order. */
    } u;
} NyNodeSetObject;

NyNodeSetObject *NyMutNodeSet_New(struct SetscState *ms);
NyNodeSetObject *NyMutNodeSet_NewFlags(struct SetscState *ms, int flags);
NyNodeSetObject *NyMutNodeSet_NewHiding(struct SetscState *ms, PyObject *hiding_tag);

int NySTWMutNodeSet_InitOnStack(struct SetscState *ms, NyNodeSetObject *v);
void NySTWMutNodeSet_Destroy(NyNodeSetObject *v);

int NyNodeSet_setobj(NyNodeSetObject *v, PyObject *obj);
int NyNodeSet_clrobj(NyNodeSetObject *v, PyObject *obj);
int NyNodeSet_hasobj(NyNodeSetObject *v, PyObject *obj);

int NyNodeSet_iterate(NyNodeSetObject *hs,
                      int (*visit)(PyObject *, void *),
                      void *arg);

NyNodeSetObject *NyImmNodeSet_NewCopy(NyNodeSetObject *v);
NyNodeSetObject *NyImmNodeSet_NewSingleton(struct SetscState *ms, PyObject *element, PyObject *hiding_tag);
int NyNodeSet_be_immutable(NyNodeSetObject **nsp);


typedef struct NyNodeSet_Exports {
    int flags;
    int size;
    char *ident_and_version;
    struct SetscState *ms;
    PyTypeObject *nodeset_type;
    PyTypeObject *mutnodeset_type;
    PyTypeObject *immnodeset_type;
    NyNodeSetObject *(*newMut)(struct SetscState *ms);
    NyNodeSetObject *(*newMutHiding)(struct SetscState *ms, PyObject *tag);
    NyNodeSetObject *(*newMutFlags)(struct SetscState *ms, int flags);
    NyNodeSetObject *(*newImmCopy)(NyNodeSetObject *v);
    NyNodeSetObject *(*newImmSingleton)(struct SetscState *ms, PyObject *v, PyObject *hiding_tag);
    int (*be_immutable)(NyNodeSetObject **nsp);
    int (*setobj)(NyNodeSetObject *v, PyObject *obj);
    int (*clrobj)(NyNodeSetObject *v, PyObject *obj);
    int (*hasobj)(NyNodeSetObject *v, PyObject *obj);
    int (*iterate)(NyNodeSetObject *ns,
                   int (*visit)(PyObject *, void *),
                   void *arg);
    int (*initStw)(struct SetscState *ms, NyNodeSetObject *v);
    void (*destroyStw)(NyNodeSetObject *v);
} NyNodeSet_Exports;

#endif /* Ny_NODESETOBJECT_H */
