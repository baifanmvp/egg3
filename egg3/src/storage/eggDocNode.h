#ifndef EGG_DOCNODE_H_
#define EGG_DOCNODE_H_

#include "../EggDef.h"

typedef struct eggDocNode  EGGDOCNODE;
typedef struct eggDocNode* HEGGDOCNODE;

#pragma pack(push)
#pragma pack(4)
struct eggDocNode
{
    char type;
    size32_t size;
    count_t count;
    int weight;
   // offset16_t bufOff;
    type_t stype;


    //size32_t tablesize;
};
#pragma pack(pop)

#endif
