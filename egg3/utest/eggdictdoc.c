#include "egg3/Egg3.h"


#include <stdio.h>
#include <time.h>
#include <sys/time.h>




int main(int argc, char* argv[])
{
    char* dir_path = argv[1];
    struct timeval tv_start, tv_end;
    int i = 0;
    
    
    HEGGHANDLE hHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hHandle,  "");
    HEGGDOCUMENT hDocument = eggDocument_new();
        
    HEGGFIELD hField1 = eggField_new("typeInfo", "dict", strlen("dict"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
    eggDocument_add(hDocument, hField1);

    HEGGFIELD hField2 = eggField_new("analyName", "ImCwsLexAnalyzer", strlen("ImCwsLexAnalyzer"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
    eggDocument_add(hDocument, hField2);

    HEGGFIELD hField3 = eggField_new("dictName", "xuzhou", strlen("xuzhou"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
    eggDocument_add(hDocument, hField3);

    HEGGFIELD hField4 = eggField_new("dictKey", "鳌拜", strlen("鳌拜"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
    eggDocument_add(hDocument, hField4);

    
    eggIndexWriter_add_document(hIndexWrite, hDocument);
    
    
    eggDocument_delete(hDocument);
        
   eggIndexWriter_optimize(hIndexWrite);
        
        
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hHandle);
    return 0;
}

