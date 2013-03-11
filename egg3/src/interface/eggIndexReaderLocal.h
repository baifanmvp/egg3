/**
   \file eggIndexReader.h
   \brief 索引模块
   \ingroup egg
*/
#ifndef EGG_INDEXREADERLOCAL_H_
#define EGG_INDEXREADERLOCAL_H_
#include "../EggDef.h"
#include "../eggIndexReader.h"
#include "../eggIndexCache.h"
#include "../index/eggIdView.h"
#include "../index/eggDocView.h"
#include "../index/eggFieldView.h"
#include "../index/eggFieldWeight.h"
#include "../index/eggIndexView.h"


E_BEGIN_DECLS

/*!
  \fn HEGGINDEXREADER EGGAPI eggIndexReader_open(HEGGDIRECTORY)
  \brief 检索打开函数
  \param hDirectory 目录句柄。
  \return 检索句柄实例。
*/
HEGGINDEXREADER EGGAPI eggIndexReader_open_local(void *hEggHandle);

/*!
  \fn EBOOL EGGAPI eggIndexReader_close(HEGGINDEXREADER)
  \brief 检索关闭函数。
  \param hEggIndexReader 检索句柄实例。
  \return
*/
EBOOL EGGAPI eggIndexReader_close_local(HEGGINDEXREADER hIndexReader);

typedef struct eggIndexReaderResult EGGINDEXREADERRESULT;
typedef struct eggIndexReaderResult *HEGGINDEXREADERRESULT;
HEGGINDEXREADERRESULT eggIndexReaderResult_new();
EBOOL eggIndexReaderResult_pop_idnodes(HEGGINDEXREADERRESULT hReaderResult, HEGGIDNODE *hIdNodes, count_t *lpCntDoc);
EBOOL eggIndexReaderResult_pop_rangecache(HEGGINDEXREADERRESULT hReaderResult, HEGGWRESULT *hhWeightResult, count_t *p_nWeightResult);
EBOOL eggIndexReaderResult_delete(HEGGINDEXREADERRESULT hReaderResult);


/*!
  \fn EBOOL EGGAPI egg_IndexReader_query_documents(HEGGINDEXREADER, HEGGQUERYEXP, HEGGIDNODE*, count_t*)
  \brief 通过输入的关键字检索查询文章，返回文章ID。
  \param hIndexReader 检索句柄实例。
  \param hQueryExp 查询语句句柄实例
  \param *hhReaderResult 结果集句柄
  \return
*/
EBOOL EGGAPI eggIndexReader_query_documents_local(HEGGINDEXREADER hIndexReader, HEGGQUERYEXP hQueryExp, HEGGINDEXREADERRESULT *hhReaderResult);


/*!
  \fn EBOOL EGGAPI eggIndexReader_get_document(HEGGINDEXREADER, did_t, HEGGDOCUMENT*)
  \brief 通过文章ID返回对应的文章内容。
  \param hIndexReader 检索句柄实例。
  \param nIdNum 文章ID。
  \param ppeggDocument 返回对应的文章内容。外参，外部需要释放 。
  \return
*/
EBOOL EGGAPI eggIndexReader_get_document_local(HEGGINDEXREADER hEggIndexReader, EGGDID dId, HEGGDOCUMENT* ppeggDocument);

EBOOL EGGAPI eggIndexReader_get_documentset_local(HEGGINDEXREADER hIndexReader_, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument);

HEGGDOCUMENT EGGAPI eggIndexReader_export_document_local(HEGGINDEXREADER hIndexReader, offset64_t* pCursor);

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo_local(HEGGINDEXREADER hIndexReader, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo);

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo_local(HEGGINDEXREADER hIndexReader, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo);

EBOOL  EGGAPI eggIndexReader_clean_cache(HEGGINDEXREADER hIndexReader_);

HEGGINDEXREADER EGGAPI eggIndexReader_alloc_local();

EBOOL EGGAPI eggIndexReader_free_local(HEGGINDEXREADER hIndexReader);


EBOOL EGGAPI eggIndexReader_set_indexview_local(HEGGINDEXREADER hIndexReader, HEGGINDEXVIEW hIndexView);

EBOOL EGGAPI eggIndexReader_set_idview_local(HEGGINDEXREADER hIndexReader, HEGGIDVIEW hIdView);

EBOOL EGGAPI eggIndexReader_set_docview_local(HEGGINDEXREADER hIndexReader, HEGGDOCVIEW hDocView);

EBOOL EGGAPI eggIndexReader_set_fieldview_local(HEGGINDEXREADER hIndexReader, HEGGFIELDVIEW hFieldView);

EBOOL EGGAPI eggIndexReader_set_fieldweight_local(HEGGINDEXREADER hIndexReader, HEGGFIELDWEIGHT hFieldWeight);

EBOOL EGGAPI eggIndexReader_ref_indexcache_local(HEGGINDEXREADER hIndexReader, HEGGINDEXCACHE hIndexCache);

count_t EGGAPI eggIndexReader_get_doctotalcnt_local(HEGGINDEXREADER hIndexReader_);

//count_t EGGAPI eggIndexReader_get_docCnt_with_key_local(HEGGINDEXREADER hIndexReader_, char* lpFieldName, char* lpKey, size16_t kSz);

EBOOL EGGAPI eggIndexReader_set_handle_local(HEGGINDEXREADER hIndexReader, void *hEggHandle_);

HEGGFIELDVIEW EGGAPI eggIndexReader_get_fieldview_local(HEGGINDEXREADER hIndexReader);

HEGGFIELDWEIGHT EGGAPI eggIndexReader_get_fieldweight_local(HEGGINDEXREADER hIndexReader);

HEGGINDEXRANGE eggIndexReader_query_idsrange_local(HEGGINDEXREADER hIndexReader_,  fdid_t fdid);

E_END_DECLS

#endif //EGG_INDEXREADERLOCAL_H_
