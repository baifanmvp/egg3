#ifndef _DOC_EGG2_2_1_REQM_H_
#define _DOC_EGG2_2_1_REQM_H_

/**
   \page egg2_2_1_reqm egg2.2.1 需求
   
   \section section_introduce 概述
   \li egg集群需要实现读写分离，解决因为写操作对读操作的性能影响
   
   \section section_detail 详细需求
   
   \li Read/Write Splitting Manager, 用于安排读操作和写操作的chunks (也可能在客户端以配置文件的形式体现)
   
   \li cachedb 放在指定的chunk上 处理所有的写操作，定期同步到负责读相应的chunk上
   
   \li 更改id生成的规则

   \li 定期同步机制

   
*/



#endif // _DOC_EGG2_2_1_REQM_H_

