#include "eggClientInfoServ.h"
#include "../log/eggPrtLog.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

struct eggInfoServ
{
    char* host;
    int port;
    int fd;
    int retry;
    char *buf;
    size_t bufSz;
    size_t bufcurlen;
};

static int connectIt(int *pfd, char *host, short port);
static int writeIt(int fd, char *buf, size_t size);
    
PUBLIC HEGGINFOSERV eggInfoServ_open(char* host, short port)
{
    HEGGINFOSERV hInfoServ = calloc(1, sizeof(*hInfoServ));
    assert(hInfoServ);
    hInfoServ->port = port;
    hInfoServ->host = strdup(host);
    assert(hInfoServ->host);
    hInfoServ->retry = 3;
    
    int i;
    for (i = 0; i < hInfoServ->retry; i++)
    {
        if (connectIt(&hInfoServ->fd,
                            hInfoServ->host, hInfoServ->port) == 0)
        {
            break;
        }
        int nsleep = 4 << i;
        //fprintf(stderr, "Sleep %ds\n", nsleep);
        eggPrtLog_info("eggClientInfoServer", "Sleep %ds\n", nsleep);
        sleep(nsleep);
    }
    if (i == hInfoServ->retry)
    {
        goto err;
    }
    
    return hInfoServ;
err:
    if (hInfoServ->fd > 0)
    {
        close(hInfoServ->fd);
    }
    free(hInfoServ->host);
    free(hInfoServ);
    return NULL;
}

int connectIt(int *pfd, char *host, short port)
{
    int fd;
    if (*pfd <= 0)
    {
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            //fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));
            eggPrtLog_error("eggClientInfoServer", "%s:%d %s\n", __func__, __LINE__, strerror(errno));
            return -1;
        }

        long flags;
        if ((flags = fcntl(fd, F_GETFD)) < 0)
        {
            /* fprintf(stderr, "%s:%d:%s fcntl F_GETFD ERR %s\n", */
            /*         __FILE__, __LINE__, __func__, strerror(errno)); */
            eggPrtLog_error("eggClientInfoServer", "%s:%d:%s fcntl F_GETFD ERR %s\n",
                    __FILE__, __LINE__, __func__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        flags |= FD_CLOEXEC;
        if (fcntl(fd, F_SETFD, flags) < 0)
        {
            /* fprintf(stderr, "%s:%d:%s fcntl F_SETFD(fd_CLOEXEC) ERR %s\n", */
            /*         __FILE__, __LINE__, __func__, strerror(errno)); */
            eggPrtLog_error("eggClientInfoServer", "%s:%d:%s fcntl F_SETFD(fd_CLOEXEC) ERR %s\n",
                    __FILE__, __LINE__, __func__, strerror(errno));

            exit(EXIT_FAILURE);
        }

        *pfd = fd;
    }
    else
    {
        fd = *pfd;
    }

    int ret;
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, host, &addr.sin_addr);
    addr.sin_port = htons(port);

    if ((ret = connect(fd, &addr, sizeof(addr))) < 0)
    {
        //printf("eggConnectinfo error !\n");
        eggPrtLog_error("eggClientInfoServer", "eggConnectinfo error !\n");
        int e = errno;
        char h[30] = "";
        char p[20] = "";
        struct sockaddr_in laddr;
        int l = sizeof(laddr);
        getsockname(fd, &laddr, &l);
        getnameinfo((struct sockaddr *)&laddr, l,
                    h, sizeof(h),
                    p, sizeof(p), NI_NUMERICHOST|NI_NUMERICSERV);
        /* fprintf(stderr, "%s:%d %s local[%s:%s] remote[%s:%hd]\n", */
	    /*      __func__, __LINE__, strerror(e), h, p, host, port); */
        eggPrtLog_error("eggClientInfoServer", "%s:%d %s local[%s:%s] remote[%s:%hd]\n",
	         __func__, __LINE__, strerror(e), h, p, host, port);

    }
    
    return ret;
}
PUBLIC EBOOL eggInfoServ_close(HEGGINFOSERV hInfoServ)
{
    if (!hInfoServ)
    {
        return EGG_FALSE;
    }
    if (hInfoServ->fd > 0)
    {
        close(hInfoServ->fd);
    }
    free(hInfoServ->host);
    free(hInfoServ->buf);
    free(hInfoServ);

    return EGG_TRUE;
}

PUBLIC HEGGSPANUNIT eggInfoServ_inquire(HEGGINFOSERV hInfoServ,
                                        HEGGSPANUNIT inquire,
                                        count_t *outCount)
{
    HEGGSPANUNIT hEggSpanUnit = NULL;
    *outCount = 0;
    if (!hInfoServ || !inquire)
    {
        return NULL;
    }

    char *inbuf;
    size_t inbufsz;
    if (!(inbuf = eggSpanUnit_to_inquirestring(inquire, &inbufsz)))
    {
        return NULL;
    }
    
    if (writeIt(hInfoServ->fd, inbuf, inbufsz) < 0)
    {
        goto err;
    }

    int fd = hInfoServ->fd;
    char *p;
    size_t len = hInfoServ->bufcurlen;
    int needbytes = 0;
    for ( ; ; )
    {
        hEggSpanUnit = eggSpanUnit_get_spanunits(hInfoServ->buf,
                                                &len,
                                                &needbytes);
        if (hEggSpanUnit)
        {
            *outCount = needbytes;
            break;
        }
        else if (needbytes == 0)
        {
            *outCount = 0;
            break;
        }

        if (hInfoServ->bufcurlen + needbytes > hInfoServ->bufSz)
        {
            hInfoServ->bufSz = hInfoServ->bufcurlen + needbytes;
            hInfoServ->buf = realloc(hInfoServ->buf,
                                     hInfoServ->bufSz);
            assert(hInfoServ->buf);
        }
        p = hInfoServ->buf + hInfoServ->bufcurlen;
        int n;
        if ((n = recv(fd, p, needbytes, 0)) <= 0)
        {
            if (n < 0)
            {
                /* fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, */
                /*         strerror(errno)); */
                eggPrtLog_error("eggClientInfoServer", "%s:%d %s\n", __func__, __LINE__,
                        strerror(errno));
            }
            /* fprintf(stderr, "%s:%d Err fd %d Closed. need %d bytes\n", */
            /*         __func__, __LINE__, fd, needbytes); */
            eggPrtLog_error("eggClientInfoServer", "%s:%d Err fd %d Closed. need %d bytes\n",
                    __func__, __LINE__, fd, needbytes);

            *outCount = 0;
            goto err;
        }
        hInfoServ->bufcurlen += n;
        len = hInfoServ->bufcurlen;
    }
    hInfoServ->bufcurlen -= len;
    memmove(hInfoServ->buf, hInfoServ->buf + len, hInfoServ->bufcurlen);

err:    
    free(inbuf);
    return hEggSpanUnit;
}

int writeIt(int fd, char *buf, size_t bufsz)
{
    int ret = 0;
    char *p = buf;
    int n = bufsz;
    while (n > 0)
    {
        int nn;
        if ((nn = send(fd, p, n, 0)) < 0)
        {
            //fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));
            eggPrtLog_error("eggClientInfoServer", "%s:%d %s\n", __func__, __LINE__, strerror(errno));
            ret = -1;
            break;
        }
        p += nn;
        n -= nn;
    }
    
    return ret == 0 ? bufsz : ret;
}
