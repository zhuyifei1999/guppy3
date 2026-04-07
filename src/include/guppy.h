#ifndef GUPPY_H_INCLUDED

#if PY_VERSION_HEX < 0x030e0000
# define Py_PACK_FULL_VERSION(X, Y, Z, LEVEL, SERIAL) ( \
   (((X) & 0xff) << 24) |                               \
   (((Y) & 0xff) << 16) |                               \
   (((Z) & 0xff) << 8) |                                \
   (((LEVEL) & 0xf) << 4) |                             \
   (((SERIAL) & 0xf) << 0))

# define Py_PACK_VERSION(major, minor) \
   Py_PACK_FULL_VERSION(major, minor, 0, 0, 0)
#endif


#endif /* GUPPY_H_INCLUDED */
