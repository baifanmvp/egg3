/**
   \file Cluster.h
   \brief Egg簇存储管理模块。
   模拟FAT32的磁盘存储格式以及动态内存管理的形式对Egg的存储进行有效的划分管理。簇存储有三部分组成：
   \par 1. 簇头(Cluster Head)
   簇头中记录了未使用空间的大小，并以双链表的形式链接前后簇之间的关系。
   \par 2. 存储空间(Space)
   存储空间分为可用空间与已用空间。新记录录入时存放入可用空间中，记录检索是从已用空间中读取；可用空间与已用空间之间由簇尾分割。
   \par 3. 簇尾(Cluster Foot)
   簇尾的作用就在于：检验取值地址是否有效；检验空间是否为有效簇。\n\n
   \image html cluster_overview.png
   \ingroup storage
*/ 
#ifndef _CLUSTER_H_
#define _CLUSTER_H_

#include "../EggDef.h"


E_BEGIN_DECLS


/*!
  \def CLUSTER_ALLOC_SIZE
  \brief 簇空间的分配大小
*/
#define CLUSTER_ALLOC_SIZE  (0x4000000)

/*!
  \def CLUSTER_ACCESS_READ
  \brief 簇空间读操作
*/
#define CLUSTER_ACCESS_READ   0x0001
/*!
  \def CLUSTER_ACCESS_WRITE
  \brief 簇空间写操作
*/
#define CLUSTER_ACCESS_WRITE  0x0002
/*!
  \def CLUSTER_ACCESS_UPDATE
  \brief 簇空间更新操作
*/
#define CLUSTER_ACCESS_UPDATE 0x0004


typedef struct tagCluster CLUSTER;
typedef struct tagCluster* HCLUSTER;

typedef struct tagClusterHead CLUSTERHEAD;
typedef struct tagClusterHead* HCLUSTERHEAD;

typedef struct tagClusterFoot CLUSTERFOOT;
typedef struct tagClusterFoot* HCLUSTERFOOT;

#pragma pack(push)
#pragma pack(4)
struct tagClusterHead
{
    u64 owner;
    u32 tag;
    u32 used;
};
#pragma pack(pop)


#pragma pack(push)
#pragma pack(4)
struct tagClusterFoot
{
    u64 head;
    u32 tag;
    u32 rev;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(4)
struct tagCluster
{
    CLUSTERHEAD head;
};
#pragma pack(pop)

/*!
  \fn HCLUSTER Cluster_format(epointer)
  \brief 对一块无序空间进行格式化。
  \param lpBuf 无序空间
  \return 返回格式化后的簇空间
  \todo
*/
extern HCLUSTER Cluster_format(epointer lpBuf, offset64_t nOffset);

/*!
  \fn EBOOL Cluster_unformat(HCLUSTER)
  \brief 对一块簇空间无序化。
  \param hCluster 簇空间句柄
  \return Rev.
  \todo
*/
extern EBOOL Cluster_unformat(HCLUSTER hCluster);

/*!
  \fn EBOOL Cluster_memcpy(HCLUSTER, epointer, size32_t, offset32_t*, type_t)
  \brief 簇空间拷贝操作
  \param hCluster 簇句柄
  \param ePointer 要操作的指针
  \param nSize 操作的长度
  \param nOffset 在簇空间中的偏移地址
  \param nAccessType 操作类型。\ref CLUSTER_ACCESS_READ \ref CLUSTER_ACCESS_WRITE \ref CLUSTER_ACCESS_UPDATE
  \return Rev.
  \todo
*/
extern EBOOL Cluster_memcpy(HCLUSTER hCluster,
                            epointer ePointer,
                            size32_t nSize,
                            offset32_t* nOffset,
                            type_t nAccessType);

/*!
  \fn EBOOL Cluster_is_object(HCLUSTER)
  \brief 判断对象是否为有效簇。
  \param hCluster
  \return Rev.
  \todo
*/
extern EBOOL Cluster_is_object(HCLUSTER hCluster);



#define Cluster_owner_offset(hCluster) \
  (Cluster_is_object(hCluster)?(((HCLUSTERHEAD)(hCluster))->owner):(u64)0)


#ifdef __cplusplus
#define Cluster_object(_PTR) \
    (reinterpret_cast<HCLUSTER>(_PTR))
#else
#define Cluster_object(_PTR) \
    ((HCLUSTER)_PTR)
#endif


E_END_DECLS

#endif //_CLUSTER_H_

