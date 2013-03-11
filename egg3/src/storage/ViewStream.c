#include "ViewStream.h"
#include "../EggDef.h"
#include "Cluster.h"
#include "File.h"
#include "../uti/Utility.h"
#include "../log/eggPrtLog.h"
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t s_freearea_mutex = PTHREAD_MUTEX_INITIALIZER;

PRIVATE vstream_t* ViewStream_mapping_alloc(HVIEWSTREAM hViewStream, offset64_t nStartOff, size32_t nSize);
PRIVATE EBOOL ViewStream_mapping_free(HVIEWSTREAM hViewStream);
PRIVATE EBOOL ViewStream_flush_map(HVIEWSTREAM hViewStream);
PRIVATE EBOOL ViewStream_modify_map(HVIEWSTREAM hViewStream, size32_t nSize, offset64_t nOffset);
PRIVATE offset64_t ViewStream_getArea(HVIEWSTREAM hViewStream, size32_t nSize);
PRIVATE int dumpAndMergeFreeArea(HVIEWSTREAM hViewStream);
static const size32_t MINSIZE = 256;
static const size32_t MAXSIZE = 4096;
static inline size32_t alignSize(size32_t size)
{
    size32_t alignSz = MINSIZE;
    while (alignSz < size && alignSz <= MAXSIZE)
    {
        alignSz <<= 1;
    }
    if (alignSz > MAXSIZE)
    {
        alignSz = (size + MAXSIZE - 1) / MAXSIZE * MAXSIZE;
    }
    return size == 0 ? 0 : alignSz;
}

PUBLIC offset64_t ViewStream_write_nolock(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize);


#define ViewStream_object(_MODULEPTR) \
    ((VIEWSTREAM*)_MODULEPTR)


#define UNIT_ALLOC_SIZE  (0x400000)
#define UNIT_ALLOC_COUNT (CLUSTER_ALLOC_SIZE / UNIT_ALLOC_SIZE)


SESSION1 type_t s1_DesiredAccess = MAP_FILE_NOCACHE;//MAP_FILE_MAP;

PUBLIC HVIEWSTREAM ViewStream_new(HEGGFILE hEggFile)
{
    if (!EggFile_is_object(hEggFile))
    {
        return EGG_NULL;
    }

    HVIEWSTREAM hViewStream = (HVIEWSTREAM)malloc(sizeof(VIEWSTREAM));
    memset(hViewStream, 0, sizeof(VIEWSTREAM));

    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    lp_viewstream->sizeOfMapView = 0;
    lp_viewstream->offOfMapView = -1;
    lp_viewstream->hEggFile = hEggFile;
    
//    lp_viewstream->mapView = ViewStream_mapping_alloc(hViewStream, 0, EggFile_size(lp_viewstream->hEggFile));
        
    
    return hViewStream;
}

PUBLIC EBOOL ViewStream_delete(HVIEWSTREAM hViewStream)
{
    if (ViewStream_is_object(hViewStream))
    {
#ifdef WIN32        
        ::CloseHandle(hViewStream->hMappingFile);
        ::UnmapViewOfFile(hViewStream->mapView);
#else
        munmap(ViewStream_object(hViewStream)->mapView,
                     ViewStream_object(hViewStream)->sizeOfMapView);
#endif
        
        dumpAndMergeFreeArea(hViewStream);
        free(ViewStream_object(hViewStream)->pFreeArea);
        EggFile_close(ViewStream_object(hViewStream)->freeAreaFile);
        free(ViewStream_object(hViewStream)->nameFreeArea);
        EggFile_close(ViewStream_object(hViewStream)->hEggFile);
        free(hViewStream);
        return EGG_TRUE;
    }
    
    return EGG_FALSE;
}

PUBLIC EBOOL ViewStream_read(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    
    if (nOffset == -1 )//|| nOffset > EggFile_size(lp_viewstream->hEggFile))
    {
        return EGG_FALSE;
    }

    if (s1_DesiredAccess & MAP_FILE_NOCACHE)
    {
      //        EggFile_lock_rd_wait(ViewStream_object(hViewStream)->hEggFile,
      //                           SEEK_SET, nOffset, nSize);
        EggFile_read(ViewStream_object(hViewStream)->hEggFile,
                     ePointer,
                     nSize,
                     nOffset);
	// EggFile_unlock(ViewStream_object(hViewStream)->hEggFile,
        //                   SEEK_SET, nOffset, nSize);
        
    }
    else if (s1_DesiredAccess & MAP_FILE_MAP)
    {
        ViewStream_modify_map(hViewStream, nSize, nOffset);
    
        //memcpy(ePointer, hViewStream->mapView + (nOffset - n_base_offset), nSize);
        offset32_t off32 = LOLONGLONG(nOffset - Cluster_owner_offset((HCLUSTER)(lp_viewstream->mapView)));
    
        Cluster_memcpy(Cluster_object(lp_viewstream->mapView),
                       ePointer,
                       nSize,
                       &off32,
                       CLUSTER_ACCESS_READ);
    }
    
                                               
    return EGG_TRUE;
}

PUBLIC EBOOL ViewStream_read_nolock(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    
    if (nOffset == -1 || nOffset > EggFile_size(lp_viewstream->hEggFile))
    {
        return EGG_FALSE;
    }

    if (s1_DesiredAccess & MAP_FILE_NOCACHE)
    {
        EggFile_read(ViewStream_object(hViewStream)->hEggFile,
                     ePointer,
                     nSize,
                     nOffset);
        
    }
    else if (s1_DesiredAccess & MAP_FILE_MAP)
    {
        ViewStream_modify_map(hViewStream, nSize, nOffset);
    
        //memcpy(ePointer, hViewStream->mapView + (nOffset - n_base_offset), nSize);
        offset32_t off32 = LOLONGLONG(nOffset - Cluster_owner_offset((HCLUSTER)(lp_viewstream->mapView)));
    
        Cluster_memcpy(Cluster_object(lp_viewstream->mapView),
                       ePointer,
                       nSize,
                       &off32,
                       CLUSTER_ACCESS_READ);
    }
    
                                               
    return EGG_TRUE;
}


PUBLIC EBOOL ViewStream_read_info(HVIEWSTREAM hViewStream, epointer ePointer, size16_t nSize, offset16_t nOffset)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

//    ViewStream_modify_map(hViewStream, nSize, nOffset);
//    memcpy(ePointer, hViewStream->mapView + nOffset, nSize);
    
    EggFile_lock_rd_wait(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, nOffset, nSize);

    EggFile_read(ViewStream_object(hViewStream)->hEggFile,
                 ePointer,
                 nSize,
                 nOffset);
    
    EggFile_unlock(ViewStream_object(hViewStream)->hEggFile,
                   SEEK_SET, nOffset, nSize);
    
    
    return EGG_TRUE;
}

PUBLIC EBOOL ViewStream_update(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);

    EggFile_lock_wr_wait(ViewStream_object(hViewStream)->hEggFile,
                       SEEK_END, 0, 0);
    size64_t nFileSize;
    if (nOffset >= (nFileSize = EggFile_size(lp_viewstream->hEggFile)))
    {
        //ViewStream_append(hViewStream, ePointer, nSize);
        
        ViewStream_write_nolock(hViewStream, ePointer, nSize);
        EggFile_unlock(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, nFileSize, 0);
        
        return EGG_TRUE;
    }
    EggFile_unlock(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, nFileSize, 0);

    if (s1_DesiredAccess & MAP_FILE_NOCACHE)
    {

        EggFile_lock_wr_wait(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, nOffset, nSize);
        EggFile_write(ViewStream_object(hViewStream)->hEggFile,
                      ePointer,
                      nSize,
                      nOffset);
        EggFile_unlock(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, nOffset, nSize);
        
    }
    else if (s1_DesiredAccess & MAP_FILE_MAP)
    {
        ViewStream_modify_map(hViewStream, nSize, nOffset);
        
        //memcpy(hViewStream->mapView + (nOffset - n_base_offset), ePointer, nSize);
        offset32_t off32 = LOLONGLONG(nOffset - Cluster_owner_offset(Cluster_object(lp_viewstream->mapView)));
        Cluster_memcpy(Cluster_object(ViewStream_object(hViewStream)->mapView),
                       ePointer,
                       nSize,
                       &off32,
                       CLUSTER_ACCESS_UPDATE);
    }
    
    return EGG_FALSE;
}


PUBLIC EBOOL ViewStream_update_nolock(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);

    size64_t nFileSize;
    if (nOffset >= (nFileSize = EggFile_size(lp_viewstream->hEggFile)))
    {
        //ViewStream_append(hViewStream, ePointer, nSize);
        
        ViewStream_write_nolock(hViewStream, ePointer, nSize);
        
        return EGG_TRUE;
    }

    if (s1_DesiredAccess & MAP_FILE_NOCACHE)
    {

        EggFile_write(ViewStream_object(hViewStream)->hEggFile,
                      ePointer,
                      nSize,
                      nOffset);
        
    }
    else if (s1_DesiredAccess & MAP_FILE_MAP)
    {
        ViewStream_modify_map(hViewStream, nSize, nOffset);
        
        //memcpy(hViewStream->mapView + (nOffset - n_base_offset), ePointer, nSize);
        offset32_t off32 = LOLONGLONG(nOffset - Cluster_owner_offset(Cluster_object(lp_viewstream->mapView)));
        Cluster_memcpy(Cluster_object(ViewStream_object(hViewStream)->mapView),
                       ePointer,
                       nSize,
                       &off32,
                       CLUSTER_ACCESS_UPDATE);
    }
    
    return EGG_FALSE;
}

PUBLIC EBOOL ViewStream_update_info(HVIEWSTREAM hViewStream, ecpointer ecPointer, size16_t nSize, offset16_t nOffset)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    if (s1_DesiredAccess & MAP_FILE_NOCACHE)
    {
        EggFile_lock_wr_wait(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, nOffset, nSize);

        EggFile_write(ViewStream_object(hViewStream)->hEggFile,
                     ecPointer,
                     nSize,
                     nOffset);
        
        EggFile_lock_wr_wait(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, nOffset, nSize);
        
    }
    else if(s1_DesiredAccess & MAP_FILE_MAP)
    {
        ViewStream_modify_map(hViewStream, nSize, nOffset);
        
        memcpy(ViewStream_object(hViewStream)->mapView + nOffset, ecPointer, nSize);

#ifndef WIN32
    msync(ViewStream_object(hViewStream)->mapView + nOffset, nSize, MS_SYNC);

    EggFile_write(ViewStream_object(hViewStream)->hEggFile, ecPointer, nSize, nOffset);
    
#endif
    }    
    return EGG_TRUE;
}

PUBLIC offset64_t ViewStream_write_nolock(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    size32_t nSizeAlloc = alignSize(nSize);
    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    EBOOL eRet = 0;
    offset32_t off32 = 0;
    size64_t nFileSize;

    {
        offset64_t off;
        if ((off = ViewStream_getArea(hViewStream, nSize)) > 0)
        {
            EggFile_write(lp_viewstream->hEggFile,
                          ePointer,
                          nSize,
                          off);
            return off;
        }
    }
        
    if (s1_DesiredAccess & MAP_FILE_NOCACHE)
    {
        nFileSize = EggFile_size(lp_viewstream->hEggFile);        
        CLUSTER cluster;
       if (nFileSize <= MAP_VIEW_OFFSET)
       {
           eRet = ERR_CLUSTER_LESS_MEMORY;
       }
       else
       {

           offset64_t n_base_offset = Uti_base_offset(nFileSize - 0x100,
                                                      MAP_VIEW_OFFSET,
                                                      CLUSTER_ALLOC_SIZE);

           EggFile_read(ViewStream_object(hViewStream)->hEggFile,
                        &cluster,
                        sizeof(cluster),
                        n_base_offset);

           if (cluster.head.owner != n_base_offset)
           {                    /* may happen (after recovery) */
	     /* fprintf(stderr, "%s:%d:%s: WARN cluster.head.owner %llu != n_base_offset %llu Reset data\n", */
		 /*     __FILE__, __LINE__, __func__, */
		 /*     (long long unsigned)cluster.head.owner, (long long unsigned)n_base_offset); */
               eggPrtLog_warn("ViewStream", "%s:%d:%s: WARN cluster.head.owner %llu != n_base_offset %llu Reset data\n",
		     __FILE__, __LINE__, __func__,
		     (long long unsigned)cluster.head.owner, (long long unsigned)n_base_offset);
               cluster.head.owner = n_base_offset;
               off32 = sizeof(CLUSTERHEAD);
               cluster.head.used = nSizeAlloc;
               eRet = EGG_TRUE;
           }
           else if (nSizeAlloc <= CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD) - cluster.head.used )
           {
               off32 = cluster.head.used + sizeof(CLUSTERHEAD);
               cluster.head.used += nSizeAlloc;
               eRet = EGG_TRUE;
           }
           else if (nSizeAlloc > CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD))
           {
               /* fprintf(stderr, "%s:%d:%s: ERR write size %llu > max size %llu [%s]\n", */
               /*         __FILE__, __LINE__, __func__, */
               /*         (long long unsigned)nSizeAlloc, */
               /*         (long long unsigned)CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD), */
               /*         EggFile_name(ViewStream_object(hViewStream)->hEggFile)); */
               eggPrtLog_error("ViewStream", "%s:%d:%s: ERR write size %llu > max size %llu [%s]\n",
                       __FILE__, __LINE__, __func__,
                       (long long unsigned)nSizeAlloc,
                       (long long unsigned)CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD),
                       EggFile_name(ViewStream_object(hViewStream)->hEggFile));

               
               return 0;
               
           }           
           else
           {
               eRet = ERR_CLUSTER_LESS_MEMORY;
           }           
       }
       
       if (eRet == ERR_CLUSTER_LESS_MEMORY)
       {
           
           size64_t nOrgSize = nFileSize;
           epointer lpBuf = (epointer)malloc(UNIT_ALLOC_SIZE);
           memset(lpBuf, 0, UNIT_ALLOC_SIZE);
           
           count_t i = 0;
           for (;
                i < UNIT_ALLOC_COUNT;
                EggFile_write(lp_viewstream->hEggFile,
                              lpBuf,
                              UNIT_ALLOC_SIZE,
                              EggFile_size(lp_viewstream->hEggFile)), i++);
           
           memset(&cluster, 0, sizeof(cluster));
           cluster.head.owner = nOrgSize;
           cluster.head.used = nSizeAlloc;
           off32 = 0 + sizeof(CLUSTERHEAD);

           EggFile_write(lp_viewstream->hEggFile,
                         &cluster,
                         sizeof(cluster),
                         cluster.head.owner);

           EggFile_write(lp_viewstream->hEggFile,
                         ePointer,
                         nSize,
                         nOrgSize + ((offset64_t)off32));
           free(lpBuf);

           return nOrgSize + ((offset64_t)off32);
       }
       else
       {
           offset64_t n_base_offset = Uti_base_offset(nFileSize - 0x100,
                                                      MAP_VIEW_OFFSET,
                                                      CLUSTER_ALLOC_SIZE);
           EggFile_write(lp_viewstream->hEggFile,
                         &cluster,
                         sizeof(cluster),
                         cluster.head.owner);

           EggFile_write(lp_viewstream->hEggFile,
                         ePointer,
                         nSize,
                         n_base_offset + ((offset64_t)off32));

           return n_base_offset + ((offset64_t)off32);
       }
        
    }
    else if(s1_DesiredAccess & MAP_FILE_MAP)
    {
        nFileSize = EggFile_size(lp_viewstream->hEggFile);
        
        if (nFileSize <= MAP_VIEW_OFFSET)
        {
            eRet = ERR_CLUSTER_LESS_MEMORY;
        }
        else
        {
            ViewStream_modify_map(hViewStream, nSize, nFileSize - 0x100);
            eRet = Cluster_memcpy(Cluster_object(lp_viewstream->mapView),
                                  ePointer,
                                  nSize,
                                  &off32,
                                  CLUSTER_ACCESS_WRITE);
        }
            
        if (eRet == ERR_CLUSTER_LESS_MEMORY)
        {
                
            size64_t nOrgSize = nFileSize;
            epointer lpBuf = (epointer)malloc(UNIT_ALLOC_SIZE);
            memset(lpBuf, 0, UNIT_ALLOC_SIZE);
                
            count_t i = 0;
            for (;
                 i < UNIT_ALLOC_COUNT;
                 EggFile_write(lp_viewstream->hEggFile,
                               lpBuf,
                               UNIT_ALLOC_SIZE,
                               EggFile_size(lp_viewstream->hEggFile)), i++);
                
            nFileSize = EggFile_size(lp_viewstream->hEggFile);
                
            ViewStream_remap(hViewStream);
            ViewStream_modify_map(hViewStream, nSize, nFileSize - 0x100);
                
            Cluster_format(lp_viewstream->mapView, nOrgSize);
            Cluster_memcpy(Cluster_object(lp_viewstream->mapView),
                           ePointer,
                           nSize,
                           &off32,
                           CLUSTER_ACCESS_WRITE);
                
                
            free(lpBuf);
            return nOrgSize + ((offset64_t)off32);
        }
        else
        {
            offset64_t n_base_offset = Uti_base_offset(lp_viewstream->offOfMapView,
                                                       MAP_VIEW_OFFSET,
                                                       CLUSTER_ALLOC_SIZE);
                
            return n_base_offset + ((offset64_t)off32);
        }
    }
    
}


PUBLIC offset64_t ViewStream_write(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize)
{
//struct timeval start, end;
//gettimeofday(&start, NULL);

    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    size32_t nSizeAlloc = alignSize(nSize);
    
    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    
    EBOOL eRet = 0;
    offset32_t off32 = 0;
    size64_t nFileSize;

    {
        offset64_t off = 0;
        if ((off = ViewStream_getArea(hViewStream, nSizeAlloc)) > 0)
        {
        
            EggFile_lock_wr_wait(ViewStream_object(hViewStream)->hEggFile,
                               SEEK_SET, off, nSizeAlloc);
            
            /* not nSizeAlloc */
            EggFile_write(lp_viewstream->hEggFile,
                          ePointer,
                          nSize,
                          off);
            
            EggFile_unlock(ViewStream_object(hViewStream)->hEggFile,
                           SEEK_SET, off, nSizeAlloc);

            
            return off;
            
        }
        
    }

    EggFile_lock_wr_wait(lp_viewstream->hEggFile, SEEK_SET, 0, 0);
    
    if (s1_DesiredAccess & MAP_FILE_NOCACHE)
    {
        
        nFileSize = EggFile_size(lp_viewstream->hEggFile);        
        CLUSTER cluster;
       if (nFileSize <= MAP_VIEW_OFFSET)
       {
           eRet = ERR_CLUSTER_LESS_MEMORY;
           
       }
       else
       {

           offset64_t n_base_offset = Uti_base_offset(nFileSize - 0x100,
                                                      MAP_VIEW_OFFSET,
                                                      CLUSTER_ALLOC_SIZE);
           
           EggFile_read(ViewStream_object(hViewStream)->hEggFile,
                        &cluster,
                        sizeof(cluster),
                        n_base_offset);

           if (cluster.head.owner != n_base_offset)
           {
               /* may happen (after recovery), reinitialise */
	     /* fprintf(stderr, "%s:%d:%s: Warn cluster.head.owner %llu != n_base_offset %llu Reset data[%s]\n", */
		 /*     __FILE__, __LINE__, __func__, */
         /*         (long long unsigned)cluster.head.owner, (long long unsigned)n_base_offset,EggFile_name(ViewStream_object(hViewStream)->hEggFile)); */
         	     eggPrtLog_warn("ViewStream", "%s:%d:%s: Warn cluster.head.owner %llu != n_base_offset %llu Reset data[%s]\n",
		     __FILE__, __LINE__, __func__,
                 (long long unsigned)cluster.head.owner, (long long unsigned)n_base_offset,EggFile_name(ViewStream_object(hViewStream)->hEggFile));
               cluster.head.owner = n_base_offset;
               off32 = sizeof(CLUSTERHEAD);
               cluster.head.used = nSizeAlloc;
               eRet = EGG_TRUE;
           }
           else if (nSizeAlloc <= CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD) - cluster.head.used )
           {
               off32 = cluster.head.used + sizeof(CLUSTERHEAD);
               cluster.head.used += nSizeAlloc;
               eRet = EGG_TRUE;
           }
           else if (nSizeAlloc > CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD))
           {
               /* fprintf(stderr, "%s:%d:%s: ERR write size %llu > max size %llu [%s]\n", */
               /*         __FILE__, __LINE__, __func__, */
               /*         (long long unsigned)nSizeAlloc, */
               /*         (long long unsigned)CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD), */
               /*         EggFile_name(ViewStream_object(hViewStream)->hEggFile)); */
               eggPrtLog_error("ViewStream", "%s:%d:%s: ERR write size %llu > max size %llu [%s]\n",
                       __FILE__, __LINE__, __func__,
                       (long long unsigned)nSizeAlloc,
                       (long long unsigned)CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD),
                       EggFile_name(ViewStream_object(hViewStream)->hEggFile));
               return 0;
               
           }
           else
           {
               eRet = ERR_CLUSTER_LESS_MEMORY;
           }           
       }
       
       if (eRet == ERR_CLUSTER_LESS_MEMORY)
       {
           
           size64_t nOrgSize = nFileSize;
           epointer lpBuf = (epointer)malloc(UNIT_ALLOC_SIZE);
           memset(lpBuf, 0, UNIT_ALLOC_SIZE);
           
           count_t i = 0;
           for (;
                i < UNIT_ALLOC_COUNT;
                EggFile_write(lp_viewstream->hEggFile,
                              lpBuf,
                              UNIT_ALLOC_SIZE,
                              EggFile_size(lp_viewstream->hEggFile)), i++);
           
           memset(&cluster, 0, sizeof(cluster));
           cluster.head.owner = nOrgSize;
           cluster.head.used = nSizeAlloc;
           off32 = 0 + sizeof(CLUSTERHEAD);

           EggFile_write(lp_viewstream->hEggFile,
                         &cluster,
                         sizeof(cluster),
                         cluster.head.owner);

           /* not nSizeAlloc */
           EggFile_write(lp_viewstream->hEggFile,
                         ePointer,
                         nSize,
                         nOrgSize + ((offset64_t)off32));
           
           free(lpBuf);


           EggFile_unlock(lp_viewstream->hEggFile, SEEK_SET, 0, 0);

           return nOrgSize + ((offset64_t)off32);
       }
       else
       {
           
           offset64_t n_base_offset = Uti_base_offset(nFileSize - 0x100,
                                                      MAP_VIEW_OFFSET,
                                                      CLUSTER_ALLOC_SIZE);
           EggFile_write(lp_viewstream->hEggFile,
                         &cluster,
                         sizeof(cluster),
                         cluster.head.owner);

           EggFile_write(lp_viewstream->hEggFile,
                         ePointer,
                         nSize,
                         n_base_offset + ((offset64_t)off32));
           
           EggFile_unlock(lp_viewstream->hEggFile, SEEK_SET, 0, 0);           
           
           return n_base_offset + ((offset64_t)off32);
       }
        
    }
    return 0;
}

PRIVATE int comparePosLen_byPos(eggPosLen *key, eggPosLen *q)
{
    if (key->pos < q->pos)
        return -1;
    else if (key->pos == q->pos)
        return 0;
    else
        return 1;
}
PRIVATE int comparePosLen_byPos2(eggPosLen *key, eggPosLen *q)
{
    if (key->pos < q->pos)
        return 1;
    else if (key->pos == q->pos)
        return 0;
    else
        return -1;
}
PRIVATE int comparePosLen_byLen(eggPosLen *key, eggPosLen *q)
{
    if (key->len < q->len)
        return -1;
    else if (key->len == q->len)
        return 0;
    else
        return 1;
}
PRIVATE int comparePosLen_byLen2(eggPosLen *key, eggPosLen *q)
{
    if (key->len < q->len)
        return 1;
    else if (key->len == q->len)
        return 0;
    else
        return -1;
}
typedef struct {
    uint64_t size;
    uint64_t nelem;
    uint16_t mergeInterval;
    uint16_t syncInterval;
    uint32_t flag;
    char reserv[8];
} FreeAreaInfo;
#define SetConsistentStatus(flag) ((flag) = (flag) & (uint32_t)~0x1)
#define SetInconsistentStatus(flag) ((flag) = (flag) | (uint32_t)0x1)
#define TestConsistentStatus(flag) (!((flag) & 0x1))
#define initFreeAreaInfo(p) (((FreeAreaInfo*)(p))->size = FREEAREASIZE, \
                             ((FreeAreaInfo*)(p))->nelem = 0,           \
                             ((FreeAreaInfo*)(p))->mergeInterval = 0,   \
                             ((FreeAreaInfo*)(p))->syncInterval = 0,    \
                             ((FreeAreaInfo*)(p))->flag = 0,            \
                             SetConsistentStatus(((FreeAreaInfo*)(p))->flag))
PRIVATE EBOOL FreeAreaInfo_add(FreeAreaInfo *freeAreaInfo,
                               offset64_t nOffset, size32_t nSize);
PRIVATE eggPosLen FreeAreaInfo_remove(FreeAreaInfo *freeAreaInfo,
                                      size32_t nSizeAlign);
PRIVATE EBOOL ViewStream_initFreeArea(HVIEWSTREAM hViewStream, int ifCreate);
PRIVATE int restoreFreeArea(HVIEWSTREAM hViewStream);
PRIVATE int dumpFreeArea(HVIEWSTREAM hViewStream, FreeAreaInfo *freeAreaInfo);

/* #define SetSyncStatus(flag) ((flag) = (flag) & (uint32_t)~0x2) */
/* #define SetUnsyncStatus(flag) ((flag) = (flag) | (uint32_t)0x2) */
/* #define TestSyncStatus(flag) (!((flag) & 0x2)) */
//#define SYNCINTERVAL_THRESHOLD 1000
#define FREEAREASIZE (1<<20)
#define FREEAREAMAX (4<<20)
PRIVATE EBOOL ViewStream_initFreeArea(HVIEWSTREAM hViewStream, int ifCreate)
{
struct timeval start, end;
    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    int isFull = 0;
    FreeAreaInfo *freeAreaInfo;

    if(!lp_viewstream->pFreeArea)
    {
        if (!lp_viewstream->nameFreeArea)
        {
            char *baseName = ViewStream_name(lp_viewstream);
            lp_viewstream->nameFreeArea = malloc(strlen(baseName) + 20);
            assert(lp_viewstream->nameFreeArea);
            strcpy(lp_viewstream->nameFreeArea, baseName);
            strcat(lp_viewstream->nameFreeArea, ".fa");
            
        }
        if (!ifCreate && access(lp_viewstream->nameFreeArea, R_OK | W_OK) != 0)
        {                       /* file not exist */

            return EGG_FALSE;
        }
        if (!(lp_viewstream->freeAreaFile
              = EggFile_open(lp_viewstream->nameFreeArea)))
        {
            /* fprintf(stderr, "%s:%d:%s ERR EggFile_open[%s]\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         lp_viewstream->nameFreeArea); */
            eggPrtLog_error("ViewStream", "%s:%d:%s ERR EggFile_open[%s]\n",
                    __FILE__, __LINE__, __func__,
                    lp_viewstream->nameFreeArea);
            lp_viewstream->freeAreaFile = 0;
    
            return EGG_FALSE;
        }
        if ((lp_viewstream->szFreeArea
             = EggFile_size(lp_viewstream->freeAreaFile))
            < FREEAREASIZE)
        {
            if (lp_viewstream->szFreeArea > 0)
            {
                /* fprintf(stderr, "%s:%d:%s WARN %s is inconsistent\n", */
                /*         __FILE__, __LINE__, __func__, */
                /*         lp_viewstream->nameFreeArea); */
                eggPrtLog_warn("ViewStream", "%s:%d:%s WARN %s is inconsistent\n",
                        __FILE__, __LINE__, __func__,
                        lp_viewstream->nameFreeArea);
            }
            lp_viewstream->szFreeArea = FREEAREASIZE;
            freeAreaInfo = (FreeAreaInfo *)calloc(1, lp_viewstream->szFreeArea);
            assert(freeAreaInfo);
            lp_viewstream->pFreeArea = freeAreaInfo;
            freeAreaInfo->size = FREEAREASIZE;
            SetConsistentStatus(freeAreaInfo->flag);
            freeAreaInfo->nelem = 0;
            freeAreaInfo->mergeInterval = 0;

            if (dumpFreeArea(hViewStream, NULL) < 0)
            {
                /* fprintf(stderr, "%s:%d:%s ERR dumpFreeArea < 0.\n", */
                /*         __FILE__, __LINE__, __func__); */
                eggPrtLog_error("ViewStream", "%s:%d:%s ERR dumpFreeArea < 0.\n",
                        __FILE__, __LINE__, __func__);

            }
                        
            return EGG_TRUE;
        }
        lp_viewstream->pFreeArea = calloc(1, lp_viewstream->szFreeArea);
        assert(lp_viewstream->pFreeArea);

        if (restoreFreeArea(hViewStream) < 0
            || !TestConsistentStatus(
                ((FreeAreaInfo *)lp_viewstream->pFreeArea)->flag))
        {
            /* fprintf(stderr, */
            /*         "%s:%d:%s ERR %s is inconsistent. All free block lost.\n", */
            /*         __FILE__, __LINE__, __func__, lp_viewstream->nameFreeArea); */
            eggPrtLog_error("ViewStream",
                    "%s:%d:%s ERR %s is inconsistent. All free block lost.\n",
                    __FILE__, __LINE__, __func__, lp_viewstream->nameFreeArea);
            lp_viewstream->szFreeArea = FREEAREASIZE;
            lp_viewstream->pFreeArea = realloc(lp_viewstream->pFreeArea,
                                               lp_viewstream->szFreeArea);
            assert(lp_viewstream->pFreeArea);
            freeAreaInfo = (FreeAreaInfo *)lp_viewstream->pFreeArea;
            freeAreaInfo->size = FREEAREASIZE;
            SetConsistentStatus(freeAreaInfo->flag);
            freeAreaInfo->nelem = 0;
            freeAreaInfo->mergeInterval = 0;
        }
        
        if (dumpFreeArea(hViewStream, NULL) < 0)
        {
            /* fprintf(stderr, "%s:%d:%s ERR dumpFreeArea < 0.\n", */
            /*         __FILE__, __LINE__, __func__); */
            eggPrtLog_error("ViewStream", "%s:%d:%s ERR dumpFreeArea < 0.\n",
                    __FILE__, __LINE__, __func__);

        }
    }

    assert(TestConsistentStatus(
               ((FreeAreaInfo *)lp_viewstream->pFreeArea)->flag));

    freeAreaInfo = (FreeAreaInfo *)lp_viewstream->pFreeArea;
    if (freeAreaInfo->size < FREEAREAMAX
        && freeAreaInfo->size == (sizeof(FreeAreaInfo)
                                  + freeAreaInfo->nelem * sizeof(eggPosLen)))
    {                           /* full */
        freeAreaInfo->size += FREEAREASIZE;
        freeAreaInfo = realloc(freeAreaInfo, freeAreaInfo->size);
        assert(freeAreaInfo);
        lp_viewstream->pFreeArea = freeAreaInfo;
        lp_viewstream->szFreeArea = freeAreaInfo->size;
    }

//gettimeofday(&end, NULL);
//fprintf(stderr, "-------------%s %f\n", __func__, end.tv_sec-start.tv_sec+(end.tv_usec-start.tv_usec)/1000000.);

    return EGG_TRUE;
}
static int const MERGE_AREA_THRESHOLD = 10000;
PRIVATE void FreeAreaInfo_mergeArea(FreeAreaInfo *freeAreaInfo)
{
//struct timeval start, end;
//gettimeofday(&start, NULL);
    
    eggPosLen *p = (eggPosLen *)(freeAreaInfo+1);
    size32_t len = freeAreaInfo->nelem;
    Uti_sedgesort(p,
                  freeAreaInfo->nelem,
                  sizeof(eggPosLen),
                  comparePosLen_byPos2); /* asc order */
    
    int i = 1;
    int j = 0;
    while (i < len)
    {
        if (p[i].pos == p[j].pos + p[j].len)
        {
            p[j].len += p[i].len;
            freeAreaInfo->nelem--;
            i++;
        }
        else
        {
            p[++j] = p[i++];
        }
    }
    Uti_sedgesort(p,
                  freeAreaInfo->nelem,
                  sizeof(eggPosLen),
                  comparePosLen_byLen2); /* asc order */
//gettimeofday(&end, NULL);
//fprintf(stderr, "-------------%s %f\n", __func__, end.tv_sec-start.tv_sec+(end.tv_usec-start.tv_usec)/1000000.);
}
PUBLIC EBOOL ViewStream_test_freeaddress(HVIEWSTREAM hViewStream, 
					offset64_t nOffset)
{
	VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
	if (!lp_viewstream || !lp_viewstream->pFreeArea)
	{
		return EGG_FALSE;	
	}
	FreeAreaInfo *freeAreaInfo = (FreeAreaInfo *)lp_viewstream->pFreeArea;
	if (!TestConsistentStatus(freeAreaInfo->flag))
	{
		return EGG_FALSE;
	}
        eggPosLen *p;
        eggPosLen *endp;
        endp = (eggPosLen *)(freeAreaInfo+1) + freeAreaInfo->nelem;

        for (p = (eggPosLen*)(freeAreaInfo+1); p < endp; p++)
        {
            if (p->pos <= nOffset && nOffset < p->pos + p->len)
		return EGG_TRUE;
        }
	return EGG_FALSE;
}
	
PRIVATE EBOOL FreeAreaInfo_add(FreeAreaInfo *freeAreaInfo,
                               offset64_t nOffset, size32_t nSize)
{
    if (1)
    {                           /* check double free */
        eggPosLen *p;
        eggPosLen *endp;
        endp = (eggPosLen *)(freeAreaInfo+1) + freeAreaInfo->nelem;

        for (p = (eggPosLen*)(freeAreaInfo+1); p < endp; p++)
        {
            assert (p->pos >= nOffset + nSize
                    || nOffset >= p->pos + p->len);
        }
    }
    SetInconsistentStatus(freeAreaInfo->flag);
    
    int isFull = 0;
    if (freeAreaInfo->mergeInterval >= MERGE_AREA_THRESHOLD)
    {
        FreeAreaInfo_mergeArea(freeAreaInfo);
        freeAreaInfo->mergeInterval = 0;
    }
    if (freeAreaInfo->size
        == sizeof(FreeAreaInfo) + freeAreaInfo->nelem * sizeof(eggPosLen))
    {
        isFull = 1;
        freeAreaInfo->mergeInterval += 20; /* merge factor */
    }
    else if (freeAreaInfo->size * .8
             <= (sizeof(FreeAreaInfo)
                      + freeAreaInfo->nelem * sizeof(eggPosLen)))
    {
        freeAreaInfo->mergeInterval += 1; /* merge factor */
    }
    if (isFull)
    {
        eggDebug_printf(stdout, "%s:%d: freeArea size(%llu) is full, ignore small block.\n",
               __FILE__, __LINE__, (long long unsigned)freeAreaInfo->size);
        eggPrtLog_info("ViewStream", "%s:%d: freeArea size(%llu) is full, ignore small block.\n",
               __FILE__, __LINE__, (long long unsigned)freeAreaInfo->size);
        eggPosLen *pEggPosLen;
        pEggPosLen = (eggPosLen *)(freeAreaInfo + 1);
        if (pEggPosLen->len >= nSize)
        {
            SetConsistentStatus(freeAreaInfo->flag);            
            return EGG_TRUE;
        }
        memmove(&pEggPosLen[0],
                &pEggPosLen[1],
                (freeAreaInfo->nelem - 1) * sizeof(eggPosLen));
        freeAreaInfo->nelem--;
    }
    eggPosLen pl;
    pl.pos = nOffset;
    pl.len = nSize;
    eggPosLen *p = NULL;
    p = Uti_binary_searchleft(&pl, (freeAreaInfo+1),
                              freeAreaInfo->nelem,
                              sizeof(eggPosLen),
                              comparePosLen_byLen);
    char *endp = (char *)(freeAreaInfo+1)
        + sizeof(eggPosLen) * freeAreaInfo->nelem;
    memmove(&p[1], &p[0], endp - (char *)&p[0]);
    p[0] = pl;
    freeAreaInfo->nelem++;
    SetConsistentStatus(freeAreaInfo->flag);
    return EGG_TRUE;
}
PRIVATE eggPosLen FreeAreaInfo_remove(FreeAreaInfo *freeAreaInfo,
                                      size32_t nSizeAlign)
{
    
    SetInconsistentStatus(freeAreaInfo->flag);
    
    if (freeAreaInfo->mergeInterval >= MERGE_AREA_THRESHOLD)
    {
        FreeAreaInfo_mergeArea(freeAreaInfo);
        freeAreaInfo->mergeInterval = 0;
    }
    
    eggPosLen pl = {};
    pl.len = nSizeAlign;
    eggPosLen *p;
    p = Uti_binary_searchleft(&pl, (freeAreaInfo+1),
                              freeAreaInfo->nelem,
                              sizeof(eggPosLen),
                              comparePosLen_byLen);
    char *endp;
    endp = (char*)(freeAreaInfo+1) + freeAreaInfo->nelem * sizeof(eggPosLen);
    if ((void*)p == (void*)endp)
    {
        freeAreaInfo->mergeInterval += 1; /* merge factor */
        SetConsistentStatus(freeAreaInfo->flag);

        pl.pos = pl.len = 0;
        return pl;
    }
    
    assert(p[0].len >= nSizeAlign);
    pl = p[0];
    memmove(&p[0], &p[1],
            endp - (char *)&p[1]);
    freeAreaInfo->nelem--;

    if (pl.len > nSizeAlign)
    {                       /* insert */
        eggPosLen item;
        item.pos = pl.pos + nSizeAlign;
        item.len = pl.len - nSizeAlign;
        pl.len = nSizeAlign;
        endp = (char *)(freeAreaInfo+1)
            + sizeof(eggPosLen) * freeAreaInfo->nelem;
        char *posInsert;
        posInsert = Uti_binary_searchright(&item, (freeAreaInfo+1),
                                           freeAreaInfo->nelem,
                                           sizeof(eggPosLen),
                                           comparePosLen_byLen);
        memmove(posInsert + sizeof(eggPosLen),  posInsert, endp - posInsert);
        *(eggPosLen *)posInsert = item;
        freeAreaInfo->nelem++;
    }

    SetConsistentStatus(freeAreaInfo->flag);
    
    if (1)
    {
        eggPosLen *p;
        eggPosLen *endp;
        endp = (eggPosLen *)(freeAreaInfo+1) + freeAreaInfo->nelem;

        int i = 0;
        for (p = (eggPosLen *)(freeAreaInfo+1); p < endp; p++)
        {
            assert(p->pos != pl.pos);
        }
    }

    return pl;
}
PUBLIC EBOOL ViewStream_free_area(HVIEWSTREAM hViewStream, offset64_t nOffset, size32_t nSize)
{
    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    eggPosLen pl;
    pl.pos = nOffset;
    pl.len = nSize;
    addToSuspend(&lp_viewstream->suspendFree, &pl);

    
    return EGG_TRUE;
}
PUBLIC EBOOL ViewStream_dofree_area(HVIEWSTREAM hViewStream, offset64_t nOffset, size32_t nSize)
{
    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    if (nSize == 0)
        return EGG_FALSE;

    pthread_mutex_lock(&s_freearea_mutex);
    EggFile_lock_wr_wait(lp_viewstream->hEggFile, SEEK_SET, 0, 0);
    
    if (ViewStream_initFreeArea(hViewStream, 1) == EGG_FALSE)
    {
	pthread_mutex_unlock(&s_freearea_mutex);
        EggFile_unlock(lp_viewstream->hEggFile, SEEK_SET, 0, 0);
        return EGG_FALSE;
    }
    pthread_mutex_unlock(&s_freearea_mutex);    
    EggFile_unlock(lp_viewstream->hEggFile, SEEK_SET, 0, 0);        

    
    nSize = alignSize(nSize);

    FreeAreaInfo_add((FreeAreaInfo *)lp_viewstream->pFreeArea,
                     nOffset, nSize);
    
    return EGG_TRUE;
}
PRIVATE offset64_t ViewStream_getArea(HVIEWSTREAM hViewStream, size32_t nSize)
{
    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    if (nSize == 0)
        return 0;

    pthread_mutex_lock(&s_freearea_mutex);
    EggFile_lock_wr_wait(lp_viewstream->hEggFile, SEEK_SET, 0, 0);
    
    if (ViewStream_initFreeArea(hViewStream, 0) == EGG_FALSE)
    {
     pthread_mutex_unlock(&s_freearea_mutex);
        EggFile_unlock(lp_viewstream->hEggFile, SEEK_SET, 0, 0);
        
        return 0;
    }
     pthread_mutex_unlock(&s_freearea_mutex);    
    EggFile_unlock(lp_viewstream->hEggFile, SEEK_SET, 0, 0);    
    
    FreeAreaInfo *freeAreaInfo;
    freeAreaInfo = (FreeAreaInfo *)lp_viewstream->pFreeArea;
    size32_t nSizeAlign;
    nSizeAlign = alignSize(nSize);
    eggPosLen pl;
    pl = FreeAreaInfo_remove(freeAreaInfo, nSizeAlign);
    
    return pl.pos;
}

PRIVATE int restoreFreeArea(HVIEWSTREAM hViewStream)
{
    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
    
    if (!lp_viewstream->freeAreaFile)
    {
        return -1;
    }
    
    size64_t fileSize;
    fileSize = EggFile_size(lp_viewstream->freeAreaFile);
    if (fileSize > lp_viewstream->szFreeArea)
    {
        lp_viewstream->szFreeArea = fileSize;
        lp_viewstream->pFreeArea = realloc(lp_viewstream->pFreeArea,
                                           lp_viewstream->szFreeArea);
        assert(lp_viewstream->pFreeArea);
        ((FreeAreaInfo *)lp_viewstream->pFreeArea)->size = fileSize;
    }
    if (lp_viewstream->szFreeArea < FREEAREASIZE)
    {
        return -1;
    }
    

    FreeAreaInfo *freeAreaInfo = (FreeAreaInfo *)lp_viewstream->pFreeArea;
    
    if (EggFile_read(lp_viewstream->freeAreaFile,
                     freeAreaInfo, sizeof(FreeAreaInfo), 0) == EGG_FALSE)
    {
        goto err;
    }

    if (!TestConsistentStatus(freeAreaInfo->flag))
    {
        /* fprintf(stderr, "%s:%d:%s ERR %s is inconsistent\n", */
        /*         __FILE__, __LINE__, __func__, */
        /*         lp_viewstream->nameFreeArea); */
        eggPrtLog_error("ViewStream", "%s:%d:%s ERR %s is inconsistent\n",
                __FILE__, __LINE__, __func__,
                lp_viewstream->nameFreeArea);
        goto err;
    }

    if (EggFile_read(lp_viewstream->freeAreaFile,
                     freeAreaInfo+1,
                     freeAreaInfo->nelem * sizeof(eggPosLen),
                     sizeof(FreeAreaInfo))
        == EGG_FALSE)
    {
        goto err;
    }

//gettimeofday(&end, NULL);
//fprintf(stderr, "-------------%s %f\n", __func__, end.tv_sec-start.tv_sec+(end.tv_usec-start.tv_usec)/1000000.);
    return 0;
err:
    /* fprintf(stderr, "%s:%d:%s WARN FreeArea Information lost.\n", */
    /*         __FILE__, __LINE__, __func__); */
    eggPrtLog_warn("ViewStream", "%s:%d:%s WARN FreeArea Information lost.\n",
            __FILE__, __LINE__, __func__);

    SetInconsistentStatus(freeAreaInfo->flag);    
    return -1;

}
PRIVATE int dumpAndMergeFreeArea(HVIEWSTREAM hViewStream)
{
    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
    if (!lp_viewstream->pFreeArea)
    {
        return 0;
    }

    pthread_mutex_lock(&s_freearea_mutex);
    
    EggFile_lock_wr_wait(lp_viewstream->hEggFile, SEEK_SET, 0, 0);

    size64_t fileSize;
    fileSize = EggFile_size(lp_viewstream->freeAreaFile);
    FreeAreaInfo *diskFreeAreaInfo = NULL;
    if (fileSize >= FREEAREASIZE)
    {
        diskFreeAreaInfo = malloc(fileSize);
        assert(diskFreeAreaInfo);
        if (EggFile_read(lp_viewstream->freeAreaFile,
                         diskFreeAreaInfo,
                         fileSize, 0) == EGG_FALSE)
        {
            /* fprintf(stderr, "%s:%d:%s ERR EggFile_read %s\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         lp_viewstream->nameFreeArea); */
            eggPrtLog_error("ViewStream", "%s:%d:%s ERR EggFile_read %s\n",
                    __FILE__, __LINE__, __func__,
                    lp_viewstream->nameFreeArea);
            free(diskFreeAreaInfo);
            diskFreeAreaInfo = NULL;
            fileSize = 0;
        }
        if (diskFreeAreaInfo && !TestConsistentStatus(diskFreeAreaInfo->flag))
        {
            /* fprintf(stderr, "%s:%d:%s ERR %s is inconsistent\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         lp_viewstream->nameFreeArea); */
            eggPrtLog_error("ViewStream", "%s:%d:%s ERR %s is inconsistent\n",
                    __FILE__, __LINE__, __func__,
                    lp_viewstream->nameFreeArea);
            free(diskFreeAreaInfo);
            diskFreeAreaInfo = NULL;
            fileSize = 0;
        }
    }
    else
    {
        fileSize = 0;
    }
    
    if (lp_viewstream->szFreeArea < fileSize)
    {
        lp_viewstream->szFreeArea = fileSize;
        lp_viewstream->pFreeArea = realloc(lp_viewstream->pFreeArea,
                                           fileSize);
        assert(lp_viewstream->pFreeArea);
        ((FreeAreaInfo *)lp_viewstream->pFreeArea)->size = fileSize;
    }
    FreeAreaInfo *freeAreaInfo = (FreeAreaInfo *)lp_viewstream->pFreeArea;
    if (diskFreeAreaInfo)
    {
        eggPosLen *p;
        p = (eggPosLen*)(diskFreeAreaInfo+1);
        uint64_t i;
        for (i = 0; i < diskFreeAreaInfo->nelem; i++)
        {
            FreeAreaInfo_add(freeAreaInfo, p[i].pos, p[i].len);
        }
        free(diskFreeAreaInfo);
    }
    if (dumpFreeArea(hViewStream, freeAreaInfo) < 0)
    {
        /* fprintf(stderr, "%s:%d:%s ERR dumpFreeArea < 0.\n", */
        /*         __FILE__, __LINE__, __func__); */
        eggPrtLog_error("ViewStream", "%s:%d:%s ERR dumpFreeArea < 0.\n",
                __FILE__, __LINE__, __func__);

    }
    
    EggFile_unlock(lp_viewstream->hEggFile, SEEK_SET, 0, 0);
    pthread_mutex_unlock(&s_freearea_mutex);    
    return 0;
}
PRIVATE int dumpFreeArea(HVIEWSTREAM hViewStream, FreeAreaInfo *freeAreaInfo)
{
//struct timeval start, end;
//gettimeofday(&start, NULL);
    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
    
    if (!freeAreaInfo)
    {
        FreeAreaInfo tmpFreeAreaInfo;
        initFreeAreaInfo(&tmpFreeAreaInfo);
        uint64_t off;
        for (off = 0; off + sizeof(FreeAreaInfo) < FREEAREASIZE;)
        {
            if (EggFile_write(lp_viewstream->freeAreaFile,
                              &tmpFreeAreaInfo,
                              sizeof(FreeAreaInfo),
                              off) == EGG_FALSE)
            {
                /* fprintf(stderr, "%s:%d:%s ERR EggFile_write == EGG_FALSE\n", */
                /*         __FILE__, __LINE__, __func__); */
                eggPrtLog_error("ViewStream", "%s:%d:%s ERR EggFile_write == EGG_FALSE\n",
                        __FILE__, __LINE__, __func__);
                return -1;
            }
            off += sizeof(FreeAreaInfo);
        }

        if (EggFile_write(lp_viewstream->freeAreaFile,
                          &tmpFreeAreaInfo,
                          FREEAREASIZE - off,
                          off) == EGG_FALSE)
        {
            /* fprintf(stderr, "%s:%d:%s ERR EggFile_write == EGG_FALSE\n", */
            /*         __FILE__, __LINE__, __func__); */
            eggPrtLog_error("ViewStream", "%s:%d:%s ERR EggFile_write == EGG_FALSE\n",
                    __FILE__, __LINE__, __func__);
            return -1;
        }
        return 0;
    }
    
    if (!TestConsistentStatus(freeAreaInfo->flag))
    {
        /* fprintf(stderr, "%s:%d:%s ERR FreeArea is inconsistent.\n", */
        /*         __FILE__, __LINE__, __func__); */
        eggPrtLog_error("ViewStream", "%s:%d:%s ERR FreeArea is inconsistent.\n",
                __FILE__, __LINE__, __func__);

        return -1;
    }
    size64_t fileSize = EggFile_size(lp_viewstream->freeAreaFile);
    size64_t n;
    if (fileSize < freeAreaInfo->size)
    {
        n = freeAreaInfo->size;
    }
    else
    {
        n = sizeof(FreeAreaInfo)
            + freeAreaInfo->nelem * sizeof(eggPosLen);
    }
    if (EggFile_write(lp_viewstream->freeAreaFile, freeAreaInfo, n, 0)
        == EGG_FALSE)
    {
        /* fprintf(stderr, "%s:%d:%s ERR EggFile_write: %s\n", */
        /*         __FILE__, __LINE__, __func__, lp_viewstream->nameFreeArea); */
        eggPrtLog_error("ViewStream", "%s:%d:%s ERR EggFile_write: %s\n",
                __FILE__, __LINE__, __func__, lp_viewstream->nameFreeArea);

        return -1;
        
    }
    
    if (fileSize < freeAreaInfo->size)
    {
        fsync(((struct tagEggFile *)lp_viewstream->freeAreaFile)->hFile);
    }
    else
    {
        fdatasync(((struct tagEggFile *)lp_viewstream->freeAreaFile)->hFile);
    }
    
//gettimeofday(&end, NULL);
//fprintf(stderr, "-------------%s %f\n", __func__, end.tv_sec-start.tv_sec+(end.tv_usec-start.tv_usec)/1000000.);
    
    return 0;
err:
    return -1;
}

int addToSuspend(eggPosLen **head, eggPosLen *pl)
{
    if (!*head)
    {
        *head = malloc(sizeof(eggPosLen) * 21);
        assert(*head);
        (*head)->pos = 0;       /* current num of elements */
        (*head)->len = 20;      /* total capability */
    }
    else if ((*head)->pos == (*head)->len)
    {
        *head = realloc(*head,
                        sizeof(eggPosLen) * (20 + (*head)->len + 1));
        assert(*head);
        (*head)->len += 20;
    }
    eggPosLen *data = *head + 1;
    data[(*head)->pos] = *pl;
    (*head)->pos++;
    return 0;
}

PUBLIC epointer  ViewStream_location(HVIEWSTREAM hViewStream,  size32_t nSize, offset64_t nOffset)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }
    
    if(nOffset == -1 || nOffset > EggFile_size(ViewStream_object(hViewStream)->hEggFile) )
    {
          return EGG_FALSE;
    }
    ViewStream_modify_map(hViewStream, nSize, nOffset);
    
    offset32_t off32 = LOLONGLONG(nOffset - Cluster_owner_offset((HCLUSTER)(ViewStream_object(hViewStream)->mapView)));

    return (echar*)(ViewStream_object(hViewStream)->mapView) + off32;
                                               
}


PUBLIC EBOOL ViewStream_desired_access(HVIEWSTREAM hViewStream, type_t nDesiredAccess)
{
    if (ViewStream_is_object(hViewStream))
    {
        s1_DesiredAccess = nDesiredAccess;
        return EGG_TRUE;
    }
    
    return EGG_FALSE;
}


PUBLIC EBOOL ViewStream_check_offset(HVIEWSTREAM hViewStream, offset64_t off)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    VIEWSTREAM* lp_viewstream = ViewStream_object(hViewStream);
    size64_t nFileSize;
    CLUSTER cluster;
    
    nFileSize = EggFile_size(lp_viewstream->hEggFile);
    
    if(nFileSize <= MAP_VIEW_OFFSET || nFileSize <= off)
    {
        return EGG_FALSE;
    }
    else
    {
        offset64_t n_base_offset = Uti_base_offset(off-0x100,
                                                   MAP_VIEW_OFFSET,
                                                   CLUSTER_ALLOC_SIZE);
        EggFile_read(ViewStream_object(hViewStream)->hEggFile,
                     &cluster,
                     sizeof(cluster),
                     n_base_offset);
        if(cluster.head.owner != n_base_offset)
        {
//	  printf("cluster.head.owner %llu != n_base_offset %llu\n", (long long unsigned)cluster.head.owner, (long long unsigned) n_base_offset);
            eggPrtLog_error("ViewStream", "cluster.head.owner %llu != n_base_offset %llu\n", (long long unsigned)cluster.head.owner, (long long unsigned) n_base_offset);
            return EGG_FALSE;
        }
        if(cluster.head.used + sizeof(CLUSTERHEAD) > off - n_base_offset)
        {
            return EGG_TRUE;
        }
        else
        {
            return EGG_FALSE;
        }
    }
}



PRIVATE vstream_t* ViewStream_mapping_alloc(HVIEWSTREAM hViewStream, offset64_t nStartOff, size32_t nSize)
{
#ifdef WIN32    
    DWORD dw_offset_low = LOLONGLONG(nStartOff);
    DWORD dw_offset_high = HILONGLONG(nStartOff);
    hViewStream->hMappingFile = ::CreateFileMapping(EggFile_object(hViewStream->hEggFile),
                                                    NULL,
                                                    PAGE_READWRITE,
                                                    0,
                                                    0,
                                                    NULL);

    vstream_t* lp_vstream = (vstream_t*)::MapViewOfFile(hViewStream->hMappingFile,
                                                        FILE_MAP_ALL_ACCESS,
                                                        dw_offset_high,
                                                        dw_offset_low,
                                                        nSize);
#else
    vstream_t* lp_vstream = (vstream_t*)mmap(NULL,
                                             nSize,
                                             PROT_READ|PROT_WRITE,
                                             MAP_SHARED,
                                             EggFile_object(ViewStream_object(hViewStream)->hEggFile)->hFile,
                                             nStartOff);
    
    if (lp_vstream == MAP_FAILED)
    {
        perror("ViewStream_mapping_alloc");
        sleep(5);
        getchar();
        return EGG_NULL;
    }
#endif

    ViewStream_object(hViewStream)->sizeOfMapView = nSize;
    ViewStream_object(hViewStream)->offOfMapView = nStartOff;
    
    return lp_vstream;
}

PRIVATE EBOOL ViewStream_mapping_free(HVIEWSTREAM hViewStream)
{
#ifdef WIN32    
    ::UnmapViewOfFile(hViewStream->mapView);
    ::CloseHandle(hViewStream->hMappingFile);
    hViewStream->hMappingFile = EGG_NULL;
#else
    munmap(ViewStream_object(hViewStream)->mapView,
           ViewStream_object(hViewStream)->sizeOfMapView);
#endif
    ViewStream_object(hViewStream)->mapView = EGG_NULL;

    return EGG_FALSE;
}

PUBLIC EBOOL ViewStream_remap(HVIEWSTREAM hViewStream)
{
    ViewStream_mapping_free(hViewStream);
    ViewStream_object(hViewStream)->mapView = ViewStream_mapping_alloc(hViewStream,
                                                                       MAP_VIEW_OFFSET,
                                                                       CLUSTER_ALLOC_SIZE);

    return EGG_FALSE;
}
PRIVATE EBOOL ViewStream_flush_map(HVIEWSTREAM hViewStream)
{
    return EGG_FALSE;
}

PRIVATE EBOOL ViewStream_modify_map(HVIEWSTREAM hViewStream, size32_t nSize, offset64_t nOffset)
{
    offset64_t n_base_offset = Uti_base_offset(nOffset,
                                               MAP_VIEW_OFFSET,
                                               CLUSTER_ALLOC_SIZE);
    if (n_base_offset != ViewStream_object(hViewStream)->offOfMapView)
    {
        ViewStream_mapping_free(hViewStream);

        ViewStream_object(hViewStream)->mapView = ViewStream_mapping_alloc(hViewStream,
                                                                    n_base_offset,
                                                                    (nOffset > MAP_VIEW_OFFSET)?CLUSTER_ALLOC_SIZE:(nSize + nOffset));
        if ((nOffset - n_base_offset) + nSize > ViewStream_object(hViewStream)->sizeOfMapView)
        {
//            printf("ViewStream_modify_map error !\n");
            eggPrtLog_error("ViewStream", "ViewStream_modify_map error !\n");
            exit(-1);
//            return EGG_FALSE;
        }
    }
    
    return EGG_FALSE;
}


PUBLIC EBOOL ViewStream_startlog(HVIEWSTREAM hViewStream)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }
    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);

    if (lp_viewstream->suspendAlloc)
    {
        /* fprintf(stderr, "%s:%d:%s WARN suspendAlloc not NULL\n", */
        /*         __FILE__, __LINE__, __func__); */
        eggPrtLog_warn("ViewStream", "%s:%d:%s WARN suspendAlloc not NULL\n",
                __FILE__, __LINE__, __func__);        
        free(lp_viewstream->suspendAlloc);
        lp_viewstream->suspendAlloc = NULL;
    }
    if (lp_viewstream->suspendFree)
    {
        /* fprintf(stderr, "%s:%d:%s WARN suspendFree not NULL\n", */
        /*         __FILE__, __LINE__, __func__); */
        eggPrtLog_warn("ViewStream", "%s:%d:%s WARN suspendFree not NULL\n",
                __FILE__, __LINE__, __func__);

        free(lp_viewstream->suspendFree);
        lp_viewstream->suspendFree = NULL;
    }    
    
    EBOOL retv;
    retv = EggFile_startlog(ViewStream_object(hViewStream)->hEggFile);
    return retv;
    
}

PUBLIC EBOOL ViewStream_endlog(HVIEWSTREAM hViewStream)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }
    EBOOL retv;
    retv =  EggFile_endlog(ViewStream_object(hViewStream)->hEggFile);

    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
    if (lp_viewstream->suspendAlloc)
    {
        ;/* do nothing */
        
        free(lp_viewstream->suspendAlloc);
        lp_viewstream->suspendAlloc = NULL;
    }
    if (lp_viewstream->suspendFree)
    {
        int n = lp_viewstream->suspendFree->pos;
        eggPosLen *data = lp_viewstream->suspendFree + 1;
        int i;
        for (i = 0; i < n; i++)
        {
            ViewStream_dofree_area(hViewStream, data[i].pos, data[i].len);
        }   
        
        free(lp_viewstream->suspendFree);
        lp_viewstream->suspendFree = NULL;
    }
    
    return retv;
}    

PUBLIC EBOOL ViewStream_set_actinfo(HVIEWSTREAM hViewStream,
                                   ActInfo *hActInfo)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
    EBOOL retv;
    retv = EggFile_set_actinfo(ViewStream_object(hViewStream)->hEggFile,
                              hActInfo);
    return retv;
    
}
ActInfo *ViewStream_get_actinfo(HVIEWSTREAM hViewStream)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
    return EggFile_get_actinfo(ViewStream_object(hViewStream)->hEggFile);
}

PUBLIC EBOOL ViewStream_clean_actinfo(HVIEWSTREAM hViewStream,
                                     ActInfo *hActInfo)
{
    if (!ViewStream_is_object(hViewStream))
    {
        return EGG_FALSE;
    }

    EBOOL retv;
    retv = EggFile_set_actinfo(ViewStream_object(hViewStream)->hEggFile,
                              NULL);
    
    VIEWSTREAM *lp_viewstream = ViewStream_object(hViewStream);
    if (lp_viewstream->suspendAlloc)
    {
        ;/* do nothing */
        
        free(lp_viewstream->suspendAlloc);
        lp_viewstream->suspendAlloc = NULL;
    }
    if (lp_viewstream->suspendFree)
    {
        int n = lp_viewstream->suspendFree->pos;
        eggPosLen *data = lp_viewstream->suspendFree + 1;
        int i;
        for (i = 0; i < n; i++)
        {
            ViewStream_dofree_area(hViewStream, data[i].pos, data[i].len);
        }   
        
        free(lp_viewstream->suspendFree);
        lp_viewstream->suspendFree = NULL;
    }
    
    return retv;
}

PUBLIC HEGGRECOVERYHANDLE ViewStream_get_recoveryhandle(HVIEWSTREAM hViewStream)
{
    return EggFile_get_recoveryhandle(ViewStream_object(hViewStream)->hEggFile);
}

char *ViewStream_name(HVIEWSTREAM hViewStream)
{
    return EggFile_name(ViewStream_object(hViewStream)->hEggFile);
}


PUBLIC EBOOL ViewStream_xlock(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen)
{
    if (nLen == 0)
    {
        return EGG_TRUE;
    }
    return EggFile_lock_wr_wait(ViewStream_object(hViewStream)->hEggFile,
                              SEEK_SET, off64FileOff, nLen);
}
PUBLIC EBOOL ViewStream_slock(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen)
{
    if (nLen == 0)
    {
        return EGG_TRUE;
    }
    return EggFile_lock_rd_wait(ViewStream_object(hViewStream)->hEggFile,
                              SEEK_SET, off64FileOff, nLen);
    
}
PUBLIC EBOOL ViewStream_unlock(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen)
{
    if (nLen == 0)
    {
        return EGG_TRUE;
    }
    return EggFile_unlock(ViewStream_object(hViewStream)->hEggFile,
                          SEEK_SET, off64FileOff, nLen);
    
}
PUBLIC EBOOL ViewStream_xlock_try(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen)
{
    if (nLen == 0)
    {
        return EGG_TRUE;
    }
    return EggFile_lock_wr_try(ViewStream_object(hViewStream)->hEggFile,
                              SEEK_SET, off64FileOff, nLen);
}
PUBLIC EBOOL ViewStream_slock_try(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen)
{
    if (nLen == 0)
    {
        return EGG_TRUE;
    }
    return EggFile_lock_rd_try(ViewStream_object(hViewStream)->hEggFile,
                              SEEK_SET, off64FileOff, nLen);
    
}
