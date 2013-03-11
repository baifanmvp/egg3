#ifndef _EGG_MALLOC_H_
#define _EGG_MALLOC_H_

#include "../EggDef.h"
E_BEGIN_DECLS

typedef struct tagEMemVTable EMEMVTABLE;
typedef struct tagEMemVTable* HEMEMVTABLE;

struct tagEMemVTable
{
    _PTR  (*MALLOC)  (esize bytes);
    _PTR  (*REALLOC) (_PTR p, esize bytes);
    _VOID (*FREE)    (_PTR p);
};

extern _PTR e_malloc(esize bytes);

extern _VOID e_free(_PTR p);

extern _PTR e_realloc(_PTR p);



//#


E_END_DECLS


#endif //_EGG_MALLOC_H_

