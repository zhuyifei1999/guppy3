/* RootState implmentation */

char rootstate_doc[] =

"The type of an object with special functionality that gives access to\n"
"internals of the Python interpreter and thread structures.  It is used\n"
"as a top level root when traversing the heap to to make sure to find\n"
"some special objects that may otherwise be hidden.\n"
"\n"
"There are no references from the RootState object to the special\n"
"objects. But the heap traversal and related functions defined for\n"
"RootStateType look into the Python interpreter and thread structures.\n"
"The visibility is controlled by options set in the HeapView object\n"
"which is passed to the traversal function. This makes it possible to\n"
"hide an interpreter and/or some frames referring to system objects\n"
"that should not be traversed. (See the attributes\n"
"'is_hiding_calling_interpreter' and 'limitframe' in HeapView.)\n"
"\n"
"The objects found in interpreter and thread structures are related to\n"
"the RootState object via attributes with special names. These names\n"
"have a special form which will be described below. The name starts\n"
"with either an interpreter designator or a thread designator.  It is\n"
"then followed by the name of a member in the corresponding interpreter\n"
"or thread structure. These names are the same as the names of the\n"
"members in the C structures defining them. Some of the names may be\n"
"dependent on the Python interpreter version used.\n"
"\n"
"The attribute names are used for two purposes:\n"
"\n"
"o To be the name used in the result of the 'relate' operation between\n"
"  the RootState object and some object that is referred to via an\n"
"  internal Python interpreter or thread structure.\n"
"\n"
"o To be used as attribute names when selecting objects\n"
"  from the RootState object. This may be used to get at such\n"
"  an object knowing only its attribute name.\n"
"\n"
"\n"
"An attribute name is of one of the following three forms.\n"
"\n"
"    i<interpreter number>_<interpreter attribute>\n"
"\n"
"    i<interpreter number>_t<thread number>_<thread attribute>\n"
"\n"
"    i<interpreter number>_t<thread number>_f<frame number>\n"
"\n"
"<interpreter number>\n"
"\n"
"The interpreter number identifies a particular interpreter structure.\n"
"Often there is only one interpreter used, in which case the number is\n"
"0. It is possible to use more than one interpreter. The interpreters\n"
"are then numbered from 0 and up in the order they were started. [This\n"
"applies as long as no interpreter is terminated while there is still a\n"
"newer interpreter running. Then the newer interpreters will be\n"
"renumbered. If this is found to be a problem, a solution may be\n"
"devised for a newer release.]\n"
"\n"
"<interpreter attribute>\n"
"\n"
"The interpreter attribute is a member with PyObject pointer type \n"
"in the PyInterpreterState structure and can be, but not limited to, \n"
"one of the following:\n"
"\n"
"    modules\n"
"    sysdict\n"
"    builtins\n"
"    codec_search_path\n"
"    codec_search_cache\n"
"    codec_error_registry\n"
"\n"
"<thread number>\n"
"\n"
"The thread numbers are taken from the thread identity number assigned\n"
"by Python. [ In older versions without thread identity numbers the hex\n"
"address will be used.]\n"
"\n"
"<thread attribute>\n"
"\n"
"The thread attribute is a member with PyObject pointer type \n"
"in the PyThreadState structure and can be, but not limited to, \n"
"one of the following:\n"
"\n"
"    c_profileobj\n"
"    c_traceobj\n"
"    curexc_type\n"
"    curexc_value\n"
"    curexc_traceback\n"
"    exc_type\n"
"    exc_value\n"
"    exc_traceback\n"
"    dict\n"
"    async_exc\n"
"\n"
"<frame number>\n"
"\n"
"The frame list is treated specially. The frame list is continually\n"
"changed and the object that the frame member points to is not valid\n"
"for long enough to be useful. Therefore frames are referred to by a\n"
"special designator using the format shown above with a frame\n"
"number. The frame number is the number of the frame starting from 0\n"
"but counting in the reversed order of the frame list. Thus the first\n"
"started frame is 0, and in general the most recent frame has a number\n"
"that is the number of frames it has before it in call order.\n"
;

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION == 8
# define Py_BUILD_CORE
/* PyInterpreterState */
#  include <internal/pycore_pystate.h>
# undef Py_BUILD_CORE
#elif PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 9
# define Py_BUILD_CORE
/* PyInterpreterState */
#  include <internal/pycore_interp.h>
# undef Py_BUILD_CORE
#endif

#define THREAD_ID(ts)    (ts->thread_id)

static PyObject *
rootstate_repr(PyObject *op)
{
    return PyUnicode_FromString("RootState");
}

static void
rootstate_dealloc(void *arg)
{
    /* This should never get called, but we also don't want to SEGV if
     * we accidently decref RootState out of existance.
     */
    abort();
}



#define MEMBER(name) {#name, T_OBJECT, offsetof(PyInterpreterState, name)}

static struct PyMemberDef is_members[] = {
    MEMBER(modules),
    MEMBER(modules_by_index),
    MEMBER(sysdict),
    MEMBER(builtins),
    MEMBER(importlib),

    MEMBER(codec_search_path),
    MEMBER(codec_search_cache),
    MEMBER(codec_error_registry),

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
    MEMBER(dict),
#endif

    MEMBER(builtins_copy),
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
    MEMBER(import_func),
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
#ifdef HAVE_FORK
    MEMBER(before_forkers),
    MEMBER(after_forkers_parent),
    MEMBER(after_forkers_child),
#endif

    MEMBER(pyexitmodule),
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
    MEMBER(audit_hooks),
#endif
    {0} /* Sentinel */
};

#undef MEMBER

#define MEMBER(name) {#name, T_OBJECT, offsetof(PyThreadState, name)}
#define RENAMEMEMBER(name, newname) {#newname, T_OBJECT, offsetof(PyThreadState, name)}

static struct PyMemberDef ts_members[] = {
    MEMBER(frame),
    MEMBER(c_profileobj),
    MEMBER(c_traceobj),

    MEMBER(curexc_type),
    MEMBER(curexc_value),
    MEMBER(curexc_traceback),

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
    RENAMEMEMBER(exc_state.exc_type, exc_type),
    RENAMEMEMBER(exc_state.exc_value, exc_value),
    RENAMEMEMBER(exc_state.exc_traceback, exc_traceback),
#else
    MEMBER(exc_type),
    MEMBER(exc_value),
    MEMBER(exc_traceback),
#endif

    MEMBER(dict),
    MEMBER(async_exc),
    // trash_delete_later not included

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 8
    MEMBER(coroutine_wrapper),
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
    MEMBER(async_gen_firstiter),
    MEMBER(async_gen_finalizer),
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
    MEMBER(context),
#endif
    {0} /* Sentinel */
};

#undef MEMBER

#define ISATTR(name)                                                                  \
    if ((PyObject *)is->name == r->tgt) {                                             \
        if (r->visit(NYHR_ATTRIBUTE, PyUnicode_FromFormat("i%d_%s", isno, #name), r)) \
            return 1;                                                                 \
    }

#define TSATTR(name)                                                                                      \
    if ((PyObject *)ts->name == r->tgt) {                                                                 \
        if (r->visit(NYHR_ATTRIBUTE, PyUnicode_FromFormat("i%d_t%lu_%s", isno, THREAD_ID(ts), #name), r)) \
            return 1;                                                                                     \
    }

#define RENAMETSATTR(name, newname)                                                                          \
    if ((PyObject *)ts->name == r->tgt) {                                                                    \
        if (r->visit(NYHR_ATTRIBUTE, PyUnicode_FromFormat("i%d_t%lu_%s", isno, THREAD_ID(ts), #newname), r)) \
            return 1;                                                                                        \
    }

#define ISATTR_DIR(name)                                \
    attr = PyUnicode_FromFormat("i%d_%s", isno, #name); \
    if (attr) {                                         \
        if (PyList_Append(list, attr)) {                \
            Py_DECREF(attr);                            \
            goto Err;                                   \
        }                                               \
        Py_DECREF(attr);                                \
    }                                                   \

#define TSATTR_DIR(name)                                                    \
    attr = PyUnicode_FromFormat("i%d_t%lu_%s", isno, THREAD_ID(ts), #name); \
    if (attr) {                                                             \
        if (PyList_Append(list, attr)) {                                    \
            Py_DECREF(attr);                                                \
            goto Err;                                                       \
        }                                                                   \
        Py_DECREF(attr);                                                    \
    }                                                                       \


static int
rootstate_relate(NyHeapRelate *r)
{
    NyHeapViewObject *hv = (void *)r->hv;
    PyThreadState *ts,  *bts = PyThreadState_GET();
    PyInterpreterState *is;
    int isframe = PyFrame_Check(r->tgt);
    int isno;
    for (is = PyInterpreterState_Head(), isno = 0;
         is;
         is = PyInterpreterState_Next(is), isno++);
    for (is = PyInterpreterState_Head(), isno--;
         is;
         is = PyInterpreterState_Next(is), isno--) {
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 9
        if (is != PyInterpreterState_Get())
            continue;
#endif
        ISATTR(modules);
        ISATTR(modules_by_index);
        ISATTR(sysdict);
        ISATTR(builtins);
        ISATTR(importlib);

        ISATTR(codec_search_path);
        ISATTR(codec_search_cache);
        ISATTR(codec_error_registry);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
        ISATTR(dict);
#endif

        ISATTR(builtins_copy);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
        ISATTR(import_func);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
# ifdef HAVE_FORK
        ISATTR(before_forkers);
        ISATTR(after_forkers_parent);
        ISATTR(after_forkers_child);
# endif

        ISATTR(pyexitmodule);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
        ISATTR(audit_hooks);
#endif

        for (ts = is->tstate_head; ts; ts = ts->next) {
            if ((ts == bts && r->tgt == hv->limitframe) ||
                    (!hv->limitframe && isframe)) {
                int frameno = -1;
                int numframes = 0;
                PyFrameObject *frame;
                for (frame = (PyFrameObject *)ts->frame; frame; frame = frame->f_back) {
                    numframes++;
                    if (r->tgt == (PyObject *)frame)
                        frameno = numframes;
                }
                if (frameno != -1) {
                    frameno = numframes - frameno;
                    if (r->visit(NYHR_ATTRIBUTE, PyUnicode_FromFormat("i%d_t%lu_f%d", isno, THREAD_ID(ts), frameno), r))
                        return 1;
                }
            }
            TSATTR(c_profileobj);
            TSATTR(c_traceobj);

            TSATTR(curexc_type);
            TSATTR(curexc_value);
            TSATTR(curexc_traceback);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
            RENAMETSATTR(exc_state.exc_type, exc_type);
            RENAMETSATTR(exc_state.exc_value, exc_value);
            RENAMETSATTR(exc_state.exc_traceback, exc_traceback);
#else
            TSATTR(exc_type);
            TSATTR(exc_value);
            TSATTR(exc_traceback);
#endif

            TSATTR(dict);
            TSATTR(async_exc);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 8
            TSATTR(coroutine_wrapper);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
            TSATTR(async_gen_firstiter);
            TSATTR(async_gen_finalizer);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
            TSATTR(context);
#endif
        }
    }
    return 0;
}


int
rootstate_traverse(NyHeapTraverse *ta)
{
    visitproc visit = ta->visit;
    NyHeapViewObject *hv = (void *)ta->hv;
    void *arg = ta->arg;
    PyThreadState *ts, *bts = PyThreadState_GET();
    PyInterpreterState *is;

    for (is = PyInterpreterState_Head(); is; is = PyInterpreterState_Next(is)) {
        if (hv->is_hiding_calling_interpreter && is == bts->interp)
            continue;
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 9
        if (is != PyInterpreterState_Get())
            continue;
#endif
        Py_VISIT(is->modules);
        // Not traversing through this because it is of the same level as
        // modules, making pathfinding generate an extra path.
        // Py_VISIT(is->modules_by_index);
        Py_VISIT(is->sysdict);
        Py_VISIT(is->builtins);
        Py_VISIT(is->importlib);

        Py_VISIT(is->codec_search_path);
        Py_VISIT(is->codec_search_cache);
        Py_VISIT(is->codec_error_registry);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
        Py_VISIT(is->dict);
#endif

        Py_VISIT(is->builtins_copy);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
        Py_VISIT(is->import_func);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
# ifdef HAVE_FORK
        Py_VISIT(is->before_forkers);
        Py_VISIT(is->after_forkers_parent);
        Py_VISIT(is->after_forkers_child);
# endif

        Py_VISIT(is->pyexitmodule);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
        Py_VISIT(is->audit_hooks);
#endif

        for (ts = is->tstate_head; ts; ts = ts->next) {
            if (ts == bts && hv->limitframe) {
                Py_VISIT(hv->limitframe);
            } else if (!hv->limitframe) {
                Py_VISIT(ts->frame);
            }
            Py_VISIT(ts->c_profileobj);
            Py_VISIT(ts->c_traceobj);

            Py_VISIT(ts->curexc_type);
            Py_VISIT(ts->curexc_value);
            Py_VISIT(ts->curexc_traceback);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
            Py_VISIT(ts->exc_state.exc_type);
            Py_VISIT(ts->exc_state.exc_value);
            Py_VISIT(ts->exc_state.exc_traceback);
#else
            Py_VISIT(ts->exc_type);
            Py_VISIT(ts->exc_value);
            Py_VISIT(ts->exc_traceback);
#endif

            Py_VISIT(ts->dict);
            Py_VISIT(ts->async_exc);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 8
            Py_VISIT(ts->coroutine_wrapper);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
            Py_VISIT(ts->async_gen_firstiter);
            Py_VISIT(ts->async_gen_finalizer);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
            Py_VISIT(ts->context);
#endif
        }
    }
    return 0;
}

// Ported from py2
static PyObject *
_shim_PyMember_Get(const char *addr, struct PyMemberDef *mlist, const char *name)
{
    struct PyMemberDef *l;

    for (l = mlist; l->name != NULL; l++) {
        if (strcmp(l->name, name) == 0) {
            return PyMember_GetOne(addr, l);
        }
    }
    PyErr_SetString(PyExc_AttributeError, name);
    return NULL;
}


static PyObject *
rootstate_getattr(PyObject *obj, PyObject *name)
{
    const char *s = PyUnicode_AsUTF8(name);
    PyInterpreterState *is;
    int n = 0;
    int ino;
    unsigned long tno;
    if (!s)
        return 0;
    Py_INCREF(name);
    if (sscanf(s, "i%d_%n", &ino, &n) == 1) {
        s += n;
        int countis;
        int numis;
        for (is = PyInterpreterState_Head(), numis = 0;
             is;
             is = PyInterpreterState_Next(is), numis++);
        for (is = PyInterpreterState_Head(), countis = 0;
             is;
             is = PyInterpreterState_Next(is), countis++) {
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 9
            if (is != PyInterpreterState_Get())
                continue;
#endif
            int isno = numis - countis - 1;
            if (isno == ino) {
                if (sscanf(s, "t%lu_%n", &tno, &n) == 1) {
                    s += n;
                    PyThreadState *ts;
                    for (ts = is->tstate_head; ts; ts = ts->next) {
                        if (THREAD_ID(ts) == tno) {
                            int frameno = 0;
                            if (sscanf(s, "f%d%n", &frameno, &n) == 1 && s[n] == '\0') {
                                PyFrameObject *frame;
                                int numframes = 0;
                                for (frame = ts->frame; frame; frame = frame->f_back) {
                                    numframes ++;
                                }
                                for (frame = ts->frame; frame; frame = frame->f_back) {
                                    numframes --;
                                    if (numframes == frameno) {
                                        Py_INCREF(frame);
                                        Py_DECREF(name);
                                        return (PyObject *)frame;
                                    }
                                }
                                PyErr_Format(PyExc_AttributeError,
                                             "thread state has no frame numbered %d from bottom",
                                             frameno);
                                Py_DECREF(name);
                                return 0;
                            } else {
                                PyObject *ret = _shim_PyMember_Get((char *)ts, ts_members, s);
                                if (!ret)
                                    PyErr_Format(PyExc_AttributeError,
                                                 "thread state has no attribute '%s'",
                                                 s);
                                Py_DECREF(name);
                                return ret;
                            }
                        }
                    }
                    PyErr_SetString(PyExc_AttributeError, "no such thread state number");
                    Py_DECREF(name);
                    return 0;
                } else {
                    PyObject *ret = _shim_PyMember_Get((char *)is, is_members, s);
                    if (!ret)
                        PyErr_Format(PyExc_AttributeError,
                                     "interpreter state has no attribute '%s'",
                                     s);
                    Py_DECREF(name);
                    return ret;
                }
            }
        }
        PyErr_SetString(PyExc_AttributeError, "no such interpreter state number");
        Py_DECREF(name);
        return 0;
    }
    if (sscanf(s, "t%lu_%n", &tno, &n) == 1) {
        s += n;
        int countis;
        int numis;
        for (is = PyInterpreterState_Head(), numis = 0;
             is;
             is = PyInterpreterState_Next(is), numis++);
        for (is = PyInterpreterState_Head(), countis = 0;
             is;
             is = PyInterpreterState_Next(is), countis++) {
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 9
            if (is != PyInterpreterState_Get())
                continue;
#endif
            int isno = numis - countis - 1;
            PyThreadState *ts;
            for (ts = is->tstate_head; ts; ts = ts->next) {
                if (THREAD_ID(ts) == tno) {
                    PyObject *fullname = PyUnicode_FromFormat("i%d_%U", isno, name);
                    if (!fullname) {
                        Py_DECREF(name);
                        return 0;
                    }
                    PyErr_WarnFormat(PyExc_DeprecationWarning, 1,
                        "Getting thread state without an interpreter number "
                        "is deprecated. Use %R instead", fullname);
                    PyObject *res = rootstate_getattr(obj, fullname);
                    Py_DECREF(fullname);
                    Py_DECREF(name);
                    return res;
                }
            }
        }
    }
    PyErr_Format(PyExc_AttributeError, "root state has no attribute %R", name);
    Py_DECREF(name);
    return 0;
}

/* Dummy traverse function to make hv_std_traverse optimization not bypass this */
static int
rootstate_gc_traverse(PyObject *self, visitproc visit, void *arg)
{
    return 0;
}

static PyObject *
rootstate_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
{
    Py_INCREF(Ny_RootState);
    return (PyObject *)Ny_RootState;
}

static PyObject *
rootstate_dir(PyObject *self, PyObject *args)
{
    PyObject *list = PyList_New(0);
    if (!list)
        return 0;

    PyObject *attr;
    PyThreadState *ts;
    PyInterpreterState *is;
    int isno;

    for (is = PyInterpreterState_Head(), isno = 0;
         is;
         is = PyInterpreterState_Next(is), isno++);
    for (is = PyInterpreterState_Head(), isno--;
         is;
         is = PyInterpreterState_Next(is), isno--) {
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 9
        if (is != PyInterpreterState_Get())
            continue;
#endif
        ISATTR_DIR(modules);
        ISATTR_DIR(modules_by_index);
        ISATTR_DIR(sysdict);
        ISATTR_DIR(builtins);
        ISATTR_DIR(importlib);

        ISATTR_DIR(codec_search_path);
        ISATTR_DIR(codec_search_cache);
        ISATTR_DIR(codec_error_registry);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
        ISATTR_DIR(dict);
#endif

        ISATTR_DIR(builtins_copy);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
        ISATTR_DIR(import_func);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
# ifdef HAVE_FORK
        ISATTR_DIR(before_forkers);
        ISATTR_DIR(after_forkers_parent);
        ISATTR_DIR(after_forkers_child);
# endif

        ISATTR_DIR(pyexitmodule);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
        ISATTR_DIR(audit_hooks);
#endif

        for (ts = is->tstate_head; ts; ts = ts->next) {
            int numframes = 0;
            PyFrameObject *frame;
            for (frame = (PyFrameObject *)ts->frame; frame; frame = frame->f_back) {
                numframes++;
            }
            int frameno;
            for (frameno = 0; frameno < numframes; frameno++) {
                attr = PyUnicode_FromFormat("i%d_t%lu_f%d", isno, THREAD_ID(ts), frameno);
                if (attr) {
                    if (PyList_Append(list, attr)) {
                        Py_DECREF(attr);
                        goto Err;
                    }
                    Py_DECREF(attr);
                }
            }
            TSATTR_DIR(c_profileobj);
            TSATTR_DIR(c_traceobj);

            TSATTR_DIR(curexc_type);
            TSATTR_DIR(curexc_value);
            TSATTR_DIR(curexc_traceback);

            TSATTR_DIR(exc_type);
            TSATTR_DIR(exc_value);
            TSATTR_DIR(exc_traceback);

            TSATTR_DIR(dict);
            TSATTR_DIR(async_exc);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 8
            TSATTR_DIR(coroutine_wrapper);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 6
            TSATTR_DIR(async_gen_firstiter);
            TSATTR_DIR(async_gen_finalizer);
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
            TSATTR_DIR(context);
#endif
        }
    }

    return list;

Err:
    Py_DECREF(list);
    return 0;
}

static PyMethodDef rootstate_methods[] =
{
    {"__dir__", (PyCFunction)rootstate_dir, METH_NOARGS,
        "__dir__($self, /)\n"
        "--\n"
        "\n"
        "Specialized __dir__ implementation for rootstate."},
    {0}
};


PyTypeObject NyRootState_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "guppy.heapy.heapyc.RootStateType",
    .tp_basicsize = sizeof(PyObject),
    .tp_dealloc   = (destructor)rootstate_dealloc,
    .tp_repr      = rootstate_repr,
    .tp_getattro  = rootstate_getattr,
    .tp_flags     = Py_TPFLAGS_DEFAULT,
    .tp_doc       = rootstate_doc,
    .tp_traverse  = (traverseproc)rootstate_gc_traverse, /* DUMMY */
    .tp_methods   = rootstate_methods,
    .tp_alloc     = PyType_GenericAlloc,
    .tp_new       = rootstate_new,
    .tp_free      = PyObject_Del,
};

PyObject _Ny_RootStateStruct = {
    _PyObject_EXTRA_INIT
    1, &NyRootState_Type
};
