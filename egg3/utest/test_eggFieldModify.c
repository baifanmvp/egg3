#include "egg2/Egg2.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

int prepare(char *dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "");
    HEGGDOCUMENT hDocument = eggDocument_new();
    HEGGFIELD hField1 = eggField_new("body", NULL, 0, EGG_OTHER_ANALYZED | EGG_INDEX_STRING, "ImC2LexAnalyzer");

    eggDocument_add(hDocument, hField1);
    
    eggIndexWriter_add_document(hIndexWrite, hDocument);
    eggDocument_delete(hDocument);    
    eggIndexWriter_optimize(hIndexWrite);
    
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);
    return 0;
}
int addfield(char *dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "");
    eggIndexWriter_add_field(hIndexWrite, "price", EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);
    return 0;
}
int modifyfield(char *dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "");
    eggIndexWriter_modify_field(hIndexWrite, "body", "body2", EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);
    return 0;
}
int deletefield(char *dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "");
    eggIndexWriter_delete_field(hIndexWrite, "price");
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);
    return 0;
}
int readfield(char *dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
    count_t cntFieldNameInfo;
    HEGGFIELDNAMEINFO hFieldNameInfo;
    if (eggIndexReader_get_fieldNameInfo(hIndexReader, &hFieldNameInfo, &cntFieldNameInfo) == EGG_TRUE)
    {
        int i;
        for (i = 0; i < cntFieldNameInfo; i++)
        {
            printf("=TEST=");
            printf("%s: ", hFieldNameInfo[i].name);
	    printf("%llu ", (long long unsigned)hFieldNameInfo[i].fdid);
            if (hFieldNameInfo[i].type & EGG_NOT_ANALYZED)
            {
                printf("NotAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CWS_ANALYZED)
            {
                printf("CwsAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CN_ANALYZED)
            {
                printf("CnAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CY_ANALYZED)
            {
                printf("CyAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CX_ANALYZED)
            {
                printf("CxAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_OTHER_ANALYZED)
            {
                printf("OtherAnalyzed:%s ", hFieldNameInfo[i].analyzerName);
            }
            
            if (hFieldNameInfo[i].type & EGG_INDEX_STRING)
            {
                printf("String ");
            }
            else if (hFieldNameInfo[i].type & EGG_INDEX_INT32)
            {
                printf("Int32 ");
            }
            else if (hFieldNameInfo[i].type & EGG_INDEX_INT64)
            {
                printf("Int64 ");
            }
            else if (hFieldNameInfo[i].type & EGG_INDEX_DOUBLE)
            {
                printf("Double ");
            }
            printf("\n");
        }   
    }
    eggFieldView_delete_fieldNameInfo(hFieldNameInfo, cntFieldNameInfo);
    
    eggIndexReader_close(hIndexReader);
    eggPath_close(hEggHandle);
    return 0;
}

int main(int argc, char* argv[])
{
    prepare(argv[1]);
    readfield(argv[1]);

    addfield(argv[1]);
    readfield(argv[1]);

    modifyfield(argv[1]);
    readfield(argv[1]);

    deletefield(argv[1]);
    readfield(argv[1]);
        
    return 0;
}

