/* Support for multiple Python interpreters */

static char hp_interpreter_doc[] =

"interpreter(command:string [,locals:dict] ) -> int\n"
"\n"
"Create a new interpreter structure with a new thread and return the\n"
"thread identity number.\n"
"\n"
"The arguments are:\n"
"\n"
"    command   A command that will be exec'd in the new environment.\n"
"    locals    Local variables passed to the command when exec'd.\n"
"\n"
"\n"
"The new interpreter and thread is started in a new environment.  This\n"
"environment consists of a new '__main__' module, with the optional\n"
"locals dict as local variables.\n"
"\n"
"The interpreter() function will return after the new thread structure\n"
"has been created. The command will execute sooner or later.  The\n"
"thread will terminate, and the interpreter structure be deallocated,\n"
"when the command has been executed, and dependent threads have\n"
"terminated.";


static char hp_set_async_exc_doc[] =
"set_async_exc(thread_id:integer, exception:object)\n"
"\n"
"Set an exception to be raised asynchronously in a thread.\n"
;

#include "pythread.h"
#include "eval.h"

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 8
# define Py_BUILD_CORE
/* _PyMem_SetDefaultAllocator */
#  include <internal/pycore_pymem.h>
# undef Py_BUILD_CORE
#elif PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 7
PyAPI_FUNC(int) _PyMem_SetDefaultAllocator(
    PyMemAllocatorDomain domain,
    PyMemAllocatorEx *old_alloc);
#else
// source: cpython: Objects/obmalloc.c
static void *
_PyMem_RawMalloc(void *ctx, size_t size)
{
    if (size == 0)
        size = 1;
    return malloc(size);
}

static void *
_PyMem_RawCalloc(void *ctx, size_t nelem, size_t elsize)
{
    if (nelem == 0 || elsize == 0) {
        nelem = 1;
        elsize = 1;
    }
    return calloc(nelem, elsize);
}

static void *
_PyMem_RawRealloc(void *ctx, void *ptr, size_t size)
{
    if (size == 0)
        size = 1;
    return realloc(ptr, size);
}

static void
_PyMem_RawFree(void *ctx, void *ptr)
{
    free(ptr);
}

static int _PyMem_SetDefaultAllocator(
    PyMemAllocatorDomain domain,
    PyMemAllocatorEx *old_alloc)
{
    assert(domain == PYMEM_DOMAIN_RAW);
    PyMem_GetAllocator(domain, old_alloc);

    PyMemAllocatorEx alloc = {
        .malloc  = _PyMem_RawMalloc,
        .calloc  = _PyMem_RawCalloc,
        .realloc = _PyMem_RawRealloc,
        .free    = _PyMem_RawFree,
        .ctx     = NULL
    };
    PyMem_SetAllocator(domain, &alloc);
    return 0;
}
#endif

struct bootstate {
    PyObject *cmd;
    PyObject *locals;
    // used by child to signal parent that thread has started
    PyThread_type_lock evt_ready;
};

static void
t_bootstrap(void *boot_raw)
{
    struct bootstate *boot = (struct bootstate *)boot_raw;
    PyThreadState *tstate;
    PyObject *v;
    int res = 0;
    const char *str;

    // This is needed so that tracemalloc won't deadlock on us
    PyMemAllocatorEx old_alloc;
    _PyMem_SetDefaultAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    // borrow GIL from parent
    PyThreadState *save_tstate = PyThreadState_Swap(NULL);
    tstate = Py_NewInterpreter();
    PyThreadState_Swap(save_tstate);

    PyMem_SetAllocator(PYMEM_DOMAIN_RAW, &old_alloc);

    if (!tstate) {
        Py_DECREF(boot->cmd);
        Py_XDECREF(boot->locals);

        // PyMem_DEL must be called with GIL held. Once we release evt_ready,
        // GIL state is undefind.
        PyThread_type_lock evt_ready = boot->evt_ready;
        PyMem_DEL(boot_raw);
        PyThread_release_lock(evt_ready);

        PyThread_exit_thread();
    }

    // return GIL to parent, wait for it to unlock
    PyThread_release_lock(boot->evt_ready);
    PyEval_RestoreThread(tstate);

    if ((str = PyUnicode_AsUTF8(boot->cmd))) {
        PyObject *mainmod = PyImport_ImportModule("__main__");
        PyObject *maindict = PyModule_GetDict(mainmod);

        // Not using locals or otherwise functions defined inside would not be
        // able to access any newly defined global, just like how methods
        // defined in a class cannot access attributes of the class directly
        // without self.
        if (boot->locals) {
            res = PyDict_Update(maindict, boot->locals);
        }

        if (!res) {
            v = PyRun_String(str, Py_file_input, maindict, NULL);
            if (!v)
                res = -1;
            else {
                Py_DECREF(v);
                res = 0;
            }
            Py_DECREF(mainmod);
        }
    } else
        res = -1;
    if (res == -1 && PyErr_Occurred()) {
        if (PyErr_ExceptionMatches(PyExc_SystemExit))
            PyErr_Clear();
        else {
            PySys_FormatStderr("Unhandled exception in interpreter started by %R\n", boot->cmd);
            PyErr_PrintEx(0);
        }
	}
    Py_DECREF(boot->cmd);
    Py_XDECREF(boot->locals);
    PyMem_DEL(boot_raw);

    if (!(tstate->interp->tstate_head == tstate && tstate->next == NULL)) {
        PyObject *timemod = PyImport_ImportModule("time");
        PyObject *sleep = 0;
        PyObject *time;
        if (timemod) {
            sleep = PyObject_GetAttrString(timemod, "sleep");
            Py_DECREF(timemod);
        }
        time = PyFloat_FromDouble(0.05);
        while (!(tstate->interp->tstate_head == tstate && tstate->next == NULL)) {
            PyObject *res;
            res = PyObject_CallFunction(sleep, "O", time);
            if (res) {
                Py_DECREF(res);
            }
        }
        Py_DECREF(time);
        Py_DECREF(sleep);
    }

    Py_EndInterpreter(tstate);
    // We can't use _PyEval_ReleaseLock(NULL) here on Py3.9+
    // because of fvisibility=hidden.
    PyEval_ReleaseLock();
    PyThread_exit_thread();
}

static PyObject *
hp_interpreter(PyObject *self, PyObject *args)
{
    PyObject *cmd = NULL;
    PyObject *locals = NULL;
    PyThread_type_lock evt_ready;

    struct bootstate *boot;
    long ident;

    if (!PyArg_ParseTuple(args, "O!|O!:interpreter",
                    &PyUnicode_Type, &cmd,
                    &PyDict_Type, &locals))
            return NULL;

    boot = PyMem_NEW(struct bootstate, 1);
    if (boot == NULL)
        return PyErr_NoMemory();

    boot->cmd = cmd;
    boot->locals = locals;
    Py_INCREF(cmd);
    Py_XINCREF(locals);

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION < 9
    PyEval_InitThreads(); // Start the interpreter's thread-awareness
#endif

    evt_ready = PyThread_allocate_lock();
    if (evt_ready == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "lock creation failed");
        goto Err;
    }
    boot->evt_ready = evt_ready;

    ident = PyThread_start_new_thread(t_bootstrap, (void *)boot);
    if (ident == -1) {
        PyThread_free_lock(boot->evt_ready);
        PyErr_SetString(PyExc_RuntimeError, "can't start new thread");
        goto Err;
    }

    PyThread_acquire_lock(evt_ready, 1);

    // wait for child to release it
    PyThread_acquire_lock(evt_ready, 1);
    PyThread_free_lock(evt_ready);

    return PyLong_FromLong(ident);

Err:
    Py_DECREF(cmd);
    Py_XDECREF(locals);
    PyMem_DEL(boot);
    return NULL;
}

/* As PyThreadState_SetAsyncExc in pystate.c,
   but searches all interpreters.
   Thus it finds any task, and it should not be of
   any disadvantage, what I can think of..
*/


static Py_ssize_t
NyThreadState_SetAsyncExc(long id, PyObject *exc) {
    PyInterpreterState *interp;
    Py_ssize_t count = 0;

    // We should lock the interp list, but PyThreadState_SetAsyncExc
    // relies on that it is not locked,,,
    for (interp = PyInterpreterState_Head(); interp;
         interp = PyInterpreterState_Next(interp)) {
        if (!interp->tstate_head)
            continue;

        PyThreadState *save_tstate = PyThreadState_Swap(interp->tstate_head);
        count += PyThreadState_SetAsyncExc(id, exc);
        PyThreadState_Swap(save_tstate);
    }

    return count;
}



static PyObject *
hp_set_async_exc(PyObject *self, PyObject *args)
{
    PyObject *idobj, *exc;
    long id;
    Py_ssize_t r;
    if (!PyArg_ParseTuple(args, "OO",
                          &idobj, &exc))
        return NULL;
    if ((id = PyLong_AsLong(idobj)) == -1 && PyErr_Occurred())
        return NULL;
    if ((r = NyThreadState_SetAsyncExc(id, exc)) > 1) {
        NyThreadState_SetAsyncExc(id, NULL);
        r = -1;
    }
    return PyLong_FromSsize_t(r);
}
