/**
   \file eggIndexReader.h
   \brief 索引模块 (API)
   \ingroup egg
*/
#ifndef EGG_INDEXREADER_H_
#define EGG_INDEXREADER_H_
#include "./EggDef.h"
#include "./eggDocument.h"
#include "./eggQuery.h"
#include "./index/eggIdView.h"
#include "./eggHandle.h"

#include "./eggIndexCache.h"
E_BEGIN_DECLS


/*!
  \fn HEGGINDEXREADER EGGAPI eggIndexReader_open(HEGGDIRECTORY)
  \brief 检索打开函数
  \param hDirectory 目录句柄。
  \return 检索句柄实例。
*/
HEGGINDEXREADER EGGAPI eggIndexReader_open(void *hEggHandle);

/*!
  \fn EBOOL EGGAPI eggIndexReader_close(HEGGINDEXREADER)
  \brief 检索关闭函数。
  \param hEggIndexReader 检索句柄实例。
  \return
*/
EBOOL EGGAPI eggIndexReader_close(HEGGINDEXREADER hIndexReader);

/*!
  \fn EBOOL EGGAPI eggIndexReader_get_document(HEGGINDEXREADER, did_t, HEGGDOCUMENT*)
  \brief 通过文章ID返回对应的文章内容。
  \param hIndexReader 检索句柄实例。
  \param nIdNum 文章ID。
  \param ppeggDocument 返回对应的文章内容。外参，外部需要释放 。
  \return
*/
EBOOL EGGAPI eggIndexReader_get_document(HEGGINDEXREADER hEggIndexReader, EGGDID dId, HEGGDOCUMENT* ppeggDocument);


EBOOL EGGAPI eggIndexReader_get_documentset(HEGGINDEXREADER hIndexReader_, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument);

HEGGDOCUMENT EGGAPI eggIndexReader_export_document(HEGGINDEXREADER hIndexReader, offset64_t* pCursor);

count_t EGGAPI eggIndexReader_get_doctotalcnt(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo(HEGGINDEXREADER hIndexReader, struct eggFieldNameInfo **hhFieldNameInfo, count_t *lpCntFieldNameInfo);

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo(HEGGINDEXREADER hIndexReader, char *fieldName, struct eggFieldNameInfo **hhFieldNameInfo);

EBOOL EGGAPI eggIndexReader_free(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexReader_ref_indexcache(HEGGINDEXREADER hEggIndexReader, HEGGINDEXCACHE hIndexCache);


/* for back-compliance */

#define eggIndexReader_ref_indexCache(hEggIndexReader, hIndexCache) eggIndexReader_ref_indexcache(hEggIndexReader, hIndexCache) 
#define eggIndexReader_get_singleFieldNameInfo(hIndexReader, fieldName, hhFieldNameInfo) eggIndexReader_get_singlefieldnameinfo(hIndexReader, fieldName, hhFieldNameInfo) 
#define eggIndexReader_get_fieldNameInfo(hIndexReader, hhFieldNameInfo, lpCntFieldNameInfo) eggIndexReader_get_fieldnameinfo(hIndexReader, hhFieldNameInfo, lpCntFieldNameInfo) 
#define eggIndexReader_get_documentSet(hIndexReader_, hScoreDoc, nDocCnt, pppeggDocument) eggIndexReader_get_documentset(hIndexReader_, hScoreDoc, nDocCnt, pppeggDocument) 
#define eggIndexReader_get_docTotalCnt(hIndexReader) eggIndexReader_get_doctotalcnt(hIndexReader) 

/* for back-compliance end */


E_END_DECLS

#endif //EGG_INDEXREADER_H_
