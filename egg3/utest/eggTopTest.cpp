#include "egg2TopTest.h"

 #include <time.h>
#include <sys/time.h>

Cegg2TopTest::Cegg2TopTest(void)
{
    
}

Cegg2TopTest::~Cegg2TopTest(void)
{
    
}

void Cegg2TopTest::setUp(void)
{
    
}
void Cegg2TopTest::tearDown(void)
{
    
}

void Cegg2TopTest::testMerge(void)
{
    count_t count = 1000000;
    
    TOPCOLLECTOR* lp_top_dest = (TOPCOLLECTOR*)egg_TopCollector_new(0, EGG_TRUE);
    TOPCOLLECTOR* lp_top_src = (TOPCOLLECTOR*)egg_TopCollector_new(0, EGG_TRUE);
    
    SCOREDOC* lp_score_dest = (SCOREDOC*)malloc(sizeof(SCOREDOC) * count);
    SCOREDOC* lp_score_src = (SCOREDOC*)malloc(sizeof(SCOREDOC) * count);
    
    int idx = 0;
    while(idx != count)
    {
        lp_score_src->idDoc = idx;
        lp_score_dest->idDoc = idx;
        idx++;
    }

    lp_top_dest->docs = lp_score_dest;
    lp_top_src->docs = lp_score_src;
    
    lp_top_src->cntHits = count;
    lp_top_dest->cntHits = count;
    
    struct timeval startTv = {0};
    struct timeval endTv = {0};
    
    gettimeofday(&startTv, EGG_NULL);
    egg_TopCollector_merge(lp_top_dest, lp_top_src, 0);
    gettimeofday(&endTv, EGG_NULL);
    
    printf("lp_top_dest->->cntHits : %d\n", lp_top_dest->cntHits);
    printf("time : %f\n", ((double)(endTv.tv_sec) - (double)(startTv.tv_sec)) +  ((double)(endTv.tv_usec) - (double)(startTv.tv_usec))/1000000 );
    return;
}
