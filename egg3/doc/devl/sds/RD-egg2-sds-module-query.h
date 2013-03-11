#ifndef EGG2_RECOVERY_H
#define EGG2_RECOVERY_H
/**
  \page query_module query查询语法树模块

   \section section_eggquerymodule eggQuery模块
   \subsection eggQuery_introduction 1. Introduction
      文章id形成id列表。

      根据(fieldName, keyword)查得id列表(HEGGIDNODE)。

      根据fieldName得到相应的B+树的索引信息。再用keyword在B+树上查找，得到id列表。

      一个基本查询得到一个id列表。
     复合查询利用eggQuery_and及eggQuery_or进行逻辑操作。
   
   \subsection eggQuery_impl 2. interface & implementation
   复合查询利用如下逻辑操作：
   \code
   HEGGQUERY eggQuery_and(HEGGQUERY Express1, HEGGQUERY Express2);              
   HEGGQUERY eggQuery_or(HEGGQUERY Express1, HEGGQUERY Express2);              
   \endcode
   keyword为字符串，由analyzerName进行分词(NULL则不分词），分词后进行eggQuery_and.
   \code
   HEGGQUERY eggQuery_new_string(const char* fieldName, 
                                 const echar* keyword, size16_t keywordSz, 
                                 char *analyzerName);
   \endcode
   keyword为字符串，由annalyzerName分词器分词后，eggQuery_and各词,得到id列表.过滤id，
   留下这样的文章id, 这些词处于这样文章的一个短语里.
   \code
   HEGGQUERY eggQuery_new_phrase(const char* fieldName, 
                                 const echar* keyword, size16_t keywordSz, 
                                 char *analyzerName);
   \endcode
   keyword为字符串，由analyzerName进行分词(NULL则不分词）。
   当keyword中含"时, "...abc..."作为eggQuery_new_phrase查询，
   其他作为eggQuery_new_string查询，最后eggQuery_and.
   \code
   HEGGQUERY eggQuery_new_sentence(const char* fieldName, 
                                   const echar* keyword, size16_t keywordSz,
                                   char *analyzerName);
   \endcode
   keyword 为数字.
   \code
   HEGGQUERY eggQuery_new_int64(const char* fieldName, int64_t number1);
   \endcode
   在B+树上查找number1至number2之间的id列表.
   \code
   HEGGQUERY eggQuery_new_int64Range(const char* fieldName, 
                                     int64_t number1, int64_t number2);
   \endcode
   由于Query要实现eggQuery_or及eggQuery_and逻辑操作，eggQuery的数据结构为二叉树。
   
   将Query转化为array。实现为二叉树前序遍历。
   \code
   char * eggQuery_serialise(HEGGQUERY hQuery, int *sz);
   HEGGQUERY eggQuery_unserialise(char *query, int sz);
   \endcode
   

   
   
*/
#endif  /*  EGG2_RECOVERY_H */
