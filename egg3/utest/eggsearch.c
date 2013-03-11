#include "egg3/Egg3.h"



#include <stdio.h>
#include <time.h>
#include <sys/time.h>




int main(int argc, char* argv[])
{
    char* dir_path = argv[1];
    char* key = argv[3];
    
    HEGGDIRECTORY hDirectory = eggDirectory_open(dir_path);
        
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hDirectory);
    
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    char *fieldName = argv[2];
    HEGGQUERY h1;
    
    h1 = eggQuery_new_string(argv[2], key, strlen(key), "");
    
        
    
        HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
        struct timeval tv_start, tv_end;
        gettimeofday(&tv_start, EGG_NULL);
        EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);
        if(ret ==EGG_FALSE)
        {
        printf("no key !\n");
        exit(1);
        }
        gettimeofday(&tv_end, EGG_NULL);
        printf("time : %f\n", (tv_end.tv_sec - tv_start.tv_sec) + (double)(tv_end.tv_usec - tv_start.tv_usec)/1000000 );
        
        eggTopCollector_normalized(hTopCollector, EGG_TOPSORT_NOT);
        
        HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
        count_t cnt =  eggTopCollector_total_hits(hTopCollector);
        index_t idx = 0;
        printf("count : %d\n", cnt);
        eggTopCollector_delete(hTopCollector);
    return 0;
}






