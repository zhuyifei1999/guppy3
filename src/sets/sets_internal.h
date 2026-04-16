#ifndef SETS_INTERNAL_H
#define SETS_INTERNAL_H

#include "sets.h"

/* BitSet */

extern int fsb_dx_nybitset_init(PyObject *m);

extern PyType_Spec BitSet_Spec;
extern PyType_Spec NyImmBitSet_Spec;
extern PyType_Spec NyImmBitSetIter_Spec;
extern PyType_Spec NyCplBitSet_Spec;
extern PyType_Spec NyMutBitSet_Spec;
extern PyType_Spec NyUnion_Spec;

NyImmBitSetObject *NyImmBitSet_New(struct SetscState *ms, NyBit size);
NyCplBitSetObject *NyCplBitSet_New(NyImmBitSetObject *v);
NyMutBitSetObject *NyMutBitSet_New(struct SetscState *ms);

typedef int (*NySetVisitor)(NyBit, void *) ;

typedef int (*NyIterableVisitor)(PyObject *, void *);


extern int NyAnyBitSet_iterate(PyObject *v,
                               NySetVisitor visit,
                               void *arg);


extern Py_ssize_t NyAnyBitSet_length(PyObject *v);


extern PyObject *NyMutBitSet_AsImmBitSet(NyMutBitSetObject *v);
extern int NyMutBitSet_clrbit(NyMutBitSetObject *v, NyBit bit);
extern int NyMutBitSet_setbit(NyMutBitSetObject *v, NyBit bit);
extern int NyMutBitSet_hasbit(NyMutBitSetObject *v, NyBit bit);
extern int NyImmBitSet_hasbit(NyImmBitSetObject *v, NyBit bit);

extern int NyMutBitSet_clear(NyMutBitSetObject *v);
extern NyBit NyMutBitSet_pop(NyMutBitSetObject *v, NyBit i);

int cplbitset_traverse(NyHeapTraverse *ta);
size_t mutbitset_indisize(NyMutBitSetObject *v);
size_t anybitset_indisize(PyObject *obj);
size_t generic_indisize(PyObject *v);

/* NodeSet */

extern int fsb_dx_nynodeset_init(PyObject *m);

size_t nodeset_indisize(PyObject *v);
int nodeset_traverse(NyHeapTraverse *ta);
int nodeset_relate(NyHeapRelate *r);

extern PyType_Spec NyNodeSet_Spec;
extern PyType_Spec NyMutNodeSet_Spec;
extern PyType_Spec NyImmNodeSet_Spec;
extern PyType_Spec NyImmNodeSetIter_Spec;

PyObject *nodeset_richcompare(NyNodeSetObject *v, NyNodeSetObject *w, int op);
PyObject *nodeset_ior(NyNodeSetObject *v, PyObject *w);

PyObject *nodeset_get_is_immutable(NyNodeSetObject *self, void *unused);

NyNodeSetObject *
NyMutNodeSet_SubtypeNewIterable(PyTypeObject *type, PyObject *iterable, PyObject *hiding_tag);
NyNodeSetObject *NyImmNodeSet_New(struct SetscState *ms, NyBit size, PyObject *hiding_tag);

NyNodeSetObject *
immnodeset_op(NyNodeSetObject *v, NyNodeSetObject *w, int op);

struct SetscState {
    PyTypeObject *BitSet_Type;
    PyTypeObject *ImmBitSet_Type;
    PyTypeObject *ImmBitSetIter_Type;
    PyTypeObject *CplBitSet_Type;
    PyTypeObject *MutBitSet_Type;
    PyTypeObject *Union_Type;
    PyTypeObject *NodeSet_Type;
    PyTypeObject *MutNodeSet_Type;
    PyTypeObject *ImmNodeSet_Type;
    PyTypeObject *ImmNodeSetIter_Type;

    NyImmBitSetObject *ImmBitSet_Empty; /* The predefined empty set */
    NyCplBitSetObject *ImmBitSet_Omega; /* The predefined set of all bits */

    PyObject *BitSet_FormMethod;

    NyBitSet_Exports bitset_exports;
    NyNodeSet_Exports nodeset_exports;
    NyHeapDef Sets_HeapDef[4];
};

extern struct PyModuleDef setsc_def;

#ifdef Py_GIL_DISABLED
#define _NY_IS_IMM(ms, op) (PyObject_TypeCheck(op, ms->ImmBitSet_Type) || \
                            PyObject_TypeCheck(op, ms->CplBitSet_Type))
#define NY_ASSERT_OBJ_IMM_OR_LOCKED(ms, op)                \
    assert(_NY_IS_IMM(ms, (PyObject *)(op)) ||             \
        PyMutex_IsLocked(&((PyObject *)(op))->ob_mutex))
#define NY_ASSERT_OBJ_IMM_OR_LOCKED_OR_SINGLEREF(ms, op)   \
    assert(_NY_IS_IMM(ms, (PyObject *)(op)) ||             \
        PyMutex_IsLocked(&((PyObject *)(op))->ob_mutex) || \
        PyUnstable_Object_IsUniquelyReferenced((PyObject *)(op)))
#else
#define NY_ASSERT_OBJ_IMM_OR_LOCKED(ms, op) do {} while (0)
#define NY_ASSERT_OBJ_IMM_OR_LOCKED_OR_SINGLEREF(ns, op) do {} while (0)
#endif

#endif /* SETS_INTERNAL_H */
