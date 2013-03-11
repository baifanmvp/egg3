#ifndef _PROJECT_RM_FEATURE_H_
#define _PROJECT_RM_FEATURE_H_


/**
  \page page_requirement_feature 系统特征
  \section sec_feature_functional_requirement 功能性需求
  \subsection ffr_modifiable_field 一、可修改字段
  \par
  针对某篇文章的某字段内容进行修改。若此字段参与分词检索，则需要重新进行分词并检索。
  \par 修改以后的字段分词结果
  
  \subsection ffr_compressiblity 二、可压缩
  \par
  对文章或字段数据进行压缩，减小磁盘空间占有量以及降低IO的使用率。
  \par 对文章进行压缩
  \par 对连续数值压缩
  
  \subsection ffr_time_stamp 三、时间戳
  \par
  Egg中增加时间戳。
  \par
  \subsection ffr_indexable_field 四、可检索的字段
  \par
  对一些特定或部分字段进行检索。相当于在单表结构中，对某一列的数据进行索引。
  区别于通过关键字查找相关内容，字段检索直接定位在某个字段的数值上。可适用类似于“Top 10”。
  \par

  \subsection ffr_hash_table 五、哈希检索
  \par
  在BTree上层添加哈希结构，Egg由单棵树变为树林。HashTable作为一级索引，BTree作为二级索引。
  如果能提供一个较为完善的哈希算法，K-V键值堆就能有效的分布在每单棵树上，可缓解Tree-Locking的压力。
  \par

  \subsection frr_tree_locking_protocol 六、索引树上的锁机制
  \par
  BTree上众多操作（添加、查询、删除、分裂）中，添加和分裂操作会影响BTree的整个结构，
  因此建立完善有效的索引树锁机制不但可以保证存储数据的完整性、树结构的稳定性，而且还要保证并发访问的有效性。
  \par
  
  \subsection ffr_multy_query 七、提供多种查询
  \par
  提供多种查询方式。例如：统配查询、模糊查询、间距查询、并/交查询、表达式查询等。
  \par

  \subsection ffr_xml_schema_api 八、XML接口形式
  \par
  支持XML操作Egg。以XML流的形式访问/控制Egg，用户无需阅读大量的Api操作。
  此外，XML接口不用关心Egg Api是否变化或升级。因此需要建立针对Egg访问的XML协议。
  \par

  \subsection ffr_remote_access 九、远程访问
  \par
  Egg由单机访问转变为异地访问。建立服务端-客户端通讯。
  \par

  \subsection ffr_egg_for_hadoop 十、分布式Egg
  \par
  
  \par
  \subsectin ffr_concurrent_operations 十一、并发操作的控制
  \subsection ffr_transactional_egg 十二、拥有事物机制的Egg
  \par
  
  \par
  
  \section sec_feature_nonfunctional_requirement 非功能性需求
  \subsection fnr_performance
  \subsection fnr_stable
  \subsection fnr_safety
*/

#endif //_PROJECT_RM_FEATURE_H_



