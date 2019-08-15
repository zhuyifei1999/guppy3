#ifndef NY_STDDEFS_INCLUDED
#define NY_STDDEFS_INCLUDED

/*
	Definitions of type structure(s) that were not exported
	but were needed anyway.
	XXX dangerous, if Python changes this may break.
	Should be made officially exported.
	Or pull some from offset in tp_members, but that seems
	a too complicated workaround for now.

*/


typedef struct {
    PyObject_HEAD
    PyObject *mapping;
} mappingproxyobject;


#endif /* NY_STDDEFS_INCLUDED */
