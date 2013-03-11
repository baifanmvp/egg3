#include <egg3/Egg3.h>

struct personinfo
{
    char sex;
    int num;
    int year;
};

struct personinfo persons[] = {{'M', 4, 1920}, {'M', 3, 1898},  {'M', 1, 1900},  {'M', 2, 1923},  {'M', 7, 1910},  {'M', 5, 1923},  {'M', 5, 1924},  {'M', 9, 1911},  {'M', 4, 1910},  {'M', 2, 1913},  {'M', 5, 1911},  {'M', 4, 1921},  {'M', 6, 1920},  {'W', 1, 1921},  {'W', 2, 1928},  {'W', 3, 1918},  {'W', 7, 1919},  {'M', 5, 1901},  {'W', 4, 1930},  {'W', 4, 1909}};
int add()
{
     HEGGHANDLE hHandle = eggPath_open("file:///tmp/");
     
     HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hHandle, "");
     
     int cnt = 20;
     index_t i = 0;
     while(i != cnt)
     {
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new("sex", (char*)&persons[i].sex, 1,
                                          EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         
         HEGGFIELD hField2 = eggField_new("num", (void*)&persons[i].num, sizeof(int),
                                          EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);

         HEGGFIELD hField3 = eggField_new("year", (void*)&persons[i].year, sizeof(int),
                                          EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
         
         eggDocument_add(hDocument, hField1);
         eggDocument_add(hDocument, hField2);
         eggDocument_add(hDocument, hField3);
         
         eggIndexWriter_add_document(hIndexWriter, hDocument);
                  
         eggDocument_delete(hDocument);
         i++;
     }
     
     if(eggIndexWriter_optimize(hIndexWriter))
     {
         printf("optimize success! \n");
     }
     eggIndexWriter_close(hIndexWriter);
     eggPath_close(hHandle);

     return 0;
}

int main()
{

    add();
    
    HEGGHANDLE hHandle = eggPath_open("/tmp/");
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hHandle);
      
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    
    HEGGQUERY hq1 = eggQuery_new_string("sex", "M", strlen("M"), "");
    
    HEGGQUERY hq2 = eggQuery_new_int32range("num", 4, 7);
    
    hq1 = eggQuery_and(hq1, hq2);

    if(hq1 != EGG_NULL)
    {
        printf("query init OK! \n");
    }
    //填0取所有结果,非0按填的值取个数
    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    
    eggTopCollector_set_orderby(hTopCollector, 2, "year", 1, "num", 0);
    
    printf("eggTopCollector_set_orderBy is year[asc] num[dsc]\n");
    
    EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, hq1);
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
           
            HEGGFIELD lp_field1 = eggDocument_get_field(lp_eggDocument, "num");
            unsigned len1 = 0;
            char *val1 = eggField_get_value(lp_field1, &len1);

            HEGGFIELD lp_field2 = eggDocument_get_field(lp_eggDocument, "sex");
            unsigned len2 = 0;
            char *val2 = eggField_get_value(lp_field2, &len2);

            HEGGFIELD lp_field3 = eggDocument_get_field(lp_eggDocument, "year");
            unsigned len3 = 0;
            char *val3 = eggField_get_value(lp_field3, &len3);

            
            printf("id : [%llu], num : [%d] sex : [%s] year : [%d]\n",
		   EGGDID_DOCID(&lp_score_doc[i].idDoc), *((int*)val1), val2, *((int*)val3) );
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
