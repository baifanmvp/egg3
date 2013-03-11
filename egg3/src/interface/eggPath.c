#include "../eggHandle.h"
#include "../eggDirectory.h"
#include "../eggCluster.h"
#include "../eggHttp.h"
#include "../eggServiceClient.h"
#include "../eggPath.h"

#define EGGPATH_DIRECTORY       0 
#define EGGPATH_HTTP            1
#define EGGPATH_CLUSTER         2
#define EGGPATH_SERVICECLIENT   3
#define EGGPATH_ERROR          -1

typedef int EGGPATHTYPE;

HEGGHANDLE getPathHandle(const char * path);


#ifdef __CYGWIN__ 
extern flag_t g_eggLibLoad_cygwin;
extern pthread_mutex_t counter_mutex;

#endif


HEGGHANDLE eggPath_open(const char * path)
{

#ifdef __CYGWIN__
    if(g_eggLibLoad_cygwin == EGG_FALSE)
    {
        pthread_mutex_lock(&counter_mutex);
        if(g_eggLibLoad_cygwin == EGG_FALSE)
        {
            eggLibLoad_init();
            g_eggLibLoad_cygwin = EGG_TRUE;
        }
        pthread_mutex_unlock(&counter_mutex);
    }
#endif
        
    HEGGHANDLE hEggHandle;
//    printf("1111111111111\n");  
    hEggHandle = getPathHandle(path);
    
    return hEggHandle;
}
EBOOL eggPath_close(HEGGHANDLE hEggHandle )
{
    if (!hEggHandle)
    {
        return EGG_TRUE;
    }
    return hEggHandle->eggHandle_close(hEggHandle);
}



HEGGHANDLE getPathHandle(const char * path)
{
    if(POINTER_IS_INVALID(path))
        return EGG_NULL;

    if(strncmp(path, "fcgi:", 5) == 0) //fastfcgi
    {
        char *ppos = strchr(path + 5, '/');
        if(!ppos) return EGG_NULL;
        
        char* p_host_name = strndup(path + 5,  (long)ppos - (long)(path+5));
        char* p_local_path = ppos;
        char p_http_path[1024] = {0};
        
        EGG_COMPOSE_HTTPPATH(p_http_path, p_host_name, p_local_path);
        
        return eggHttp_open(p_http_path);
    }
    else if(strncmp(path, "tcp://", 6) == 0) //socket thru tcp port
    {
        return eggServiceClient_open(path + 6);
    }
    else if(strncmp(path, "rws://", 6) == 0) //socket thru tcp port
    {
        return eggServiceClient_open(path + 6);
    }
    else if(strncmp(path, "unixsock://", 11) == 0) //socket thru unix sock file
    {
        return eggServiceClient_open(path + 11);
    }
    else if(strncmp(path, "cluster://", 10) == 0) //socket
    {
        return eggCluster_open(path + 10);
    }
    else if(strncmp(path, "file://", 7) == 0) // local
    {
        return eggDirectory_open(path + 7);
        
    }
    else if(strstr(path, "EGG_DIR_PATH=")) //fastfcgi
    {
        return eggHttp_open(path);
    }
    else
    {
        if(access(path, F_OK) == 0)
        {
            return eggDirectory_open(path);  // local
        }
        else
        {
            return EGG_NULL;
        }
    }
        
}
