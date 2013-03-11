#include "eggServiceClientConfig.h"
#include "../log/eggPrtLog.h"
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

static int trimspace(char *str)
{
    char *p;
    p = str;
    while (*str)
    {
        if (isspace(*str))
        {
            str++;
        }
        else
        {
            *p++ = *str++;
        }
    }
    *p = *str;
    return 0;
}

HEGGSERVICECLIENTCONFIG eggServiceClientConfig_new(char *configfile)
{
    char *socketfile = NULL;
    unsigned short port = 0;
    char *ip = NULL;

    unsigned long n_line_sz = 0;
    char* lp_line_str = EGG_NULL;

    FILE *fp;
    if (!(fp = fopen(configfile, "r")))
    {
        /* fprintf(stderr, "%s:%d:%s: ERR read [%s] %s\n", */
        /*         __FILE__, __LINE__, __func__, configfile, strerror(errno)); */
        eggPrtLog_error("eggServiceClientConfig", "%s:%d:%s: ERR read [%s] %s\n",
                __FILE__, __LINE__, __func__, configfile, strerror(errno));
                
        return NULL;
    }

    while(getline(&lp_line_str, &n_line_sz, fp) != -1)
    {
        trimspace(lp_line_str);
        if (strncasecmp(lp_line_str, "port=", 5) == 0)
        {
            long prt;
            prt = strtol(lp_line_str+5, NULL, 10);
            if (prt > 0)
            {
                port = prt;
            }
        }
        else if (strncasecmp(lp_line_str, "ip=", 3) == 0)
        {
            free(ip);
            ip = strdup(lp_line_str+3);
            assert(ip);
        }
        else if (strncasecmp(lp_line_str, "socket=", 7) == 0)
        {
            free(socketfile);
            socketfile = strdup(lp_line_str+7);
            assert(socketfile);
        }
        else
        {
            ;
        }
    }
    free(lp_line_str);
    fclose(fp);
    if (!(port != 0 && ip && ip[0]))
    {
        free(ip);
        ip = NULL;
        port = 0;
    }
    if (port == 0 && socketfile == NULL)
    {
        /* fprintf(stderr, "%s:%d:%s NO port Or socketfile\n", */
        /*         __FILE__, __LINE__, __func__); */
        eggPrtLog_error("eggServiceClientConfig", "%s:%d:%s NO port Or socketfile\n",
                __FILE__, __LINE__, __func__);

        return NULL;
    }
    
    HEGGSERVICECLIENTCONFIG hEggServiceClientConfig;
    hEggServiceClientConfig = (HEGGSERVICECLIENTCONFIG)
        malloc(sizeof(eggServiceClientConfig));
    hEggServiceClientConfig->socketfile = socketfile;
    hEggServiceClientConfig->port = port;
    hEggServiceClientConfig->ip = ip;
    return hEggServiceClientConfig;

}

HEGGSERVICECLIENTCONFIG eggServiceClientConfig_new_default(void)
{
    HEGGSERVICECLIENTCONFIG hEggServiceClientConfig;
    hEggServiceClientConfig = (HEGGSERVICECLIENTCONFIG)
        calloc(1, sizeof(eggServiceClientConfig));
    return hEggServiceClientConfig;
    
}

PUBLIC EBOOL eggServiceClientConfig_delete(HEGGSERVICECLIENTCONFIG hEggServiceClientConfig)
{
    if (!hEggServiceClientConfig)
    {
        return EGG_TRUE;
    }
    free(hEggServiceClientConfig->socketfile);
    free(hEggServiceClientConfig->ip);
    free(hEggServiceClientConfig);
    return EGG_TRUE;
}
