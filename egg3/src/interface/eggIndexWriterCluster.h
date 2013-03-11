#ifndef EGG_INDEXWRITER_CLUSTER_H_
#define EGG_INDEXWRITER_CLUSTER_H_
#include "../eggIndexWriter.h"
#include "../eggCluster.h"


E_BEGIN_DECLS
#define EGG_MODIFY_IDINVALID 2;
typedef struct eggIndexWriterCluster  EGGINDEXWRITERCLUSTER;
typedef struct eggIndexWriterCluster* HEGGINDEXWRITERCLUSTER;

HEGGINDEXWRITER EGGAPI eggIndexWriter_open_cluster(void *hEggHandle, char *lpSpanField);

EBOOL EGGAPI eggIndexWriter_close_cluster(HEGGINDEXWRITER hEggIndexWriter);


EBOOL EGGAPI eggIndexWriter_add_document_cluster(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_optimize_cluster(HEGGINDEXWRITER hEggIndexWriter);

HEGGINDEXREADER EGGAPI eggIndexWriter_init_reader_cluster(HEGGINDEXWRITER hIndexWriter);


EBOOL EGGAPI eggIndexWriter_delete_document_cluster(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_modify_document_cluster(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_incrementmodify_document_cluster(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_add_field_cluster(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_modify_field_cluster(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_delete_field_cluster(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName);

//HEGGREMOTEHANDLE eggIndexWriter_getHandle_remote(HEGGINDEXWRITER hEggIndexWriter);

E_END_DECLS

#endif //EGG_INDEXWRITER_REMOTE_H_

