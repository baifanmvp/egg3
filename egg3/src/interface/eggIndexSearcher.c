#include "../eggIndexSearcher.h"

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new(HEGGINDEXREADER hIndexReader)
{
    if (!hIndexReader) return EGG_NULL;
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexSearcher_new))
    {
        return EGG_FALSE;
    }

    return hIndexReader->hEggHandle->eggIndexSearcher_new(hIndexReader);
}

EBOOL EGGAPI eggIndexSearcher_delete(HEGGINDEXSEARCHER hIndexSearcher)
{
    if (!hIndexSearcher) return EGG_FALSE;
    if(POINTER_IS_INVALID(hIndexSearcher->hEggHandle->eggIndexSearcher_delete))
    {
        return EGG_FALSE;
    }
    
    return hIndexSearcher->hEggHandle->eggIndexSearcher_delete(hIndexSearcher);
}

EBOOL EGGAPI eggIndexSearcher_search_with_query(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    if (!hIndexSearcher) return EGG_FALSE;
    if(POINTER_IS_INVALID(hIndexSearcher->hEggHandle->eggIndexSearcher_search_with_query))
    {
        return EGG_FALSE;
    }
    
    return hIndexSearcher->hEggHandle->eggIndexSearcher_search_with_query(hIndexSearcher,
                                                                        hTopCollector,
                                                                        hQuery);
}


EBOOL EGGAPI eggIndexSearcher_count_with_query(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    if (!hIndexSearcher) return EGG_FALSE;
    if(POINTER_IS_INVALID(hIndexSearcher->hEggHandle->eggIndexSearcher_count_with_query))
    {
        return EGG_FALSE;
    }
    
    return hIndexSearcher->hEggHandle->eggIndexSearcher_count_with_query(hIndexSearcher,
                                                                        hTopCollector,
                                                                        hQuery);
}


HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter(HEGGINDEXSEARCHER hIndexSearcher)
{
    if (!hIndexSearcher) return EGG_FALSE;
    
    if(POINTER_IS_INVALID(hIndexSearcher->hEggHandle->eggIndexSearcher_get_queryiter))
    {
        return EGG_FALSE;
    }
    
    return hIndexSearcher->hEggHandle->eggIndexSearcher_get_queryiter(hIndexSearcher);
    
}


EBOOL EGGAPI eggIndexSearcher_search_with_queryiter(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter)
{
    if (!hIndexSearcher) return EGG_FALSE;
    
    if(POINTER_IS_INVALID(hIndexSearcher->hEggHandle->eggIndexSearcher_search_with_queryiter))
    {
        return EGG_FALSE;
    }
    
    return hIndexSearcher->hEggHandle->eggIndexSearcher_search_with_queryiter(hIndexSearcher,
                                                                              hTopCollector,
                                                                              hQuery,
                                                                              hIter);

}


EBOOL EGGAPI eggIndexSearcher_filter(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit)
{
    if (!hIndexSearcher) return EGG_FALSE;
    if(POINTER_IS_INVALID(hIndexSearcher->hEggHandle->eggIndexSearcher_filter))
    {
        return EGG_FALSE;
    }
    
    return hIndexSearcher->hEggHandle->eggIndexSearcher_filter(hIndexSearcher, hTopCollector, hQuery, iforderbyit);
}
