#ifndef EGG_INDEXWRITER_SERVICECLIENT_H_
#define EGG_INDEXWRITER_SERVICECLIENT_H_
#include "../eggIndexWriter.h"


E_BEGIN_DECLS

typedef struct eggIndexWriterServiceClient EGGINDEXWRITERSERVICECLIENT;
typedef struct eggIndexWriterServiceClient* HEGGINDEXWRITERSERVICECLIENT;


HEGGINDEXWRITER EGGAPI eggIndexWriter_open_serviceclient(void *hEggHandle, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_close_serviceclient(HEGGINDEXWRITER hEggIndexWriter);

EBOOL EGGAPI eggIndexWriter_set_analyzer_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_add_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_optimize_serviceclient(HEGGINDEXWRITER hEggIndexWriter);

HEGGINDEXREADER EGGAPI eggIndexWriter_init_reader_serviceclient(HEGGINDEXWRITER hIndexWriter);

EBOOL EGGAPI eggIndexWriter_reindex_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_delete_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_modify_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_incrementmodify_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_add_field_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_modify_field_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_delete_field_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName);

//HEGGSERVICECLIENTHANDLE eggIndexWriter_getHandle_serviceClient(HEGGINDEXWRITER hEggIndexWriter);

E_END_DECLS

#endif //EGG_INDEXWRITER_SERVICECLIENT_H_

