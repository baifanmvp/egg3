#ifndef EGG_SYSRECORDER_H_
#define EGG_SYSRECORDER_H_
#include "EggDef.h"


#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#define EGG_SYS_TYPEINFO "typeInfo"
#define EGG_SYS_ANALYNAME "analyName"
#define EGG_SYS_DICTNAME "dictName"
#define EGG_SYS_DICTKEY "dictKey"

typedef struct eggSySRecorder* HEGGSYSRECORDER;
typedef struct eggSySRecorder EGGSYSRECORDER;


PUBLIC EBOOL EGGAPI eggSySRecorder_init();


EBOOL EGGAPI eggSySRecorder_destroy();

void* EGGAPI eggSySRecorder_get_records(char* infotype );

void* EGGAPI eggSySRecorder_get_dict(char* infotype, char* analyName );

void* EGGAPI eggSySRecorder_alloc_reader();

EBOOL EGGAPI eggSySRecorder_free_reader(void** hIndexReader);



#endif
