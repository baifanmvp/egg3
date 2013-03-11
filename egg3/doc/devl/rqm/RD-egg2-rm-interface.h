#ifndef _PROJECT_RM_INTERFACE_H_
#define _PROJECT_RM_INTERFACE_H_

/**
  \page page_requirement_interface 外部接口
  \section sec_interface_ui 1.用户接口
  \section sec_interface_hardware 2.硬件接口
  \section sec_interface_software 3.软件接口
  \subsection is_build 3.1.环境搭建
  \par 作用
  可通过配置部署Egg。这是启动Egg的准备工作。
  \par 描述
  可通过配置文件或配置程序对EGG进行部署，包括文件所存放的位置。
  \subsection is_initial 3.2.初始化
  \par 作用
  启动Egg。
  \par 描述
  启动EGG时可分为可配置启动方式与无配置启动方式。可配置启动时读取配置信息，加载相应的EGG存储文件；无配置启动时
  就在应用程序的当前目录下建立并加载EGG存储文件。
  \subsection is_final 3.3.终止化
  \par 作用
  关闭Egg。
  \par 描述
  关闭EGG不代表删除EGG的存储文件。只是结束了EGG操作，释放EGG的资源。
  \subsection is_purge 3.4.销毁文件
  \par 作用
  删除磁盘上的Egg存储文件。
  \par 描述
  \subsection is_add 3.5.添加操作
  \par 作用
  往Egg中增加键值对。
  \par 描述
  \subsection is_query 3.6.查询操作
  \par 作用
  通过变长关键字，查询Egg中的数据字段。
  \par 描述
  \subsection is_remove 3.7.删除操作
  \par 作用
  \par 描述
  \subsection is_multi-key_merge 3.8.多关键字合并
  \par 作用
  将多个关键字查询结果归并，提取相同结果并返回。
  \par 描述
  \subsection is_multi-process 3.9.多进程访问
  \par 作用
  控制多进程访问的并发问题。
  \par 描述
  \subsection is_ioput 3.10.导入导出
  \par 作用
  \par 描述
  \subsection is_attribute 3.11.支持属性列表
  \par 作用
  建立“关键字-属性”的对应关系。
  \par 描述
  \subsection is_sort 3.12.支持多种排序
  \par 作用
  支持希尔排序、快速排序、堆排序等多种排序方式。
  \par 描述
*/

#endif //_PROJECT_RM_INTERFACE_H_
