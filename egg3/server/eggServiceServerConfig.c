#include "eggServiceServerConfig.h"
#include <egg3/conf/eggConfig.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

int trimspace(char *str)
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

HEGGSERVICESERVERCONFIG eggServiceServerConfig_new(char *configfile)
{
    char *socketfile = NULL;
    unsigned short port = 0;
    char *ip = NULL;

    unsigned long n_line_sz = 0;
    char* lp_line_str = EGG_NULL;
    int loglevel = 1;
    char* logfile = EGG_NULL;
    if (configfile)
    {

        HEGGCONFIG hConf = eggConfig_load_config(configfile);
        eggCfgVal* p_list_val;
        
        p_list_val = eggConfig_get_cfg(hConf, "ip");
        if(p_list_val)
        {

            GList* list_value_iter = g_list_first(p_list_val);
            ip = strdup(list_value_iter->data);
            assert(ip);
        }
        
        p_list_val = eggConfig_get_cfg(hConf, "port");
        if(p_list_val)
        {
            GList* list_value_iter = g_list_first(p_list_val);
            char *p_tmp;
            long prt;
            p_tmp = list_value_iter->data;
            prt = strtol(p_tmp, NULL, 10);
            if (prt > 0)
            {
                port = prt;
            }
        }

        p_list_val = eggConfig_get_cfg(hConf, "socket");
        if(p_list_val)
        {

            GList* list_value_iter = g_list_first(p_list_val);
            socketfile = strdup(list_value_iter->data);
            assert(socketfile);
        }

        p_list_val = eggConfig_get_cfg(hConf, "logpath");
        if(p_list_val)
        {

            GList* list_value_iter = g_list_first(p_list_val);
            logfile = strdup(list_value_iter->data);
            assert(logfile);
        }
        else
        {
            printf("[WARN] eggd logfile is not set!\n");
        }
        
        p_list_val = eggConfig_get_cfg(hConf, "loglevel");
        if(p_list_val)
        {

            GList* list_value_iter = g_list_first(p_list_val);
            loglevel = atoi(list_value_iter->data);
        }
        else
        {
            printf("[WARN] eggd loglevel is not set!\n");
        }
        
        
        eggConfig_delete(hConf);


        /* FILE *fp; */
        /* if (!(fp = fopen(configfile, "r"))) */
        /* { */
        /*     fprintf(stderr, "%s:%d:%s: ERR read [%s] %s\n", */
        /*             __FILE__, __LINE__, __func__, configfile, strerror(errno)); */
        /*     return NULL; */
        /* } */

        /* while(getline(&lp_line_str, &n_line_sz, fp) != -1) */
        /* { */
        /*     trimspace(lp_line_str); */
        /*     if (strncasecmp(lp_line_str, "port=", 5) == 0) */
        /*     { */
        /*         long prt; */
        /*         prt = strtol(lp_line_str+5, NULL, 10); */
        /*         if (prt > 0) */
        /*         { */
        /*             port = prt; */
        /*         } */
        /*     } */
        /*     else if (strncasecmp(lp_line_str, "ip=", 3) == 0) */
        /*     { */
        /*         free(ip); */
        /*         ip = strdup(lp_line_str+3); */
        /*         assert(ip); */
        /*     } */
        /*     else if (strncasecmp(lp_line_str, "socket=", 7) == 0) */
        /*     { */
        /*         free(socketfile); */
        /*         socketfile = strdup(lp_line_str+7); */
        /*         assert(socketfile); */
        /*     } */
        /*     else */
        /*     { */
        /*         ; */
        /*     } */
        /* } */
        /* free(lp_line_str); */
        /* fclose(fp); */
        
        if (!(port != 0 && ip && ip[0]))
        {
            free(ip);
            ip = NULL;
            port = 0;
        }
        if (port == 0 && socketfile == NULL)
        {
            fprintf(stderr, "%s:%d:%s NO port Or socketfile in [%s]\n",
                    __FILE__, __LINE__, __func__, configfile);
            return NULL;
        }
    }
    
    HEGGSERVICESERVERCONFIG hEggServiceServerConfig;
    hEggServiceServerConfig = (HEGGSERVICESERVERCONFIG)
        malloc(sizeof(eggServiceServerConfig));
    hEggServiceServerConfig->socketfile = socketfile;
    hEggServiceServerConfig->port = port;
    hEggServiceServerConfig->ip = ip;
    hEggServiceServerConfig->logfile = logfile;
    hEggServiceServerConfig->loglevel = loglevel;
    return hEggServiceServerConfig;

}

PUBLIC EBOOL eggServiceServerConfig_delete(HEGGSERVICESERVERCONFIG hEggServiceServerConfig)
{
    if (!hEggServiceServerConfig)
    {
        return EGG_TRUE;
    }
    free(hEggServiceServerConfig->socketfile);
    free(hEggServiceServerConfig->logfile);
    free(hEggServiceServerConfig->ip);
    free(hEggServiceServerConfig);
    return EGG_TRUE;
}
