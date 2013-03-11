#include "./Cluster.h"
#include "./EggDef.h"
#include "./EggError.h"

PUBLIC HCLUSTER Cluster_format(epointer lpBuf, offset64_t nOffset)
{
    if (lpBuf == EGG_NULL)
    {
        return EGG_NULL;
    }
    
    CLUSTERHEAD st_cluster_head = {0};
    st_cluster_head.owner = nOffset;
    st_cluster_head.used = 0;
    
    memcpy(lpBuf, &st_cluster_head, sizeof(CLUSTERHEAD));
    
    
    return (HCLUSTER)lpBuf;
}

PUBLIC EBOOL Cluster_unformat(HCLUSTER hCluster)
{
    return EGG_FALSE;
}

PUBLIC EBOOL Cluster_offset_invalid(HCLUSTER hCluster, offset64_t nOffset)
{
    if (!Cluster_is_object(hCluster))
    {
        return EGG_TRUE;
    }

}

PUBLIC EBOOL Cluster_memcpy(HCLUSTER hCluster,
                            epointer ePointer,
                            size32_t nSize,
                            offset32_t* nOffset,
                            type_t nAccessType)
{
    if (!Cluster_is_object(hCluster))
    {
        return EGG_FALSE;
    }

    
    switch (nAccessType)
    {
    case CLUSTER_ACCESS_READ:
        if (*nOffset >= sizeof(CLUSTERHEAD) 
            && *nOffset + nSize <= sizeof(CLUSTERHEAD) + hCluster->head.used)
        {
            memcpy(ePointer, ((echar*)hCluster) + *nOffset, nSize);

            return EGG_TRUE;
        }
        return ERR_CLUSTER_READ_INVALID;
        
    case CLUSTER_ACCESS_UPDATE:
        if (*nOffset >= sizeof(CLUSTERHEAD)
            && *nOffset + nSize <= sizeof(CLUSTERHEAD) + hCluster->head.used)
        {
            memcpy(((echar*)hCluster) + *nOffset, ePointer, nSize);

            return EGG_TRUE;
        }
        return ERR_CLUSTER_WRITE_INVALID;
        
    case CLUSTER_ACCESS_WRITE:
        if (nSize <= CLUSTER_ALLOC_SIZE - sizeof(CLUSTERHEAD) - hCluster->head.used )
        {

            *nOffset = hCluster->head.used + sizeof(CLUSTERHEAD);
            memcpy(((echar*)hCluster) + *nOffset, ePointer, nSize);
            hCluster->head.used += nSize;
            return EGG_TRUE;
        }
        return ERR_CLUSTER_LESS_MEMORY;
        
    default:
        break;
    }
    
    return EGG_FALSE;
}

PUBLIC EBOOL Cluster_is_object(HCLUSTER hCluster)
{
    if (hCluster == EGG_NULL)
    {
        return EGG_FALSE;
    }

    HCLUSTERHEAD h_cluster_head = (HCLUSTERHEAD)hCluster; // reinterpret_cast<HCLUSTERHEAD>(hCluster);

    
    return h_cluster_head;
}

