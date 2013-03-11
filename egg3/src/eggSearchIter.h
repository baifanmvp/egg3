#ifndef EGG_SEARCH_ITER_H_
#define EGG_SEARCH_ITER_H_

#include "EggDef.h"
#define EGG_SEARCHITER_SINGLE 1
#define EGG_SEARCHITER_CLUSTER  2


#define EGG_PAGEFRONT -100000000
#define EGG_PAGEBACK  -200000000

#define EGG_PAGEFIRST 200000000
#define EGG_PAGELAST  300000000

#define EGG_ITER_UNITCNT 10

E_BEGIN_DECLS

#pragma pack(push)
#pragma pack(4)


typedef  struct eggSearchIter  EGGSEARCHITER;
typedef  struct eggSearchIter* HEGGSEARCHITER;

struct eggSearchIter
{
    index_t chunkIdx; //host
    index_t scoreDocIdx; //index
    count_t unitCnt; //pagenum
    int iterCnt; //rec
};
#pragma pack(pop)

HEGGSEARCHITER eggSearchIter_new();

EBOOL eggSearchIter_reset(HEGGSEARCHITER hIter, count_t unitCnt);

EBOOL eggSearchIter_iter(HEGGSEARCHITER hIter, int iterCnt);

EBOOL eggSearchIter_dup(HEGGSEARCHITER hDestIter, HEGGSEARCHITER hSrcIter);

EBOOL eggSearchIter_delete(HEGGSEARCHITER hIter);

index_t eggSearchIter_get_idx(HEGGSEARCHITER hIter);



#define EGGITER_OVERFIRST(hIter) (eggSearchIter_get_idx(hIter) == EGG_PAGEFIRST)
#define EGGITER_OVERLAST(hIter) (eggSearchIter_get_idx(hIter) == EGG_PAGELAST)

E_END_DECLS




#endif
