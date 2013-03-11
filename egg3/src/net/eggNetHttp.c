#include "eggNetHttp.h"
#include "./eggNetPackage.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


PRIVATE char* eggNetHttp_get_httpHead(HEGGNETHTTP hNetHttp, int length);

PRIVATE char* eggNetHttp_recv_httpHead(HEGGNETHTTP hNetHttp);

PRIVATE char* eggNetHttp_get_ip(HEGGNETHTTP hNetHttp);

PRIVATE int eggNetHttp_get_length_by_httpHead(char *headInfo);

HEGGNETHTTP eggNetHttp_new(char* url, char* ty)
{
    HEGGNETHTTP lp_net_http = (HEGGNETHTTP)calloc(1, sizeof(EGGNETHTTP));
//    lp_net_http->hSocket = eggNetSocket_new(AF_INET, SOCK_STREAM, 0);
    
    lp_net_http->host = (char*)malloc(strlen(url) + 1);
    lp_net_http->page = (char*)malloc(strlen(url) + 1);
    lp_net_http->ty = (char*)malloc(strlen(ty) + 1);
    
    memcpy(lp_net_http->ty, ty, strlen(ty) + 1);
    
    memcpy(lp_net_http->page, "/", 2);
    
    sscanf(url, "%[^:]:%d%s", lp_net_http->host, &(lp_net_http->port), lp_net_http->page);
    
    return lp_net_http;
}


EBOOL eggNetHttp_connect(HEGGNETHTTP hNetHttp)
{
    if(POINTER_IS_INVALID(hNetHttp))
    {
        return EGG_FALSE;
    }
    
    char* p_ip = hNetHttp->host;//eggNetHttp_get_ip(hNetHttp);
//    printf("eggNetHttp connect:\n host : [%s]\n ip : [%s]\n"" args : [%s] pthread id [%lu]\n", hNetHttp->host, p_ip, hNetHttp->page, pthread_self());
    struct sockaddr_in st_remote = {0};
    
    st_remote.sin_family = AF_INET;
    
    if(inet_pton(AF_INET, p_ip, (void *)(&(st_remote.sin_addr.s_addr))) == EGG_NULL)
    {
        free(p_ip);
        return EGG_FALSE;
    }

    st_remote.sin_port = htons(hNetHttp->port);

    if (eggNetSocket_connect(hNetHttp->hSocket, (struct sockaddr*)&st_remote, sizeof(st_remote)) == EGG_FALSE)
    {
        free(p_ip);
        return EGG_FALSE;
    }
    
//    free(p_ip);
    return EGG_TRUE;
}

EBOOL eggNetHttp_send(HEGGNETHTTP hNetHttp, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetHttp))
    {
        return EGG_FALSE;
    }
    char* lp_head_info = eggNetHttp_get_httpHead(hNetHttp, size);
    
    /*if(!eggNetSocket_send(hNetHttp->hSocket, lp_head_info, strlen(lp_head_info), 0))
    {
        free(lp_head_info);
        return EGG_FALSE;
    }
    if(!eggNetSocket_send(hNetHttp->hSocket, ePointer, size, 0))
    {
        free(lp_head_info);
        return EGG_FALSE;
    }
    */
    int n_head_len = strlen(lp_head_info);
    char* lp_send_buf = (char*)malloc(size + n_head_len);
    
    memcpy(lp_send_buf, lp_head_info, n_head_len);
    memcpy(lp_send_buf + n_head_len, ePointer, size);

    if(!eggNetSocket_send(hNetHttp->hSocket, lp_send_buf, size + n_head_len, flags))
    {
        free(lp_head_info);
        free(lp_send_buf);
        return EGG_FALSE;
    }
    
    free(lp_head_info);
        free(lp_send_buf);
    
    return EGG_TRUE;
}

EBOOL eggNetHttp_recv(HEGGNETHTTP hNetHttp, epointer* ePointer, size32_t* lpSize, int flags)
{
    if(POINTER_IS_INVALID(hNetHttp))
    {
        return EGG_FALSE;
    }
    
    char* lp_head_info = eggNetHttp_recv_httpHead(hNetHttp);
//    printf(lp_head_info);
    *lpSize = eggNetHttp_get_length_by_httpHead(lp_head_info);
    
    if(!(*lpSize))
    {
        //printf("-------------\n");
    }
    *ePointer = malloc(*lpSize);
    
    
    eggNetSocket_recv(hNetHttp->hSocket, *ePointer, *lpSize, flags);
    
    free(lp_head_info);

    return EGG_TRUE;
}


EBOOL eggNetHttp_delete(HEGGNETHTTP hNetHttp)
{
    if(POINTER_IS_INVALID(hNetHttp))
    {
        return EGG_FALSE;
    }
    
    eggNetSocket_delete(hNetHttp->hSocket);
    free(hNetHttp->host);
    free(hNetHttp->page);
    free(hNetHttp->ty);
    free(hNetHttp);
    
    return EGG_TRUE;
}

EBOOL eggNetHttp_restart_network(HEGGNETHTTP hNetHttp)
{
    if(POINTER_IS_INVALID(hNetHttp))
    {
        return EGG_FALSE;
    }
    
    eggNetSocket_delete(hNetHttp->hSocket)    ;
    hNetHttp->hSocket = eggNetSocket_new(AF_INET, SOCK_STREAM, 0);
    return EGG_TRUE;
}


PRIVATE int eggNetHttp_get_length_by_httpHead(char *headInfo)
{
    char* p_iter = headInfo;
    char p_content_length[64];
    int len = 0;
    
    while(p_iter = strstr(p_iter, "\r\n"))
    {
        if(strncmp(p_iter, "\r\n\r\n", 4) == 0)
        {
            return 0;
        }
        p_iter += 2;
        if(strncasecmp(p_iter, "content-length", 14) == 0)
        {
            memcpy(p_iter, "content-length", 14);
            sscanf(p_iter, "content-length:%d\r\n", &len);
            return len;
        }
    }
    
    return 0;
}

PRIVATE char* eggNetHttp_get_httpHead(HEGGNETHTTP hNetHttp, int length)
{
    char *headInfo = (char*)malloc(4096);
    sprintf(headInfo, EGG_HTTP_HEAD, hNetHttp->ty, hNetHttp->page, hNetHttp->host, length);
    return headInfo;
}

PRIVATE char* eggNetHttp_recv_httpHead(HEGGNETHTTP hNetHttp)
{
    char *headInfo = (char*)malloc(4096);
    int len = 0;
    
    while(eggNetSocket_recv(hNetHttp->hSocket, headInfo + len, 1, 0) && len <= 4096)
    {
        len++;
        if(len > 4 && strncmp(headInfo + len - 4, "\r\n\r\n", 4) == 0)
        {
            headInfo[len] = '\0';
            return headInfo;
        }
    }
    
    free(headInfo);
    
    return EGG_NULL;
}

PRIVATE char* eggNetHttp_get_ip(HEGGNETHTTP hNetHttp)
{
  struct hostent *hent;
  int n_ip_len = 15; //XXX.XXX.XXX.XXX
  
  char *ip = (char *)malloc(n_ip_len + 1);
  ip[n_ip_len] = '\0';
  
  if((hent = gethostbyname(hNetHttp->host)) == NULL)
  {
    herror("Can't get IP");
    exit(1);
  }
  
  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, n_ip_len) == NULL)
  {
    perror("Can't resolve host");
    exit(1);
  }
  
  return ip;
}

