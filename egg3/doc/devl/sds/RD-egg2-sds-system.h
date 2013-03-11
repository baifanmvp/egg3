#ifndef _PROJECT_SDS_SYSTEM_H_
#define _PROJECT_SDS_SYSTEM_H_


/**
   \page page_design_system 一、系统总体结构
   \section system_overview 1. 系统概述
   
   \section system_structure 2. 系统基础结构
   
   \subsection section_introduce 基本模块简介
   
   \arg eggField :\n\n
   field相当于数据库里的一条字段，比如标题，正文等;\n\n
   一个field取任意的名字，但是必须为字符串。\n\n
   field的类型支持字符串，整型，长整型和双精度类型\n\n
   field可以选择内容是否索引，并且指定不同的分词器\n\n
   field可以选择是否可持续化存储\n\n
   
   \code
   //analyzer
   #define  EGG_NOT_INDEX                           (1 << 0)
   #define  EGG_ANALYZED                            (1 << 1)
   #define  EGG_NOT_ANALYZED                        (1 << 2)
   
   #define  EGG_CWS_ANALYZED                        EGG_ANALYZED
   #define  EGG_CN_ANALYZED                         (1 << 9)
   #define  EGG_CY_ANALYZED                         (1 << 10)
   #define  EGG_OTHER_ANALYZED                      (1 << 11)
   
   //index type
   #define  EGG_INDEX_STRING                    (1 << 3)
   #define  EGG_INDEX_INT32                     (1 << 4)
   #define  EGG_INDEX_INT64                     (1 << 5)
   #define  EGG_INDEX_DOUBLE                    (1 << 6)
   //storage
   #define  EGG_STORAGE                             (1 << 7)
   #define  EGG_NOT_STORAGE                         (1 << 8)


   HEGGFIELD EGGAPI  eggField_new(const echar* lpName,
                            const echar* lpValue, size32_t nSize,
                              type_t type, ...);
   //example
   HEGGFIELD hField1 = eggField_new("content", buf, strlen(buf), EGG_CWS_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
   
   HEGGFIELD hField2 = eggField_new("title", buf, strlen(buf), EGG_OTHER_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE, "ImC2LexAnalyzer");

   HEGGFIELD hField3 = eggField_new("price", &price_num, sizeof(price_num), EGG_NOT_ANALYZED | EGG_INDEX_DOUBLE | EGG_STORAGE);
   
   \endcode

   
   \arg eggDocument:\n\n
   document相当于数据库里的一条记录，是field的关系集合, 比如有一篇文章包括了标题，正文, 作者等字段, 里面的字段就是field,和起来组成的文章就是document
   \code
   
   EBOOL EGGAPI eggDocument_add(HEGGDOCUMENT hEggDocument, HEGGFIELD hEggField);
   
   EBOOL EGGAPI eggDocument_set_weight(HEGGDOCUMENT hEggDocument, float weight);
   
   HEGGFIELD EGGAPI eggDocument_removeField_byName(HEGGDOCUMENT hEggDocument, const echar* lpNameField);

   //add field
   HEGGDOCUMENT hDocument = eggDocument_new();
   
   HEGGFIELD hField1 = eggField_new(fieldname, buf, strlen(buf), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE, "ImCwsAnalyzer");
   HEGGFIELD hField2 = eggField_new(fieldname, buf, strlen(buf), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE, "ImCwsAnalyzer");
   
   eggDocument_add(hDocument, hField1);
   eggDocument_add(hDocument, hField2);
   
   //remove field
   
   HEGGFIELD hField = eggDocument_removeField_byName(hEggDocument, lpNameField);
   
   eggField_delete(hEggField);
   
   \endcode

   \arg eggQuery :\n\n
   eggQuery是个复杂化查询的载体, 专为实现复杂的逻辑化("与或")多域联合查询;\n\n
   一次 eggQuery_new_*产生的query句柄仅可能对单一的field进行多关键字的索引,且为多关键字'与'查询(结果包括所有关键字)\n\n
   但是多个query可以通过eggQuery_and和eggQuery_or，进行多次组合，生成一个多重查询要求的逻辑语法树，在加载进IndexSearcher里进行多重查找\n\n
   eggQuery_new_*Range还支持基于关键字的范围查询，比如要查询price域里的价格为99－199的document，则可写成eggQuery_new_int32Range("price", 99, 199);\n\n
   \code
   
   HEGGQUERY h1, h2, h3;
   h1 = eggQuery_new_string("time", "2001-01-01 12:00:00", len1, NULL);
   h2 = eggQuery_new_string("body", "num1", len2, ANALYZER_CWSLEX);   
   h3 = eggQuery_new_int32("price", 199);
   h2 = eggQuery_and(h3, h2);
   h1 = eggQuery_or(h2, h1);
   h3 = h2 = 0; //
   
   //h1查出的结果就是 (body:[num1] && price:[199]) || (time:[2001-01-01 12:00:00]) 
   eggQuery_delete(h1);//h2, h3不用delete

   \endcode
   
   \arg eggDirectory和eggHttp:\n\n
   eggDirectory和eggHttp都是指定EGG存放的路径\n\n
   eggDirectory支持本地路径\n\n
   eggHttp支持远程路径("$IP:$PORT/cgi-bin/egg.fcgi?EGG_DIR_PATH=$DIR")\n\n

   \code
   
    HEGGHTTP hEggHttp  = eggHttp_open("localhost:80/cgi-bin/egg.fcgi?EGG_DIR_PATH=/tmp/egg/");
    
    HEGGDIRECTORY hDirectory = eggDirectory_open("/tmp/egg/");

    \endcode
    
   \arg eggIndexWriter, eggIndexReader 和 eggIndexSearcher :\n\n
   都是由eggDirectory和eggHttp延伸出来的EGG文件操作对象\n\n
   eggIndexWriter负责EGG的添加,修改和删除等写操作
   eggIndexReader负责EGG的读操作
   eggIndexSearcher是eggIndexReader的升级版,配合eggQuery可进行复杂的多域 "与或" 联合查询

   \code
      
   HEGGHTTP hEggHttp  = eggHttp_open("localhost:80/cgi-bin/egg.fcgi?EGG_DIR_PATH=/tmp/egg/");
   
   //create IndexWriter
   HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hEggHttp, ANALYZER_CWSLEX);
   
   //create IndexReader
   HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHttp);
   
   //create IndexSearcher
   HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
  
   \endcode

   


   \section system_process 3.系统流程   
   \subsection section_add 添加
      \li 添加流程示意图\n\n
      \image html egg2-add-process.png \n
      
      \li 1. 假设一份网页有3个部分——title(标题)，content(正文)，url(网址)\n\n
      \li 2. 这份网页当成一个doument，其3个部分就是该document的3个域\n\n
      \li 3. egg会将这份document交由eggDocView模块(管理document)进行存储，并为其分配document_id\n\n
      \li 4. 语言解析器会对document各域的内容进行分词（key）\n\n
      \li 5. 所有key的会和document_id组成key-id列表的键值对形式放于(eggIndexCache)缓冲区中；\n\n
      \li 6. 待做optimize（提交）操作是，所有的key将写入index(倒排索引表)，id列表将写入idView（id倒排表）中，并且建立关联\n\n
      
      \subsection section_query 查询
      \li 查询流程示意图\n\n
      \image html egg2-query-process.png \n
      
      \li 1. 假定现在有如下查询要求——查找的document是content里同时key1，key2和key3; 或者是title里同时出现了key1和key2；或者是url里出现了key3\n\n
      \li 2. 将其查询要求按照query模块api的要求填入，将生成一棵与或逻辑关系的语法树；\n\n
      \li 3. 该语法树会被相应的模块所解析，并且到各域所对应的倒排索引表里查找到所需关键字所关联的id列表\n\n
      \li 4.各关键字的id列表会按照与或逻辑关系进行合并，并且可按照多种打分方式进行打分排序\n\n
      \li 5.最后生成一份新的符合查询要求的document_id列表，可通过该id列\n\n
   
   \section file_struct 4.文件结构
   \li egg.dat：存储序列化后的document实体\n\n
   \li egg.dat.idt：管理document的id\n\n
   \li egg.fdd：存放域信息，管理和配置各个域下的倒排表索引（indexView）\n\n
   \li egg.idd：所有的倒排表\n\n
   \li egg.idx：所有的倒排表索引\n\n
   \li egg.rlog：日志数据\n\n
   \li egg.rlog.info：日志操作信息\n\n
   \li *.fa： 存放每个文件废弃数据信息，产生废弃数据时建立\n\n
   
*/

#endif //_PROJECT_SDS_SYSTEM_H_
