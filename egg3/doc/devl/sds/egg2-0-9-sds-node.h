#ifndef _DOC_EGG2_0_9_RELEASE_NOTE_H_
#define _DOC_EGG2_0_9_RELEASE_NOTE_H_
/**
   \page egg2_0_9_sds_note egg2.0.9 sds note
   \section newFeature  新特性
   \subsection section_distributedstore 数据分组式的分布存储
   \li数据录入时通过某种规格划分成多组，储存在计算机集群上，以便分担单机时的磁盘和处理器的压力\n\n
   \li查询采取多机联合查询的机制，集群上的各个chunk先通过同一个查询要求在本地整理一套符合要求的结果，然后再汇总自客户机进行合并
   
   \section section_jobs jobs 任务安排
   该版本开发任务主要egg的数据分布存储，整个开发由以下几个部分组成\n\n

   \li master \n
   功能: 负责为client提供egg在各个chunk上分布情况(主机地址， 数据划分范围值)\n
   实现: 网络通信直接采用最基本的socket，采用纯字符流明码传输，并发访问采用epoll机制,并且拟定chunks配置格式\n
   负责：徐文刚
   
   \li chunks \n
   功能: 提供单机egg的增删改查功能，为client服务\n
   实现: 网络数据传输依旧采用fastcgi模式，可能会迎合client的实现在接口和功能上进行适当的\n
   负责：白帆

   \li client \n
   功能: 负责联系指定egg集群上的所有chunks， 发起egg各项操作。为chunks分配录入的数据，合并整合chunks传回的搜索结果\n
   实现: api调用流程和egg的local调用和fastcgi调用保持一致，增加一个eggCluster模块（与eggHttp和eggDirectory类似），用于发起对egg集群的操作，每次操作采取多线程的方式同时对各个chunks发起申请， api在接口上会适当调整比如doc id的结构，废弃 eggTopCollector_normalized的功能等（暂定，搜索结果在chunks上直接打分排序等等）\n
   负责：白帆

   \li threadpool \n
   功能: 提供一个线程池，该线程池可用于执行任务、发送工作项、处理异步 I/O、代表其他线程等待以及处理计时器。\n
   实现: \n
   负责：马磊

   \li chunks split\n
   功能: 某个chunk的数据过大，将把数据分化成2个chunk，解决chunk上数据分布不均匀的情况\n
   实现: \n
   负责：白帆(暂定)

*/

#endif
