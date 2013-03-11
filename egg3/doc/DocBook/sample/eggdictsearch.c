#include <egg3/Egg3.h>
int main(int argc, char* argv[])
{
    HEGGHANDLE hHandle = eggPath_open("file:///egg/");
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hHandle);
      
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    
    HEGGQUERY hq = eggQuery_new_string("content", argv[1], strlen(argv[1]), "");
    if(hq != EGG_NULL)
    {
        printf("query init OK! \n");
    }
    
    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    
    
    EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, hq);
    if (ret == EGG_TRUE)
    {

 
        HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
        count_t cnt =  eggTopCollector_total_hits(hTopCollector);
        index_t i = 0;
        printf("have hit %u documents\n", cnt);

        while (i != cnt)
        {
            HEGGDOCUMENT lp_eggDocument = EGG_NULL;
           
            eggIndexReader_get_document(hIndexReader,
                                        lp_score_doc[i].idDoc, &lp_eggDocument);
           
            HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "content");
            unsigned len = 0;
            char *val = eggField_get_value(lp_field, &len);
            printf("id : [%llu], content : [%s], \n", EGGDID_DOCID(&lp_score_doc[i].idDoc), val);
            lp_field = 0;
            eggDocument_delete(lp_eggDocument);
            
            i++;
        }
    }
    eggTopCollector_delete(hTopCollector);
    eggQuery_delete(hq);
    eggIndexSearcher_delete(hIndexSearcher);
    eggIndexReader_close(hIndexReader);
    eggPath_close(hHandle);

    return 0;
}
