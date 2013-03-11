#ifndef _EGG_REG_H_
#define _EGG_REG_H_
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include <EggDef.h>
struct _eggReg
{
    regex_t relpath;
    regex_t abspath;
    regex_t dbname;
    
};

typedef struct _eggReg eggReg;
typedef eggReg  EGGREG; 
typedef eggReg* HEGGREG; 



EBOOL eggReg_init();

EBOOL eggReg_destory();

EBOOL eggReg_is_relPath(const char* filepath);

EBOOL eggReg_is_absPath(const char* filepath);

EBOOL eggReg_is_dbName(const char* gfilepath);

#endif
