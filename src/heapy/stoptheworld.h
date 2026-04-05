#ifndef NY_STOPTHEWORLD_H
#define NY_STOPTHEWORLD_H

#ifdef Py_GIL_DISABLED

#define NY_ASSERT_WORLD_STOPPED() \
    assert(PyInterpreterState_Get()->stoptheworld.world_stopped)
#define NY_ASSERT_WORLD_RUNNING() \
    assert(!PyInterpreterState_Get()->stoptheworld.world_stopped)

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
extern _Py_thread_local int _world_stopped;

#define NY_ASSERT_WORLD_STOPPED() assert(_world_stopped)
#define NY_ASSERT_WORLD_RUNNING() assert(!_world_stopped)

#define NY_STOP_WORLD() do {   \
    NY_ASSERT_WORLD_RUNNING(); \
    _world_stopped = 1;        \
} while (0)
#define NY_START_WORLD() do {  \
    NY_ASSERT_WORLD_STOPPED(); \
    _world_stopped = 0;        \
} while (0)

#else

#define NY_ASSERT_WORLD_STOPPED() ((void)0)
#define NY_ASSERT_WORLD_RUNNING() ((void)0)

#define NY_STOP_WORLD() do {} while (0)
#define NY_START_WORLD() do {} while (0)

#endif

#endif /* NY_STOPTHEWORLD_H */
