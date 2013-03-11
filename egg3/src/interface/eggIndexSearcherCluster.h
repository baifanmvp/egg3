/**
   \file eggIndexSearcher.h
   \brief 索引模块
   \ingroup egg
*/
#ifndef EGG_INDEX_SEARCHER_CLUSTER_H_
#define EGG_INDEX_SEARCHER_CLUSTER_H_

#include "../eggIndexSearcher.h"

E_BEGIN_DECLS
typedef struct eggIndexSearcherCluster EGGINDEXSEARCHERCLUSTER;
typedef struct eggIndexSearcherCluster* HEGGINDEXSEARCHERCLUSTER;

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new_cluster(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexSearcher_delete_cluster(HEGGINDEXSEARCHER hIndexSearcher);

HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter_cluster(HEGGINDEXSEARCHER hIndexSearcher);

EBOOL EGGAPI eggIndexSearcher_search_with_query_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_count_with_query_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery);

EBOOL EGGAPI eggIndexSearcher_search_with_queryiter_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hClusterIter);

EBOOL EGGAPI eggIndexSearcher_filter_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit);

E_END_DECLS

#endif //EGG_INDEX_SEARCHER_CLUSTER_H_
