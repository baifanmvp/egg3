/**
   \file Analyzer.h
   \brief
   \ingroup egg
*/
#ifndef EGG_ANALYZER_H_
#define EGG_ANALYZER_H_
#include "EggDef.h"

#include "./conf/eggConfig.h"
#include "./eggField.h"
#include <cwsplugin/cwsplugin.h>
#include "./eggSySRecorder.h"

#define ANALYZER_CNLEX "ImCnLexAnalyzer"
#define ANALYZER_CYLEX "ImCyLexAnalyzer"
#define ANALYZER_CWSLEX "ImCwsLexAnalyzer"
#define ANALYZER_CXLEX "ImCxLexAnalyzer"

E_BEGIN_DECLS

#define EGG_FNANALYZER_NEW "_new"
#define EGG_FNANALYZER_UPDATEWORD "_UpdateWord"
#define EGG_FNANALYZER_TOKENIZE "_tokenize"
#define EGG_FNANALYZER_DELETE "_delete"
#define EGGANALYDICT_FLUSHTIME (60*5)

typedef void* HEGGANALYZERHANDLE;

typedef HEGGANALYZERHANDLE (*FNANALYZERNEW)(P_NEW_BLOCK_ITEM);
typedef int (*FNANALYZERTOKENIZE)(HEGGANALYZERHANDLE, epointer, epointer*, epointer);
typedef int (*FNANALYZERDELETE)(HEGGANALYZERHANDLE);


typedef struct eggAnalyzer* HEGGANALYZER;
typedef struct eggAnalyzer EGGANALYZER;

struct eggAnalyzer
{
    echar* pName;
    HEGGANALYZERHANDLE pHandle;
    FNANALYZERNEW fnNew;
    FNANALYZERTOKENIZE fnTokenize;
    FNANALYZERTOKENIZE fnUpdateword;
    FNANALYZERDELETE fnDelete;
};

typedef struct eggAnalyzerSet* HEGGANALYZERSET;
typedef struct eggAnalyzerSet EGGANALYZERSET;

struct eggAnalyzerSet
{
    EGGANALYZER pUnitSet[1024];
    count_t cnt;
    echar* pCurName;
    pthread_mutex_t mutex;
    time_t time;

};


EBOOL EGGAPI eggAnalyzerSet_build();

HEGGANALYZER EGGAPI eggAnalyzer_get(char *analyzerName);

EBOOL EGGAPI eggAnalyzerSet_destroy();

PUBLIC void  eggAnalyzerSet_update_dict();
E_END_DECLS

#endif //EGG_ANALYZER_H_
