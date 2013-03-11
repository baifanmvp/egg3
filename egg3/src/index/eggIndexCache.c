#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "../eggIndexCache.h"
#include "uti/eggUtiMd5.h"
extern pthread_mutex_t counter_mutex ;


PRIVATE int egg_hash_equal(const void* key1, const void* key2);

PRIVATE unsigned int egg_hash_value(const void* key);

PRIVATE EBOOL eggIndexCache_hashDestroy(HEGGINDEXCACHE hEggIndexCache);


HEGGINDEXCACHEKEY eggIndexCacheKey_new(type_t type, size16_t mask, void* pKey, size16_t sz)
{
    HEGGINDEXCACHEKEY lp_cache_key = (HEGGINDEXCACHEKEY)malloc(sizeof(EGGINDEXCACHEKEY) + sz + 1);
    memset(lp_cache_key, 0, sizeof(EGGINDEXCACHEKEY) + sz + 1);
    lp_cache_key->type = type;
    lp_cache_key->size = sizeof(EGGINDEXCACHEKEY) + sz + 1;
    lp_cache_key->mask = mask;
    lp_cache_key->lifeCycle = 1;
    memcpy(lp_cache_key + 1, pKey, sz);
    
    return lp_cache_key;
}

EBOOL eggIndexCacheKey_delete(HEGGINDEXCACHEKEY hCacheKey)
{
    if(hCacheKey)
    {
        free(hCacheKey);
    }
    return EGG_TRUE;
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
HEGGINDEXCACHEVAL eggIndexCacheVal_new()
{
    return Uti_vector_create(sizeof(EGGIDNODE));
}

EBOOL eggIndexCacheVal_delete(HEGGINDEXCACHEVAL hCacheVal)
{
    if(hCacheVal)
        Uti_vector_destroy(hCacheVal, EGG_TRUE);
    return EGG_TRUE;
}

EBOOL eggIndexCacheVal_fetch(HEGGINDEXCACHEVAL hCacheVal, HEGGIDNODE* hhEggIdNode, count_t* lpNodeCnt)
{
    if (!hCacheVal || !hhEggIdNode || !lpNodeCnt)
    {
        return EGG_FALSE;
    }
    
    *hhEggIdNode = Uti_vector_data(hCacheVal);
    *lpNodeCnt = Uti_vector_count(hCacheVal);
    
    return EGG_TRUE;
}

HEGGINDEXCACHEVAL eggIndexCacheVal_add(HEGGINDEXCACHEVAL hCacheVal, HEGGIDNODE hEggIdNode)
{
    if (!hEggIdNode )
    {
        return EGG_NULL;
    }
    
    if(!hCacheVal)
    {
        hCacheVal = eggIndexCacheVal_new();
    }
    
//    printf("eggIndexCacheVal_add start \n");
    Uti_vector_push(hCacheVal, hEggIdNode, 1);
    //  printf("eggIndexCacheVal_add end \n");
    return hCacheVal;
}
count_t eggIndexCacheVal_get_idcnt(HEGGINDEXCACHEVAL hCacheVal)
{
    if (!hCacheVal )
    {
        return EGG_NULL;
    }
    return Uti_vector_count(hCacheVal);
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
PUBLIC HEGGINDEXCACHEITER eggIndexCacheIter_new(HEGGINDEXCACHE hIndexCache)
{
    if (EGGINDEXCACHE_IS_INVALID(hIndexCache))
    {
        return EGG_NULL;
    }
    
    HEGGINDEXCACHEITER lp_indexCache_iter = (HEGGINDEXCACHEITER)malloc(sizeof(EGGINDEXCACHEITER));
    if (!lp_indexCache_iter)
    {
        return EGG_NULL;
    }

    memset(lp_indexCache_iter, 0, sizeof(EGGINDEXCACHEITER));
    
    lp_indexCache_iter->refMutex = &(hIndexCache->mutex);
    
    g_hash_table_iter_init(&(lp_indexCache_iter->iter), hIndexCache->hashTable);

    return lp_indexCache_iter;
}

PUBLIC EBOOL eggIndexCacheIter_delete(HEGGINDEXCACHEITER hEggIndexCacheIter)
{
    if (hEggIndexCacheIter)
    {
        free(hEggIndexCacheIter);
        return EGG_TRUE;
    }

    return EGG_FALSE;
}

PUBLIC EBOOL eggIndexCacheIter_next(HEGGINDEXCACHEITER hCacheIter, HEGGINDEXCACHEKEY* hCacheKey, HEGGINDEXCACHEVAL* hCacheVal)
{
    if (!hCacheIter)
    {
        *hCacheKey = EGG_NULL;
        *hCacheVal = EGG_NULL;
        
        return EGG_FALSE;
    }
    EBOOL ret = g_hash_table_iter_next(&(hCacheIter->iter), (gpointer*)hCacheKey, (gpointer*)hCacheVal);
    return ret; 
}
//////////////////////////
//////////////////////////
//////////////////////////
//////////////////////////


PUBLIC HEGGINDEXCACHE eggIndexCache_new()
{
    EGGINDEXCACHE* lp_index_cache = (HEGGINDEXCACHE)malloc(sizeof(EGGINDEXCACHE));
    
    if (EGGINDEXCACHE_IS_INVALID(lp_index_cache))
    {
        return EGG_NULL;
    }

    lp_index_cache->hashTable = g_hash_table_new(egg_hash_value, egg_hash_equal);
    lp_index_cache->refCnt = 0;
    lp_index_cache->aCnt = 0;
    lp_index_cache->aSize = 0;
     pthread_mutex_init( &lp_index_cache->mutex, NULL);
//     counter_mutex = PTHREAD_MUTEX_INITIALIZER;

//     EGGINDEXCACHE_PLUSREF(lp_index_cache);
    return lp_index_cache;
}


PUBLIC HEGGINDEXCACHE eggIndexCache_reinit(HEGGINDEXCACHE hIndexCache)
{
    if (EGGINDEXCACHE_IS_INVALID(hIndexCache))
    {
        return EGG_NULL;
    }
    
    eggIndexCache_hashDestroy(hIndexCache);
    hIndexCache->hashTable = g_hash_table_new(egg_hash_value, egg_hash_equal);
//     counter_mutex = PTHREAD_MUTEX_INITIALIZER;

//     EGGINDEXCACHE_PLUSREF(lp_index_cache);
    return hIndexCache;
    

}

PUBLIC EBOOL eggIndexCache_ref(HEGGINDEXCACHE* hIndexCacheRef, HEGGINDEXCACHE hIndexCacheOrg)
{
    if (EGGINDEXCACHE_IS_INVALID(hIndexCacheRef) || EGGINDEXCACHE_IS_INVALID(hIndexCacheOrg))
    {
        return EGG_FALSE;
    }

    * hIndexCacheRef = hIndexCacheOrg;
    EGGINDEXCACHE_PLUSREF(hIndexCacheOrg);

    return  EGG_TRUE;
}


PUBLIC EBOOL eggIndexCache_insert2(HEGGINDEXCACHE hEggIndexCache,
                                  HEGGINDEXCACHEKEY hCacheKey,
                                  HEGGIDNODE hIdNode)
{
    if(EGGINDEXCACHE_IS_INVALID(hEggIndexCache))
    {
        return EGG_FALSE; 
    }
//    pthread_mutex_lock(&hEggIndexCache->mutex);
    
    HEGGINDEXCACHEVAL lp_cache_val_ret = g_hash_table_lookup(hEggIndexCache->hashTable, hCacheKey);        

    HEGGINDEXCACHEVAL lp_cache_val = lp_cache_val_ret;
    
    lp_cache_val = eggIndexCacheVal_add(lp_cache_val, hIdNode);
    
    if(!lp_cache_val_ret)
    {
        HEGGINDEXCACHEKEY lp_cache_key_ist = (HEGGINDEXCACHEKEY)malloc(hCacheKey->size);
        memcpy(lp_cache_key_ist, hCacheKey, hCacheKey->size);
        hEggIndexCache->aCnt ++;
        g_hash_table_insert(hEggIndexCache->hashTable, lp_cache_key_ist, lp_cache_val);
    }
   

    //  pthread_mutex_unlock(&hEggIndexCache->mutex);

    return EGG_TRUE;
}


PUBLIC EBOOL eggIndexCache_insert(HEGGINDEXCACHE hEggIndexCache,
                                  HEGGINDEXCACHEKEY hCacheKey,
                                  HEGGIDNODE hIdNode)
{
    if(EGGINDEXCACHE_IS_INVALID(hEggIndexCache))
    {
        return EGG_FALSE; 
    }
    
    HEGGINDEXCACHEVAL lp_cache_val_ret = g_hash_table_lookup(hEggIndexCache->hashTable, hCacheKey);        

    HEGGINDEXCACHEVAL lp_cache_val = lp_cache_val_ret;
    
    lp_cache_val = eggIndexCacheVal_add(lp_cache_val, hIdNode);
    
    if(!lp_cache_val_ret)
    {
        HEGGINDEXCACHEKEY lp_cache_key_ist = (HEGGINDEXCACHEKEY)malloc(hCacheKey->size);
        memcpy(lp_cache_key_ist, hCacheKey, hCacheKey->size);
        hEggIndexCache->aCnt ++;
        hEggIndexCache->aSize += sizeof(EGGINDEXCACHEVAL);
        g_hash_table_insert(hEggIndexCache->hashTable, lp_cache_key_ist, lp_cache_val);
    }
   
    hEggIndexCache->aSize += sizeof(EGGIDNODE);


    return EGG_TRUE;
}


PUBLIC HEGGINDEXCACHEVAL eggIndexCache_lookup(HEGGINDEXCACHE hEggIndexCache,
                                              HEGGINDEXCACHEKEY hCacheKey)
{
    if(EGGINDEXCACHE_IS_INVALID(hEggIndexCache))
    {
        return EGG_FALSE; 
    }
    
//    pthread_mutex_lock(&hEggIndexCache->mutex);
    
    HEGGINDEXCACHEVAL lp_cache_val = g_hash_table_lookup(hEggIndexCache->hashTable, hCacheKey);
    
    //     pthread_mutex_unlock(&hEggIndexCache->mutex);

    return lp_cache_val;
}

PUBLIC EBOOL eggIndexCache_remove(HEGGINDEXCACHE hEggIndexCache,
                                  HEGGINDEXCACHEKEY hCacheKey)
{
    if(EGGINDEXCACHE_IS_INVALID(hEggIndexCache))
    {
        return EGG_FALSE; 
    }
//    pthread_mutex_lock(&hEggIndexCache->mutex);

    EBOOL ret =  g_hash_table_remove(hEggIndexCache->hashTable, hCacheKey);
    //    pthread_mutex_unlock(&hEggIndexCache->mutex);

    return ret;
}


PUBLIC EBOOL eggIndexCache_delete(HEGGINDEXCACHE hEggIndexCache)
{
    if (EGGINDEXCACHE_IS_INVALID(hEggIndexCache))
    {
        return EGG_FALSE;
    }
    /* EGGINDEXCACHE_MINUSREF(hEggIndexCache); */

    /* if(EGGINDEXCACHE_IS_REFING(hEggIndexCache)) */
    /* { */
    /*     return EGG_FALSE; */
    /* } */
    
    eggIndexCache_hashDestroy(hEggIndexCache);
    pthread_mutex_destroy(&hEggIndexCache->mutex);
        
    free(hEggIndexCache);
    
    return EGG_TRUE;
        
}

PRIVATE EBOOL eggIndexCache_hashDestroy(HEGGINDEXCACHE hEggIndexCache)
{
    if (EGGINDEXCACHE_IS_INVALID(hEggIndexCache))
    {
        return EGG_FALSE;
    }
    
    GList* list_key = g_hash_table_get_keys(hEggIndexCache->hashTable);
    GList* list_value = g_hash_table_get_values(hEggIndexCache->hashTable);
    //    printf("KEY CNT : %d\n", g_hash_table_size(hEggIndexCache->hashTable));
    if (list_key != EGG_NULL)
    {
        GList* list_key_iter = g_list_first(list_key);
        count_t nkeycnt = 0;
        do {
            free(list_key_iter->data);
            nkeycnt++;
        } while ((list_key_iter = g_list_next(list_key_iter)) != EGG_NULL);
        //printf("KEY CNT : %d\n", nkeycnt);       
        g_list_free(list_key);
    }

    if (list_value != EGG_NULL)
    {
        GList* list_value_iter = g_list_first(list_value);
        count_t nvalcnt = 0;
        do {
            
            HEGGINDEXCACHEVAL lp_cache_val = list_value_iter->data;
//	    printf("----------------\n");
            if(lp_cache_val)
            {
                Uti_vector_destroy(lp_cache_val, EGG_TRUE);
            }
//            printf("*****************\n");
            nvalcnt++;
        } while ((list_value_iter = g_list_next(list_value_iter)) != EGG_NULL);
        //printf("VAL CNT : %d\n", nvalcnt);       

        g_list_free(list_value);
    }
    
    g_hash_table_destroy(hEggIndexCache->hashTable);
    hEggIndexCache->hashTable = EGG_NULL;
    
    return EGG_TRUE;
}

PRIVATE int egg_hash_equal(const void* key1, const void* key2)
{
    HEGGINDEXCACHEKEY lp_cache_key1 = (HEGGINDEXCACHEKEY)key1;
    HEGGINDEXCACHEKEY lp_cache_key2 = (HEGGINDEXCACHEKEY)key2;
    
    char* lp_key1 = (char*)(lp_cache_key1 + 1);
    char* lp_key2 = (char*)(lp_cache_key2 + 1);
    
    if(lp_cache_key2->size != lp_cache_key1->size)
    {
        return EGG_FALSE;
    }
    
    if(lp_cache_key2->mask != lp_cache_key1->mask)
    {
        return EGG_FALSE;
    }   
    
    if(memcmp(lp_key1 , lp_key2, lp_cache_key2->size - sizeof(EGGINDEXCACHEKEY)))
    {
        return EGG_FALSE;
    }
    return EGG_TRUE;
    
    
}

PRIVATE unsigned int egg_hash_value(const void* key)
{
    HEGGINDEXCACHEKEY lp_cache_key = (HEGGINDEXCACHEKEY)key;
    
    return g_str_hash((char*)(lp_cache_key+1));
}

