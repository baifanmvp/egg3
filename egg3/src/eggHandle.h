#ifndef EGGHANDLE_H
#define EGGHANDLE_H

#include "EggDef.h"
#include "eggDocument.h"
#include "index/eggFieldView.h"
#include "eggAnalyzer.h"
#include "eggQuery.h"
#include "eggIndexCache.h"
#include "eggDid.h"
#include "eggScoreDoc.h"
#include "eggSearchIter.h"

E_BEGIN_DECLS
struct eggTopCollector;
struct EGGHANDLE;
typedef struct EGGINDEXHEAD
{
    struct EGGHANDLE *hEggHandle;
} *HEGGINDEXHEAD;

typedef struct EGGHANDLE {
    HEGGINDEXHEAD EGGAPI (*eggHandle_dup)(struct EGGHANDLE *hEggHandle);
    EBOOL EGGAPI (*eggHandle_delete)(struct EGGHANDLE *hEggHandle);
    char* EGGAPI (*eggHandle_getName)(struct EGGHANDLE *hEggHandle);
    
    HEGGINDEXHEAD EGGAPI (*eggIndexReader_open)(void *hEggHandle);
    EBOOL EGGAPI (*eggIndexReader_close)(HEGGINDEXHEAD hIndexReader);
    EBOOL EGGAPI (*eggIndexReader_ref_indexcache)(HEGGINDEXHEAD hIndexReader, HEGGINDEXCACHE hIndexCache);    

    EBOOL EGGAPI (*eggIndexReader_get_document)(HEGGINDEXHEAD hEggIndexReader,
                                                EGGDID nIdNum,
                                                HEGGDOCUMENT ppeggDocument);
    
    EBOOL EGGAPI (*eggIndexReader_get_documentset)(HEGGINDEXHEAD hEggIndexReader,
                                                   HEGGSCOREDOC hScoreDoc,
                                                   count_t nDocCnt,
                                                   HEGGDOCUMENT** pppeggDocument);
    HEGGDOCUMENT EGGAPI (*eggIndexReader_export_document)(HEGGINDEXHEAD hIndexReader,
                                                          offset64_t* pCursor);
    count_t EGGAPI (*eggIndexReader_get_doctotalcnt)(HEGGINDEXHEAD hIndexReader);
    EBOOL EGGAPI (*eggIndexReader_get_fieldnameinfo)(HEGGINDEXHEAD hIndexReader,
                                                     HEGGFIELDNAMEINFO *hhFieldNameInfo,
                                                     count_t *lpCntFieldNameInfo);
    EBOOL EGGAPI (*eggIndexReader_get_singlefieldnameinfo)(HEGGINDEXHEAD hIndexReader,
                                               char *fieldName,
                                               HEGGFIELDNAMEINFO *hhFieldNameInfo);
    EBOOL EGGAPI (*eggIndexReader_free)(HEGGINDEXHEAD hIndexReader);

    HEGGINDEXHEAD EGGAPI (*eggIndexSearcher_new)(HEGGINDEXHEAD hIndexReader);
    EBOOL EGGAPI (*eggIndexSearcher_delete)(HEGGINDEXHEAD hIndexSearcher);
    EBOOL EGGAPI (*eggIndexSearcher_search_with_query)(HEGGINDEXHEAD hIndexSearcher,
                                                       struct eggTopCollector *hTopCollector,
                                                       HEGGQUERY hQuery);

    EBOOL EGGAPI (*eggIndexSearcher_count_with_query)(HEGGINDEXHEAD hIndexSearcher,
                                                      struct eggTopCollector *hTopCollector,
                                                      HEGGQUERY hQuery);


    HEGGSEARCHITER EGGAPI (*eggIndexSearcher_get_queryiter)(HEGGINDEXHEAD hIndexSearcher);
    
    EBOOL EGGAPI (*eggIndexSearcher_search_with_queryiter)(HEGGINDEXHEAD hIndexSearcher,
                                                           struct eggTopCollector *hTopCollector,                                                           
                                                           HEGGQUERY hQuery,
                                                           HEGGSEARCHITER hIter);
    EBOOL EGGAPI (*eggIndexSearcher_filter)(HEGGINDEXHEAD hIndexSearcher,
                                                           struct eggTopCollector *hTopCollector,
                                            HEGGQUERY hQuery, int iforderbyit);

    HEGGINDEXHEAD EGGAPI (*eggIndexWriter_open)(void *hEggHandle,
                                                char *analyzerName);
    EBOOL EGGAPI (*eggIndexWriter_close)(HEGGINDEXHEAD hEggIndexWriter);
    EBOOL EGGAPI (*eggIndexWriter_ref_indexcache)(HEGGINDEXHEAD hEggIndexWriter, HEGGINDEXCACHE hIndexCache);    
    EBOOL EGGAPI (*eggIndexWriter_set_analyzer)(HEGGINDEXHEAD hEggIndexWriter, char *analyzerName);    
    EBOOL EGGAPI (*eggIndexWriter_add_document)(HEGGINDEXHEAD hEggIndexWriter,
                                                HEGGDOCUMENT hEggDocument);
    EBOOL EGGAPI (*eggIndexWriter_optimize)(HEGGINDEXHEAD hEggIndexWriter);
    HEGGINDEXHEAD  (*eggIndexWriter_init_reader)(HEGGINDEXHEAD hIndexWriter);
    EBOOL EGGAPI (*eggIndexWriter_reindex_document)(HEGGINDEXHEAD hEggIndexWriter, HEGGDOCUMENT hEggDocument, EGGDID dId);

    EBOOL EGGAPI (*eggIndexWriter_delete_document)(HEGGINDEXHEAD hEggIndexWriter, EGGDID dId);
    EBOOL EGGAPI (*eggIndexWriter_modify_document)(HEGGINDEXHEAD hEggIndexWriter, EGGDID dId, HEGGDOCUMENT hEggDocument);
    
    EBOOL EGGAPI (*eggIndexWriter_incrementmodify_document)(HEGGINDEXHEAD hEggIndexWriter, EGGDID dId, HEGGDOCUMENT hEggDocument);
    EBOOL EGGAPI (*eggIndexWriter_add_field)(HEGGINDEXHEAD hEggIndexWriter, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
    EBOOL EGGAPI (*eggIndexWriter_modify_field)(HEGGINDEXHEAD hEggIndexWriter, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */);
    EBOOL EGGAPI (*eggIndexWriter_delete_field)(HEGGINDEXHEAD hEggIndexWriter, char *fieldName);
    

    EBOOL (*eggRemote_send)(HEGGINDEXHEAD hEggHandle,
                            char *buf, int size, int flags);
    EBOOL (*eggRemote_receive)(HEGGINDEXHEAD hEggHandle,
                               char **buf, int *size, int flags);
    EBOOL (*eggRemote_serve)();
    EBOOL (*eggHandle_close)();
} EGGHANDLE;

typedef EGGHANDLE *HEGGHANDLE;

typedef HEGGINDEXHEAD HEGGINDEXWRITER;
typedef HEGGINDEXHEAD HEGGINDEXREADER;
typedef HEGGINDEXHEAD HEGGINDEXSEARCHER;

#define EGG_PACKAGE_OPTIMIZE     1
#define EGG_PACKAGE_SEARCH       2
#define EGG_PACKAGE_SEARCH_SORT  3 
#define EGG_PACKAGE_SEARCH_ITER  4
#define EGG_PACKAGE_GETDOC       5
#define EGG_PACKAGE_EXPORTDOC    6 
#define EGG_PACKAGE_SEARCHFILTER 7
#define EGG_PACKAGE_GETDOCSET    8
#define EGG_PACKAGE_GETDOCTOTALCNT    9
#define EGG_PACKAGE_LOADEGG     10
#define EGG_PACKAGE_GETFIELDNAMEINFO    11
#define EGG_PACKAGE_GETSINGLEFIELDNAMEINFO    12
#define EGG_PACKAGE_ADDFIELD    13
#define EGG_PACKAGE_MODIFYFIELD    14
#define EGG_PACKAGE_DELETEFIELD    15

#define EGG_PACKAGE_DOC 16
#define EGG_PACKAGE_ID 17
#define EGG_PACKAGE_TOPCOLLECTOR 18
#define EGG_PACKAGE_QUERY 19
#define EGG_PACKAGE_RET 20
#define EGG_PACKAGE_ANALYZERNAME 21
#define EGG_PACKAGE_FLAG 22
#define EGG_PACKAGE_SCOREDOCSET 23
#define EGG_PACKAGE_ITER 28
#define EGG_PACKAGE_COUNT    29
#define EGG_PACKAGE_PATH    30
#define EGG_PACKAGE_FIELDNAMEINFO    31
#define EGG_PACKAGE_FIELDNAME    32
#define EGG_PACKAGE_FIELDTYPE    33

#define EGG_PACKAGE_OPTIMIZE_ADD 24
#define EGG_PACKAGE_OPTIMIZE_DELETE 25
#define EGG_PACKAGE_OPTIMIZE_MODIFY 26

#define EGG_PACKAGE_OPTIMIZE_ICREMENT  27

#define EGG_PACKAGE_SEARCH_COUNT  34



E_END_DECLS
#endif  /* EGGHANDLE_H */
