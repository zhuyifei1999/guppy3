#ifndef NY_STDTYPES_INTERNAL_H
#define NY_STDTYPES_INTERNAL_H

extern NyHeapDef NyStdTypes_HeapDef[];
extern void NyStdTypes_init(void);
extern int dict_relate_kv(NyHeapRelate *r, PyObject *dict, int k, int v);

#endif
