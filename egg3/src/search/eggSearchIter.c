#include "../eggSearchIter.h"



HEGGSEARCHITER eggSearchIter_new()
{
    HEGGSEARCHITER lp_iter_search = (HEGGSEARCHITER)malloc(sizeof(EGGSEARCHITER));
    
    memset(lp_iter_search, 0, sizeof(EGGSEARCHITER));
    lp_iter_search->unitCnt = EGG_ITER_UNITCNT;

    return lp_iter_search;
}

EBOOL eggSearchIter_reset(HEGGSEARCHITER hIter, count_t unitCnt)
{
    if(POINTER_IS_INVALID(hIter))
    {
        return EGG_FALSE;
    }

    memset(hIter, 0, sizeof(EGGSEARCHITER));
    
    hIter->unitCnt = unitCnt;
    
    return EGG_TRUE;
    
}

EBOOL eggSearchIter_iter(HEGGSEARCHITER hIter, int iterCnt)
{
    if(POINTER_IS_INVALID(hIter))
    {
        return EGG_FALSE;
    }
    
    hIter->iterCnt += iterCnt;
    
    return EGG_TRUE;

}

EBOOL eggSearchIter_dup(HEGGSEARCHITER hDestIter, HEGGSEARCHITER hSrcIter)
{
    if(POINTER_IS_INVALID(hDestIter))
    {
        return EGG_FALSE;
    }
    if(POINTER_IS_INVALID(hSrcIter))
    {
        return EGG_FALSE;
    }
    memcpy(hDestIter, hSrcIter, sizeof(EGGSEARCHITER));
     
    return EGG_TRUE;
}

EBOOL eggSearchIter_delete(HEGGSEARCHITER hIter)
{
    if(POINTER_IS_INVALID(hIter))
    {
        return EGG_FALSE;
    }
    
    
    free(hIter);
    
    return EGG_TRUE;
}



index_t eggSearchIter_get_idx(HEGGSEARCHITER hIter)
{
    if(POINTER_IS_INVALID(hIter))
    {
        return -1;
    }
    
    return (hIter)->scoreDocIdx;
}

