/* eggcluster.c */
#include <egg3/Egg3.h>
#include <stdio.h>

int main()
{
    char *dir_path="cluster://127.0.0.1:10000/bas";
    HEGGHANDLE hEggHandle;

    /* add */
    hEggHandle = eggPath_open(dir_path);    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  "weightfield");
    HEGGDOCUMENT hDocument;
    char *buf;
    int32_t weightfield;
    {    
      buf = "good bad ugly";
      weightfield = 100;
      hDocument = eggDocument_new();    
      HEGGFIELD hField1 = eggField_new("content", buf, strlen(buf)+1, EGG_CWS_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
      HEGGFIELD hField2 = eggField_new("weightfield", (char*)&weightfield, sizeof(weightfield), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
      eggDocument_add(hDocument, hField1);
      eggDocument_add(hDocument, hField2);
      eggIndexWriter_add_document(hIndexWrite, hDocument);
      eggDocument_delete(hDocument);
    }
    {    
      buf = "good movie star";
      weightfield = 10100;
      hDocument = eggDocument_new();    
      HEGGFIELD hField1 = eggField_new("content", buf, strlen(buf)+1, EGG_CWS_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
      HEGGFIELD hField2 = eggField_new("weightfield", (char*)&weightfield, sizeof(weightfield), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
      eggDocument_add(hDocument, hField1);
      eggDocument_add(hDocument, hField2);
      eggIndexWriter_add_document(hIndexWrite, hDocument);
      eggDocument_delete(hDocument);
    }

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
	printf("total hits: %u\n", (unsigned)cnt);
	int i;
	for (i = 0; i < cnt; i++)
        {        
            printf("document: id[%llu]\n", (long long unsigned)EGGDID_DOCID(&lp_score_doc[i].idDoc));
            HEGGDOCUMENT lp_eggDocument = EGG_NULL;
           
            eggIndexReader_get_document(hIndexReader,
                                        lp_score_doc[i].idDoc, &lp_eggDocument);
           
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
