#ifndef _DOC_EGG2_0_5_TUTORIAL_H_
#define _DOC_EGG2_0_5_TUTORIAL_H_


/**
   \page egg2_0_5_tutorial Egg2(<2.0.5)教程
   \section sec_tutorial_overview 一、概述

   \section sec_tutorial_interface 二、接口
   \subsection tutor_if_index 2.1 索引
   \par 2.1.1 IndexWrite 索引写操作
   IndexWrite是Egg2创建索引最主要的接口。在写入索引的过程汇中可以设定是否要缓存。
   \see \ref HINDEXWRITE， \ref Index_Policy
   \par
   \code
   // 创建索引写操作实例，设置索引过程中使用缓存。
   HINDEXWRITE hIndexWrite = egg_IndexWrite_open(egg_Directory_open("/home/eggroot/"), hAnalyzer, CACHE);
   for (int i = 0; i < 100; i++)
   {
      eggDocument* peggDocument = egg_Document_new();
      egg_Document_add(peggDocument, egg_Field_new("test", "value", 5));

      egg_IndexWrite_add_document(hIndexWrite, peggDocument);

      egg_Document_delete(peggDocument);
   }
   ...
   // 将缓存的数据一并写入磁盘中。
   egg_IndexWrite_optimize(hIndexWrite);
   egg_IndexWrite_close(hIndexWrite);
   \endcode
   \par
   若要对IndexWrite进行读操作，可通过以下方式获得HINDEXREADER
   \code
   // 创建hIndexWrite实例
   ...
   HINDEXREADER hIndexReader = egg_IndexWrite_get_reader(hIndexWrite);
   
   \endcode
   
   \par
   Since 2.0.1
   
   \par 2.1.2 IndexReader 索引读操作
   IndexReader相对IndexWrite，只能进行读操作。
   \see \ref HINDEXREADER

   \par
   \code
   HINDEXREADER hIndexReader = egg_IndexReader_open(egg_Directory_open("/home/eggroot/"));
   ...
   HSCOREDOC hScoreDoc = W_NULL; // 变量用于返回分数文章
   count_t nCount = 0;           // 变量用于返回文章个数
   HQUERY hQuery1 = egg_Query_new_string(W_NULL, "标题字段名"， "标题内容", W_FALSE);

   // 通过字段查询返回相应的文章
   egg_IndexReader_query_documents(hIndexReader, hQuery1, &hScoreDoc, &nCount);

   eggDocument* pEggDocument = W_NULL;
   // 通过文章ID，返回文章内容。
   egg_IndexReader_document(hIndexReader, nIdNum, &pEggDocument);

   egg_IndexReader_close(hIndexReader);
   \endcode

   \par
   Since 2.0.1

   \par 2.1.3 FieldIndex 单字段索引
   对一些特定或部分字段进行检索。相当于在单表结构中，对某一列的数据进行索引。
   区别于通过关键字查找相关内容，字段检索直接定位在某个字段的数值上。可适用类似于“Top 10”。
   \see \ref FieldIndex

   \par
   \code
   ...
   // 判断特定的字段索引是否存在
   if (!FieldIndex_index_is_exist(p_field_index, "reply_pub_date"))
   {
        // 设置字段索引配置
        FieldIndexConfig* p_config = FieldIndexConfig_new();
        FieldIndexConfig_set_property_type(p_config, EGG_PROP_VARSTR);

        // create index in FieldIndex
        FieldIndex_create_index(p_field_index, "reply_pub_date", p_config);
        FieldIndexConfig_delete(p_config);
   }
   \endcode

   \par
   \code
   // 利用Property构建变长字符串
   eggVarStrProperty* p_str_property = eggVarStrProperty_new();
   eggVarStrProperty_set(p_str_property, "reply_pub_date", p_document_str);
   ...

   // 加入到字段索引中
   FieldIndex_add_in_index(p_field_index, p_str_property, nId);
   ...
   \endcode
   
   \par
   Since 2.0.4
   
   \subsection tutor_if_search 2.2检索
   \par 2.2.1 IndexSearcher 检索
   在Egg2检索中常用到的“无域查询”、“单域查询”、“多域查询”，都是通过IndexSearcher检索调用。
   \see \ref HINDEXSEARCHER

   \code
   // sample
   HQUERY hQuery = egg_Query_new_string(hAnalyzer, "search keywords");
   HINDEXSEARCHER hIndexSearcher = egg_IndexSearcher_new(hIndexReader);
   HTOPCOLLECTOR hTopCollector = egg_TopCollector_new(10, W_TRUE);
   
   ...
   
   egg_IndexSearcher_search_with_query(hIndexSearcher, hTopCollector, hQuery);
   
   ...
   
   egg_TopCollector_delete(hTopCollector);
   egg_IndexSearcher_delete(hIndexSearcher);
   egg_Query_delete(hQuery);
   \endcode

   \par
   Since 2.0.1
   
   \subsection tutor_if_interactive_data 2.3交互数据
   \par 2.3.1 Field 域
   \par
   Field域是构成文章的基本单位，设置域名及对应的内容。对于域的存储，还可以设定
   是否要进行分词或索引。
   \see \ref HFIELD， \ref Field_Index
   \par
   \code
   // 字段分词并索引
   HFIELD h_title_field = egg_Field_new("标题字段名", "标题内容", strlen("标题内容"), ANANLYZED);
   // 字段不分词但索引
   HFIELD h_author_field = egg_Field_new("作者字段名", "作者内容", strlen("作者内容"), NOT_ANANLYZED);
   // 字段不分词不索引
   HFIELD h_date_field = egg_Field_new("时间字段名", "时间内容", strlen("时间内容"), NOT_ANANLYZED_NOT_INDEX);
   ...
   egg_Field_delete(h_title_field);
   egg_Field_delete(h_author_field);
   egg_Field_delete(h_date_field);
   \endcode
   
   \par
   Since 2.0.1

   \par 2.3.2 Document 文章
   Document文章是有若干个域组成，作为Egg2索引和检索的主要对象。
   \see \ref eggDocument。
   \par
   \code
   eggDocument* pEggDocument = eggDocument_new();

   eggDocument_add(pEggDocument, h_title_field);  // 添加标题域
   eggDocument_add(pEggDocument, h_author_field); // 添加作者域
   eggDocument_add(pEggDocument, h_date_field);   // 添加时间域

   eggDocument_delete(pEggDocument);
   ...
   \endcode

   \par
   Since 2.0.1
   
   \par 2.3.3 ScoreDoc 分数文档
   ScoreDoc分数文档集合用于检索返回的结果，其中包含两个重要的信息，一个是Document文章的ID，一个是文章的分数。
   文章的分数按照紧密度从高到低排序。
   \see \ref HSCOREDOC。
   \par
   \code
   HSCOREDOC hScoreDocs = W_NULL;
   ScoreDoc_new_n(hScoreDoc, 10);

   for (int i = 0; i < 10; i++)
   {
       ScoreDoc_id_i(hScoreDocs, i) = 文章ID;
       ScoreDoc_score_i(hScoreDocs, i) = 文章分数;
       ...
   }

   ScoreDoc_delete(hScoreDocs);
   \endcode

   \par
   Since 2.0.1
   
   \par 2.3.4 Query 查询字段
   Query查询字段是Egg2检索中主要的操作的对象。它针对单一字段进行检索。检索的字段可设置分词器，一般来说，
   为了确保结果的合理性一致性，检索过程中的分词器与索引过程中的分词器应保持一致。
   此外，还可以对关键字组合进行交操作和并操作。
   \see \ref HQUERY
   \par
   \code
   HANALYZER hAnalyzer = Analyzer_new();
   ...
   // 没有并交集表达式的查询字段
   HQUERY hQuery1 = egg_Query_new_string(hAnalyzer, "标题字段名"， "标题内容", W_FALSE);
   HQUERY hQuery2 = egg_Query_new(hAnalyzer, "标题字段名", "%s%s", "标题", "内容");
   ...
   egg_Query_delete(hQuery1);
   egg_Query_delete(hQuery2);
   \endcode

   \par
   \code
   ...
   // 交集表达式的查询字段
   HQUERY hQuery1 = egg_Query_new_string(W_NULL, "标题字段名"， "标题&&内容", W_TRUE);
   ...
   egg_Query_delete(hQuery1);
   \endcode

   \par
   \code
   ...
   // 并集表达式的查询字段
   HQUERY hQuery1 = egg_Query_new_string(W_NULL, "标题字段名"， "标题||内容", W_TRUE);
   ...
   egg_Query_delete(hQuery1);
   \endcode
   
   \par
   Since 2.0.1
   
   \par 2.3.5 MultiQuery 组合查询字段
   MultiQuery组合查询字段（多字段查询）是在Query上进一步的操作。针对多组字段查询，
   每个Query的操作不变。
   \see \ref HMULTIQUERY
   \par
   \code
   HANALYZER hAnalyzer = Analyzer_new();
   ...
   // 设定并集表达式的标题字段
   HQUERY h_title_query = egg_Query_new_string(W_NULL, "标题字段名", "标题||内容", W_TRUE);
   // 需要分词的作者字段
   HQUERY h_author_query = egg_Query_new_string(hAnalyzer, "作者字段名", "作者内容", W_FALSE);
   // 不需分词的时间字段
   HQUERY h_date_query = egg_Query_string(W_NULL, "时间字段名", "时间内容", W_FALE);

   HMULTIQUERY h_multi_query = egg_MultiQuery_new();

   egg_MultiQuery_add(h_multi_query, h_title_query);
   egg_MultiQuery_add(h_multi_query, h_author_query);
   egg_MultiQuery_add(h_multi_query, h_date_query);

   // 由MultiQuery来释放所有的Query
   egg_MultiQuery_delete(h_multi_query);
   \endcode

   \par
   Since 2.0.1
   
   \section sec_tutorial_install 三、安装
   \subsection tutor_install_checkout 3.1 取源代码
   \par 3.1.1 设置CVSROOT
   \code
   export CVSROOT = :pserver:username@cmmi.ape-tech.com:2401/ImRoBot5
   \endcode
   \par 3.1.2 取版本
   目前Egg2有四个版本:
   \arg egg2-0-1
   \arg egg2-0-2
   \arg egg2-0-3
   \arg egg2-trunk （开发版本）
   \par
   \code
   cvs co -d egg2-0-1 -r egg2-0-1 egg2
   \endcode
   \subsection tutor_install_ready_for 3.2 准备工作
   \par
   编译Egg2前，必须满足以下要求
   \arg glib版本要求2.24以上
   \arg 需要安装bzip2
   \arg 需要安装scholar 版本号scholar0-2-patches
   \subsection tutor_install_building 3.3 编译安装
   \code
   make
   sudo make install
   \endcode

   \section sec_tutorial_example 四、范例
   \see \ref page_example
*/




#endif //_DOC_EGG2_TUTORIAL_H_

