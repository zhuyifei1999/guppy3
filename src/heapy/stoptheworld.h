#ifndef NY_STOPTHEWORLD_H
#define NY_STOPTHEWORLD_H

#ifdef Py_GIL_DISABLED

#include <stdbool.h>

/* This is in <internal/pycore_pystate.h>, but I don't want to Py_BUILD_CORE
   for everything using this header */
extern PyAPI_FUNC(void) _PyEval_StopTheWorld(PyInterpreterState *interp);
extern PyAPI_FUNC(void) _PyEval_StartTheWorld(PyInterpreterState *interp);

extern bool NY_IS_WORLD_STOPPED(void);
#define NY_ASSERT_WORLD_STOPPED() \
    assert(NY_IS_WORLD_STOPPED())
#define NY_ASSERT_WORLD_RUNNING() \
    assert(!NY_IS_WORLD_STOPPED())

#define NY_ASSERT_OBJ_LOCKED_OR_STW(op)                       \
    assert(PyMutex_IsLocked(&((PyObject *)(op))->ob_mutex) || \
           NY_IS_WORLD_STOPPED())

#define NY_STOP_WORLD() do {                         \
    NY_ASSERT_WORLD_RUNNING();                       \
    _PyEval_StopTheWorld(PyInterpreterState_Get());  \
} while (0)
#define NY_START_WORLD() do {                        \
    NY_ASSERT_WORLD_STOPPED();                       \
    _PyEval_StartTheWorld(PyInterpreterState_Get()); \
} while (0)

#elif !defined(NDEBUG)

/* This is technically per-interp and not per-thread, but on !Py_GIL_DISABLED
   it makes no difference */

/* From pyport.h */
# ifdef thread_local
#   define Ny_thread_local thread_local
# elif __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
#   define Ny_thread_local _Thread_local
# elif defined(_MSC_VER)  /* AKA NT_THREADS */
#   define Ny_thread_local __declspec(thread)
# elif defined(__GNUC__)  /* includes clang */
#   define Ny_thread_local __thread
# endif

extern Ny_thread_local int _world_stopped;

#define NY_IS_WORLD_STOPPED() _world_stopped
#define NY_ASSERT_WORLD_STOPPED() assert(NY_IS_WORLD_STOPPED())
#define NY_ASSERT_WORLD_RUNNING() assert(!NY_IS_WORLD_STOPPED())

#define NY_ASSERT_OBJ_LOCKED_OR_STW(op) do {} while (0)

#define NY_STOP_WORLD() do {   \
    NY_ASSERT_WORLD_RUNNING(); \
    _world_stopped = 1;        \
} while (0)
#define NY_START_WORLD() do {  \
    NY_ASSERT_WORLD_STOPPED(); \
    _world_stopped = 0;        \
} while (0)

#else

#define NY_IS_WORLD_STOPPED() \
    static_assert(0, 'world stop assertion should be compiled out')
#define NY_ASSERT_WORLD_STOPPED() ((void)0)
#define NY_ASSERT_WORLD_RUNNING() ((void)0)

#define NY_ASSERT_OBJ_LOCKED_OR_STW(op) do {} while (0)

#define NY_STOP_WORLD() do {} while (0)
#define NY_START_WORLD() do {} while (0)

#endif

#endif /* NY_STOPTHEWORLD_H */
