/* eggrws.c */
#include <egg3/Egg3.h>
#include <stdio.h>

int main()
{
    char *dir_path="rws://127.0.0.1:12000/";
    HEGGHANDLE hEggHandle;

    /* add */
    hEggHandle = eggPath_open(dir_path);    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "");
    HEGGDOCUMENT hDocument = eggDocument_new();
    char *buf = "good bad ugly";
    HEGGFIELD hField1 = eggField_new("content", buf, strlen(buf), EGG_CWS_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
    eggDocument_add(hDocument, hField1);
    eggIndexWriter_add_document(hIndexWrite, hDocument);
    eggDocument_delete(hDocument);
    eggIndexWriter_optimize(hIndexWrite);
    eggIndexWriter_close(hIndexWrite);
    
    eggPath_close(hEggHandle);
    
    return 0;
}
