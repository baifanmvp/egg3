#include <egg3/Egg3.h>
int main()
{
    HEGGHANDLE hHandle = eggPath_open("file:///tmp/");
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hHandle);
      
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    
    HEGGQUERY hq = eggQuery_new_string("content", "is good", strlen("is good"), ANALYZER_CWSLEX);
    if(hq != EGG_NULL)
    {
        printf("query init OK! \n");
    }
    //填0取所有结果,非0按填的值取个数
    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    
    eggTopCollector_set_sorttype(hTopCollector, EGG_TOPSORT_WEIGHT);
    printf("eggTopCollector sortType is EGG_TOPSORT_WEIGHT \n");
    
    EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, hq);
    if (ret == EGG_TRUE)
    {
        //对最后结果进行排序
        //EGG_TOPSORT_WEIGHT:  按document的weight排序
        //EGG_TOPSORT_SCORE： 按查询关键字的相关度排序（打分排序）
        //EGG_TOPSORT_NOT：  不排序

 
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
            printf("id : [%llu], content : [%s], weight : [%d]\n", EGGDID_DOCID(&lp_score_doc[i].idDoc), val, eggDocument_get_weight(lp_eggDocument));
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
