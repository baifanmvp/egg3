/**
   \file eggCluster.h
   \brief egg 集群目录 (API)
   \ingroup egg
*/
#ifndef EGG_CLUSTER_H_
#define EGG_CLUSTER_H_

#include "EggDef.h"
#include "./net/eggSpanUnit.h"
#include "./net/eggNetSocket.h"
#include "./cluster/eggClusterCommon.h"

E_BEGIN_DECLS


typedef struct eggCluster EGGCLUSTER;
typedef struct eggCluster* HEGGCLUSTER;


/*!
  \fn HEGGCLUSTER EGGAPI eggCluster_open(const path_t*)
  \brief 目录打开函数
  \param filepath 文件目录字符串
  \return 文件目录句柄
*/
HEGGCLUSTER EGGAPI eggCluster_open(const path_t*);


/*!
  \fn EBOOL EGGAPI eggCluster_close(HEGGCLUSTER)
  \brief 目录关闭函数
  \param hEggCluster  文件目录句柄
  \return
*/
EBOOL EGGAPI eggCluster_close(HEGGCLUSTER hCluster);

count_t EGGAPI eggCluster_get_chunkcnt(HEGGCLUSTER hEggCluster);

HEGGCHUNKHAND EGGAPI eggCluster_get_chunkhands(HEGGCLUSTER hEggCluster, count_t* lpCnt);

PUBLIC HEGGCLUSTER EGGAPI eggCluster_dup(HEGGCLUSTER hClusterOrg);

E_END_DECLS

#endif //EGG_HTTP_H_

