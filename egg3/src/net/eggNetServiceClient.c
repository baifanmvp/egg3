#include "eggNetServiceClient.h"
#include "./eggNetPackage.h"
#include "./eggNetType.h"
#include "../log/eggPrtLog.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


struct eggNetServiceClient
{
    HEGGNETSOCKET hSocket;
    const eggServiceClientConfig *hEggServiceClientConfig;
};

PUBLIC HEGGNETSERVICECLIENT eggNetServiceClient_new(HEGGSERVICECLIENTCONFIG hEggServiceClientConfig)
{
    HEGGNETSERVICECLIENT hEggNetServiceClient = calloc(1, sizeof(eggNetServiceClient));
    assert(hEggNetServiceClient);
    hEggNetServiceClient->hEggServiceClientConfig = hEggServiceClientConfig;
    return hEggNetServiceClient;
}

PUBLIC EBOOL eggNetServiceClient_connect(HEGGNETSERVICECLIENT hEggNetServiceClient)
{
    if(POINTER_IS_INVALID(hEggNetServiceClient))
    {
        return EGG_FALSE;
    }
    const eggServiceClientConfig *hEggServiceClientConfig;
    if (!(hEggServiceClientConfig = hEggNetServiceClient->hEggServiceClientConfig))
    {
        return EGG_FALSE;
    }
    if (hEggServiceClientConfig->socketfile) /* local unix sock first */
    {
        hEggNetServiceClient->hSocket = eggNetSocket_new(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un st_remote = {};
        st_remote.sun_family = AF_UNIX;
        strncpy(st_remote.sun_path, hEggServiceClientConfig->socketfile, sizeof(st_remote.sun_path)-1);
        if (eggNetSocket_connect(hEggNetServiceClient->hSocket, (struct sockaddr *)&st_remote, sizeof(st_remote)) == EGG_FALSE)
        {
            return EGG_FALSE;
        }
    }
    else if (hEggServiceClientConfig->port)
    {
        hEggNetServiceClient->hSocket = eggNetSocket_new(AF_INET, SOCK_STREAM, 0);
        char* p_ip = hEggServiceClientConfig->ip;
        struct sockaddr_in st_remote = {0};
        st_remote.sin_family = AF_INET;
        inet_pton(AF_INET, p_ip, (void *)(&(st_remote.sin_addr.s_addr)));
        st_remote.sin_port = htons(hEggServiceClientConfig->port);
        if (eggNetSocket_connect(hEggNetServiceClient->hSocket, (struct sockaddr*)&st_remote, sizeof(st_remote)) == EGG_FALSE)
        {
            return EGG_FALSE;
        }
    }
    else
    {
        /* fprintf(stderr, "%s:%d:%s: ERR sockfile == NULL or port == 0\n", */
        /*         __FILE__, __LINE__, __func__); */
        eggPrtLog_error("eggNetServiceClient", "%s:%d:%s: ERR sockfile == NULL or port == 0\n",
                __FILE__, __LINE__, __func__);

        return EGG_FALSE;
    }
    return EGG_TRUE;
}

PUBLIC EBOOL eggNetServiceClient_send(HEGGNETSERVICECLIENT hEggNetServiceClient, epointer ePointer, size32_t size, int flags)
{
    EBOOL retv;
    retv = eggNetSocket_send(hEggNetServiceClient->hSocket, ePointer, size, flags);
    return retv;
}

PUBLIC EBOOL eggNetServiceClient_recv(HEGGNETSERVICECLIENT hEggNetServiceClient, epointer* ePointer, size32_t* lpSize, int flags)
{
    EBOOL retv;
    EGGNETPACKAGE res_package;
    retv = eggNetSocket_recv(hEggNetServiceClient->hSocket, &res_package, sizeof(res_package), flags);
    if (retv == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    *lpSize = eggNetPackage_get_packagesize(&res_package);
    *ePointer = malloc(*lpSize);
    assert(*ePointer);
    memcpy(*ePointer, &res_package, sizeof(EGGNETPACKAGE));
    retv = eggNetSocket_recv(hEggNetServiceClient->hSocket,
                             *ePointer+sizeof(EGGNETPACKAGE), *lpSize-sizeof(EGGNETPACKAGE), flags);
    
    return retv;
}

PUBLIC EBOOL eggNetServiceClient_delete(HEGGNETSERVICECLIENT hEggNetServiceClient)
{
    if(POINTER_IS_INVALID(hEggNetServiceClient))
    {
        return EGG_FALSE;
    }
    
    eggNetSocket_delete(hEggNetServiceClient->hSocket);

    free(hEggNetServiceClient);
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggNetServiceClient_restart_network(HEGGNETSERVICECLIENT hEggNetServiceClient)
{
    if(POINTER_IS_INVALID(hEggNetServiceClient))
    {
        return EGG_FALSE;
    }
    EBOOL retv;
    eggNetSocket_delete(hEggNetServiceClient->hSocket);
    retv = eggNetServiceClient_connect(hEggNetServiceClient);
    return retv;
}

