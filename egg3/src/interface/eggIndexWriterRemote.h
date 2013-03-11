#ifndef EGG_INDEXWRITER_REMOTE_H_
#define EGG_INDEXWRITER_REMOTE_H_
#include "../eggIndexWriter.h"


E_BEGIN_DECLS

typedef struct eggIndexWriterRemote EGGINDEXWRITERREMOTE;
typedef struct eggIndexWriterRemote* HEGGINDEXWRITERREMOTE;


HEGGINDEXWRITER EGGAPI eggIndexWriter_open_remote(void *hEggHandle, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_close_remote(HEGGINDEXWRITER hEggIndexWriter);

EBOOL EGGAPI eggIndexWriter_set_analyzer_remote(HEGGINDEXWRITER hEggIndexWriter_, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_add_document_remote(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_optimize_remote(HEGGINDEXWRITER hEggIndexWriter);

HEGGINDEXREADER EGGAPI eggIndexWriter_init_reader_remote(HEGGINDEXWRITER hIndexWriter);

EBOOL EGGAPI eggIndexWriter_reindex_document_remote(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_delete_document_remote(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_modify_document_remote(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_incrementmodify_document_remote(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_add_field_remote(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_modify_field_remote(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_delete_field_remote(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName);


//HEGGREMOTEHANDLE eggIndexWriter_getHandle_remote(HEGGINDEXWRITER hEggIndexWriter);

E_END_DECLS

#endif //EGG_INDEXWRITER_REMOTE_H_

