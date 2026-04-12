#ifndef Ny_HEAPY_H

#include "heapdef.h"

typedef struct ExtraType ExtraType;
struct HeapycState;

typedef struct NyHeapViewObject {
    PyObject_HEAD
    struct HeapycState *ms;
    PyObject *root;
    PyObject *limitframe;
    PyObject *_hiding_tag_;
    PyObject *_hiding_tag__name;
    PyObject *static_types;
    PyObject *weak_type_callback;
    char is_hiding_calling_interpreter;
    ExtraType **xt_table;
    int xt_mask;
    size_t xt_size;
} NyHeapViewObject;

typedef struct ExtraType {
    PyTypeObject *xt_type;
    size_t (*xt_size)(PyObject *obj);
    int (*xt_traverse)(struct ExtraType *, PyObject *, visitproc, void *);
    int (*xt_relate)(struct ExtraType *, NyHeapRelate *r);
    ExtraType *xt_next;
    ExtraType *xt_base, *xt_he_xt;
    int (*xt_he_traverse)(struct ExtraType *, PyObject *, visitproc, void *);
    NyHeapViewObject *xt_hv;
    PyObject *xt_weak_type;
    NyHeapDef *xt_hd;
    Py_ssize_t xt_he_offs;
    int xt_trav_code;
} ExtraType;

typedef struct NyHeapClassifier {
    PyObject_HEAD
    PyObject (*classify)(struct NyHeapClassifier *self, PyObject *obj);
    void *extra0, *extra1, *extra2, *extra3;
} NyHeapClassifier;

typedef struct NyNodeSet_Exports NyNodeSet_Exports;
struct HeapycState {
    PyTypeObject *HeapView_Type;
    PyTypeObject *Horizon_Type;
    PyTypeObject *NodeGraph_Type;
    PyTypeObject *NodeGraphIter_Type;
    PyTypeObject *NodeTuple_Type;
    PyTypeObject *ObjectClassifier_Type;
    PyTypeObject *Relation_Type;
    PyTypeObject *RootState_Type;

    PyObject *RootState;
    NyNodeSet_Exports *nodeset_exports;

    NyHeapDef HvTypes_HeapDef[4];
};

extern struct PyModuleDef heapyc_def;

#endif /* Ny_HEAPY_H */
