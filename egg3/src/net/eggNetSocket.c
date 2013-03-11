#include "eggNetSocket.h"
#include "../log/eggPrtLog.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#define EGG_MAXSLEEP 128

HEGGNETSOCKET eggNetSocket_new(int damin, int type, int protocol)
{
    HEGGNETSOCKET lp_net_socket = (HEGGNETSOCKET) malloc(sizeof(EGGNETSOCKET));

    int fd;
    fd = socket(damin, type, protocol);

    long flags;
    if ((flags = fcntl(fd, F_GETFD)) < 0)
    {
        /* fprintf(stderr, "%s:%d:%s fcntl F_GETFD ERR %s\n", */
        /*         __FILE__, __LINE__, __func__, strerror(errno)); */
        eggPrtLog_error("eggNetSocket", "%s:%d:%s fcntl F_GETFD ERR %s\n",
                __FILE__, __LINE__, __func__, strerror(errno));

        exit(EXIT_FAILURE);
    }
    flags |= FD_CLOEXEC;
    if (fcntl(fd, F_SETFD, flags) < 0)
    {
        /* fprintf(stderr, "%s:%d:%s fcntl F_SETFD(fd_CLOEXEC) ERR %s\n", */
        /*         __FILE__, __LINE__, __func__, strerror(errno)); */
        eggPrtLog_error("eggNetSocket", "%s:%d:%s fcntl F_SETFD(fd_CLOEXEC) ERR %s\n",
                __FILE__, __LINE__, __func__, strerror(errno));

        exit(EXIT_FAILURE);
    }

    
    struct timeval timeout;      
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    
    if (setsockopt (fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        eggPrtLog_warn("eggNetSocket", "%s:%d:%s setsockopt(SO_SNDTIMEO) ERR %s\n", __FILE__, __LINE__, __func__, strerror(errno));
    }
    
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;
    
    if (setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        eggPrtLog_warn("eggNetSocket", "%s:%d:%s setsockopt(SO_RCVTIMEO) ERR %s\n", __FILE__, __LINE__, __func__, strerror(errno));
    }


    lp_net_socket->sockId = fd;
    return lp_net_socket;
}


EBOOL eggNetSocket_connect(HEGGNETSOCKET hNetSocket, struct sockaddr* addr, socklen_t alen)
{
    if(POINTER_IS_INVALID(hNetSocket))
    {
        return EGG_FALSE;
    }

    int nsec = 1;
    for(nsec = 1; nsec <= EGG_MAXSLEEP; nsec <<= 1)
    {
        int n_sys_err = 0;
        if((n_sys_err = connect(hNetSocket->sockId, addr, alen)) == EGG_SYSCALL_TRUE)
        {

            {
                char host[30] = "";
                char port[20] = "";
                struct sockaddr_in sck;
                int scksz = sizeof(sck);
                getsockname(hNetSocket->sockId, &sck, &scksz);
                getnameinfo(&sck, scksz, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);

                char remoteHost[30] = "";
                char remotePort[20] = "";
                getnameinfo(addr, sizeof(*addr), remoteHost, sizeof(remoteHost), remotePort, sizeof(remotePort), NI_NUMERICHOST|NI_NUMERICSERV);		    
                eggPrtLog_info("eggNetSocket", "info : [file: %s | line : %d] connect fd[%d] local[%s:%s] remote[%s:%s]\n", __FILE__, __LINE__, hNetSocket->sockId, host, port, remoteHost, remotePort);

            }            




            return EGG_TRUE;
        }
        else
        {
            //printf("eggNetSocket connect error !\n");
            eggPrtLog_error("eggNetSocket", "eggNetSocket connect error !\n");

            if(nsec <= EGG_MAXSLEEP)
            {
                {
                    char host[30] = "";
                    char port[20] = "";
                    struct sockaddr_in sck;
                    int scksz = sizeof(sck);
                    getsockname(hNetSocket->sockId, &sck, &scksz);
                    getnameinfo(&sck, scksz, host, sizeof(host), port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);

		    char remoteHost[30] = "";
		    char remotePort[20] = "";
                    getnameinfo(addr, sizeof(*addr), remoteHost, sizeof(remoteHost), remotePort, sizeof(remotePort), NI_NUMERICHOST|NI_NUMERICSERV);		    
                    /* fprintf(stderr, "warnning : [%d:%s] [file: %s | line : %d] local[%s:%s] remote[%s:%s] sleep %ds\n", */
                    /*         errno, strerror(errno), __FILE__, __LINE__, host, port, remoteHost, remotePort, nsec); */
                    eggPrtLog_warn("eggNetSocket", "warnning : [%d:%s] [file: %s | line : %d] local[%s:%s] remote[%s:%s] sleep %ds\n",
                            errno, strerror(errno), __FILE__, __LINE__, host, port, remoteHost, remotePort, nsec);

                }
                
                sleep(nsec);
            }
        }
    }
    return EGG_FALSE;
}

EBOOL eggNetSocket_connect_byhost(HEGGNETSOCKET hNetSocket, char* ip, short port)
{
    if(POINTER_IS_INVALID(hNetSocket))
    {
        return EGG_FALSE;
    }
    
    
    struct sockaddr_in st_remote = {0};
    
    st_remote.sin_family = AF_INET;
    
    if(inet_pton(AF_INET, ip, (void *)(&(st_remote.sin_addr.s_addr))) == EGG_NULL)
    {
        return EGG_FALSE;
    }

    st_remote.sin_port = htons(port);

    if (eggNetSocket_connect(hNetSocket, (struct sockaddr*)&st_remote, sizeof(st_remote)) == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    
    return EGG_TRUE;
}

#define EGG_SEND_LEN (4096*10)

EBOOL eggNetSocket_send(HEGGNETSOCKET hNetSocket, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetSocket))
    {
        return EGG_FALSE;
    }
    size32_t org_size = size;
    
    char* lp_str =  (char*)ePointer;
    while(size)
    {
        int n_send_unit = size > EGG_SEND_LEN? EGG_SEND_LEN : size;

        int n_send;
        
        n_send = send(hNetSocket->sockId, lp_str, n_send_unit, flags | MSG_NOSIGNAL);
        
        // printf("n_send : %d size :%d\n", n_send, size);
        if(n_send == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
	    
            //fprintf(stderr, "sys err : [%s] [file: %s | line : %d]", strerror(errno), __FILE__, __LINE__);
            eggPrtLog_error("eggNetSocket_send", "sys err : [%s] [file: %s | line : %d] [remain: %u/%u] fd[%d]", strerror(errno), __FILE__, __LINE__, size, org_size, hNetSocket->sockId);
            return EGG_FALSE;
        }
        else
        {
            size -= n_send;
            lp_str += n_send;
        }
    }

    eggPrtLog_info("eggNetSocket_send", "fd[%d] send %u\n", hNetSocket->sockId, org_size);

    

    return EGG_TRUE;
}

EBOOL eggNetSocket_recv(HEGGNETSOCKET hNetSocket, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetSocket))
    {
        return EGG_FALSE;
    }

    size32_t org_size = size;

    char* lp_str =  (char*)ePointer;
    while(size)
    {
        int n_recv_unit = size > EGG_SEND_LEN? EGG_SEND_LEN : size;
        int n_recv;
        
        n_recv = recv(hNetSocket->sockId, lp_str, n_recv_unit, flags);

        if(n_recv == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            
            //fprintf(stderr, "sys err : [%s] [file: %s | line : %d]", strerror(errno), __FILE__, __LINE__);
            eggPrtLog_error("eggNetSocket_recv", "sys err : [%s] [file: %s | line : %d] [remain: %u/%u] fd[%d]", strerror(errno), __FILE__, __LINE__, size, org_size, hNetSocket->sockId);
            return EGG_FALSE;
        }
        else if(n_recv == 0)
        {
            //fprintf(stderr, "sys err : [%s] [file: %s | line : %d]", strerror(errno), __FILE__, __LINE__);
            eggPrtLog_error("eggNetSocket_recv", "sys err : [%s] remain[%u/%u] [file: %s | line : %d] fd[%d]", "peer shutdown receive 0", size, org_size, __FILE__, __LINE__, hNetSocket->sockId);
            return EGG_FALSE;
        }   
        else
        {
            size -= n_recv;
            lp_str += n_recv;
        }
    }

    eggPrtLog_info("eggNetSocket_recv", "fd[%d] receive %u\n", hNetSocket->sockId, org_size);
    
    return EGG_TRUE;
}


EBOOL eggNetSocket_delete(HEGGNETSOCKET hNetSocket)
{
    if(POINTER_IS_INVALID(hNetSocket))
    {
        return EGG_FALSE;
    }
    
    close(hNetSocket->sockId);
    free(hNetSocket);
    
    return EGG_TRUE;
}
