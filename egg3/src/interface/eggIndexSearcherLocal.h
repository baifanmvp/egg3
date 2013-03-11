/**
   \file eggIndexSearcher.h
   \brief 索引模块
   \ingroup egg
*/
#ifndef EGG_INDEX_SEARCHER_LOCAL_H_
#define EGG_INDEX_SEARCHER_LOCAL_H_

#include "../eggIndexSearcher.h"

E_BEGIN_DECLS

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new_local(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexSearcher_delete_local(HEGGINDEXSEARCHER hIndexSearcher);

HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter_local(HEGGINDEXSEARCHER hIndexSearcher);

EBOOL EGGAPI eggIndexSearcher_search_with_query_local(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_count_with_query_local(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_filter_local(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit);

EBOOL EGGAPI eggIndexSearcher_search_with_queryiter_local(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter);

E_END_DECLS

#endif //EGG_INDEX_SEARCHER_local_H_
