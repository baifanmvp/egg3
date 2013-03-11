/**
   \file eggIndexSearcher.h
   \brief 索引模块
   \ingroup egg
*/
#ifndef EGG_INDEX_SEARCHER_REMOTE_H_
#define EGG_INDEX_SEARCHER_REMOTE_H_

#include "../eggIndexSearcher.h"
#include "../eggSearchIter.h"

E_BEGIN_DECLS

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new_remote(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexSearcher_delete_remote(HEGGINDEXSEARCHER hIndexSearcher);

HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter_remote(HEGGINDEXSEARCHER hIndexSearcher);

EBOOL EGGAPI eggIndexSearcher_search_with_query_remote(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_count_with_query_remote(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_search_with_query_sort_remote(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_filter_remote(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit);

EBOOL EGGAPI eggIndexSearcher_search_with_queryiter_remote(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter);

E_END_DECLS

#endif //EGG_INDEX_SEARCHER_REMOTE_H_
