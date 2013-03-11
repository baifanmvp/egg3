#ifndef _EGG_INDEXCACHE_H
#define _EGG_INDEXCACHE_H
#include "./EggDef.h"
#include "./storage/eggIdNode.h"
#include "./uti/Utility.h"
#include <string.h>
#include <glib.h>
#define EGG_CACHE_BLOCK_CNT (64)
#define EGG_CACHE_BLOCK_SIZE (EGG_CACHE_BLOCK_CNT*sizeof(EGGIDNODE))
#define EGG_CACHE_KEY_LIMIT (100000)
#define EGG_CACHE_SIZE_LIMIT (100*1024*1024)
#define EGG_CACHE_IDS_LIMIT (100)

/*
  EGGINDEXCACHEKEY MODE
*/


typedef struct eggIndexCacheKey EGGINDEXCACHEKEY;
typedef struct eggIndexCacheKey* HEGGINDEXCACHEKEY;

struct eggIndexCacheKey
{
    size16_t size;
    type_t type;
    size16_t mask;
    size16_t lifeCycle;
};

HEGGINDEXCACHEKEY eggIndexCacheKey_new(type_t type, size16_t mask, void* pKey, size16_t sz);

EBOOL eggIndexCacheKey_delete(HEGGINDEXCACHEKEY hCacheKey);


/*
  EGGINDEXCACHEVAL MODE
*/

typedef UTIVECTOR EGGINDEXCACHEVAL;
typedef UTIVECTOR* HEGGINDEXCACHEVAL;


HEGGINDEXCACHEVAL eggIndexCacheVal_new();

EBOOL eggIndexCacheVal_delete(HEGGINDEXCACHEVAL hCacheVal);

EBOOL eggIndexCacheVal_fetch(HEGGINDEXCACHEVAL hCacheVal, HEGGIDNODE* hhEggIdNode, count_t* lpNodeCnt);

HEGGINDEXCACHEVAL eggIndexCacheVal_add(HEGGINDEXCACHEVAL hCacheVal, HEGGIDNODE hEggIdNode);

count_t eggIndexCacheVal_get_idcnt(HEGGINDEXCACHEVAL hCacheVal);


/*
  EGGINDEXCACHE MODE
 */

typedef struct eggIndexCache EGGINDEXCACHE;
typedef struct eggIndexCache* HEGGINDEXCACHE;

struct eggIndexCache
{
    GHashTable* hashTable;
    count_t refCnt;
    count_t aCnt;
    size32_t aSize;
    pthread_mutex_t mutex;
    pthread_mutex_t allMutex;
};

PUBLIC HEGGINDEXCACHE eggIndexCache_new();

PUBLIC HEGGINDEXCACHE eggIndexCache_reinit(HEGGINDEXCACHE hIndexCache);

PUBLIC EBOOL eggIndexCache_ref(HEGGINDEXCACHE* hIndexCacheRef, HEGGINDEXCACHE hIndexCacheOrg);

PUBLIC EBOOL eggIndexCache_insert(HEGGINDEXCACHE hIndexCache, HEGGINDEXCACHEKEY hCacheKey, HEGGIDNODE hIdNode);

PUBLIC HEGGINDEXCACHEVAL eggIndexCache_lookup(HEGGINDEXCACHE hIndexCache, HEGGINDEXCACHEKEY hCacheKey);

PUBLIC EBOOL eggIndexCache_remove(HEGGINDEXCACHE hEggIndexCache, HEGGINDEXCACHEKEY hCacheKey);

PUBLIC EBOOL eggIndexCache_delete(HEGGINDEXCACHE hEggIndexCache);

#define EGGINDEXCACHE_IS_INVALID(hEggIndexCache) (!hEggIndexCache ? EGG_TRUE : EGG_FALSE)

#define EGGINDEXCACHE_PLUSREF(hEggIndexCache) (hEggIndexCache->refCnt++)

#define EGGINDEXCACHE_MINUSREF(hEggIndexCache) (hEggIndexCache->refCnt--)

#define EGGINDEXCACHE_IS_REFING(hEggIndexCache) (hEggIndexCache != 0)

/*
  EGGINDEXCACHEITER MODE
*/
typedef struct eggIndexCacheIter EGGINDEXCACHEITER;
typedef struct eggIndexCacheIter* HEGGINDEXCACHEITER;

struct eggIndexCacheIter
{
    GHashTableIter iter;
    pthread_mutex_t* refMutex;
};

PUBLIC HEGGINDEXCACHEITER eggIndexCacheIter_new(HEGGINDEXCACHE hIndexCache);
    
PUBLIC EBOOL eggIndexCacheIter_delete(HEGGINDEXCACHEITER hEggIndexCacheIter);
    
PUBLIC EBOOL eggIndexCacheIter_next(HEGGINDEXCACHEITER hCacheIter, HEGGINDEXCACHEKEY* hCacheKey, HEGGINDEXCACHEVAL* hCacheVal);


#endif
