#ifndef NY_IMPSETS_H
#define NY_IMPSETS_H

#include <stdbool.h>

#include "../sets/nodeset.h"

extern bool NyNodeSet_Check(PyObject *op);
extern bool NyImmNodeSet_Check(PyObject *op);

extern int import_sets(void);

#endif
