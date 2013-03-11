#include "eggSpanUnit.h"
#include "../log/eggPrtLog.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <assert.h>


char *eggSpanUnit_to_inquirestring(HEGGSPANUNIT hEggSpanUnit, size_t *bufsize)
{
    if (!hEggSpanUnit || !bufsize)
    {
        return NULL;
    }
    char *buf = NULL;
    *bufsize = 0;
    
    if (hEggSpanUnit->hostAddress && hEggSpanUnit->hostAddress[0])
    {
        if (hEggSpanUnit->eggDirPath && hEggSpanUnit->eggDirPath[0])
        {
            char *hostaddress = hEggSpanUnit->hostAddress;
            char *eggdirpath = hEggSpanUnit->eggDirPath;
            size_t allocSz = strlen(hostaddress) + strlen(eggdirpath) + 100;
            
            buf = malloc(allocSz);
            assert(buf);
            buf[allocSz-1] = '\0';
            snprintf(buf, allocSz,
                     "Inquire hostaddress=%s&eggdirpath=%s\n",
                     hostaddress, eggdirpath);
        }
        else
        {
            char *hostaddress = hEggSpanUnit->hostAddress;
            size_t allocSz = strlen(hostaddress) + 100;
            
            buf = malloc(allocSz);
            assert(buf);
            buf[allocSz-1] = '\0';
            snprintf(buf, allocSz,
                     "Inquire hostaddress=%s\n",
                     hostaddress);
        }
    }
    else if (hEggSpanUnit->eggDirPath && hEggSpanUnit->eggDirPath[0])
    {
        if (isValidSPANRANGE(hEggSpanUnit->range))
        {
            char *eggdirpath = hEggSpanUnit->eggDirPath;
            size_t allocSz = strlen(eggdirpath) + 100;

            buf = malloc(allocSz);
            assert(buf);
            buf[allocSz-1] = '\0';
            snprintf(buf, allocSz,
                     "Inquire eggdirpath=%s&range=%llu,%llu\n",
                     eggdirpath,
                     (long long unsigned)hEggSpanUnit->range.start,
                     (long long unsigned)hEggSpanUnit->range.end);
        }
        else
        {
            char *eggdirpath = hEggSpanUnit->eggDirPath;
            size_t allocSz = strlen(eggdirpath) + 100;
            
            buf = malloc(allocSz);
            assert(buf);
            buf[allocSz-1] = '\0';            
            snprintf(buf, allocSz,
                     "Inquire eggdirpath=%s\n",
                     eggdirpath);
        }
    }
    else
    {
        //fprintf(stderr, "Err: UnKnown inquire\n");
        eggPrtLog_warn("eggSpanUnit", "Err: UnKnown inquire\n");
    }
    
    if (buf)
    {
        *bufsize = strlen(buf);
    }
    return buf;
}

HEGGSPANUNIT eggSpanUnit_from_inquirestring(char *inquireString, int *len)
{
    *len = 0;
    char *end;
    if (!(end = strchr(inquireString, '\n')))
    {
        return NULL;
    }
    *end = '\0';                /* stop to '\n' */
    char *p;
    p = inquireString;
    while (isspace(*p))
    {
        p++;
    }

    char *eggDirPath = NULL;
    char *hostAddress = NULL;
    SPANRANGE range = {};
    if (strncasecmp(p, "Inquire hostaddress=", 20) == 0)
    {
        hostAddress = p + 20;
        if ((p = strchr(hostAddress, '&'))
            && strncasecmp(p+1, "eggdirpath=", 11) == 0)
        {
            *p = '\0';            
            eggDirPath = p + 12;   /* "&eggdirpath=" */
            p = eggDirPath;
            while (*p && !isspace(*p))
            {
                p++;
            }
            *p = '\0';
        }
        else if (p)             /* p[0] == '&' */
        {
            *p = '\0';
        }
        else
        {
            p = hostAddress;
            while (*p && !isspace(*p))
            {
                p++;
            }
            *p = '\0';
        }
    }
    else if (strncasecmp(p, "Inquire eggdirpath=", 19) == 0)
    {
        eggDirPath = p + 19;
        if ((p = strchr(eggDirPath, '&'))
            && strncasecmp(p+1, "range=", 6) == 0)
        {
            *p = '\0';
            char *e;
            p = p + 7;   /* "&range=" */
            range.start = strtoull(p, &e, 10);
            if (p == e)
            {
                range.start = 0;
            }
            if (*e)
            {
                p = e + 1;          /* skip ',' */
            }
            else
            {
                p = e;
            }
            range.end = strtoull(p, &e, 10);
            if (p == e)
            {
                range.end = range.start;
            }
        }
        else if (p)             /* p[0] == '&' */
        {
            *p = '\0';
        }
        else
        {
            p = eggDirPath;
            while (*p && !isspace(*p))
            {
                p++;
            }
            *p = '\0';
        }
    }
    else
    {
        /* fprintf(stderr, "%s:%d: unknown inquire: %.*s\n", __func__, __LINE__, */
        /*         end - inquireString, inquireString); */
        eggPrtLog_error("eggSpanUnit", "%s:%d: unknown inquire: %.*s\n", __func__, __LINE__,
                end - inquireString, inquireString);

        *len = end - inquireString + 1; /* skip \n */
        return NULL;
    }
    
    *len = end - inquireString + 1; /* skip \n */
    HEGGSPANUNIT hEggSpanUnit = calloc(1, sizeof(*hEggSpanUnit));
    assert(hEggSpanUnit);
    if (eggDirPath)
    {
        hEggSpanUnit->eggDirPath = strdup(eggDirPath);
    }
    if (hostAddress)
    {
        hEggSpanUnit->hostAddress = strdup(hostAddress);
    }
    if (range.start != 0 && range.end != 0) /* 0-2 ToDo */
    {
        hEggSpanUnit->range = range;
    }
    return hEggSpanUnit;
}

int eggSpanUnit_free(HEGGSPANUNIT hEggSpanUnit, int count)
{
    if (!hEggSpanUnit || count == 0)
    {
        return 0;
    }

    while (count--)
    {
        free(hEggSpanUnit[count].hostAddress);
        free(hEggSpanUnit[count].eggDirPath);
    }
    free(hEggSpanUnit);
    return 0;
}

char *eggSpanUnit_to_string(HEGGSPANUNIT hEggSpanUnit, int count,
                                   size_t *bufsize)
{
    if (!hEggSpanUnit)
    {
        count = 0;
    }
    *bufsize = 0;
    int i;
    char *p;
    
    for (i = 0; i < count; i++)
    {
        assert(hEggSpanUnit[i].eggDirPath && hEggSpanUnit[i].eggDirPath[0]);
        assert(hEggSpanUnit[i].hostAddress && hEggSpanUnit[i].hostAddress[0]);
        assert(isValidSPANRANGE(hEggSpanUnit[i].range));
        char b[50];
        sprintf(b, "%llu,%llu",
                (long long unsigned)hEggSpanUnit[i].range.start,
                (long long unsigned)hEggSpanUnit[i].range.end);
        *bufsize += strlen(hEggSpanUnit[i].eggDirPath) + 1 /* space */
            + strlen(hEggSpanUnit[i].hostAddress) + 1      /* space */
            + strlen(b) + 1;                               /* \n */
    }
    *bufsize += 10;                   /* head "#12345678\n" */
    char *buf = malloc(*bufsize + 1); /* '\0' */
    assert(buf);
    p = buf;
    sprintf(buf, "#%8d\n", count);
    assert(buf[10] == '\0');    /* count is too long */
    p = buf + 10;
    for (i = 0; i < count; i++)
    {
        sprintf(p, "%s %s %llu,%llu\n",
                hEggSpanUnit[i].eggDirPath, hEggSpanUnit[i].hostAddress,
                (long long unsigned)hEggSpanUnit[i].range.start,
                (long long unsigned)hEggSpanUnit[i].range.end);
        p += strlen(p);
    }
    return buf;
}

HEGGSPANUNIT eggSpanUnit_get_spanunits(char *buf, size_t *size,
                                      int *cntUnit_needMore)
{
    int i;
    char *p;

    if (*size < 10)             /* head "#12345678\n" */
    {
        *size = 0;
        *cntUnit_needMore = 10 - *size;
        return NULL;
    }
    int count = strtol(buf+1, &p, 10);      /* skip '#' */
    assert(*p == '\n');
    p++;
    i = 0;
    while (p < buf + *size && i < count)
    {
        if (*p++ == '\n')
        {
            i++;
        }
    }
    if (i < count)
    {
        *size = 0;
        *cntUnit_needMore = 256;
        return NULL;
    }
    
    HEGGSPANUNIT hEggSpanUnit = malloc(count * sizeof(*hEggSpanUnit));
    assert(hEggSpanUnit);
    
    char *org = buf;
    buf = strchr(buf, '\n') + 1; /* skip "#12345678\n" */
    for (i = 0; i < count; i++)
    {                          /* eggDirPath hostAdress start,end\n */
        char *hostaddress = 0;
        char *eggdirpath = 0;
        SPANRANGE range;

        p = buf;
        while(*p != ' ')
            p++;
        *p++ = '\0';
        eggdirpath = strdup(buf);
        assert(eggdirpath);
        buf = p;
        while(*p != ' ')
            p++;
        *p++ = '\0';
        hostaddress = strdup(buf);
        assert(hostaddress);
        buf = p;
        range.start = strtoull(buf, &p, 10);
        buf = ++p;                  /* skip ',' */
        range.end = strtoull(buf, &p, 10);
        buf = ++p;              /* skip '\n' */

        hEggSpanUnit[i].hostAddress = hostaddress;
        hEggSpanUnit[i].eggDirPath = eggdirpath;
        hEggSpanUnit[i].range = range;
    }
    *size = buf - org;
    *cntUnit_needMore = count;
    return hEggSpanUnit;
}
