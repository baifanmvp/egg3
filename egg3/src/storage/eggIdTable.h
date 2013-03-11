#ifndef EGG_IDTABLE_H_
#define EGG_IDTABLE_H_
#include "../EggDef.h"
#include "./File.h"
#include<sys/mman.h>
#define TABLE_INCREMENT_CNT (4096*4)

#define TABLE_INCREMENT_SIZE (TABLE_INCREMENT_CNT*sizeof(EGGIDTABLENODE))

#define TABLE_ID_DELETE -1
#define TABLE_ID_OVERFLOW -2

#pragma pack(push)
#pragma pack(4)

typedef struct eggIdTableInfo EGGIDTABLEINFO;
typedef struct eggIdTableInfo* HEGGIDTABLEINFO;


struct eggIdTableInfo
{
    did_t docCnt;
    did_t tableCnt;
};

typedef struct eggIdTableNode EGGIDTABLENODE;
typedef struct eggIdTableNode* HEGGIDTABLENODE;

struct eggIdTableNode
{
    offset64_t docOff;
};

#pragma pack(pop)

typedef struct eggIdTable EGGIDTABLE;
typedef struct eggIdTable* HEGGIDTABLE;
struct eggIdTable
{
  HEGGFILE hEggFile;
  HEGGIDTABLEINFO hIdTableInfo;
  
};

HEGGIDTABLE eggIdTable_new(HEGGFILE hEggFile);

HEGGIDTABLE eggIdTable_delete(HEGGIDTABLE hIdTable);

did_t eggIdTable_map_id(HEGGIDTABLE hIdTable, offset64_t docOff);

offset64_t eggIdTable_get_off(HEGGIDTABLE hIdTable, did_t id);

EBOOL eggIdTable_update_off(HEGGIDTABLE hIdTable, did_t id, offset64_t docOff);

EBOOL eggIdTable_rdlock_id(HEGGIDTABLE hIdTable, did_t id);

EBOOL eggIdTable_wrlock_id(HEGGIDTABLE hIdTable, did_t id);

EBOOL eggIdTable_unlock_id(HEGGIDTABLE hIdTable, did_t id);
#endif
