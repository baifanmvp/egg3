#include "./eggItfTest.h"
#include "./eggDirectory.h"
#include "./eggHttp.h"
#include "./eggCluster.h"
#include "./eggPath.h"
#include "./eggIndexReader.h"
#include "./eggIndexWriter.h"
#include "./eggQuery.h"
#include "./eggScoreDoc.h"
#include "./eggTopCollector.h"
#include "./eggIndexSearcher.h"


#include <stdio.h>
#include <time.h>
#include <sys/time.h>




CeggItfTest::CeggItfTest(void)
{
}

CeggItfTest::~CeggItfTest(void)
{
}

void CeggItfTest::setUp(void)
{
}

void CeggItfTest::tearDown(void)
{
    
}

    
void CeggItfTest::testIndexAdd(char* dir_path)
{

    int i = 0;

    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite;
    
    FILE* fp_file = fopen("kfc.txt", "r+");
    hIndexWrite = eggIndexWriter_open(hEggHandle,  "weightfield");

    char *buf = EGG_NULL;
    size_t fileSize = 0;
    int count = 0;
    char databuf[4096];
    srand(time(0));
    while (getline(&buf, &fileSize, fp_file) != -1 & count < 5)
    {
        count++;
        if (buf[strlen(buf)-1] == '\n')
        {
            buf[strlen(buf)-1] = '\0';
        }
        //    printf("%s\n", buf);
            
        HEGGDOCUMENT hDocument = eggDocument_new();
        char buftmp[1024];
        sprintf(buftmp, "%s",buf);
//        memcpy(buftmp, );
//        HEGGFIELD hField1 = eggField_new("content", buftmp, strlen(buftmp), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
        HEGGFIELD hField1 = eggField_new("content", "1", strlen("1"), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
        
        int count1 = count % 100;
        char numbuf[10];
        sprintf(numbuf, "%d", count1);
        HEGGFIELD hField2 = eggField_new("random", (char*)&count1, 4, EGG_NOT_ANALYZED|EGG_RANGE_INDEX | EGG_INDEX_INT32 | EGG_STORAGE);

        int num1 = rand()%100;
        HEGGFIELD hField3 = eggField_new("num1", (char*)&num1, 4, EGG_NOT_ANALYZED|EGG_RANGE_INDEX | EGG_INDEX_INT32 | EGG_STORAGE);

        
        int num2 = rand()%100;
        HEGGFIELD hField4 = eggField_new("num2", (char*)&num2, 4, EGG_NOT_ANALYZED|EGG_RANGE_INDEX | EGG_INDEX_INT32 | EGG_STORAGE);

        HEGGFIELD hField5 = eggField_new("title", "111", 3, EGG_NOT_ANALYZED|EGG_INDEX_STRING | EGG_STORAGE);

        eggDocument_add(hDocument, hField1);
	eggDocument_add(hDocument, hField2);
        eggDocument_add(hDocument, hField3);
	eggDocument_add(hDocument, hField4);
	eggDocument_add(hDocument, hField5);

        eggIndexWriter_add_document(hIndexWrite, hDocument);
    
    
        eggDocument_delete(hDocument);
        if (count  % 5000  == 0)
        {
            printf("count %d\n", count);
	    	            break;
            eggIndexWriter_optimize(hIndexWrite);
        }
        // sleep(1);
    }
    
    eggIndexWriter_optimize(hIndexWrite);
        
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(hEggHandle );

    fclose(fp_file);
    free(buf);
    
    return ;
}


void CeggItfTest::testDeleteDoc(char* dir_path)
{
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);

    
    HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hEggHandle,  ANALYZER_CWSLEX);
    offset64_t id = 0;
    printf("input id :");
    scanf("%llu", &id);
    
    EGGDID did;
    EGGDID_DOCID(&did) = id;
    eggIndexWriter_delete_document( hIndexWriter, did);
    
    eggIndexWriter_optimize(hIndexWriter);
    eggIndexWriter_close(hIndexWriter);
    eggPath_close(hEggHandle);
    
}


void CeggItfTest::testModifyDoc(char* dir_path)
{
    void *hEggHandle = eggPath_open(dir_path);

    
    
    HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hEggHandle,  ANALYZER_CWSLEX);
    char buf[1024];
    char fieldname[1024];
    offset64_t id = 0;
    printf("input id :");
    scanf("%llu", &id);
    
    printf("input fieldname :");
    scanf("%s", fieldname);
    
    printf("input content :");
    scanf("%s", buf);
    
    
    HEGGDOCUMENT hDocument = eggDocument_new();
    HEGGFIELD hField1 = eggField_new(fieldname, buf, strlen(buf), EGG_NOT_ANALYZED|EGG_RANGE_INDEX | EGG_INDEX_STRING | EGG_STORAGE);
    eggDocument_add(hDocument, hField1);

    EGGDID did;
    EGGDID_DOCID(&did) = id;
    
    eggIndexWriter_modify_document( hIndexWriter, did, hDocument);
    
    eggIndexWriter_optimize(hIndexWriter);
    eggIndexWriter_close(hIndexWriter);
    
}


void CeggItfTest::testExportDoc(char* dir_path)
{
    void *hEggHandle = eggPath_open(dir_path);
        
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
    offset64_t n_cursor = 0;
    HEGGDOCUMENT lp_eggDocument = EGG_NULL;

    while(lp_eggDocument = eggIndexReader_export_document(hIndexReader, &n_cursor))
    {
        HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "content");
        unsigned int len = 0;
        
        if(lp_field)
            printf("%s", eggField_get_value(lp_field, &len));

        eggDocument_delete(lp_eggDocument);
    }
    return ;
}


static HEGGQUERY nextQuery(char *fieldName, char *keybuf)
{
    HEGGQUERY h1 = NULL;
    if (keybuf[0] == '"')
    {
        char *key;
        char *p = keybuf+1;
        while (p[0])
        {
            if (p[0] == '"')
            {
                break;
            }
            else if (p[0] == '\\')
            {
                p++;
                if (p[0] == '\0')
                {
                    fprintf(stderr, "input string invalid \\\n");
                    exit(-1);
                }
                p++;
            }
            else
            {
                p++;
            }
        }
        p[0] = '\0';
        key = keybuf+1;

        h1 = eggQuery_new_string(fieldName, key, p-key, NULL);
    }
    else if (keybuf[0] == 'i')
    {
        int i;
        i = atoi(keybuf+1);
        
        h1 = eggQuery_new_int32(fieldName, i);
    }
    else if (keybuf[0] == 'l')
    {
        int64_t l;
        l = strtoll(keybuf+1, NULL, 0);
        h1 = eggQuery_new_int64(fieldName, l);
    }
    else if (keybuf[0] == 'f')
    {
        double f;
        f = strtod(keybuf+1, NULL);
        h1 = eggQuery_new_double(fieldName, f);
    }
    else
    {
        fprintf(stderr, "please \"string\" or i32 or l64 or f3.3333 \n");
        exit(-1);
    }

    return h1;
}
static HEGGQUERY getQuery()
{
    char key[1000] = {0};
    char fieldName[200] = "";

    HEGGQUERY h1, h2;
    printf("fieldName: ");
    scanf("%s", fieldName);
    printf("key: ");
    scanf("%s", key);
    h1 = nextQuery(fieldName, key);

    while (1)
    {
        HEGGQUERY h2 = 0;
        int c;
        printf("logic? ");
        char bb[20] = "";
        fgets(bb, sizeof(bb), stdin);
        fgets(bb, sizeof(bb), stdin);        
        c = bb[0];
        printf("--------     :%c\n",c);
        if (c == 'a' || c == 'A')
        {
            printf("FieldName: ");
            scanf("%s", fieldName);
            printf("key: ");
            scanf("%s", key);
            h2 = nextQuery(fieldName, key);
            
            h1 = eggQuery_and(h1, h2);
        }
        else if  (c == 'o' || c == 'O')
        {
            printf("FieldName: ");
            scanf("%s", fieldName);
            printf("key: ");
            scanf("%s", key);
            h2 = nextQuery(fieldName, key);
            
            h1 = eggQuery_or(h1, h2);
        }
        else if  (c == '-')
        {
            printf("FieldName: ");
            scanf("%s", fieldName);
            printf("key: ");
            scanf("%s", key);
            h2 = nextQuery(fieldName,key);

            h1 = eggQuery_minus(h1, h2);
        }
        else
        {
            break;
        }
    }

    return h1;
    
}
void CeggItfTest::testIndexSearchIter(char* dir_path)
{
    char key[1000] = {0};
    type_t op = EGG_TOPSORT_SCORE;
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
        
    
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    char fieldName[200] = "";
    HEGGQUERY h1, h2;
    
    h1 = getQuery();
    
    HEGGSEARCHITER lp_iter = eggIndexSearcher_get_queryiter(hIndexSearcher);
    
    count_t pagenum = 0;
    type_t op1;
    printf("result sort : 1. not sort, 2. score sort, 3. weight sort");
    scanf("%d", &pagenum);
    if(pagenum == 2)
    {
        op1 = EGG_TOPSORT_SCORE;
    }
    else if(pagenum == 3)
    {
        op1 = EGG_TOPSORT_WEIGHT;
    }
    else
    {
        op1 = EGG_TOPSORT_NOT;
    }

    printf("set pagenum : ");
    scanf("%d", &pagenum);
    
    eggSearchIter_reset(lp_iter, pagenum);
    EBOOL ret = 0;
    struct timeval vstart, vend;
    while(!EGGITER_OVERFIRST(lp_iter) && !EGGITER_OVERLAST(lp_iter))
    {
        HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
        eggTopCollector_set_sorttype(hTopCollector, op1);
        gettimeofday(&vstart, 0);
        ret = eggIndexSearcher_search_with_queryiter(hIndexSearcher, hTopCollector, h1, lp_iter);
        gettimeofday(&vend, 0);
        printf("iterSearch time : %f\n", (double)(vend.tv_sec - vstart.tv_sec) + (double)((vend.tv_usec - vstart.tv_usec))/1000000);
        if(ret ==EGG_FALSE)
        {
            printf("no key !\n");
            exit(1);
        }
    
    
        HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
        count_t cnt =  eggTopCollector_total_hits(hTopCollector);
        index_t idx = 0;
        printf("count : %d\n", cnt);

        while(idx != cnt)
        {
            printf("count %d id : %lld \n", idx, EGGDID_DOCID(&(lp_score_doc[idx].idDoc)) );
            /*
              HEGGDOCUMENT lp_eggDocument = EGG_NULL;
              eggIndexReader_get_document(hIndexReader, lp_score_doc[idx].idDoc, &lp_eggDocument);

              HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "content");
            unsigned int len = 0;
            
            if(lp_field)
                printf("count %d id : %lld content : %s\n", idx, EGGDID_DOCID(&(lp_score_doc[idx].idDoc)), eggField_get_value(lp_field, &len));
            eggDocument_delete(lp_eggDocument);
	  */
            idx++;
        }

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
    eggQuery_delete(h1);
    eggIndexSearcher_delete(hIndexSearcher);
    eggIndexReader_close(hIndexReader);

    eggPath_close(hEggHandle);    
}


void CeggItfTest::testIndexSearch(char* dir_path)
{
    char key[1000] = {0};
    type_t op = EGG_TOPSORT_SCORE;
    HEGGHANDLE hEggHandle = eggPath_open(dir_path);
    
    
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
    
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);
    char fieldName[200] = "";
    HEGGQUERY h1;
    
    h1 = getQuery();
    
    char c;
    printf("key range search?(y/n)");
    scanf("%c", &c);
    if(c == 'y')
    {
        printf("FieldName: ");
        scanf("%s", fieldName);
        

        int startPrice = 0;
        int endPrice = 0;
        printf("start Price: ");
        scanf("%d", &startPrice);
        printf("end Price: ");
        scanf("%d", &endPrice);
        HEGGQUERY h2 = 0;
        op = EGG_TOPSORT_ORDERBY;
        h2 = eggQuery_new_int32range(fieldName, startPrice, endPrice);
        h1 = eggQuery_and(h1, h2);
        
    }
    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    switch (1) {
    case 1:
            
        eggTopCollector_set_orderby(hTopCollector, 2, "num1", 1,
                                    "num2", 1);
        break;
    case 2:
        eggTopCollector_set_sorttype(hTopCollector, EGG_TOPSORT_SCORE);
        break;
    default:
        eggTopCollector_set_sorttype(hTopCollector, EGG_TOPSORT_NOT);
        break;
    }
        
    
    struct timeval vstart, vend;
    gettimeofday(&vstart, 0);
    EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);
    gettimeofday(&vend, 0);
    printf("search_with_query time : %f\n", (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    if(ret ==EGG_FALSE)
    {
        printf("no key !\n");
        exit(1);
    }

    // eggTopCollector_delete(hTopCollector);
    // eggQuery_delete(h1);
    // eggIndexSearcher_delete(hIndexSearcher);
    // eggIndexReader_close(hIndexReader);

    // eggPath_close(hEggHandle);    

    //     return ;
    if (0)
    {                           // deprecated
        HEGGQUERY hQuery_tmp = 0;
        //取时间范围
        hQuery_tmp = eggQuery_new_stringrange("time", "1", "2");
        //按时间排序
        eggIndexSearcher_filter(hIndexSearcher, hTopCollector, hQuery_tmp, 1);
        //按相关度排序
        //eggIndexSearcher_filter(hIndexSearcher, hTopCollector, hQuery_tmp, 0);
        eggQuery_delete(hQuery_tmp);
    }
    
    HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
    count_t cnt =  eggTopCollector_total_hits(hTopCollector);
    index_t idx = 0;
    printf("count : %d\n", cnt);
//        return ;
    #if(0)
    HEGGDOCUMENT* ppeggDocument = EGG_NULL;
    eggIndexReader_get_documentSet(hIndexReader, lp_score_doc, cnt, &ppeggDocument);
    while(idx != cnt)

    {
        printf("--------------------------\n");
        HEGGFIELD lp_field = eggDocument_get_field(ppeggDocument[idx], "f_id");
        unsigned int len = 0;
        if(lp_field)
            printf("count %d id : %lld \nf_id : %s ", idx, EGGDID_DOCID(&(lp_score_doc[idx].idDoc)), eggField_get_value(lp_field, &len));

        eggDocument_delete(ppeggDocument[idx]);
        idx++;
        
    }
    #endif
    
    #if(0)
    while(idx != cnt && idx < 10000)
    {
        HEGGDOCUMENT lp_eggDocument = EGG_NULL;
        printf("%lld ----\n", lp_score_doc[idx].idDoc);
       	eggIndexReader_get_document(hIndexReader, lp_score_doc[idx].idDoc, &lp_eggDocument);

        HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "f_id");
	//	HEGGFIELD lp_field2 = eggDocument_get_field(lp_eggDocument, "random");
	//HEGGFIELD lp_field3 = eggDocument_get_field(lp_eggDocument, "num1");
	//HEGGFIELD lp_field4 = eggDocument_get_field(lp_eggDocument, "num2");
        //      HEGGFIELD lp_field3 = eggDocument_get_field(lp_eggDocument, "spanfield2");        
        unsigned int len = 0;
        unsigned int len2 = 0;
        unsigned int len3 = 0;                
        if(lp_field)
            printf("count %d id : %lld  f_id: %s \n", idx, EGGDID_DOCID(&(lp_score_doc[idx].idDoc)),  eggField_get_value(lp_field, &len) );

//        if(lp_field3)
//	  printf("count %d id : %lld content : %s weightfield: %d\n", idx, EGGDID_DOCID(&(lp_score_doc[idx].idDoc)), eggField_get_value(lp_field3, &len), eggField_get_value(lp_field3, &len3));
/*
        {
            char **pkeywords;
            size16_t *pkeySz;
//          int **ppos = NULL;
            count_t nums;
            
            eggTopCollector_get_keyPosition(hTopCollector,
                                            EGGDID_DOCID(&lp_score_doc[idx].idDoc),
                                            "content", &pkeywords, &pkeySz,
                                            NULL, &nums);
            int i;
            for (i = 0; i < nums; i++)
            {

                printf("Key[%.*s]\n", pkeySz[i], pkeywords[i]);
            }
            free(pkeySz);
            for (i = 0; i < nums; i++)
            {
                free(pkeywords[i]);
            }
            free(pkeywords);
        }
*/
//        lp_field = eggDocument_get_field(lp_eggDocument, "price");
        
//        printf("date : [%s] \n", eggField_get_value(lp_field, &len));

        eggDocument_delete(lp_eggDocument);
        idx++;
        // usleep(5000);
    }
    #endif
    eggTopCollector_delete(hTopCollector);
    eggQuery_delete(h1);
    eggIndexSearcher_delete(hIndexSearcher);
    eggIndexReader_close(hIndexReader);

    eggPath_close(hEggHandle);    
}


void CeggItfTest::testReIndex(char* dir_path)
{

    struct timeval tv_start, tv_end;
    int i = 0;

    gettimeofday(&tv_start, EGG_NULL);
    void *hEggHandle = eggPath_open(dir_path);
    HEGGINDEXWRITER hIndexWrite;
    hIndexWrite = eggIndexWriter_open(hEggHandle,  ANALYZER_CWSLEX);

    char *buf = EGG_NULL;
    size_t fileSize = 0;
    int count = 0;
    char databuf[4096];
    char c= 0;
    getchar();
    while (( c = getchar() ) == 'c')
    {
        getchar();
        offset64_t id = 0;
        printf("id :");
        scanf("%llu", &id);
        getchar();
        HEGGDOCUMENT hDocument = eggDocument_new();
        
        sprintf(databuf, "body : %s", buf);
        HEGGFIELD hField1 = eggField_new("body1", "159", strlen("159"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
        eggDocument_add(hDocument, hField1);

//        eggIndexWriter_reIndex_document(hIndexWrite, hDocument, id);
        
    
    
        eggDocument_delete(hDocument);

    }
        
    eggIndexWriter_optimize(hIndexWrite);
        
    eggIndexWriter_close(hIndexWrite);
    gettimeofday(&tv_end, EGG_NULL);

    return ;
}



void CeggItfTest:: testAddandSearch(void)
{


    // printf("map_view_offset %ld cluster_alloc_size %ld\n",
    //        (long)MAP_VIEW_OFFSET, (long)CLUSTER_ALLOC_SIZE);
    // long long unsigned i;
    // for (i = 64*1024*1024ULL; i < 22 *1024 *1024*1024ULL; i+=64*1024*1024ULL)
    // {
    //     printf("%llu\n", (long long unsigned)Uti_base_offset(i - 0x100,
    //                                                   MAP_VIEW_OFFSET,
    //                                                   CLUSTER_ALLOC_SIZE));
    // }

    
    char buf[100] = {0};
    char* lp_dir_path = getenv("EGG_DIR_PATH");
    if(lp_dir_path ==0)
    {
        printf("EGG_DIR_PATH not set\n");
        exit(-1);
    }
    
    printf("EGG_DIR_PATH = %s\n", lp_dir_path);
    
    printf("\ninput: [add], [search] [iter] [delete] [modify]\n");
    
    scanf("%s", buf);
//    memcpy(buf, "add", strlen("add") );
    if (strcmp(buf, "add")  == 0)
    {
        testIndexAdd(lp_dir_path);
    }
    else if (strcmp(buf, "search")  == 0)
    {
        testIndexSearch(lp_dir_path);        
    }
    else if (strcmp(buf, "export")  == 0)
    {
        testExportDoc(lp_dir_path);        
    }
    else if(strcmp(buf, "reindex")  == 0 )
    {
        testReIndex(lp_dir_path);
    }
    else if(strcmp(buf, "delete")  == 0 )
    {
        testDeleteDoc(lp_dir_path);
    }

    else if(strcmp(buf, "modify")  == 0 )
    {
        testModifyDoc(lp_dir_path);
    }
    else if(strcmp(buf, "iter")  == 0 )
    {
        testIndexSearchIter(lp_dir_path);
    }

}


