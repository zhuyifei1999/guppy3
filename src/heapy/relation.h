#ifndef Ny_RELATION_H

typedef struct {
    PyObject_HEAD
    int kind;
    PyObject *relator;
} NyRelationObject;

#endif /* #ifndef Ny_RELATION_H */
