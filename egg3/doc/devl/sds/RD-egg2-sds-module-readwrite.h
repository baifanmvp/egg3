#ifndef _PROJECT_SDS_MODULE_READWRITESPLITTING_H_
#define _PROJECT_SDS_MODULE_READWRITESPLITTING_H_


/**
  \page readWriteSplitting_module 读写分离设计
  
    \section ReadWriteSplitting_overview  概述

    该Read/Write Splitting机制由1个W-single（写机器）和多个R-cluster（读机器）组成

    W-single接收单天的最新数据，每天定期同步到多个R-cluster上，

    r-cluster接收W-single发过来的数据
    
    W-single负责响应查询申请，每次查询选择合适的r-cluster做数据源

    W-single为单机，index在内存里，document实体在硬盘上，R-cluster由原来的cluster集群改编而成


    用W-single／R-cluster读写数据的3种情况

    情况1： 正在写入数据（数据时间为0105），但未做optimize。写响应index建立在内存里，读响应W-single会负责选择一个R-cluster（1234都可以，因为数据一样），把本机内存里当天的数据和该R-cluster里的数据一并返回

  
    \image html wirteandread.png \n
    
    
    情况2：W-single开始向R-cluster进行optimize。W-single只会选择部分R-cluster进行optimize，这里选择的是1，2号R-cluster，这时候如果有读操作申请，W-single会从没做optimize的R-cluster（这里是3，4号）里面选择一个做数据源， 把本机内存里当天的数据和该R-cluster里的数据一并返回，保证数据实时性。
    
    \image html optimizeandread.png \n
    
    情况3：W-single的optimize结束，开始写入新数据，这时候1，2号R-cluster上有0105那天的新数据，因此1，2号R- cluster负责将包含新数据的那个egg文件拷贝到3，4号R-cluster上（不走egg程序，脚本启动，直接文件传输，用scp等），期间若有读响应，W-single会选择1，2号R-cluster做数据源，待3，4号R-cluster接收完毕以后，自动重启各自的egg服务，重新加载 egg文件。

    \image html synandread.png \n
    

    
*/

#endif //_PROJECT_SDS_H_

