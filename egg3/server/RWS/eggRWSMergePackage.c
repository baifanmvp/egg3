#include "eggRWSMergePackage.h"
#include "eggRWSLog.h"
#include "eggRWSIntServer.h"

#define LOG_INFO(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_INFO, who, __VA_ARGS__)
#define LOG_WARN(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_WARN, who, __VA_ARGS__)
#define LOG_ERR(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_ERROR, who, __VA_ARGS__)
#define LOG_CLAIM(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_CLAIM, who, __VA_ARGS__)


HEGGNETPACKAGE eggRWSMergePackage_query(HEGGRWSFIFOQUEUE hPackQueue)
{
    count_t n_topcollect_cnt = eggRWSFifoQueue_count(hPackQueue);
    index_t n_topcollect_idx = 0;
    HEGGTOPCOLLECTOR* topCollectorSet = (HEGGTOPCOLLECTOR*)malloc(sizeof(HEGGTOPCOLLECTOR) * n_topcollect_cnt);
    memset(topCollectorSet, 0, sizeof(HEGGTOPCOLLECTOR) * n_topcollect_cnt);


    LOG_INFO("eggRWSMergePackage_query", "cnt_topcollector[%u]", n_topcollect_cnt);
    
    HEGGNETPACKAGE lp_iter_package = EGG_NULL;
    EBOOL queryret = EGG_FALSE;
    while(lp_iter_package = eggRWSFifoQueue_pop(hPackQueue))
    {
        /*
        if(n_topcollect_idx == n_topcollect_cnt)
        {
            printf("collect num is over n_topcollect_cnt\n [file: %s ] [line : %d ]", __FILE__, __LINE__);
            exit(-1);
        }
        */
        EBOOL *p_ret = EGG_NULL;
        size32_t retsz = 0;
        char *collectorbuf = NULL;
        int collectorsize = 0;

        eggNetPackage_fetch((HEGGNETPACKAGE)lp_iter_package, 4, &p_ret, &retsz,
                            &collectorbuf, &collectorsize);
    
        if(*p_ret == EGG_TRUE)
        {
            queryret = EGG_TRUE;
            topCollectorSet[n_topcollect_idx] = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)collectorbuf);


            LOG_INFO("eggRWSMergePackage_query", "idx_topcollector[%u]: hits[%u]",
                     n_topcollect_idx, eggTopCollector_total_hits(topCollectorSet[n_topcollect_idx]));
            n_topcollect_idx ++;
        }
        
        eggNetPackage_delete(lp_iter_package);
    }

    HEGGTOPCOLLECTOR lp_topcollect_res;
    if (n_topcollect_idx == 0)
    {
        lp_topcollect_res = eggTopCollector_new(0);
    }
    else
    {
        lp_topcollect_res = topCollectorSet[0];
        topCollectorSet[0] = NULL;
        
        eggTopCollector_merge_with_cluster(lp_topcollect_res, n_topcollect_idx-1, topCollectorSet+1);
        
        n_topcollect_cnt = n_topcollect_idx-1;
        n_topcollect_idx = 1;
        while (n_topcollect_idx < n_topcollect_cnt)
        {
            eggTopCollector_delete(topCollectorSet[n_topcollect_idx]);
            n_topcollect_idx++;
        }
    }
    free(topCollectorSet);

    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    queryret = EGG_TRUE;        /* 忽略单个无结果的情况 */
    lp_res_package = eggNetPackage_add(lp_res_package, &queryret, sizeof(queryret), EGG_PACKAGE_RET);
    
    if (queryret == EGG_TRUE)
    {
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(lp_topcollect_res);
        lp_res_package = eggNetPackage_add(lp_res_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
        free(lp_topctor_chunk);
        eggTopCollector_delete(lp_topcollect_res);
    }

    return lp_res_package;

}


HEGGNETPACKAGE eggRWSMergePackage_query_iter(HEGGRWSFIFOQUEUE hPackQueue)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    EBOOL ret = EGG_FALSE;

    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
}


HEGGNETPACKAGE eggRWSMergePackage_get_docTotalCnt(HEGGRWSFIFOQUEUE hPackages)
{
    count_t count_total = 0;
    
    HEGGNETPACKAGE lp_iter_package = EGG_NULL;
    EBOOL queryret = EGG_FALSE;
    while(lp_iter_package = eggRWSFifoQueue_pop(hPackages))
    {

	EBOOL* p_ret = 0;
	size32_t n_ret;    
	count_t* p_cnt = NULL;
	count_t cnt = 0;
	size32_t n_cnt;
    
	eggNetPackage_fetch(lp_iter_package, 4, &p_ret, &n_ret, &p_cnt, &n_cnt);
	if (*p_ret == EGG_TRUE)
	{
	    cnt = *p_cnt;
	}
	else
	{
	    cnt = 0;
	}
	eggNetPackage_delete(lp_iter_package);

	count_total += cnt;
    }
    
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETDOCTOTALCNT);
    EBOOL ret;
    ret = EGG_TRUE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    lp_res_package = eggNetPackage_add(lp_res_package, &count_total, sizeof(count_total), EGG_PACKAGE_COUNT);

    return lp_res_package;
    
}
