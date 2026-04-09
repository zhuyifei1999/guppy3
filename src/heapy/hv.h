#ifndef NY_HV_H
#define NY_HV_H

#include <stddef.h>

typedef struct ExtraType ExtraType;
typedef struct NyHeapDef NyHeapDef;
typedef struct NyHeapRelate NyHeapRelate;
typedef struct NyHeapViewObject NyHeapViewObject;
typedef struct NyNodeGraphObject NyNodeGraphObject;
typedef struct NyNodeSetObject NyNodeSetObject;

extern PyTypeObject NyHeapView_Type;
extern PyTypeObject NyHorizon_Type;
extern PyTypeObject NyNodeTuple_Type;
extern PyTypeObject NyRelation_Type;

ExtraType *hv_extra_type(NyHeapViewObject *hv, PyTypeObject *type);
extern size_t hv_std_size(NyHeapViewObject *hv, PyObject *obj);
extern int hv_std_relate(NyHeapRelate *hr);
extern int hv_std_traverse(NyHeapViewObject *hv, PyObject *obj, visitproc visit, void *arg);

extern const char hv_cli_and_doc[];
extern const char hv_cli_dictof_doc[];
extern const char hv_cli_findex_doc[];
extern const char hv_cli_id_doc[];
extern const char hv_cli_idset_doc[];
extern const char hv_cli_indisize_doc[];
extern const char hv_cli_inrel_doc[];
extern const char hv_cli_none_doc[];
extern const char hv_cli_prod_doc[];
extern const char hv_cli_rcs_doc[];
extern const char hv_cli_type_doc[];
extern const char hv_cli_user_defined_doc[];

extern PyObject *hv_cli_and(NyHeapViewObject *hv, PyObject *args);
extern PyObject *hv_cli_dictof(NyHeapViewObject *self, PyObject *args);
extern PyObject *hv_cli_findex(NyHeapViewObject *hv, PyObject *args);
extern PyObject *hv_cli_id(NyHeapViewObject *self, PyObject *args);
extern PyObject *hv_cli_idset(NyHeapViewObject *self, PyObject *args);
extern PyObject *hv_cli_indisize(NyHeapViewObject *self, PyObject *args);
extern PyObject *hv_cli_inrel(NyHeapViewObject *hv, PyObject *args);
extern PyObject *hv_cli_none(NyHeapViewObject *self, PyObject *args);
extern PyObject *hv_cli_prod(NyHeapViewObject *self, PyObject *args);
extern PyObject *hv_cli_rcs(NyHeapViewObject *hv, PyObject *args);
extern PyObject *hv_cli_type(NyHeapViewObject *self, PyObject *args);
extern PyObject *hv_cli_user_defined(NyHeapViewObject *self, PyObject *args, PyObject *kwds);

extern int NyHeapView_iterate(NyHeapViewObject *hv, int (*visit)(PyObject *, void *), void *arg);
extern NyNodeSetObject *hv_mutnodeset_new(NyHeapViewObject *hv);
extern int hv_cli_dictof_update(NyHeapViewObject *hv, NyNodeGraphObject *rg);
extern PyObject *hv_heap(NyHeapViewObject *self, PyObject *args, PyObject *kwds);

#ifdef NDEBUG
#define NY_ASSERT_IMMUTABLE_BUILTIN(obj) ((void)0)
#else
extern void NY_ASSERT_IMMUTABLE_BUILTIN(PyObject *obj);
#endif

#define NYTUPLELIKE_NEW(t) ((t *)PyTuple_New((sizeof(t) - sizeof(PyTupleObject)) / sizeof(PyObject *) + 1))
#define NYTUPLELIKE_ASSERT(s, m) \
    static_assert(offsetof(s, m) == offsetof(PyTupleObject, ob_item), \
                  "NYTUPLELIKE_ASSERT Header check failed for " #s ", check NYTUPLELIKE_HEAD")

/* NYTUPLELIKE_HEAD should match PyTupleObject before ob_item */
#if PY_VERSION_HEX >= Py_PACK_VERSION(3, 14)
# define NYTUPLELIKE_HEAD \
    PyObject_VAR_HEAD \
    Py_hash_t ob_hash;
#else
# define NYTUPLELIKE_HEAD \
    PyObject_VAR_HEAD
#endif

#endif
