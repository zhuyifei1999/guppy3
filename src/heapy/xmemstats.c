/* Extra, low-level memory statistics functions.
   Some is system dependent,
   some require special Python compilation.
*/

static char hp_xmemstats_doc[] =
"xmemstats()\n"
"\n"
"Print extra memory statistics. What is printed depends on the system\n"
"configuration.  ";

#ifndef MS_WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif


static size_t (*dlptr_malloc_usable_size)(void *ptr);
static void (*dlptr_malloc_stats)(void);
static int (*dlptr__PyObject_DebugMallocStats)(FILE *out);
static Py_ssize_t *dlptr__Py_RefTotal;

#ifdef HEAPY_BREAK_THREAD_SAFETY
static void **dlptr___malloc_hook, **dlptr___realloc_hook, **dlptr___free_hook;
static void *org___malloc_hook, *org___realloc_hook, *org___free_hook;

Py_ssize_t totalloc, totfree, numalloc, numfree;

static int has_malloc_hooks;
#endif

static void *addr_of_symbol(const char *symbol) {
#ifndef MS_WIN32
    return dlsym(RTLD_DEFAULT, symbol);
#else
    void *addr;
    HMODULE hMod;

    hMod = GetModuleHandle(NULL);
    if (hMod) {
        addr = GetProcAddress(hMod, symbol);
        if (addr)
            return addr;
    }

    hMod = GetModuleHandle(
        // Why isn't this in some CPython header file?
#ifdef _DEBUG
        "python" Py_STRINGIFY(PY_MAJOR_VERSION) Py_STRINGIFY(PY_MINOR_VERSION) "_d.dll"
#else
        "python" Py_STRINGIFY(PY_MAJOR_VERSION) Py_STRINGIFY(PY_MINOR_VERSION) ".dll"
#endif
    );
    if (hMod) {
        addr = GetProcAddress(hMod, symbol);
        if (addr)
            return addr;
    }

    hMod = GetModuleHandle("MSVCRT.dll");
    if (hMod) {
        addr = GetProcAddress(hMod, symbol);
        if (addr)
            return addr;
    }

    return 0;
#endif
}

#ifdef HEAPY_BREAK_THREAD_SAFETY
static void
breakit(void *p, char c)
{
    // fprintf(stderr, "breakit %p %c %d\n", p, c, dlptr_malloc_usable_size(p));
}

static void *mallochook(size_t size);
static void *reallochook(void *p, size_t size);
static void freehook(void *p);

#define HOOK_RESTORE do {                       \
    *dlptr___malloc_hook  = org___malloc_hook;  \
    *dlptr___realloc_hook = org___realloc_hook; \
    *dlptr___free_hook    = org___free_hook;    \
} while (0)

#define HOOK_SAVE do {                          \
    org___malloc_hook  = *dlptr___malloc_hook;  \
    org___realloc_hook = *dlptr___realloc_hook; \
    org___free_hook    = *dlptr___free_hook;    \
} while (0)

#define HOOK_SET do {                     \
    *dlptr___malloc_hook  = &mallochook;  \
    *dlptr___realloc_hook = &reallochook; \
    *dlptr___free_hook    = &freehook;    \
} while (0)


static void *
mallochook(size_t size) {
    void *p;

    HOOK_RESTORE;
    p = malloc(size);
    HOOK_SAVE;

    if (p) {
        totalloc += dlptr_malloc_usable_size(p);
        numalloc += 1;
    }
    breakit(p, 'm');

    HOOK_SET;
    return p;
}

static void *
reallochook(void *p, size_t size) {
    void *q;
    Py_ssize_t f;

    if (p)
        f = dlptr_malloc_usable_size(p);
    else
        f = 0;

    HOOK_RESTORE;
    q = realloc(p, size);
    HOOK_SAVE;

    if (p) {
        if (q) {
            if (q != p) {
                totfree += f;
                f = dlptr_malloc_usable_size(q);
                totalloc += f;
            } else {
                f = dlptr_malloc_usable_size(q) - f;
                if (f > 0) {
                    totalloc += f;
                } else {
                    totfree -= f;
                }
            }
        } else if (!size) {
            totfree += f;
            numfree += 1;
        }
    } else {
        if (q) {
            totalloc += dlptr_malloc_usable_size(q);
            numalloc += 1;
        }
    }

    breakit(q, 'r');

    HOOK_SET;
    return q;
}

static void
freehook(void *p) {
    totfree += dlptr_malloc_usable_size(p);
    HOOK_RESTORE;
    free(p);
    HOOK_SAVE;

    if (p)
        numfree += 1;

    HOOK_SET;
}
#endif

static void
xmemstats_init(void) {
#ifdef HEAPY_BREAK_THREAD_SAFETY
    dlptr___malloc_hook              = addr_of_symbol("__malloc_hook");
    dlptr___realloc_hook             = addr_of_symbol("__realloc_hook");
    dlptr___free_hook                = addr_of_symbol("__free_hook");
#endif
    dlptr_malloc_usable_size         = addr_of_symbol("malloc_usable_size");
    dlptr_malloc_stats               = addr_of_symbol("malloc_stats");
    dlptr__PyObject_DebugMallocStats = addr_of_symbol("_PyObject_DebugMallocStats");
    dlptr__Py_RefTotal               = addr_of_symbol("_Py_RefTotal");

#ifdef HEAPY_BREAK_THREAD_SAFETY
    has_malloc_hooks = dlptr___malloc_hook && dlptr___realloc_hook &&
        dlptr___free_hook && dlptr_malloc_usable_size;
    if (has_malloc_hooks) {
        HOOK_SAVE;
        HOOK_SET;
    }
#endif
}

static PyObject *
hp_xmemstats(PyObject *self, PyObject *args)
{
    if (dlptr__PyObject_DebugMallocStats) {
        fprintf(stderr, "======================================================================\n");
        fprintf(stderr, "Output from _PyObject_DebugMallocStats()\n\n");
        dlptr__PyObject_DebugMallocStats(stderr);
    }

    if (dlptr_malloc_stats) {
        fprintf(stderr, "======================================================================\n");
        fprintf(stderr, "Output from malloc_stats\n\n");
        dlptr_malloc_stats();
    }

#ifdef HEAPY_BREAK_THREAD_SAFETY
    if (has_malloc_hooks) {
        fprintf(stderr, "======================================================================\n");
        fprintf(stderr, "Statistics gathered from hooks into malloc, realloc and free\n\n");

        fprintf(stderr, "Allocated bytes                    =         %12zd\n", totalloc);
        fprintf(stderr, "Allocated - freed bytes            =         %12zd\n", totalloc-totfree);
        fprintf(stderr, "Calls to malloc                    =         %12zd\n", numalloc);
        fprintf(stderr, "Calls to malloc - calls to free    =         %12zd\n", numalloc-numfree);
    }
#endif

#ifndef Py_TRACE_REFS
    if (dlptr__Py_RefTotal) {
#endif
        fprintf(stderr, "======================================================================\n");
        fprintf(stderr, "Other statistics\n\n");
#ifndef Py_TRACE_REFS
    }
#endif

    if (dlptr__Py_RefTotal) {
        fprintf(stderr, "Total reference count              =         %12zd\n", *dlptr__Py_RefTotal);
    }

#ifdef Py_TRACE_REFS
    {
        PyObject *x; int i;
        for (i = 0, x = this_module->_ob_next; x != this_module; x = x->_ob_next, i++);
        fprintf(stderr, "Total heap objects                 =         %12d\n", i);
    }
#endif
    fprintf(stderr, "======================================================================\n");

    Py_INCREF(Py_None);
    return Py_None;
}
