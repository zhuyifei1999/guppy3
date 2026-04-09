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

#endif /* GUPPY_H_INCLUDED */
