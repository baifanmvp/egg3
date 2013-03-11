#include <egg3/Egg3.h>
int main()
{
    HEGGHANDLE hHandle = eggPath_open("file:///tmp/");
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hHandle);
      
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    
    HEGGQUERY hq1 = eggQuery_new_string("content", "is good", strlen("is good"), ANALYZER_CWSLEX);
    
    int time = 20120102;
    HEGGQUERY hq2 = eggQuery_new_int32("time", time);

    hq1 = eggQuery_and(hq1, hq2);
    
    if(hq1 != EGG_NULL)
    {
        printf("query init OK! \n");
    }
    //填0取所有结果,非0按填的值取个数
    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    
    eggTopCollector_set_sorttype(hTopCollector, EGG_TOPSORT_WEIGHT);
    printf("eggTopCollector sortType is EGG_TOPSORT_WEIGHT \n");
    
    EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, hq1);
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
           
            HEGGFIELD lp_field1 = eggDocument_get_field(lp_eggDocument, "time");
            unsigned len1 = 0;
            char *val1 = eggField_get_value(lp_field1, &len1);

            HEGGFIELD lp_field2 = eggDocument_get_field(lp_eggDocument, "content");
            unsigned len2 = 0;
            char *val2 = eggField_get_value(lp_field2, &len2);

            
            printf("id : [%llu], time : [%d] content : [%s], weight : [%d]\n", EGGDID_DOCID(&lp_score_doc[i].idDoc), *((int*)val1), val2, eggDocument_get_weight(lp_eggDocument));
            eggDocument_delete(lp_eggDocument);
            
            i++;
        }
    }
    eggTopCollector_delete(hTopCollector);
    eggQuery_delete(hq1);
    eggIndexSearcher_delete(hIndexSearcher);
    eggIndexReader_close(hIndexReader);
    eggPath_close(hHandle);

    return 0;
}
