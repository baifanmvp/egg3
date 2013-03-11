/**
   \file eggIndexReaderHttp.h
   \brief 索引模块
   \ingroup egg
*/
#ifndef EGG_INDEXREADER_REMOTE_H_
#define EGG_INDEXREADER_REMOTE_H_
#include "../EggDef.h"
#include "../eggIndexReader.h"


E_BEGIN_DECLS
typedef struct eggIndexReaderRemote EGGINDEXREADERREMOTE;
typedef struct eggIndexReaderRemote *HEGGINDEXREADERREMOTE;

/*!
  \fn HEGGINDEXREADER EGGAPI eggIndexReader_open(HEGGDIRECTORY)
  \brief 检索打开函数
  \param hDirectory 目录句柄。
  \return 检索句柄实例。
*/
HEGGINDEXREADER EGGAPI eggIndexReader_open_remote(void *hEggHandle);

/*!
  \fn EBOOL EGGAPI eggIndexReader_close(HEGGINDEXREADER)
  \brief 检索关闭函数。
  \param hEggIndexReader 检索句柄实例。
  \return
*/
EBOOL EGGAPI eggIndexReader_close_remote(HEGGINDEXREADER hIndexReader);


/*!
  \fn EBOOL EGGAPI eggIndexReader_get_document(HEGGINDEXREADER, did_t, HEGGDOCUMENT*)
  \brief 通过文章ID返回对应的文章内容。
  \param hIndexReader 检索句柄实例。
  \param nIdNum 文章ID。
  \param ppeggDocument 返回对应的文章内容。外参，外部需要释放 。
  \return
*/
EBOOL EGGAPI eggIndexReader_get_document_remote(HEGGINDEXREADER hEggIndexReader, EGGDID dId, HEGGDOCUMENT* ppeggDocument);

EBOOL EGGAPI eggIndexReader_get_documentset_remote(HEGGINDEXREADER hIndexReader, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument);

HEGGDOCUMENT EGGAPI eggIndexReader_export_document_remote(HEGGINDEXREADER hIndexReader, offset64_t* pCursor);

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo_remote(HEGGINDEXREADER hIndexReader, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo);

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo_remote(HEGGINDEXREADER hIndexReader, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo);

count_t EGGAPI eggIndexReader_get_doctotalcnt_remote(HEGGINDEXREADER hIndexReader);

HEGGINDEXREADER EGGAPI eggIndexReader_alloc_remote();

EBOOL EGGAPI eggIndexReader_free_remote(HEGGINDEXREADER hIndexReader);

EBOOL EGGAPI eggIndexReader_set_handle_remote(HEGGINDEXREADERREMOTE hIndexReaderRm, void *hEggHandle_);

E_END_DECLS

#endif //EGG_INDEXREADER_REMOTE_H_
