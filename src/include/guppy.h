#ifndef GUPPY_H_INCLUDED

#define NY_MASKED_VERSION_HEX (PY_VERSION_HEX & 0xffff0000)

#if NY_MASKED_VERSION_HEX < 0x030e0000
# define Py_PACK_FULL_VERSION(X, Y, Z, LEVEL, SERIAL) ( \
   (((X) & 0xff) << 24) |                               \
   (((Y) & 0xff) << 16) |                               \
   (((Z) & 0xff) << 8) |                                \
   (((LEVEL) & 0xf) << 4) |                             \
   (((SERIAL) & 0xf) << 0))

# define Py_PACK_VERSION(major, minor) \
   Py_PACK_FULL_VERSION(major, minor, 0, 0, 0)
#endif

#ifdef Py_GIL_DISABLED
# define Ny_BEGIN_CRITICAL_SECTION(op) Py_BEGIN_CRITICAL_SECTION(op)
# define Ny_END_CRITICAL_SECTION() Py_END_CRITICAL_SECTION()
# define Ny_BEGIN_CRITICAL_SECTION2(a, b) Py_BEGIN_CRITICAL_SECTION2(a, b)
# define Ny_END_CRITICAL_SECTION2() Py_END_CRITICAL_SECTION2()
#else
/* We can't goto a normal Py_END_CRITICAL_SECTION() without using,
   C23 extensions, and certain compilers don't like that. Other
   compilers don't like empty statement at the beginning of
   a block... */
# define Ny_BEGIN_CRITICAL_SECTION(op) { (void)0
# define Ny_END_CRITICAL_SECTION() (void)0; } (void)0
# define Ny_BEGIN_CRITICAL_SECTION2(a, b) { (void)0
# define Ny_END_CRITICAL_SECTION2() (void)0; } (void)0
#endif

#if !defined(Py_GIL_DISABLED) && NY_MASKED_VERSION_HEX < Py_PACK_VERSION(3, 13)
/* For non-freethreading builds, a datarace requires per-interpreter GIL, which
   I'm not too concerned about */
#define _Py_atomic_load_int(ptr) (*(ptr))
#define _Py_atomic_store_int(ptr, val) (*(ptr) = (val))

#define _Py_atomic_load_ptr_relaxed(ptr) (*(ptr))
#define _Py_atomic_store_ptr_relaxed(ptr, val) (*(ptr) = (val))

#define _Py_atomic_fence_acquire() ((void)0)
#define _Py_atomic_fence_release() ((void)0)
#endif

#endif /* GUPPY_H_INCLUDED */
