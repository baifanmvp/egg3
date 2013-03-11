#ifndef _DOC_EGG2_0_8_TUTORIAL_H_
#define _DOC_EGG2_0_8_TUTORIAL_H_

/**
   \page egg2_0_9_tutorial egg2.0.9 教程
   \section section_introduce 概述
    egg2.0.9主要的改变在于egg集群的实现,主要采用的master－chunks的主从结构，通过配置文件对master和chunks设置，整个api接口和功能性略有改动，eggCluster模块引导整个egg集群的流程\n\n
    
    \subsection section_notice 注意事项
    \li document的chunkweight（划分权值）是根据该document的一个特定field来设置的（field类型暂只能为非string类型），这个特定field的name在writer句柄构造的时候设置,具体接口调用后面会详讲,若document并未按照要求设置特定field（没设置，类型错误或者超出配置文件上chunkweight的范围），在add操作是会判断无效,建议该特定field作为document的最后一个field加入，这样后面的处理效率相对较高
    \li 进行modify操作时，如果document的chunkweight（划分权值）超出了本机，该document会解除与原chunk的索引关系，在新chunk上建立索引，而doc id会失效
    \li 出于效率的考虑，采用cluster模式进行查询操作时，将无normalized操作，即废除了对结果的再次排序功能， 具体接口调用后面会详讲
    \li cluster排序无orderby方式（key范围查询）
    
   \section section_clusterconfig egg集群配置
   \li 配置文件默认在master主机下的/etc/egg2/eggInfoServer.cfg
   \li 配置格式
   \code
   
   eggExample   //eggName
   [1,10] 192.168.1.135:80/tmp/egg1  //[数据权值划分范围]  IP:PORT/CHUNKPATH (egg在chunk机上的绝对路径)
   [11,20] 192.168.1.136:80/tmp/egg2
   [21,] 192.168.1.120:80/tmp/egg3
   \endcode
   
   \section section_module_introduce 模块简介
   
   \arg eggCluster :\n\n
   和eggHttp和eggDirectory同级别的模块，用法类似，用于指定egg集群（cluster）上的存储路径，
   
   \code
   
   //open eggCluster handle
   //const path_t* clusterPath : egg存储路径,格式为IP:PORT:eggName (master的hostname和cluster上egg的名字（见配置文件格式）)
   
   HEGGCLUSTER EGGAPI eggCluster_open(const path_t* clusterPath);


   //close eggCluster handle
   EBOOL EGGAPI eggCluster_close(HEGGCLUSTER hCluster);

   \endcode

   \section eggCluster example
   \subsection section_add 添加
   \code
   HEGGHANDLE hEggHandle =  eggPath_open("192.168.1.1:80:eggExample");
   
   // 指定weightfield
   hIndexWrite = eggIndexWriter_open(hEggHandle,  "weightfield");

   HEGGDOCUMENT hDocument = eggDocument_new();
   //int spanpoint = 10;
   HEGGFIELD hField1 = eggField_new("content", buf, strlen(buf), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
   HEGGFIELD hField2 = eggField_new("price", (char*)&count, sizeof(count), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
   
   int spanpoint = (count%2? 50: 150);
   HEGGFIELD hField3 = eggField_new("weightfield", (char*)&spanpoint, sizeof(spanpoint), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
        
   eggDocument_add(hDocument, hField1);
   eggDocument_add(hDocument, hField2);
   eggDocument_add(hDocument, hField3);
   
   eggIndexWriter_add_document(hIndexWrite, hDocument);
   
   
   eggDocument_delete(hDocument);
   
   //after add n document 
   eggIndexWriter_optimize(hIndexWrite);
        
   ImLexAnalyzer_delete(p_la);        
   eggIndexWriter_close(hIndexWrite);
   eggPath_close(hEggHandle);
   \endcode

   \subsection modify
   \code
   HEGGHANDLE hEggHandle =  eggPath_open("192.168.1.1:80:eggExample");
   hIndexWrite = eggIndexWriter_open(hEggHandle,  "weightfield");

   int spanpoint = 150;
   HEGGDOCUMENT hDocument = eggDocument_new();
   HEGGFIELD hField1 = eggField_new(fieldname, buf, strlen(buf), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
   eggDocument_add(hDocument, hField1);
   HEGGFIELD hField2 = eggField_new("weightfield", (char*)&spanpoint, sizeof(spanpoint), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
   
   EGGDID did;
   EGGDID_DOCID(&did) = id;
   
   EBOOL ret = eggIndexWriter_modify_document( hIndexWriter, did, hDocument);
   if(ret == EGG_FALSE)
   {
   // modify false
   }
   else if(ret == EGG_MODIFY_IDINVALID)
   {
   // weight overflow and did is invalid
   }
   else if(ret == EGG_TRUE)
   {
   // modify ture
   }
   eggIndexWriter_optimize(hIndexWriter);
   eggIndexWriter_close(hIndexWriter);
   
   eggPath_close(hEggHandle);
   \endcode

   \subsection search
   \code
   HEGGHANDLE hEggHandle =  eggPath_open("192.168.1.1:80:eggExample");
   HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
   
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
   
   //对最后结果进行排序
   //EGG_TOPSORT_WEIGHT:  按document的weight排序
   //EGG_TOPSORT_SCORE： 按查询关键字的相关度排序（打分排序）
   //EGG_TOPSORT_NOT：  不排序
   //不调该函数默认是EGG_TOPSORT_NOT
   eggTopCollector_set_sortType(hTopCollector, EGG_TOPSORT_SCORE);
   int ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);
   if (ret == EGG_TRUE)
   {

   
   HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
   count_t cnt =  eggTopCollector_total_hits(hTopCollector);
   printf("have hit %u documents\n", cnt);

   if (cnt > 0)
   {
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
   eggPath_close(hCluster);

   \endcode
   \subsection section_pageiter 翻页
   
   \code
   HEGGSEARCHITER lp_iter = eggIndexSearcher_get_queryIter(hIndexSearcher);
 
   count_t pagenum = 0;
   printf("set pagenum : ");
   scanf("%d", &pagenum);
   
   eggSearchIter_reset(lp_iter, pagenum);
   EBOOL ret = 0;
   while(!EGGITER_OVERFIRST(lp_iter) && !EGGITER_OVERLAST(lp_iter))
   {
        HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
        ret = eggIndexSearcher_search_with_queryIter(hIndexSearcher, hTopCollector, h1, lp_cluster_iter);
        
        if(ret ==EGG_FALSE)
        {
          printf("no key !\n");
          exit(1);
        }
   
   
        HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
        count_t cnt =  eggTopCollector_total_hits(hTopCollector);
        index_t idx = 0;
        printf("count : %d\n", cnt);
   
   
        eggTopCollector_delete(hTopCollector);
   
        char c;
        printf("is jump result ? (y/n) ");
        getchar();
        scanf("%c", &c);
        if(c == 'y')
        {
           int jumpcnt = 0;
           printf("jump cnt : ");
           scanf("%d", &jumpcnt);
           eggSearchIter_iter(lp_iter, jumpcnt);
        }
    }
    
    eggSearchIter_delete(lp_iter);

   \endcode

*/



#endif //DOC_EGG206_TUTORIAL_H_

