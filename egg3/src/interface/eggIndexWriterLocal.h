#ifndef EGG_INDEXWRITERLOCAL_H_
#define EGG_INDEXWRITERLOCAL_H_

#include "../eggIndexWriter.h"
E_BEGIN_DECLS

HEGGINDEXWRITER EGGAPI eggIndexWriter_open_local(void *hEggHandle, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_ref_indexcache_local(HEGGINDEXWRITER EGGAPI hEggIndexWriter, HEGGINDEXCACHE hIndexCache);

EBOOL EGGAPI eggIndexWriter_set_analyzer_local(HEGGINDEXWRITER EGGAPI hEggIndexWriter, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_close_local(HEGGINDEXWRITER hEggIndexWriter);

EBOOL EGGAPI eggIndexWriter_add_document_local(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggIndexWriter_optimize_local(HEGGINDEXWRITER hEggIndexWriter);


HEGGINDEXREADER EGGAPI  eggIndexWriter_init_reader_local(HEGGINDEXWRITER hIndexWriter);

EBOOL EGGAPI eggIndexWriter_reindex_document_local(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_delete_document_local(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_modify_document_local(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hNewDocument);

EBOOL EGGAPI eggIndexWriter_incrementmodify_document_local(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hNewDocument);

EBOOL EGGAPI eggIndexWriter_add_field_local(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_modify_field_local(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_delete_field_local(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName);

E_END_DECLS

#endif //EGG_INDEXWRITERLOCAL_H_

