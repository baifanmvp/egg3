
#include "../eggHandle.h"
#include "../eggCluster.h"
#include "../eggHttp.h"
#include "../net/eggSpanUnit.h"
#include "../net/eggNetSocket.h"
#include "./eggIndexWriterCluster.h"
#include "./eggIndexReaderCluster.h"
#include "./eggIndexSearcherCluster.h"
#include "../net/eggClientInfoServ.h"

struct eggCluster
{
    EGGHANDLE eggHandle;
    HEGGINFOSERV hInfoServ;
    HEGGMASTERHAND hMasterHand;
    
    HEGGCHUNKHAND hChunkHands;
    count_t chunkCnt;
    
    char eggPath[128];
    
};
extern pthread_mutex_t counter_mutex;

PRIVATE EBOOL eggCluster_connect_master(HEGGCLUSTER hEggCluster, char* ip_master, short port_master);

PRIVATE EBOOL eggCluster_disConnect_master(HEGGCLUSTER hEggCluster);

PRIVATE EBOOL eggCluster_init_chunkHands(HEGGCLUSTER hEggCluster);

PRIVATE EBOOL eggCluster_destroy_chunkHands(HEGGCLUSTER hEggCluster);

HEGGCLUSTER EGGAPI eggCluster_open(const path_t* pEggPath)
{
    if(POINTER_IS_INVALID(pEggPath))
    {
        return EGG_NULL;
    }
    
    HEGGCLUSTER lp_egg_cluster = (HEGGCLUSTER)malloc(sizeof(EGGCLUSTER));
    memset(lp_egg_cluster, 0, sizeof(EGGCLUSTER));
    char ip_master[16];
    short port_master = 0;
    
    strcpy(lp_egg_cluster->eggPath, pEggPath);
    
    sscanf(pEggPath, "%[^:]:%hd", ip_master, &port_master);

    
    if(!eggCluster_connect_master(lp_egg_cluster, ip_master, port_master))
    {
        free(lp_egg_cluster);
        return EGG_NULL;
    }
    
    if(!eggCluster_init_chunkHands(lp_egg_cluster))
    {
        free(lp_egg_cluster);
        return EGG_NULL;
    }

    lp_egg_cluster->eggHandle.eggHandle_dup = eggCluster_dup;
    lp_egg_cluster->eggHandle.eggHandle_delete = eggCluster_close;
    
    lp_egg_cluster->eggHandle.eggIndexWriter_open = eggIndexWriter_open_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_close = eggIndexWriter_close_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_add_document = eggIndexWriter_add_document_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_optimize = eggIndexWriter_optimize_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_init_reader = eggIndexWriter_init_reader_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_delete_document = eggIndexWriter_delete_document_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_modify_document = eggIndexWriter_modify_document_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_incrementmodify_document = eggIndexWriter_incrementmodify_document_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_add_field = eggIndexWriter_add_field_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_modify_field = eggIndexWriter_modify_field_cluster;
    lp_egg_cluster->eggHandle.eggIndexWriter_delete_field = eggIndexWriter_delete_field_cluster;
    
    lp_egg_cluster->eggHandle.eggIndexReader_open = eggIndexReader_open_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_close = eggIndexReader_close_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_get_document = eggIndexReader_get_document_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_get_documentset = eggIndexReader_get_documentset_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_export_document = eggIndexReader_export_document_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_get_doctotalcnt = eggIndexReader_get_doctotalcnt_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_get_fieldnameinfo = eggIndexReader_get_fieldnameinfo_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_get_singlefieldnameinfo = eggIndexReader_get_singlefieldnameinfo_cluster;
    lp_egg_cluster->eggHandle.eggIndexReader_free = eggIndexReader_free_cluster;
    
    lp_egg_cluster->eggHandle.eggIndexSearcher_new = eggIndexSearcher_new_cluster;
    lp_egg_cluster->eggHandle.eggIndexSearcher_delete = eggIndexSearcher_delete_cluster;
    lp_egg_cluster->eggHandle.eggIndexSearcher_get_queryiter = eggIndexSearcher_get_queryiter_cluster;
    lp_egg_cluster->eggHandle.eggIndexSearcher_search_with_query = eggIndexSearcher_search_with_query_cluster;
    
    lp_egg_cluster->eggHandle.eggIndexSearcher_count_with_query = eggIndexSearcher_count_with_query_cluster;
    
    lp_egg_cluster->eggHandle.eggIndexSearcher_search_with_queryiter = eggIndexSearcher_search_with_queryiter_cluster;
    lp_egg_cluster->eggHandle.eggIndexSearcher_filter = eggIndexSearcher_filter_cluster;

    lp_egg_cluster->eggHandle.eggHandle_close = eggCluster_close;
    return lp_egg_cluster;
}

EBOOL EGGAPI eggCluster_close(HEGGCLUSTER hEggCluster)
{
    
    if(POINTER_IS_INVALID(hEggCluster))
    {
        return EGG_FALSE;
    }
    
    eggCluster_disConnect_master(hEggCluster);
    
    eggCluster_destroy_chunkHands(hEggCluster);

    free(hEggCluster);
    
    return EGG_TRUE;
}


PUBLIC HEGGCLUSTER EGGAPI eggCluster_dup(HEGGCLUSTER hClusterOrg)
{
    if(POINTER_IS_INVALID(hClusterOrg))
    {
        return EGG_NULL;
    }
    
    
    return eggCluster_open(hClusterOrg->eggPath);
}


count_t EGGAPI eggCluster_get_chunkcnt(HEGGCLUSTER hEggCluster)
{
    if(POINTER_IS_INVALID(hEggCluster))
    {
        return EGG_NULL;
    }
    return hEggCluster->chunkCnt;
}

HEGGCHUNKHAND EGGAPI eggCluster_get_chunkhands(HEGGCLUSTER hEggCluster, count_t* lpCnt)
{
    if(POINTER_IS_INVALID(hEggCluster))
    {
        return EGG_NULL;
    }
    *lpCnt = hEggCluster->chunkCnt;
    return hEggCluster->hChunkHands;
}

PRIVATE EBOOL eggCluster_connect_master(HEGGCLUSTER hEggCluster, char* ip_master, short port_master)
{
    hEggCluster->hInfoServ = eggInfoServ_open(ip_master, port_master);
    
    return EGG_TRUE;
}

PRIVATE EBOOL eggCluster_disConnect_master(HEGGCLUSTER hEggCluster)
{
    eggInfoServ_close(hEggCluster->hInfoServ);
    return EGG_TRUE;
}

PRIVATE EBOOL eggCluster_init_chunkHands(HEGGCLUSTER hEggCluster)
{
    if(POINTER_IS_INVALID(hEggCluster))
    {
        return EGG_FALSE;
    }
    EGGSPANUNIT st_spanunit = {0};
    st_spanunit.eggDirPath = (char*)malloc(128);
    
    sscanf(hEggCluster->eggPath, "%*[^:]:%*hd/%s", st_spanunit.eggDirPath);

    count_t n_spanunit_cnt = 0;
    HEGGSPANUNIT lp_span_unit = eggInfoServ_inquire(hEggCluster->hInfoServ,
                                                    &st_spanunit,
                                                    &n_spanunit_cnt);
    if(POINTER_IS_INVALID(lp_span_unit))
    {
        free(st_spanunit.eggDirPath);
        return EGG_FALSE;
    }


    hEggCluster->hChunkHands = (HEGGCHUNKHAND)malloc(sizeof(EGGCHUNKHAND) * n_spanunit_cnt);
    hEggCluster->chunkCnt = n_spanunit_cnt;
    index_t i = 0;
    char lp_http_path[128];
    char host[30];
    char egg_dir_path[128];
    
    while(i < hEggCluster->chunkCnt)
    {
            
        hEggCluster->hChunkHands[hEggCluster->chunkCnt - i - 1].hChunkObj = eggPath_open(lp_span_unit[i].hostAddress);
            
        if(!hEggCluster->hChunkHands[hEggCluster->chunkCnt - i - 1].hChunkObj)
            return EGG_FALSE;

        hEggCluster->hChunkHands[hEggCluster->chunkCnt - i - 1].start = lp_span_unit[i].range.start;
        hEggCluster->hChunkHands[hEggCluster->chunkCnt - i - 1].end = lp_span_unit[i].range.end;
        i++;
    }
    eggSpanUnit_free(lp_span_unit, n_spanunit_cnt);
    free(st_spanunit.eggDirPath);
    return EGG_TRUE;

    /* ----------------------------- */
}

PRIVATE EBOOL eggCluster_destroy_chunkHands(HEGGCLUSTER hEggCluster)
{
    index_t i = 0;
    while(i < hEggCluster->chunkCnt)
    {
        
        eggPath_close(hEggCluster->hChunkHands[i].hChunkObj);
        i++;
    }
    
    free(hEggCluster->hChunkHands);
    return  EGG_TRUE;
}
