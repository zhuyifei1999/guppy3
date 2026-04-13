#ifndef NY_NODEGRAPH_H
#define NY_NODEGRAPH_H

typedef struct {
    PyObject *src, *tgt;
} NyNodeGraphEdge;

typedef struct NyNodeGraphObject {
    PyObject_HEAD
    struct HeapycState *ms;
    PyObject *_hiding_tag_;
    NyNodeGraphEdge *edges;
    Py_ssize_t used_size;
    Py_ssize_t allo_size;
    char is_mapping;
    char is_sorted;
    char is_preserving_duplicates;
} NyNodeGraphObject;

extern PyType_Spec NyNodeGraph_Spec;
extern PyType_Spec NyNodeGraphIter_Spec;


extern NyNodeGraphObject *NyNodeGraph_New(struct HeapycState *ms);
extern int NyNodeGraph_Region(NyNodeGraphObject *rg, PyObject *key,
                              NyNodeGraphEdge **lop, NyNodeGraphEdge **hip);
extern int NyNodeGraph_AddEdge(NyNodeGraphObject *rg, PyObject *src, PyObject *tgt);
extern void NyNodeGraph_Clear(NyNodeGraphObject *rg);
extern NyNodeGraphObject *NyNodeGraph_Copy(NyNodeGraphObject *rg);
extern int NyNodeGraph_Invert(NyNodeGraphObject *rg);
extern NyNodeGraphObject *NyNodeGraph_Inverted(NyNodeGraphObject *rg);
extern int NyNodeGraph_Update(NyNodeGraphObject *a, PyObject *b);

typedef struct NyHeapTraverse NyHeapTraverse;
typedef struct NyHeapRelate NyHeapRelate;

extern size_t nodegraph_size(PyObject *obj);
extern int nodegraph_traverse(NyHeapTraverse *t);
extern int nodegraph_relate(NyHeapRelate *r);

#endif /* NY_NODEGRAPH_H */
