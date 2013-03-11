/**
   \file eggIndexSearcher.h
   \brief 索引模块 (API)
   \ingroup egg
*/
#ifndef EGG_INDEX_SEARCHER_H_
#define EGG_INDEX_SEARCHER_H_

#include "eggSearchIter.h"
#include "eggIndexReader.h"
#include "eggQuery.h"

#include "./eggTopCollector.h"
#include "./similarity/eggSimilarScore.h"
#include "./eggHandle.h"


E_BEGIN_DECLS


HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexSearcher_delete(HEGGINDEXSEARCHER hIndexSearcher);


EBOOL EGGAPI eggIndexSearcher_search_with_query(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);


EBOOL EGGAPI eggIndexSearcher_count_with_query(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);


HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter(HEGGINDEXSEARCHER hIndexSearcher);

EBOOL EGGAPI eggIndexSearcher_search_with_queryiter(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter);

EBOOL EGGAPI eggIndexSearcher_filter(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit);

#define EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcher) \
    ((!hIndexSearcher) ? EGG_TRUE : EGG_FALSE)


/* for back-compliance */

#define eggIndexSearcher_search_with_queryIter(hIndexSearcher, hTopCollector, hQuery, hIter) eggIndexSearcher_search_with_queryiter(hIndexSearcher, hTopCollector, hQuery, hIter) 
#define eggIndexSearcher_get_queryIter(hIndexSearcher) eggIndexSearcher_get_queryiter(hIndexSearcher) 

/* for back-compliance end */

E_END_DECLS

#endif //_EGG_INDEX_SEARCHER_H_
