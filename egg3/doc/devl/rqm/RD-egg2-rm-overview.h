#ifndef _PROJECT_RM_OVERVIEW_H_
#define _PROJECT_RM_OVERVIEW_H_


/**
  \page page_requirement_overview 需求概况

  \section sec_requirement_overview 1.概况
  
  \section sec_requirement_feature 2.特征
  
  \subsection current_features 2.1 当前版本需求
  
  \subsubsection single_keyword 单关键字检索
  \li 根据提供的关键字，返回id列表。
  \li id列表的排序规则以进入egg的时间排序。
  
  \subsubsection multi_keyword 多关键字结果合并
  \li 根据提供的多个关键字，检索到所有匹配的id列表。
  \li 多键字合并后的顺序根据。
  \li 以时间作为打分因子。
  
  \subsubsection multi_process 多进程访问
  \li 通过文件锁，对索引文件的部分加锁，避免在写入的时候对整个文件加锁。

  \subsubsection realtime_index_modification_merge 实时索引修改与合并
  \li 在任何时候，都可以往索引文件中加入关键字或词。
  \li 在任何时候，都可以往关键字或词对应的id列表中加入id。

  \subsection pending_features 2.1 待实现需求
  \subsubsection multi_sorting_method 多种排序方式
  \subsubsection field 文档域
  \subsubsection query_parser 查询条件 
  
  \section sec_requirement_nonfunctional 3、非功能性需求
*/

#endif //_PROJECT_RM_OVERVIEW_H_

