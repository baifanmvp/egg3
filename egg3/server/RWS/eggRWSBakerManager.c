#include "eggRWSBakerManager.h"
#include "eggRWSLog.h"
#include "eggRWSIntServer.h"
#include <pthread.h>
#define LOG_INFO(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_INFO, who, __VA_ARGS__)
#define LOG_WARN(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_WARN, who, __VA_ARGS__)
#define LOG_ERR(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_ERROR, who, __VA_ARGS__)
#define LOG_CLAIM(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_CLAIM, who, __VA_ARGS__)


PRIVATE void* eggRWSBakerManager_working(HEGGRWSBAKERMANAGER hRWSBakerManager);

PRIVATE void* eggRWSBakerManager_statistics(HEGGRWSBAKERMANAGER hRWSBakerManager);

PRIVATE EBOOL eggRWSBakerManager_print_bakerReqCnt(HEGGRWSBAKERMANAGER hRWSBakerManager);


HEGGRWSREQINFO eggRWSReqInfo_new(pthread_t thrId, flag_t flag)
{
    HEGGRWSREQINFO lp_req_info = (HEGGRWSREQINFO)malloc(sizeof(EGGRWSREQINFO));
    lp_req_info->flag = flag;
    lp_req_info->thrId = thrId;
    lp_req_info->timestamp = time(0);
    pthread_cond_init(&lp_req_info->cond, EGG_NULL);
    lp_req_info->baker = EGG_NULL;
    lp_req_info->next = EGG_NULL;

    return lp_req_info;
}

EBOOL eggRWSReqInfo_delete(HEGGRWSREQINFO hReqInfo)
{
    if(POINTER_IS_INVALID(hReqInfo))
    {
        return EGG_FALSE;
    }
    
    pthread_cond_destroy(&hReqInfo->cond);
    
    
    free(hReqInfo);
    return EGG_TRUE;
}


HEGGRWSBAKERINFO eggRWSBakerInfo_new(char* path, long timestamp)
{
    if(POINTER_IS_INVALID(path))
    {
        return EGG_NULL;
    }
    HEGGRWSBAKERINFO lp_info = (HEGGRWSBAKERINFO)malloc(sizeof(EGGRWSBAKERINFO));
    lp_info->status = 0;
    lp_info->thrId = 0;
    lp_info->timestamp = timestamp;
    EGG_BAKER_SET_IDLE(lp_info->status);
    EGG_BAKER_SET_READING(lp_info->status);
    lp_info->path = strdup(path);
    pthread_mutex_init( &lp_info->mutex, NULL);
    lp_info->reqCnt = 0;

    lp_info->baker = (HEGGRWSBAKER)malloc(sizeof(EGGRWSBAKER));
    lp_info->timestamp = timestamp;
    lp_info->baker->next = EGG_NULL;
    
    HEGGHANDLE hEggHandle = eggPath_open(path);
    
    lp_info->baker->hWriter = eggIndexWriter_open(hEggHandle, "weightfield");
    lp_info->baker->hReader = eggIndexReader_open(hEggHandle);
    lp_info->baker->hSearcher = eggIndexSearcher_new(lp_info->baker->hReader);
    
    eggPath_close(hEggHandle);

    return lp_info;
}

EBOOL eggRWSBakerInfo_delete(HEGGRWSBAKERINFO hBakerInfo)
{
    if(POINTER_IS_INVALID(hBakerInfo))
    {
        return EGG_FALSE;
    }
    free(hBakerInfo->path);
    pthread_mutex_destroy(&hBakerInfo->mutex);
//    pthread_cond_destroy( &hBakerInfo->cond);
    
    eggIndexSearcher_delete(hBakerInfo->baker->hSearcher);
    eggIndexReader_close(hBakerInfo->baker->hReader);
    eggIndexWriter_close(hBakerInfo->baker->hWriter);
    free(hBakerInfo->baker);
    free(hBakerInfo);
    
    return EGG_TRUE;
}

HEGGRWSBAKERTABLE eggRWSBakerTable_new(count_t nBakerCnt)
{
    HEGGRWSBAKERTABLE lp_baker_table = (HEGGRWSBAKERTABLE)malloc(sizeof(EGGRWSBAKERTABLE));
    lp_baker_table->aCnt = nBakerCnt;
    lp_baker_table->validGroupIdx = 0;

    memset(lp_baker_table->bakGroup, 0, sizeof(lp_baker_table->bakGroup));
    
    pthread_mutex_init( &lp_baker_table->mutex, NULL);
    pthread_cond_init( &lp_baker_table->cond, NULL);
    
    lp_baker_table->infos = (struct eggRWSBakerInfo**)malloc(sizeof(struct eggRWSBakerInfo*) * nBakerCnt);
    memset(lp_baker_table->infos, 0, sizeof(struct eggRWSBakerInfo*) * nBakerCnt);
    return lp_baker_table;
}

EBOOL eggRWSBakerTable_delete(HEGGRWSBAKERTABLE hRWSBakerTable)
{
    if(POINTER_IS_INVALID(hRWSBakerTable))
    {
        return EGG_FALSE;
    }
    
    while(hRWSBakerTable->aCnt--)
    {
        eggRWSBakerInfo_delete(hRWSBakerTable->infos[hRWSBakerTable->aCnt]);
    }
    
    pthread_mutex_destroy( &hRWSBakerTable->mutex);
    pthread_cond_destroy( &hRWSBakerTable->cond);
    free(hRWSBakerTable->infos);
    free(hRWSBakerTable);
    return EGG_TRUE;
}

EBOOL eggRWSBakerTable_loadBakerInfo(HEGGRWSBAKERTABLE hRWSBakerTable, HEGGRWSBAKERINFO hRWSBakerInfo)
{
    if(POINTER_IS_INVALID(hRWSBakerTable))
    {
        return EGG_FALSE;
    }
    
    if(POINTER_IS_INVALID(hRWSBakerInfo))
    {
        return EGG_FALSE;
    }
    
    HEGGRWSBAKERGROUP lp_valid_group = hRWSBakerTable->bakGroup + hRWSBakerTable->validGroupIdx;
    
    if(hRWSBakerTable->aCnt == lp_valid_group->vCnt)
    {
        return EGG_FALSE;
    }
    
    hRWSBakerInfo->baker->bakIdx = lp_valid_group->vCnt;
    hRWSBakerTable->infos[lp_valid_group->vCnt++] = hRWSBakerInfo;

    EGG_BAKER_SET_READING(hRWSBakerInfo->status);
    
    if(lp_valid_group->vCnt == 1)
    {
        lp_valid_group->head = hRWSBakerInfo->baker;
        lp_valid_group->tail = hRWSBakerInfo->baker;
    }
    else
    {
        lp_valid_group->tail->next = hRWSBakerInfo->baker;
        lp_valid_group->tail = hRWSBakerInfo->baker;
    }
    
    return EGG_TRUE;
}


HEGGRWSBAKERMANAGER eggRWSBakerManager_new(HEGGRWSBAKERCFG hRWSConfig, long timestamp)
{
    if(POINTER_IS_INVALID(hRWSConfig))
    {
        return EGG_NULL;
    }

    
    count_t n_baker_cnt = 0;
    char** lplp_baker_names = eggRWSBakerCfg_get_bakerNames(hRWSConfig, &n_baker_cnt);
    
    if(POINTER_IS_INVALID(lplp_baker_names))
    {
        return EGG_NULL;
    }

    
    HEGGRWSBAKERMANAGER lp_baker_manage = (HEGGRWSBAKERMANAGER)malloc(sizeof(EGGRWSBAKERMANAGER));
    lp_baker_manage->hConfig = hRWSConfig;
    lp_baker_manage->maxtime = timestamp;
    lp_baker_manage->counterStat = hRWSConfig->counterStatus;
    /*
      baktab
     */
    index_t n_baker_idx = 0;
    lp_baker_manage->hBakTab = eggRWSBakerTable_new(n_baker_cnt);
    while(n_baker_idx != n_baker_cnt)
    {
        eggRWSBakerTable_loadBakerInfo(lp_baker_manage->hBakTab,eggRWSBakerInfo_new(lplp_baker_names[n_baker_idx], timestamp));
        n_baker_idx++;
    }

    /*
      ReqQueue      
     */
    lp_baker_manage->hReqQueue = (HEGGRWSREQQUENE) malloc(sizeof(EGGRWSREQQUENE));
    memset(lp_baker_manage->hReqQueue, 0, sizeof(EGGRWSREQQUENE));
    pthread_mutex_init( &lp_baker_manage->hReqQueue->mutex, NULL);
    eggRWSBakerManager_create_worker(lp_baker_manage);
    
    if(lp_baker_manage->counterStat == EGG_TRUE)
        eggRWSBakerManager_create_counter(lp_baker_manage);
    
    return  lp_baker_manage;

}
 
EBOOL eggRWSBakerManager_delete(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    if(!POINTER_IS_INVALID(hRWSBakerManager->hConfig))
    {
        hRWSBakerManager->hConfig = NULL;
        //eggRWSBakerCfg_delete(hRWSBakerManager->hConfig);  
    }

    if(!POINTER_IS_INVALID(hRWSBakerManager->hReqQueue))
    {
        if(hRWSBakerManager->hReqQueue->head || hRWSBakerManager->hReqQueue->tail || hRWSBakerManager->hReqQueue->count)
        {
            return EGG_FALSE;
        }
        pthread_mutex_destroy( &hRWSBakerManager->hReqQueue->mutex);
        
    }

    if(!POINTER_IS_INVALID(hRWSBakerManager->hBakTab))
    {
        eggRWSBakerTable_delete(hRWSBakerManager->hBakTab);
    }

    return EGG_TRUE;
}

HEGGRWSBAKER eggRWSBakerManager_alloc_baker(HEGGRWSBAKERMANAGER hRWSBakerManager, pthread_t thrId)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_NULL;
    }
    
    pthread_mutex_lock(&hRWSBakerManager->hBakTab->mutex);
    
    HEGGRWSBAKERGROUP lp_valid_group = hRWSBakerManager->hBakTab->bakGroup + hRWSBakerManager->hBakTab->validGroupIdx;
//    printf(" lp_valid_group->vCnt %d\n", lp_valid_group->vCnt);
    
    while(!lp_valid_group->vCnt)
    {
        pthread_cond_wait(&hRWSBakerManager->hBakTab->cond, &hRWSBakerManager->hBakTab->mutex);
        lp_valid_group = hRWSBakerManager->hBakTab->bakGroup + hRWSBakerManager->hBakTab->validGroupIdx;
    }

    // hRWSBakerManager->hBakTab->validGroup  may change
    
    HEGGRWSBAKER lp_alloc_baker =  lp_valid_group->head;
    lp_valid_group->head = lp_valid_group->head->next;
    lp_valid_group->vCnt--;
    
    if(!lp_valid_group->vCnt)
    {
        lp_valid_group->tail = EGG_NULL;
    }
    EGG_BAKER_SET_BUSY(hRWSBakerManager->hBakTab->infos[lp_alloc_baker->bakIdx]->status);
    hRWSBakerManager->hBakTab->infos[lp_alloc_baker->bakIdx]->thrId = thrId;
    hRWSBakerManager->hBakTab->infos[lp_alloc_baker->bakIdx]->reqCnt++;

    pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);


    return lp_alloc_baker;
}

EBOOL eggRWSBakerManager_free_baker(HEGGRWSBAKERMANAGER hRWSBakerManager, HEGGRWSBAKER hBaker)
{

    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    if(POINTER_IS_INVALID(hBaker))
    {
        return EGG_FALSE;
    }
    hBaker->next = EGG_NULL;
    
    pthread_mutex_lock(&hRWSBakerManager->hBakTab->mutex);

    //
    HEGGRWSBAKERINFO lp_baker_info = hRWSBakerManager->hBakTab->infos[hBaker->bakIdx];
    lp_baker_info->thrId=0;
    index_t n_free_idx = EGG_BAKER_IS_READING(lp_baker_info->status)? hRWSBakerManager->hBakTab->validGroupIdx : (hRWSBakerManager->hBakTab->validGroupIdx + 1)%2;
    
    HEGGRWSBAKERGROUP lp_free_group = hRWSBakerManager->hBakTab->bakGroup + n_free_idx;
    
    lp_free_group->vCnt++;
    EGG_BAKER_SET_IDLE(lp_baker_info->status);

    
    if(lp_free_group->vCnt == 1)
    {
        lp_free_group->head = hBaker;
        lp_free_group->tail = hBaker;
        
        if(EGG_BAKER_IS_READING(lp_baker_info->status))
            pthread_cond_signal(&hRWSBakerManager->hBakTab->cond);
    }
    else
    {
        lp_free_group->tail->next = hBaker;
        lp_free_group->tail = hBaker;
    }
//    printf("********free (%llu)\n", pthread_self());
    
    pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);
    
    
    return EGG_TRUE;
}

EBOOL eggRWSBakerManager_moveBakersByIds(HEGGRWSBAKERMANAGER hRWSBakerManager, eggBakerId_t* ids, count_t nIdCnt)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    pthread_mutex_lock(&hRWSBakerManager->hBakTab->mutex);
    
    HEGGRWSBAKERGROUP lp_backup_group = hRWSBakerManager->hBakTab->bakGroup + ((hRWSBakerManager->hBakTab->validGroupIdx + 1)%2) ;
    count_t n_orgbak_cnt = lp_backup_group->vCnt;
    
    HEGGRWSBAKERINFO* lplp_infos = hRWSBakerManager->hBakTab->infos;
    
    index_t n_ids_idx = 0;
    while(n_ids_idx != nIdCnt)
    {
        if(lplp_infos[ids[n_ids_idx]]->baker->bakIdx != ids[n_ids_idx])
        {
            pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);
            printf("baker->bakIdx != ids\n");
            return EGG_FALSE;
        }

        if(EGG_BAKER_IS_IDLE(lplp_infos[ids[n_ids_idx]]->status))
        {
            HEGGRWSBAKERGROUP lp_valid_group = hRWSBakerManager->hBakTab->bakGroup + hRWSBakerManager->hBakTab->validGroupIdx;

            HEGGRWSBAKER lp_baker_iter = lp_valid_group->head;
            HEGGRWSBAKER lp_baker_last = EGG_NULL;
            while(1)
            {
                if(lp_baker_iter->bakIdx == ids[n_ids_idx])
                {
                    if(lp_baker_iter == lp_valid_group->head)
                    {
                        lp_valid_group->head = lp_baker_iter->next;
                    }
                    else
                    {
                        lp_baker_last->next = lp_baker_iter->next;
                    }

                    if(lp_baker_iter == lp_valid_group->tail)
                    {
                        lp_valid_group->tail = lp_baker_last;
                    }
                    lp_valid_group->vCnt --;
                    
                    ///////////
                    
                    lp_backup_group->vCnt++;
                    if(lp_backup_group->vCnt == 1)
                    {
                        lp_backup_group->head = lp_baker_iter;
                        lp_backup_group->tail = lp_baker_iter;
                    }
                    else
                    {
                        lp_backup_group->tail->next = lp_baker_iter;
                        lp_backup_group->tail = lp_baker_iter;
                    }

                    ///////////
                    break;
                }
                lp_baker_last = lp_baker_iter;
                lp_baker_iter = lp_baker_iter->next;
            }
        }
        
        EGG_BAKER_SET_WRITEING(lplp_infos[ids[n_ids_idx]]->status);

        n_ids_idx++;

        
    }
    
    pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);
    

    return EGG_TRUE;
}

EBOOL eggRWSBakerManager_waitWBakers(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_TRUE;
    }


    count_t n_baker_aCnt = hRWSBakerManager->hBakTab->aCnt;
    index_t n_baker_idx = 0;
    
    HEGGRWSBAKERINFO* lplp_infos = hRWSBakerManager->hBakTab->infos;
    
    while(n_baker_idx != n_baker_aCnt)
    {
        
        pthread_mutex_lock(&hRWSBakerManager->hBakTab->mutex);
        flag_t flag = EGG_BAKER_IS_WRITEING(lplp_infos[n_baker_idx]->status) && EGG_BAKER_IS_BUSY(lplp_infos[n_baker_idx]->status);
        pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);
        
        
        if(flag) 
        {
        
            sleep(5);
        }
        else
        {
            n_baker_idx++;
        }
        
        
    }

    return EGG_TRUE;
    
}

HEGGRWSBAKERGROUP eggRWSBakerManager_getWGroup(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_NULL;
    }

    return hRWSBakerManager->hBakTab->bakGroup + ((hRWSBakerManager->hBakTab->validGroupIdx + 1)%2);
    
}


HEGGRWSBAKERGROUP eggRWSBakerManager_getRGroup(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_NULL;
    }

    return hRWSBakerManager->hBakTab->bakGroup + hRWSBakerManager->hBakTab->validGroupIdx;
    
}

EBOOL eggRWSBakerManager_move_baker(HEGGRWSBAKERMANAGER hRWSBakerManager )

{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    pthread_mutex_lock(&hRWSBakerManager->hBakTab->mutex);
    
    HEGGRWSBAKERGROUP lp_valid_group = hRWSBakerManager->hBakTab->bakGroup + hRWSBakerManager->hBakTab->validGroupIdx;
    
    HEGGRWSBAKERGROUP lp_backup_group = hRWSBakerManager->hBakTab->bakGroup + ((hRWSBakerManager->hBakTab->validGroupIdx + 1)%2) ;
    

    if(!lp_valid_group->vCnt)
    {
        pthread_cond_wait(&hRWSBakerManager->hBakTab->cond, &hRWSBakerManager->hBakTab->mutex);
    }
    

    HEGGRWSBAKER lp_alloc_baker =  lp_valid_group->head;

    EGG_BAKER_SET_WRITEING(hRWSBakerManager->hBakTab->infos[lp_alloc_baker->bakIdx]->status);


    lp_valid_group->head = lp_valid_group->head->next;
    lp_valid_group->vCnt--;
    
    if(!lp_valid_group->vCnt)
    {
        lp_valid_group->tail = EGG_NULL;
    }
    /////////////////////////////////////////
    
    if(lp_backup_group->vCnt == 1)
    {
        lp_backup_group->head = lp_alloc_baker;
        lp_backup_group->tail = lp_alloc_baker;
    }
    else
    {
        lp_backup_group->tail->next = lp_alloc_baker;
        lp_backup_group->tail = lp_alloc_baker;
    }
    
    
    
    pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);
    return EGG_TRUE;
    
}

EBOOL eggRWSBakerManager_alter_group(HEGGRWSBAKERMANAGER hRWSBakerManager, long timestamp)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    pthread_mutex_lock(&hRWSBakerManager->hBakTab->mutex);

    hRWSBakerManager->hBakTab->validGroupIdx = (hRWSBakerManager->hBakTab->validGroupIdx + 1)%2;
    
    HEGGRWSBAKERGROUP lp_valid_group = hRWSBakerManager->hBakTab->bakGroup + hRWSBakerManager->hBakTab->validGroupIdx;
    lp_valid_group->timestamp = timestamp;
    hRWSBakerManager->maxtime = timestamp;
    index_t n_baker_idx = 0;
    count_t n_read_cnt = 0;
    while(n_baker_idx != hRWSBakerManager->hBakTab->aCnt)
    {
        EGG_BAKER_RW_SWAP(hRWSBakerManager->hBakTab->infos[n_baker_idx]->status);
        if(EGG_BAKER_IS_READING(hRWSBakerManager->hBakTab->infos[n_baker_idx]->status))
        {
            eggRWSBakerInfo_timestamp(hRWSBakerManager->hBakTab->infos[n_baker_idx]) = timestamp;
        }
        n_baker_idx++;
    }
    
    pthread_cond_signal(&hRWSBakerManager->hBakTab->cond);

    pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);
    return EGG_TRUE;
    
}


EBOOL eggRWSBakerManager_getRespon(HEGGRWSBAKERMANAGER hRWSBakerManager, HEGGRWSREQINFO hRWSReqInfo)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    if(POINTER_IS_INVALID(hRWSReqInfo))
    {
        return EGG_FALSE;
    }

    hRWSReqInfo->next = EGG_NULL;
    pthread_mutex_lock(&hRWSBakerManager->hReqQueue->mutex);
    
    if(hRWSBakerManager->hReqQueue->count)
    {
        hRWSBakerManager->hReqQueue->tail->next = hRWSReqInfo;
        hRWSBakerManager->hReqQueue->tail = hRWSReqInfo;
    }
    else
    {
        hRWSBakerManager->hReqQueue->head = hRWSReqInfo;
        hRWSBakerManager->hReqQueue->tail = hRWSReqInfo;
        pthread_cond_signal(&hRWSBakerManager->hReqQueue->cond);

    }
    
    hRWSBakerManager->hReqQueue->count++;
    //  pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

        pthread_cond_wait(&hRWSReqInfo->cond, &hRWSBakerManager->hReqQueue->mutex);
	//  pthread_cond_wait(&hRWSReqInfo->cond, &mtx);
//    printf("--------------getRespon (%lu)\n", hRWSReqInfo->thrId);
    
    pthread_mutex_unlock(&hRWSBakerManager->hReqQueue->mutex);
    //      pthread_mutex_unlock(&mtx);
    return EGG_TRUE;
    
    
}

EBOOL eggRWSBakerManager_processReq(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    struct timeval time1, time2;
    gettimeofday(&time1, 0);

    pthread_mutex_lock(&hRWSBakerManager->hReqQueue->mutex);
    if(!hRWSBakerManager->hReqQueue->count)
    {
        
        pthread_cond_wait(&hRWSBakerManager->hReqQueue->cond, &hRWSBakerManager->hReqQueue->mutex);
        
    }
    
    HEGGRWSREQINFO lp_req_info = hRWSBakerManager->hReqQueue->head;
    
    hRWSBakerManager->hReqQueue->head = lp_req_info->next;
    
    hRWSBakerManager->hReqQueue->count --;
    if(!hRWSBakerManager->hReqQueue->count)
    {
        hRWSBakerManager->hReqQueue->tail = EGG_NULL;
    }
    
    pthread_mutex_unlock(&hRWSBakerManager->hReqQueue->mutex);

    lp_req_info->baker = eggRWSBakerManager_alloc_baker(hRWSBakerManager, lp_req_info->thrId);

    //   sleep(1);
    pthread_cond_signal(&lp_req_info->cond);
    gettimeofday(&time2, 0);
    return EGG_TRUE;
}

EBOOL eggRWSBakerManager_create_worker(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    hRWSBakerManager->workerStat = EGG_WORKER_EXEING;
    pthread_create(&hRWSBakerManager->workerId, EGG_NULL, eggRWSBakerManager_working, hRWSBakerManager);
    
    return EGG_TRUE;
}


EBOOL eggRWSBakerManager_create_counter(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    hRWSBakerManager->counterStat = EGG_COUNTER_EXEING;
    pthread_create(&hRWSBakerManager->counterId, EGG_NULL, eggRWSBakerManager_statistics, hRWSBakerManager);
    
    return EGG_TRUE;
}


EBOOL eggRWSBakerManager_destroy_worker(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    hRWSBakerManager->workerStat = EGG_WORKER_STOP;
    pthread_kill(hRWSBakerManager->workerId, SIGKILL);
    return EGG_TRUE;
}


EBOOL eggRWSBakerManager_wait_worker(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    
    EBOOL ret;
    pthread_join(hRWSBakerManager->workerId, &ret);
    return EGG_TRUE;
}

EBOOL eggRWSBakerManager_WToR_Group(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }

    pthread_mutex_lock(&hRWSBakerManager->hBakTab->mutex);

    
    HEGGRWSBAKERGROUP lp_read_group = hRWSBakerManager->hBakTab->bakGroup + hRWSBakerManager->hBakTab->validGroupIdx;
    
    HEGGRWSBAKERGROUP lp_write_group = hRWSBakerManager->hBakTab->bakGroup + (hRWSBakerManager->hBakTab->validGroupIdx + 1)%2;
    
    HEGGRWSBAKER lp_write_iter = lp_write_group->head;

    lp_write_group->head = EGG_NULL;
    lp_write_group->tail = EGG_NULL;
    lp_write_group->timestamp = 0;
    lp_write_group->vCnt = 0;

    while (lp_write_iter)
    {
        EGG_BAKER_SET_READING(hRWSBakerManager->hBakTab->infos[lp_write_iter->bakIdx]->status);
	eggRWSBakerInfo_timestamp(hRWSBakerManager->hBakTab->infos[lp_write_iter->bakIdx]) = lp_read_group->timestamp;
        
        lp_read_group->vCnt ++;
        if(lp_read_group->vCnt == 1)
        {
            lp_read_group->head = lp_write_iter;
            lp_read_group->tail = lp_write_iter;
            pthread_cond_signal(&hRWSBakerManager->hBakTab->cond);
            
        }
        else
        {
            lp_read_group->tail->next = lp_write_iter;
            lp_read_group->tail = lp_write_iter;
        }
        
        
        
        lp_write_iter = lp_write_iter->next;
    }
        
    pthread_mutex_unlock(&hRWSBakerManager->hBakTab->mutex);
    
    return EGG_TRUE;
}


HEGGRWSBAKERINFO* eggRWSBakerManager_getBakerInfo(HEGGRWSBAKERMANAGER hRWSBakerManager, count_t* pInfoCnt)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_NULL;
    }
    *pInfoCnt = hRWSBakerManager->hBakTab->aCnt;
    return hRWSBakerManager->hBakTab->infos;
}
/* HEGGRWSBAKERINFO* p = eggRWSBakerManager_getBakerInfo(HEGGRWSBAKERMANAGER hRWSBakerManager, count_t* pInfoCnt); */
/* p[0]->baker */
/* p[1]->baker */
/* p[2]->baker */
/* p[3]->baker */

PRIVATE void* eggRWSBakerManager_working(HEGGRWSBAKERMANAGER hRWSBakerManager)
    
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_NULL;
    }

    while(hRWSBakerManager->workerStat == EGG_WORKER_EXEING)
    {
        if(eggRWSBakerManager_processReq(hRWSBakerManager) != EGG_TRUE)
        {
            printf("eggRWSBakerManager_processReq is false file [%s] line [%d]\n", __FILE__, __LINE__); 
            break;
        }
    }
    

    return EGG_NULL;
}

PRIVATE EBOOL eggRWSBakerManager_print_bakerReqCnt(HEGGRWSBAKERMANAGER hRWSBakerManager)
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_FALSE;
    }
    char printbuf[4096] = {0};
    char* lp_iter = printbuf;
    long printpos;
    count_t n_baker_cnt = hRWSBakerManager->hBakTab->aCnt;
    index_t n_baker_idx = 0;
    
    printpos = sprintf(lp_iter, "================================[BAKERMANAGER INFO]================================\n");
    lp_iter += printpos;
    
    while (n_baker_idx != n_baker_cnt)
    {
        char* flag1= EGG_BAKER_IS_BUSY(hRWSBakerManager->hBakTab->infos[n_baker_idx]->status)? "busying" : "idle";

        char* flag2= EGG_BAKER_IS_READING(hRWSBakerManager->hBakTab->infos[n_baker_idx]->status)? "reading" : "writeing";

        printpos = sprintf(lp_iter, "[baker id : %d] : [reqCnt :%d] [path : %s] [load_status : %s] [wr_status : %s] [timestamp : %lu] [thrId : %ld]\n", n_baker_idx, hRWSBakerManager->hBakTab->infos[n_baker_idx]->reqCnt, hRWSBakerManager->hBakTab->infos[n_baker_idx]->path, flag1, flag2, hRWSBakerManager->hBakTab->infos[n_baker_idx]->timestamp, hRWSBakerManager->hBakTab->infos[n_baker_idx]->thrId);
        
        lp_iter += printpos;
        n_baker_idx++;
    }
    
    sprintf(lp_iter, "================================[BAKERMANAGER INFO]================================\n");

    printf("%s\n", printbuf);
    LOG_INFO("BakerManager", "%s", printbuf);
    
    return EGG_TRUE;

}

PRIVATE void* eggRWSBakerManager_statistics(HEGGRWSBAKERMANAGER hRWSBakerManager)
    
{
    if(POINTER_IS_INVALID(hRWSBakerManager))
    {
        return EGG_NULL;
    }
    while(EGG_TRUE)
    {
        sleep(5);
        eggRWSBakerManager_print_bakerReqCnt(hRWSBakerManager);
    }

    return EGG_NULL;
}


