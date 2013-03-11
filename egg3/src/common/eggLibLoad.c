#include "eggLibLoad.h"

flag_t g_eggLibLoad_flag = EGG_FALSE;

#ifdef __CYGWIN__ 
flag_t g_eggLibLoad_cygwin = EGG_FALSE; 
#else
__attribute__((constructor))
#endif
void eggLibLoad_init()
{
    g_eggLibLoad_flag = eggReg_init();

    if(!g_eggLibLoad_flag)
    {
        printf("eggReg_init error !\n");
        exit(-1);
    }

    
    g_eggLibLoad_flag = eggBaseConfig_build();

    if(!g_eggLibLoad_flag)
    {
        printf("eggConfig_build error !\n");
        exit(-1);
    }
    
    g_eggLibLoad_flag = eggAnalyzerSet_build();
    if(!g_eggLibLoad_flag)
    {
        printf("eggAnalyzerSet_build error !\n");
        exit(-1);
    }

    g_eggLibLoad_flag = eggSySRecorder_init();
    if(!g_eggLibLoad_flag)
    {
        printf("eggSySRecorder_init error !\n");
        exit(-1);
    }
    umask(0);

    g_eggLibLoad_flag = eggPrtLog_build();
    if(!g_eggLibLoad_flag)
    {
        printf("eggPrtLog_init error !\n");
        exit(-1);
    }
    
    //  alarm(EGGANALYDICT_FLUSHTIME);
//
    return ;
}

#ifdef __CYGWIN__ 
#else
__attribute__((destructor))
#endif
void eggLibLoad_release()
{

    g_eggLibLoad_flag = eggPrtLog_destroy();
    if(!g_eggLibLoad_flag)
    {
        printf("eggPrtLog_destroy error !\n");
        exit(-1);
    }

    
    g_eggLibLoad_flag = eggAnalyzerSet_destroy();
    if(!g_eggLibLoad_flag)
    {
        printf("program release libegg error !\n");
        exit(-1);
    }

    g_eggLibLoad_flag = eggBaseConfig_destroy();
    if(!g_eggLibLoad_flag)
    {
        printf("program release libegg error !\n");
        exit(-1);
    }


    
    g_eggLibLoad_flag = eggSySRecorder_destroy();
    if(!g_eggLibLoad_flag)
    {
        printf("program release libegg error !\n");
        exit(-1);
    }


    printf("egglib over!\n");
    return ;
}
