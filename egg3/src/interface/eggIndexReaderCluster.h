#ifndef EGG_INDEXREADER_CLUSTER_H_
#define EGG_INDEXREADER_CLUSTER_H_
#include "../eggIndexReader.h"
#include "../eggCluster.h"

E_BEGIN_DECLS
typedef struct eggIndexReaderCluster EGGINDEXREADERCLUSTER;
typedef struct eggIndexReaderCluster *HEGGINDEXREADERCLUSTER;

HEGGINDEXREADER EGGAPI eggIndexReader_open_cluster(void *hEggHandle);

EBOOL EGGAPI eggIndexReader_close_cluster(HEGGINDEXREADER hIndexReader);


EBOOL EGGAPI eggIndexReader_get_document_cluster(HEGGINDEXREADER hEggIndexReader, EGGDID dId, HEGGDOCUMENT* ppeggDocument);

EBOOL EGGAPI eggIndexReader_get_documentset_cluster(HEGGINDEXREADER hIndexReader, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument);

HEGGDOCUMENT EGGAPI eggIndexReader_export_document_cluster(HEGGINDEXREADER hIndexReader, offset64_t* pCursor);

count_t EGGAPI eggIndexReader_get_doctotalcnt_cluster(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo_cluster(HEGGINDEXREADER hIndexReader, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo);

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo_cluster(HEGGINDEXREADER hIndexReader, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo);

HEGGINDEXREADER EGGAPI eggIndexReader_alloc_cluster();

EBOOL EGGAPI eggIndexReader_free_cluster(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexReader_set_handle_cluster(HEGGINDEXREADERCLUSTER hIndexReaderRm, void *hEggHandle_);

count_t EGGAPI eggIndexReader_get_chunkcnt_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt);

HEGGCHUNKHAND EGGAPI eggIndexReader_get_chunkhands_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt);

PUBLIC EBOOL EGGAPI eggIndexReader_set_handle_cluster(HEGGINDEXREADERCLUSTER hIndexReader, void *hEggHandle_);

PUBLIC EBOOL EGGAPI eggIndexReader_set_chunkhandles_cluster(HEGGINDEXREADERCLUSTER hIndexReader, HEGGCHUNKHAND hChunkHands, count_t chunkCnt);
E_END_DECLS

#endif //EGG_INDEXREADER_CLUSTER_H_

