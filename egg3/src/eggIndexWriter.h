/**
   \file eggIndexWriter.h
   \brief 索引模块 (API)
   \ingroup egg
*/
#ifndef EGG_INDEXWRITER_H_
#define EGG_INDEXWRITER_H_
#include "./EggDef.h"
#include "./eggDid.h"
#include "./eggDocument.h"
#include "./eggIndexReader.h"
#include "eggAnalyzer.h"
#include "./eggHandle.h"
#include "./eggIndexCache.h"

E_BEGIN_DECLS


HEGGINDEXWRITER EGGAPI eggIndexWriter_open(void *hEggHandle, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_close(HEGGINDEXWRITER hEggIndexWriter);

EBOOL EGGAPI eggIndexWriter_ref_indexcache(HEGGINDEXWRITER hEggIndexWriter, HEGGINDEXCACHE hIndexCache);

EBOOL EGGAPI eggIndexWriter_set_analyzer(HEGGINDEXWRITER hEggIndexWriter, char *analyzerName);

EBOOL EGGAPI eggIndexWriter_add_document(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hEggDocument);

/*!
  \fn EBOOL EGGAPI eggIndexWriter_optimize(HEGGINDEXWRITER)
  \brief 写入到磁盘
  \return
*/
EBOOL EGGAPI eggIndexWriter_optimize(HEGGINDEXWRITER hEggIndexWriter);

HEGGINDEXREADER EGGAPI  eggIndexWriter_init_reader(HEGGINDEXWRITER hIndexWriter);

EBOOL EGGAPI eggIndexWriter_reindex_document(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_delete_document(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId);

EBOOL EGGAPI eggIndexWriter_modify_document(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hNewDocument);

EBOOL EGGAPI eggIndexWriter_incrementmodify_document(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hNewDocument);
EBOOL EGGAPI eggIndexWriter_add_field(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_modify_field(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
EBOOL EGGAPI eggIndexWriter_delete_field(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName);


/* for back-compliance */

#define eggIndexWriter_ref_indexCache(hEggIndexWriter, hIndexCache) eggIndexWriter_ref_indexcache(hEggIndexWriter, hIndexCache) 
#define eggIndexWriter_reIndex_document(hEggIndexWriter_, hEggDocument, dId) eggIndexWriter_reindex_document(hEggIndexWriter_, hEggDocument, dId) 
#define eggIndexWriter_incrementModify_document(hEggIndexWriter_, dId, hNewDocument) eggIndexWriter_incrementmodify_document(hEggIndexWriter_, dId, hNewDocument) 

/* for back-compliance end */

E_END_DECLS

#endif //EGG_INDEXWRITER_H_

