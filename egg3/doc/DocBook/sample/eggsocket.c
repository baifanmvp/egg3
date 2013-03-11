/* eggsocket.c */
#include <egg3/Egg3.h>
#include <stdio.h>

int main()
{
    char *dir_path="tcp://127.0.0.1:8888/tmp/";
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


    /* query */
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    HEGGQUERY h1;
    h1 = eggQuery_new_string("content", "good", strlen("good"), ANALYZER_CWSLEX);
    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    int ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);
    if (ret == EGG_TRUE)
    {
        eggTopCollector_normalized(hTopCollector, EGG_TOPSORT_NOT);
 
        HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
        count_t cnt =  eggTopCollector_total_hits(hTopCollector);
        {        
            printf("document: id[%llu]\n", (long long unsigned)EGGDID_DOCID(&lp_score_doc[0].idDoc));
            HEGGDOCUMENT lp_eggDocument = EGG_NULL;
           
            eggIndexReader_get_document(hIndexReader,
                                        lp_score_doc[cnt-1].idDoc, &lp_eggDocument);
           
            HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "content");
            unsigned len = 0;
            char *val = eggField_get_value(lp_field, &len);
            printf("content: %s\n", val);
            lp_field = 0;
            eggDocument_delete(lp_eggDocument);
        }
    }
    eggTopCollector_delete(hTopCollector);
    eggQuery_delete(h1);
    eggIndexSearcher_delete(hIndexSearcher);
    eggIndexReader_close(hIndexReader);
    
    eggPath_close(hEggHandle);
    
    return 0;
}
