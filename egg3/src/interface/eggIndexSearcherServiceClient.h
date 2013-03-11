/**
   \file eggIndexSearcher.h
   \brief 索引模块
   \ingroup egg
*/
#ifndef EGG_INDEX_SEARCHER_SERVICECLIENT_H_
#define EGG_INDEX_SEARCHER_SERVICECLIENT_H_

#include "../eggIndexSearcher.h"
#include "../eggSearchIter.h"

E_BEGIN_DECLS

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new_serviceclient(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexSearcher_delete_serviceclient(HEGGINDEXSEARCHER hIndexSearcher);

HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter_serviceclient(HEGGINDEXSEARCHER hIndexSearcher);

EBOOL EGGAPI eggIndexSearcher_search_with_query_serviceclient(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_count_with_query_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);
    
EBOOL EGGAPI eggIndexSearcher_search_with_query_sort_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_filter_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit);

EBOOL EGGAPI eggIndexSearcher_search_with_queryiter_serviceclient(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter);

E_END_DECLS

#endif //EGG_INDEX_SEARCHER_SERVICECLIENT_H_
