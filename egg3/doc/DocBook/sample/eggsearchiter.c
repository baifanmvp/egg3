#include <egg3/Egg3.h>
int main()
{
    HEGGHANDLE hHandle = eggPath_open("file:///tmp/");
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hHandle);
      
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    
    HEGGQUERY h1 = eggQuery_new_string("content", "hello", strlen("hello"), "");


    HEGGSEARCHITER lp_iter = eggIndexSearcher_get_queryiter(hIndexSearcher);
    
    count_t pagenum = 0;
    printf("set pagenum : ");
    scanf("%d", &pagenum);
    getchar();
    
    eggSearchIter_reset(lp_iter, pagenum);
    EBOOL ret = 0;
    while(!EGGITER_OVERFIRST(lp_iter) && !EGGITER_OVERLAST(lp_iter))
    {
        HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
       ret = eggIndexSearcher_search_with_queryiter(hIndexSearcher, hTopCollector, h1, lp_iter);
       
       if(ret ==EGG_FALSE)
       {
           printf("no key !\n");
           exit(1);
       }
       
       
       HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
       count_t cnt =  eggTopCollector_total_hits(hTopCollector);
       index_t idx = 0;
       printf("count : %d\n", cnt);
       index_t i = 0;

        while (i != cnt)
        {
            
            printf("id : [%llu] \n", EGGDID_DOCID(&lp_score_doc[i].idDoc) );
            
            i++;
        }

       
       eggTopCollector_delete(hTopCollector);
        
       char c;
       printf("is jump result ? (y/n) ");
       scanf("%c", &c);
       if(c == 'y')
       {
           int jumpcnt = 0;
           printf("jump cnt : ");
           scanf("%d", &jumpcnt);
           getchar();
           eggSearchIter_iter(lp_iter, jumpcnt);
       }
   }
   
   eggSearchIter_delete(lp_iter);
   eggQuery_delete(h1);
   eggIndexSearcher_delete(hIndexSearcher);
   eggIndexReader_close(hIndexReader);
   eggPath_close(hHandle);

   return 0;
}
