#include "egg2/Egg2.h"


#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

int prepare(char *dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "");
    HEGGDOCUMENT hDocument = eggDocument_new();
    
    char *buf1 = "a";
    char *field1 = "f1";
    HEGGFIELD hField1 = eggField_new(field1, buf1, strlen(buf1), EGG_NOT_ANALYZED | EGG_INDEX_STRING);
    eggDocument_add(hDocument, hField1);

    char *buf2 = "b";
    char *field2 = "f2";
    HEGGFIELD hField2 = eggField_new(field2, buf2, strlen(buf2), EGG_NOT_ANALYZED | EGG_INDEX_STRING);
    eggDocument_add(hDocument, hField2);

    char *buf3 = "c";
    char *field3 = "f3";
    HEGGFIELD hField3 = eggField_new(field3, buf3, strlen(buf3), EGG_NOT_ANALYZED | EGG_INDEX_STRING);
    eggDocument_add(hDocument, hField3);
    
    eggIndexWriter_add_document(hIndexWrite, hDocument);
    eggDocument_delete(hDocument);
    eggIndexWriter_optimize(hIndexWrite);
    
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);
    return 0;
}

EGGDID getDocId(char *dir_path, char *fieldname, char *fieldvalue)
{
    EGGDID idDoc = {};
    
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    HEGGQUERY h1;
    h1 = eggQuery_new_string(fieldname, fieldvalue, strlen(fieldvalue), NULL);

    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    int ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);
    if (ret == EGG_TRUE)
    {
        eggTopCollector_normalized(hTopCollector, EGG_TOPSORT_NOT);
 
        HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
        count_t cnt =  eggTopCollector_total_hits(hTopCollector);
        assert(cnt == 1);
        idDoc = lp_score_doc[0].idDoc;
        
        {
            printf("=TEST=document: id[%llu]\n", (long long unsigned)EGGDID_DOCID(&lp_score_doc[0].idDoc));
            HEGGDOCUMENT lp_eggDocument = EGG_NULL;
           
            eggIndexReader_get_document(hIndexReader,
                                        lp_score_doc[cnt-1].idDoc, &lp_eggDocument);
           
            HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, fieldname);
            unsigned len = 0;
            char *val = eggField_get_value(lp_field, &len);
            printf("=TEST=[%s]:[%.*s]\n", fieldname, len, val);
            lp_field = 0;
            eggDocument_delete(lp_eggDocument);
        }
    }
    eggTopCollector_delete(hTopCollector);
    eggQuery_delete(h1);
    eggIndexSearcher_delete(hIndexSearcher);
    eggIndexReader_close(hIndexReader);
    eggPath_close(hEggHandle);
    return idDoc;
}

int modify(char *dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "");
    EGGDID docId = getDocId(dir_path, "f1", "a");
    
    HEGGDOCUMENT hDocument = eggDocument_new();

    char *buf2 = "B";
    HEGGFIELD hField2 = eggField_new("f2", buf2, strlen(buf2), EGG_NOT_ANALYZED | EGG_INDEX_STRING);
    eggDocument_add(hDocument, hField2);

    char *buf3 = "C";
    HEGGFIELD hField3 = eggField_new("f3", buf3, strlen(buf3), EGG_NOT_ANALYZED | EGG_INDEX_STRING);
    eggDocument_add(hDocument, hField3);
    
    eggIndexWriter_modify_document(hIndexWrite, docId, hDocument);
    hDocument = NULL;
    
    eggIndexWriter_optimize(hIndexWrite);
    
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);
    return 0;
}

int main(int argc, char* argv[])
{
    prepare(argv[1]);
    modify(argv[1]);
    getDocId(argv[1], "f1", "a");
    getDocId(argv[1], "f2", "b");
    getDocId(argv[1], "f3", "c");

    getDocId(argv[1], "f2", "B");
    getDocId(argv[1], "f3", "C");

    return 0;
}

