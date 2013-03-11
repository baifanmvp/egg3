#include <egg2/Egg2.h>
#include <eggPath.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define PATH "./"

void  testIndexAdd(char* dir_path)
{
    //define
    int i = 0;
    void *hEggHandle;
    HEGGINDEXWRITER hIndexWrite;

    //open
    hEggHandle=eggPath_open(dir_path);
    hIndexWrite = eggIndexWriter_open(hEggHandle,  "spanfield");
    FILE* fp_file = fopen("aaa.txt", "r+");

    char *buf = EGG_NULL;
    size_t fileSize = 0;
    int count = 0;
    char databuf[4096];
    srand(time(0));
    while (getline(&buf, &fileSize, fp_file) != -1 && count < 10000)
    {
        count++;

        //add 
        HEGGDOCUMENT hDocument = eggDocument_new();
        HEGGFIELD hField1 = eggField_new("content", buf, strlen(buf),\
                EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
        eggDocument_add(hDocument, hField1);
        eggIndexWriter_add_document(hIndexWrite, hDocument);
        eggDocument_delete(hDocument);

        if (count  % 10000  == 0)
        {
            printf("count %d\n", count);
            eggIndexWriter_optimize(hIndexWrite);
        }
    }
    //optimize
    eggIndexWriter_optimize(hIndexWrite);

    //close
    eggIndexWriter_close(hIndexWrite);
    eggPath_close(HEGGHANDLE(hEggHandle));
    fclose(fp_file);
    free(buf);

    return ;
}


void  testDeleteDoc(char* dir_path)
{
    void *hEggHandle;
    //open
    hEggHandle=eggPath_open(dir_path);
    HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hEggHandle,  ANALYZER_CWSLEX);

    //initial
    offset64_t id = 0;
    printf("input id :");
    scanf("%llu", &id);
    EGGDID did;
    EGGDID_DOCID(&did) = id;
    
    //del
    eggIndexWriter_delete_document( hIndexWriter, did);
    
    //optimize
    eggIndexWriter_optimize(hIndexWriter);

    //close
    eggIndexWriter_close(hIndexWriter);
    eggPath_close(HEGGHANDLE(hEggHandle));
    
    return ;
}


void  testModifyDoc(char* dir_path)
{
    void *hEggHandle;
    //open
    hEggHandle=eggPath_open(dir_path);
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
    
    //modify
    HEGGDOCUMENT hDocument = eggDocument_new();
    HEGGFIELD hField1 = eggField_new(fieldname, buf, strlen(buf), EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
    eggDocument_add(hDocument, hField1);
    EGGDID did;
    EGGDID_DOCID(&did) = id;
    eggIndexWriter_modify_document( hIndexWriter, did, hDocument);
    
    //optimize
    eggIndexWriter_optimize(hIndexWriter);
    //close
    eggIndexWriter_close(hIndexWriter);
    eggPath_close(HEGGHANDLE(hEggHandle));
    
}


void  testIndexSearch(char* dir_path)
{
    char key[1000] = {0};
    type_t op = EGG_TOPSORT_SCORE;
    void *hEggHandle;
    //open
    hEggHandle=eggPath_open(dir_path);
    HEGGINDEXREADER hIndexReader = eggIndexReader_open(hEggHandle);
    HEGGINDEXSEARCHER hIndexSearcher = eggIndexSearcher_new(hIndexReader);

    char fieldName[200] = "";
    HEGGQUERY h1;
    
    printf("fieldName: ");
    scanf("%s", fieldName);
    printf("key: ");
    scanf("%s", key);

    h1 = eggQuery_new_sentence(fieldName, key, strlen(key), ANALYZER_CWSLEX);
    
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
            h2 = eggQuery_new_sentence(fieldName, key, strlen(key), ANALYZER_CWSLEX);
            h1 = eggQuery_and(h1, h2);
        }
        else if  (c == 'o' || c == 'O')
        {
            printf("FieldName: ");
            scanf("%s", fieldName);
            printf("key: ");
            scanf("%s", key);
            h2 = eggQuery_new_sentence(fieldName, key, strlen(key), ANALYZER_CWSLEX);
            h1 = eggQuery_or(h1, h2);
        }
        else
        {
            break;
        }
    }

    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    struct timeval vstart, vend;
    gettimeofday(&vstart, 0);
    EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcher, hTopCollector, h1);

    gettimeofday(&vend, 0);
    printf("time : %f\n", (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    if(ret ==EGG_FALSE)
    {
        printf("no key !\n");
        return ;
    }
    
    
    HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
    count_t cnt =  eggTopCollector_total_hits(hTopCollector);
    index_t idx = 0;
    printf("count : %d\n", cnt);

    while(idx != cnt && idx < 10)
    {
        HEGGDOCUMENT lp_eggDocument = EGG_NULL;
        printf("%lld ----\n", lp_score_doc[idx].idDoc);
       	eggIndexReader_get_document(hIndexReader, lp_score_doc[idx].idDoc, &lp_eggDocument);

        HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "content");
        unsigned int len = 0;
        if(lp_field)
        printf("count %d id : %lld content : %s\n", idx, EGGDID_DOCID(&(lp_score_doc[idx].idDoc)), eggField_get_value(lp_field, &len));
        eggDocument_delete(lp_eggDocument);
        idx++;
        // usleep(5000);
    }

    //close
    eggTopCollector_delete(hTopCollector);
    eggQuery_delete(h1);
    eggIndexSearcher_delete(hIndexSearcher);
    eggIndexReader_close(hIndexReader);
    eggPath_close(HEGGHANDLE(hEggHandle));

    return ;
}

void output()
{
    cout<<"++++++++++++++++++++++++++++++++++"<<endl;
    cout<<"+ 1. add                         +"<<endl;
    cout<<"+ 2. delete                      +"<<endl;
    cout<<"+ 3. mofidy                      +"<<endl;
    cout<<"+ 4. search                      +"<<endl;
    cout<<"+ 0. bye                         +"<<endl;
    cout<<"++++++++++++++++++++++++++++++++++"<<endl;
    cout<<"hello world,input the number:";

}
int main(int argc,char * argv[])
{
    int n;
    //ifstream in("aaa.txt");
    output();
    while(cin>>n)
    {
        switch(n)
        {
            case 1: 
                cout<<"1. add"<<endl;
                testIndexAdd(PATH);
                break;
            case 2:
                cout<<"2. delete"<<endl;
                testDeleteDoc(PATH);
                break;
            case 3:
                cout<<"3. modify"<<endl;
                testModifyDoc(PATH);
                break;
            case 4:
                cout<<"4. search"<<endl;
                testIndexSearch(PATH);
                break;
            case 0:
                goto loop;
                break;
            default:
                cout<<"Please input right number"<<endl;
        }
        output();
    }
loop:
    cout<<"bye"<<endl;
    return 0;
}

