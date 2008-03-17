#ifndef Ny_BITSET_H
#define Ny_BITSET_H

#ifdef __cplusplus
extern "C" {
#endif

/* Defining Py_ssize_t for backwards compatibility, from PEP 353 */

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

typedef unsigned long NyBits;


/* Number of bits in a NyBits field */

#define NyBits_N	((long)(sizeof(NyBits) * 8))


#ifdef __LP64__		/* can't use sizeof */
#define NyBits_32 0
#define NyBits_64 1
#define NyBit_MAX LONG_MAX
#define NyBit_MIN LONG_MIN
#else
#define NyBits_32 1
#define NyBits_64 0
#define NyBit_MAX INT_MAX
#define NyBit_MIN INT_MIN
#endif

typedef Py_intptr_t NyBit;	/* What is the limits for this type? */


#define NyPos_MAX	(NyBit_MAX/NyBits_N)
#define NyPos_MIN	(NyBit_MIN/NyBits_N)

typedef struct {
    NyBit pos;		/* The position of the first bit / NyBits_N */
    NyBits bits;	/* The bits as a mask */
} NyBitField;

/* Immutable bitset */

typedef struct {
    PyObject_VAR_HEAD
    Py_ssize_t ob_length;	/* Result for len(), -1 if not yet calculated */
    NyBitField ob_field[1];	/* The bit fields, ob_size of these */
} NyImmBitSetObject;
			      
typedef struct {
    PyObject_HEAD
    NyImmBitSetObject *ob_val;
} NyCplBitSetObject;
			      
typedef struct {
    long pos;
    NyBitField *lo, *hi;
    NyImmBitSetObject *set;
} NySetField;

#define NyUnion_MINSIZE 1

typedef struct {
    PyObject_VAR_HEAD
    int cur_size;
    NySetField ob_field[NyUnion_MINSIZE];
} NyUnionObject;

/* Mutable bitset */

typedef struct {
    PyObject_HEAD
    int cpl;
    int splitting_size;
    NyBitField *cur_field;
    NyUnionObject *root;
    NyUnionObject fst_root;
} NyMutBitSetObject;


#define	NyBits_EMPTY		0
#define NyBits_AND		1	/*  a & b */
#define NyBits_OR		2	/*  a | b */
#define NyBits_XOR		3	/*  a ^ b */
#define NyBits_SUB		4	/*  a & ~b */
#define NyBits_SUBR		5	/* ~a & b */
#define NyBits_FALSE		6	/* ~a & a */
#define NyBits_TRUE		7	/* ~a | a */

/* Table for import of C objects & functions via Python's cobject mechanism
   in the module at name 'NyBitSet_Exports'
*/

typedef struct {
    int flags;
    int size;
    char *ident_and_version;
    NyMutBitSetObject *(*mbs_new)(void);
    /* setbit & clrbit sets or clears bit bitno
       set_or_clr sets or clears it depending on set_or_clr parameter
       All 3 functions return previous bit: 0 (clr) or 1 (set)
       On error, -1 is returned.
    */
    int (*mbs_setbit)(NyMutBitSetObject *v, NyBit bitno);
    int (*mbs_clrbit)(NyMutBitSetObject *v, NyBit bitno); 
    int (*mbs_set_or_clr)(NyMutBitSetObject *v, NyBit bitno, int set_or_clr);
    PyObject *(*mbs_as_immutable)(NyMutBitSetObject *v);
    int (*iterate)(PyObject *v,
		   int (*visit)(NyBit, void *),
		   void *arg
		   );

    int (*mbs_hasbit)(NyMutBitSetObject *v, NyBit bitno);
    int (*ibs_hasbit)(NyImmBitSetObject *v, NyBit bitno);
    int (*cpl_hasbit)(NyCplBitSetObject *v, NyBit bitno);
    int (*mbs_clear)(NyMutBitSetObject *v);
} NyBitSet_Exports;



#ifdef __cplusplus
}
#endif

#endif /* Ny_BITSET_H */

