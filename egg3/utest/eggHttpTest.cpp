#include "eggHttpTest.h"
 #include <time.h>
#include <sys/time.h>

CeggHttpTest::CeggHttpTest(void)
{
    
}

CeggHttpTest::~CeggHttpTest(void)
{
    
}

void CeggHttpTest::setUp(void)
{
    
    return ;
}

void CeggHttpTest::tearDown(void)
{
    return ;
    
}
void CeggHttpTest::testConnect(void)
{
    HEGGNETHTTP lp_net_http = eggNetHttp_new("localhost:80//cgi-bin/test.fcgi", "POST");

    if(!eggNetHttp_connect(lp_net_http))
    {
        printf("eggNetHttp_connect error \n");
    }
    char buf1[] = "123456789";
    char* buf2 = EGG_NULL;
    size32_t n_size2 = 0;
    int num  = 123;
    HEGGNETPACKAGE lp_net_package = eggNetPackage_new(1);
    lp_net_package = eggNetPackage_add(lp_net_package, (void*)"1111", strlen("1111"), 2);
    lp_net_package = eggNetPackage_add(lp_net_package, (void*)"22222", strlen("22222"), 3);

    eggNetHttp_send(lp_net_http, lp_net_package, eggNetPackage_get_packagesize(lp_net_package), 0);

    eggNetHttp_recv(lp_net_http, (void**)&buf2, &n_size2, 0);

    char* result1 = 0;
    char* result2 = 0;
    size32_t size1 = 0;
    size32_t size2 = 0;
    eggNetPackage_fetch((HEGGNETPACKAGE)buf2, 4, &result1, &size1, &result2, &size2);
    printf("%s  %d  %s %d\n", result1, size1, result2, size2);
    eggNetHttp_delete(lp_net_http);

    return ;
}
