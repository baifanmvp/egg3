#ifndef _DOC_EGG2_0_8_TUTORIAL_H_
#define _DOC_EGG2_0_8_TUTORIAL_H_

/**
   \page egg2_0_8_tutorial egg2.0.8 教程

   \section section_introduce 基本模块简介
   
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
   //create three document
   HEGGDOCUMENT hDocument1 = eggDocument_new();
   HEGGDOCUMENT hDocument2 = eggDocument_new();
   HEGGDOCUMENT hDocument3 = eggDocument_new();
   
   //add one document
   eggIndexWriter_add_document(hIndexWriter, hDocument1);
   eggIndexWriter_add_document(hIndexWriter, hDocument2);
   
   //modify one document
   eggIndexWriter_modify_document(hIndexWriter, id_modify, hDocument3);
   
   //delete one document
   eggIndexWriter_delete_document(hIndexWriter, id_delete, hDocument3);

   //submit data
   eggIndexWriter_optimize(hIndexWriter);
   
   //create IndexSearcher
   HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
   
   //query
   HEGGQUERY hQuery = eggQuery_new_string("body", "hello world", strlen("hello world"), ANALYZER_CWSLEX);

   //result set
   HEGGTOPCOLLECTOR hTopCollector =  eggTopCollector_new(0);
   //search with query
   eggIndexSearcher_search_with_query(hIndexSearcher,  hTopCollector, hQuery);
   
   //create IndexReader
   HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHttp);
   HEGGDOCUMENT hDocument = EGG_NULL;
   
   //fetch document by id(id from hTopCollector)
   eggIndexReader_get_document(hIndexReader, idNum, &hDocument)
   
  
   \endcode
   
   \arg eggTopCollector:\n\n
   搜索以后的结果集，存放了结果的doc id和相关信息，可对结果进行整合（如相关度打分，各类排序），使得结果按照要求的次序排列
   
   \code
   HEGGTOPCOLLECTOR hTopCollector =  eggTopCollector_new(0);
   
   //search with query
   eggIndexSearcher_search_with_query(hIndexSearcher,  hTopCollector, hQuery);
   
   ////result normalized
   //op为整合方式,
   //1. EGG_TOPSORT_SCORE(相关度打分排序)
   //2. EGG_TOPSORT_WEIGHT(权重打分排序, document权重排序,可自设，默认权重是按时间排序)
   //3. EGG_TOPSORT_NOT(什么都不操作 直接出结果)
   eggTopCollector_normalized(hTopCollector, op)
   
   //得到打分完毕的id数组和个数
   HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
   count_t cnt =  eggTopCollector_total_hits(hTopCollector);
   
   //通过过数组脚标获取id和score
   index_t idx = 0;
   did_t id = EGGSCOREDOC_ID_I(lp_score_doc, idx);
   score_t score = EGGSCOREDOC_SCORE_I(lp_score_doc, idx);

   \endcode

   
   \section section_add 添加
   \arg 例子\n\n
        record_1如下所示\n\n   
        title:   [来自现场一位记者报道]\n\n
        content: [三位日本的摄影记者，他们用生硬的普通话告诉我，对中国施救表示质疑，翻落的车厢与车头不应该毁坏并掩埋，那是在破坏事故证据。【这个坑从两点就开始挖了，难道是有预谋吗？，我有些受不了了，CCTV新闻里一直说，经检测没有生命迹象了，随后就被切割了，难道相信机器吗？]  \n\n
        url:     [www.example.com]\n\n
        record_2如下所示\n\n       
        name:    [小明]\n\n
        age:     [20]  \n\n
        sex:     [男]\n\n
   \code
     #include <Egg2.h>
   
     HEGGDIRECTORY hDirectory = eggDirectory_open("/tmp/egg/");
     
     HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hDirectory, "");
     
     HEGGDOCUMENT hDocument1 = eggDocument_new();
     HEGGDOCUMENT hDocument2 = eggDocument_new();
     
     {
       HEGGFIELD hField1 = eggField_new("title", title, strlen(title),
       EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
       HEGGFIELD hField2 = eggField_new("content", content, strlen(content),
       EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
       HEGGFIELD hField3 = eggField_new("url", url, strlen(url),
       EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_NOT_STORAGE);
       eggDocument_add(hDocument1, hField1);
       eggDocument_add(hDocument1, hField2);
       eggDocument_add(hDocument1, hField3);
     }

     {
       HEGGFIELD hField1 = eggField_new("name", name, strlen(name),
       EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
       HEGGFIELD hField2 = eggField_new("age", &age, sizeof(age),
       EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
       HEGGFIELD hField3 = eggField_new("sex", sex, strlen(sex),
       EGG_NOT_INDEX | EGG_STORAGE);
       eggDocument_add(hDocument2, hField1);
       eggDocument_add(hDocument2, hField2);
       eggDocument_add(hDocument2, hField3);
     }
     
     eggDocument_set_weight(hDocument1, -1);
     eggDocument_set_weight(hDocument2, -1);

     eggIndexWriter_add_document(hIndexWriter, hDocument1);
     eggIndexWriter_add_document(hIndexWriter, hDocument2);
     
     eggDocument_delete(hDocument1);
     eggDocument_delete(hDocument2);
     
     eggIndexWriter_optimize(hIndexWriter);
     eggIndexWriter_close(hIndexWriter);
     eggDirectory_close(hDirectory);
     
   \endcode
        
   \section section_modify 修改
   把指定document中要修改的field和要额外增加的field(倘若有)组成新的document,和id一起传入eggIndexWriter_modify_document\n\n
   
   
   \arg 例子\n\n
   假设知道record_2 的docid为2
   修改record_2里age字段,改为30,并且添加一个新的字段 introduction : [1996-2001: 小学毕业; 2001-2003: 初中毕业; 2003-2005: 高中毕业;]\n\n
   
   \code
   
   HEGGDOCUMENT hDocument = eggDocument_new();
   
   HEGGFIELD hField1 = eggField_new("age", &age, sizeof(age), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
   eggDocument_add(hDocument, hField1);
   
   HEGGFIELD hField2 = eggField_new("introduction", introduction, strlen(age), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
   eggDocument_add(hDocument, hField2);
   
   
   eggIndexWriter_modify_document(hIndexWriter, id, hDocument);
   
   eggIndexWriter_optimize(hIndexWriter);

   \endcode


   \section section_delete 删除
   把根据id删除指定document
   
   
   \arg 例子\n\n
   假设知道record_1 的docid为1 删除record_1\n\n
   
   \code
   
   
   
   eggIndexWriter_delete_document(hIndexWriter, id);
   
   eggIndexWriter_optimize(hIndexWriter);

   \endcode
   
   \arg note: \n\n
   添加, 修改, 删除都要执行eggIndexWriter_optimize后操作才会生效
   
   \section section_query 查询
   \arg 例子\n\n
   查询条件:\n\n
   content:[日本的摄影记者] || (age:[20] && name:[小明])\n\n
   
   \code
      HEGGDIRECTORY hDirectory = eggDirectory_open("/tmp/egg/");
      HEGGINDEXREADER hIndexReader = eggIndexReader_open(hDirectory);
      
      HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
      HEGGQUERY h1, h2, h3;
      h1 = eggQuery_new_string("content", "日本的摄影记者", strlen(日本的摄影记者), ANALYZER_CWSLEX);
      h2 = eggQuery_new_int32("age", 20);   
      h3 = eggQuery_new_string("name", "小明", strlen("小明"), EGG_NULL);
      h2 = eggQuery_and(h3, h2);
      h1 = eggQuery_or(h2, h1);
      h3 = h2 = 0;

      //填0取所有结果,非0按填的值取个数
      HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
      int ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);
      if (ret == EGG_TRUE)
      {
         //对最后结果进行排序
         //EGG_TOPSORT_WEIGHT:  按document的weight排序
         //EGG_TOPSORT_SCORE： 按查询关键字的相关度排序（打分排序）
         //EGG_TOPSORT_NOT：  不排序

         eggTopCollector_normalized(hTopCollector, EGG_TOPSORT_SCORE);
 
         HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
         count_t cnt =  eggTopCollector_total_hits(hTopCollector);
         printf("have hit %u documents\n", cnt);

         if (cnt > 0)
         {
           printf("last document: id[%llu]\n", lp_score_doc[cnt-1].idDoc);
           HEGGDOCUMENT lp_eggDocument = EGG_NULL;
           
           eggIndexReader_get_document(hIndexReader,
           lp_score_doc[cnt-1].idDoc, &lp_eggDocument);
           
           HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "content");
           unsigned len = 0;
           char *val = eggField_get_value(lp_field, &len);
           lp_field = 0;
           eggDocument_delete(lp_eggDocument);
          }
      }
      eggTopCollector_delete(hTopCollector);
      eggQuery_delete(h1);
      eggIndexSearcher_delete(hIndexSearcher);
      eggIndexReader_close(hIndexReader);
      eggDirectory_close(hDirectory);
      
   \endcode
 */



#endif //DOC_EGG206_TUTORIAL_H_

