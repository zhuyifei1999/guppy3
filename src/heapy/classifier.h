#ifndef NY_CLASSIFIER_H
#define NY_CLASSIFIER_H

struct HeapycState;

typedef struct {
    int flags;
    int size;
    char *name;
    char *doc;
    PyObject *(*classify)(struct HeapycState *ms, PyObject *self, PyObject *arg);
    PyObject *(*memoized_kind)(struct HeapycState *ms, PyObject *self, PyObject *kind);
    int (*cmp_le)(PyObject *self, PyObject *a, PyObject *b);
} NyObjectClassifierDef;

typedef struct{
    PyObject_HEAD
    NyObjectClassifierDef *def;
    PyObject *self;
} NyObjectClassifierObject;

typedef PyObject *(*modstatebinaryfunc)(struct HeapycState *ms, PyObject *self, PyObject *arg);

extern PyType_Spec NyObjectClassifier_Spec;

extern PyObject *NyObjectClassifier_New(struct HeapycState *ms, PyObject *self, NyObjectClassifierDef *def);
extern int NyObjectClassifier_Compare(NyObjectClassifierObject *cli, PyObject *a, PyObject *b, int cmp);

/* cmp argument (to select etc)
   The first 6 happen to correspond to Py_LT , Py_LE etc
   but I didn't want to define them as such to not introduce a dependency.
*/

#define CLI_LT	0
#define CLI_LE	1
#define CLI_EQ  2
#define CLI_NE	3
#define CLI_GT	4
#define CLI_GE	5
#define CLI_MAX	5	/* Current end of definitions */

extern int cli_cmp_as_int(PyObject *cmp);

#endif /* NY_CLASSIFIER_H */
