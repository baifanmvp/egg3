#include "eggReg.h"
EGGREG* g_reg = EGG_NULL;

EBOOL eggReg_init()
{
    g_reg = (EGGREG*)malloc(sizeof(EGGREG));
    
    if(regcomp(&g_reg->relpath, "^\/*([A-Za-z0-9_]+\/*)*$", REG_EXTENDED | REG_NEWLINE))
    {
        return EGG_FALSE;
    }

    if(regcomp(&g_reg->abspath, "^\/+%%%(\/+[^\/]+)*\/*$", REG_EXTENDED | REG_NEWLINE))
    {
        return EGG_FALSE;
    }

    
    if(regcomp(&g_reg->dbname, "^[A-Za-z0-9_]+$", REG_EXTENDED | REG_NEWLINE))
    {
        return EGG_FALSE;
    }
    return EGG_TRUE;
}

EBOOL eggReg_destory()
{
    regfree(&g_reg->relpath);
    regfree(&g_reg->abspath);
    regfree(&g_reg->dbname);
    return EGG_TRUE;
}

EBOOL eggReg_is_relPath(const char* filepath)
{
    const size_t nmatch = 1;
    regmatch_t pmatch[1];
    
    int status = regexec(&g_reg->relpath, filepath, nmatch, pmatch, 0);

    if(status == 0)
    {
        return EGG_TRUE;
    }
    else
    {
        return EGG_FALSE;
    }
}

EBOOL eggReg_is_absPath(const char* filepath)
{
    const size_t nmatch = 1;
    regmatch_t pmatch[1];
    
    int status = regexec(&g_reg->abspath, filepath, nmatch, pmatch, 0);

    if(status == 0)
    {
        return EGG_TRUE;
    }
    else
    {
        return EGG_FALSE;
    }
}


EBOOL eggReg_is_dbName(const char* filepath)
{
    const size_t nmatch = 1;
    regmatch_t pmatch[1];
    
    int status = regexec(&g_reg->dbname, filepath, nmatch, pmatch, 0);

    if(status == 0)
    {
        return EGG_TRUE;
    }
    else
    {
        return EGG_FALSE;
    }
}
