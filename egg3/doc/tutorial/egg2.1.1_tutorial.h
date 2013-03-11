#ifndef _DOC_EGG2_1_1_TUTORIAL_H_
#define _DOC_EGG2_1_1_TUTORIAL_H_

/**
   \page egg2_1_1_tutorial egg2.1.1 教程
   \section section_introduce 概述
    egg2.1.1 新特性
    \li egg通信增加了socket直接访问方式
    \li 存了各域的value，可对多个域进些order by操作，并简化对各域的修改操作
    \li 获取所有egg中所有field的信息
 
    \subsection section_notice 注意事项(后面代码有示例)
    \li 若采用socket直接访问方式, client需要将一些配置信息以配置文件的形式发给server, path格式为 "::path1:path2" path1为server机上配置文件路径，path2为egg下入的路径

    \li 数字域（int32，int64，double）进些orderby操作，不支持string类型的域

    \li string类型的域不按weight进行保存，因此key范围查询的效率比不上数字域
   
    \li 对过多的域进些orderby联合排序，会造成时间的线性增长
    
    \li orderby操作提供了新的接口，原来的方式不可用
    
    \li egg集群不支持orderby操作
    
   \section section_serverconfig server的配置
   \li 配置文件默认在各chunk主机下的/etc/egg2/eggServiceServer.cfg, 同时监听两种端口
   \code
   socket=/tmp/egg2.sock
   ip=192.168.1.136
   port=8888
   \endcode
   
   \section section_clientconfig client的配置
   \li 配置文件默认在各chunk主机下的/etc/egg2/eggServiceClientLocal.cfg, unixsock本地通信
   \li 配置格式
   \code
   socket=/tmp/egg2.sock
   \endcode
   
   \li 配置文件默认在各chunk主机下的/etc/egg2/eggServiceClientTcp.cfg, tcp通信
   \li 配置格式
   \code
   ip=192.168.1.136
   port=8888
   \endcode
   
   \section section_code 代码示例
   
   \li查询content下包含 “上海”关键字，time范围在 20110101－20120101，且结果先按照按照time排序，再按照price排序
   
   \code

   //tcp通信模式
   HEGGHANDLE hEggHandle = eggPath_open("::/etc/egg2/eggServiceClientTcp.cfg:/tmp/"); 
   
   
   HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
   
   HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);

   HEGGQUERY h1  = eggQuery_new_string("content", "上海", strlen("上海"), ANALYZER_CWSLEX);
   
   HEGGQUERY h2 = eggQuery_new_int32Range("time", 20110101, 20120101);

   h1 = eggQuery_and(h1, h2);

   HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);

   //eggTopCollector_set_orderBy 为变参函数
   
   //参数1： TopCollector句柄
   //参数2： 需要orderby的域的个数，各域按照从左到右优先级别降低
   //参数3： 域名
   //参数4： 1为升序 0为降序 

   eggTopCollector_set_orderBy(hTopCollector, 2, "time", 1, "price", 1);
   
   EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);
   if(ret ==EGG_FALSE)
   {
      printf("no key !\n");
      exit(1);
   }

   HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
   count_t cnt =  eggTopCollector_total_hits(hTopCollector);
   .
   .
   .
   \endcode
   
   \li field操作

   获取field的信息
   \code
   
   HEGGHANDLE hEggHandle = eggPath_open("/tmp/");
   HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
   count_t cntFieldNameInfo;
   HEGGFIELDNAMEINFO hFieldNameInfo;
   if (eggIndexReader_get_fieldNameInfo(hIndexReader, &hFieldNameInfo, &cntFieldNameInfo) == EGG_TRUE)
   {
        int i;
        for (i = 0; i < cntFieldNameInfo; i++)
        {
            printf("%s: ", hFieldNameInfo[i].name);
            if (hFieldNameInfo[i].type & EGG_NOT_ANALYZED)
            {
                printf("NotAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CWS_ANALYZED)
            {
                printf("CwsAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CN_ANALYZED)
            {
                printf("CnAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CY_ANALYZED)
            {
                printf("CyAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_CX_ANALYZED)
            {
                printf("CxAnalyzed ");
            }
            else if (hFieldNameInfo[i].type & EGG_OTHER_ANALYZED)
            {
                printf("OtherAnalyzed:%s ", hFieldNameInfo[i].analyzerName);
            }
            
            if (hFieldNameInfo[i].type & EGG_INDEX_STRING)
            {
                printf("String ");
            }
            else if (hFieldNameInfo[i].type & EGG_INDEX_INT32)
            {
                printf("Int32 ");
            }
            else if (hFieldNameInfo[i].type & EGG_INDEX_INT64)
            {
                printf("Int64 ");
            }
            else if (hFieldNameInfo[i].type & EGG_INDEX_DOUBLE)
            {
                printf("Double ");
            }
            printf("\n");
        }
   }
   eggFieldView_delete_fieldNameInfo(hFieldNameInfo, cntFieldNameInfo);

   if (eggIndexReader_get_singleFieldNameInfo(hIndexReader, "item", &hFieldNameInfo) == EGG_TRUE)
   {
       if (hFieldNameInfo->type & EGG_INDEX_STRING && hFieldNameInfo->type & EGG_OTHER_ANALYZED)
           printf("\nitem: string EGG_OTHER_ANALYZED: %s\n", hFieldNameInfo->analyzerName);
   }
   eggFieldView_delete_fieldNameInfo(hFieldNameInfo, 1);
   
   eggIndexReader_close(hIndexReader);
   eggPath_close(hEggHandle);
   
   \endcode
   
   生成新的field,但不生成document.
   \code
   
    HEGGHANDLE hEggHandle = eggPath_open("/tmp/");
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  NULL);
    
    HEGGDOCUMENT hDocument = eggDocument_new();
    // 域 内容为NULL
    HEGGFIELD hField1 = eggField_new("body", NULL, 0, EGG_CWS_ANALYZED | EGG_INDEX_STRING);
    eggDocument_add(hDocument, hField1);
    
    eggIndexWriter_add_document(hIndexWrite, hDocument);
    eggDocument_delete(hDocument);    
    eggIndexWriter_optimize(hIndexWrite);
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);

   \endcode
   修改已有的field.
   \code
   
    HEGGHANDLE hEggHandle = eggPath_open("/tmp/");
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hEggHandle,  NULL);
    
    HEGGDOCUMENT hDocument = eggDocument_new();
    // 域 内容为NULL
    HEGGFIELD hField1 = eggField_new("body", NULL, 0, EGG_OTHER_ANALYZED | EGG_INDEX_STRING, "ImC2LexAnalyzer");
    eggDocument_add(hDocument, hField1);
    
    eggIndexWriter_add_document(hIndexWrite, hDocument);
    eggDocument_delete(hDocument);    
    eggIndexWriter_optimize(hIndexWrite);
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle);

   \endcode
   
*/



#endif //DOC_EGG211_TUTORIAL_H_

