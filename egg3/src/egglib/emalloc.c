#include "emalloc.h"


GLOBAL EMEMVTABLE g_mem_vtable =
{
    malloc,
    realloc,
    free,
};


PUBLIC _PTR e_malloc(esize bytes)
{
    _PTR p = EGG_NULL;
    p = g_mem_vtable.MALLOC(bytes);
    if (p)
    {
        return p;
    }

    return EGG_NULL;
}

PUBLIC _VOID e_free(_PTR p)
{
    if (p)
    {
        g_mem_vtable.FREE(p);
    }
}

PUBLIC _PTR e_realloc(_PTR p)
{
    return p;
}

