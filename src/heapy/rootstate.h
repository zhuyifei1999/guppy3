#ifndef NY_ROOTSTATE_H
#define NY_ROOTSTATE_H

extern PyTypeObject NyRootState_Type;

struct NyHeapTraverse;
struct NyHeapRelate;

extern int rootstate_traverse(struct NyHeapTraverse *ta);
extern int rootstate_relate(struct NyHeapRelate *r);

#endif
