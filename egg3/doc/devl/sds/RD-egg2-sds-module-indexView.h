#ifndef _PROJECT_SDS_MODULE_INDEXVIEW_H_
#define _PROJECT_SDS_MODULE_INDEXVIEW_H_


/**
  \page indexView_module 关键字索引模块
  
    \section indexView_overview  概述
    
      关键字索引模块（indexView）是一套基于B＋树的索引机制，通过key－value的流程完成整个查找过程。\n\n
      由于该模块采用的是以B＋树为存储结构的设计思想，因此继承其优良的特性,  可支持海量的数据容量，有着良好的数据延展性， 而且key支持int32（int）， int64（long long） ，double和string四种数据类型，value为非结构化的数据类型，使得该模块用途更加广泛，更加范型化。\n\n
      
    \note B＋树是一种平衡的多路查找树，tree的结构特性使其插入操作比一般的线性数组效率要高的多，而且B＋树自动平衡的特质使其面对超大量的数据时依然能保持着高效的查找效率（仅稍逊于二分查找）。
    
    \section indexView_function 功能
    
    负责存储和索引文档中的key和该文档id之间的关系，建立一个key(doc中的关键字)－>value(包含该关键字的doc的id列表集)的倒排索引关系表(B+树)\n\n
    
    \section indexView_structure  类型结构
    
    \par 多个key-value(EGGINDEXRECORD)组成了B+树上一个节点(EGGINDEXNODEVIEW),多个节点构成该倒排索引表(EGGINDEXVIEW)\n\n\n

    EGGINDEXRECORD为index上的最小记录单元(b+树上的一个record)，用来存储key-value的键值对，其中key可以支持int，long long，double和string类型,value是无结构化的数据类型\n
    
    \code
    
    //EGGINDEXRECORD 记录模块(tree的每一个key-value)
    typedef struct eggIndexRecord  EGGINDEXRECORD;
    typedef struct eggIndexRecord* HEGGINDEXRECORD;

    struct eggIndexRecord
    {
       offset64_t childOff;
       offset64_t hostOff;
    
       size16_t kSz;
       size16_t vSz;
    };
    \endcode\n\n\n

    EGGINDEXNODEVIEW是一个record集，默认数量为32个，该模块相当于b＋树上的一个node，负责该node上的record（key－value键值对）的插入和删除，还有自身的分裂（split），维护着自身层上的record平衡，也直接影响了整个b＋树上的平衡
    
    \code

    //EGGINDEXNODEVIEW 节点模块(tree的每一层)
    typedef struct eggIndexNodeView EGGINDEXNODEVIEW;
    typedef struct eggIndexNodeView* HEGGINDEXNODEVIEW;

    struct eggIndexNodeView
    {
       offset64_t nodeOff;
       HEGGINDEXNODEINFO reInfo;
       HEGGINDEXNODE hNode;
    };
    \endcode\n\n\n

    操控着index上的所有node，使得整个b＋树保持着平衡的状态，有读写文件的权限，负责整个index上可持久化数据的存储
    \code
    //EGGINDEXVIEW 主模块
    typedef struct eggIndexView EGGINDEXVIEW;
    typedef struct eggIndexView* HEGGINDEXVIEW;
    
    struct eggIndexView
    {
       HVIEWSTREAM hViewStream;
       HEGGINDEXINFO hInfo;
       HEGGFIELDVIEW hFieldView;
    };
    
    \endcode\n\n\n

    \section indexView_interaction 对外接口
    
    \code
    //创建IndexView对象
    HEGGINDEXVIEW eggIndexView_new(HEGGFILE hEggFile, HEGGINDEXINFO hInfo);
    \endcode
    \note 创建IndexView对象, hEggFile为b+可持久化数据的文件储存句柄, hInfo为该棵树的一些配置信息(key类型，record的大小，node上record最大数量等等)和rootNode(根节点)信息\n\n

    \code
    //销毁IndexView对象
    EBOOL eggIndexView_delete(HEGGINDEXVIEW hEggIndexView);
    \endcode
    \note 销毁IndexView对象\n\n
    
    \code
    //以key－value的形式插入Index
    EBOOL eggIndexView_insert(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz);
    \endcode
    \note 把key－value键值对插入Index中\n\n

    \code
    //通过key获取对应的record（key－value）
    HEGGINDEXRECORD eggIndexView_fetch(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz);
    \endcode
    \note 通过key获取对应的record\n\n
    
    \code
    //通过key定位对应的record（key－value）在index上的位置
    PUBLIC HEGGINDEXRECORD eggIndexView_locate(HEGGINDEXVIEW hEggIndexView, void* key, size16_t kSz, offset64_t* pNdPos, index_t* pRdIdx);
    \endcode
    \note 通过key定位对应的record（key－value）在index上的位置, pNdPos为所在该key所在的node的文件位置, pRdIdx为在node上的线性数组脚标\n\n
    
    \par EGGINDEXVIEW结构类图
    \image html egg2-indexView-class.png \n
    
    \note
    hViewStream : 该index所在的filestream（文件流）\n\n
    hInfo : b+树根信息\n\n
    hFieldView : B＋树域信息\n\n
    
    \par EGGINDEXVIEW B+tree示意图
    \image html b+.png \n
    
    \note 该图是一棵典型的b＋树（key为整数值，每一个node里面的record阀值是3个，真实际程序里的是32个record），record和node已经在上面标记清楚了，最下层的叶子节点是个按特定顺序排好的list表，这是B＋树和其他平衡B树不同的地方， 这样可以把这棵树上的record按照树指定的顺序依次导出，对树上key的信息统计和以key作为范围的查询提供了极大的便利。\n\n
    
    \section ModuleA_process 流程
    
    一篇document， 通过analysis（分析）会得到若干关键字（key），document的id和其所以的key将建立多个倒排索引的关系，该document id会被加入各个key所对应的倒排索引表（非indexView模块）中，而每一个key和其倒排索引表的表头将组成一个record，加入到IndexView中，完成构建索引过程。

    \image html indexviewprocess.png \n
    
    
  
*/

#endif //_PROJECT_SDS_H_

