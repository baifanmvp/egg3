#include "eggIdViewTest.h"
 #include <time.h>
#include <sys/time.h>

CeggIdViewTest::CeggIdViewTest(void)
{
    
}

CeggIdViewTest::~CeggIdViewTest(void)
{
    
}

void CeggIdViewTest::setUp(void)
{
    
    return ;
}

void CeggIdViewTest::tearDown(void)
{
    return ;
    
}
    offset64_t g_info_off = 0;
void idsIntegration(int num)
{
  /*    HEGGFILE lp_egg_file = EggFile_open("./idnodes/egg.idd");
    HEGGIDVIEW lp_id_view = eggIdView_new(lp_egg_file);
    EGGLISTINF st_list_inf = {0};
    offset64_t n_info_off = 0;
    HEGGIDNODE IdNodes = (HEGGIDNODE)malloc(sizeof(EGGIDNODE)* num);

    int i =0;
    st_list_inf.aSz = sizeof(EGGLISTINF);
    srand(time(0));
    while(i < num)
    {
        IdNodes[i].id = rand()%50;
        printf("IdNodes[%d].id : %llu\n", i, IdNodes[i].id);
        i++;
    }
        n_info_off = g_info_off;
//    scanf("%llu", &n_info_off);

    
    if(!n_info_off)
    {
        st_list_inf.nodeSz = sizeof(EGGIDNODE);
        n_info_off = eggIdView_reg(lp_id_view, &st_list_inf);
        g_info_off = lp_id_view->hEggListView->hInfo->ownOff;
        printf("n_info_off = %llu\n", lp_id_view->hEggListView->hInfo->ownOff);
        
        eggIdView_add(lp_id_view, IdNodes, num);
    }
    else
    {
        eggIdView_load(lp_id_view, n_info_off);    
        eggIdView_add(lp_id_view, IdNodes, num);
    }
    count_t cnt = 0;
    HEGGIDNODE lpIdNodesTmp = 0;
    
    eggIdView_find(lp_id_view, lp_id_view->hEggListView->hInfo->ownOff, &lpIdNodesTmp, &cnt);
    i = 0;    
    while(i < cnt)
    {
//        IdNodes[i].id = rand()%50;
        printf("IdNodesTmp[%d].id : %llu\n", i, lpIdNodesTmp[i].id);
        i++;
    }
    
    
    eggIdView_delete(lp_id_view);
    free(IdNodes);
    free(lpIdNodesTmp);
  */
    return ;
}
void CeggIdViewTest::testIntegration(void)
{
     system("cp ./idnodes/egg.idd.bak ./idnodes/egg.idd");
     int i = 1;
     while(i < 1000)
     {
         idsIntegration(i);
         i++;
     }
    return ;
}



