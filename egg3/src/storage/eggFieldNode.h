#ifndef EGG_FIELDNODE_H_
#define EGG_FIELDNODE_H_

#include "../EggDef.h"

typedef struct eggFieldNode EGGFIELDNODE;
typedef struct eggFieldNode* HEGGFIELDNODE;

#pragma pack(push)
#pragma pack(4)
struct eggFieldNode
{
    size32_t size;
    type_t type;
    offset16_t bufOff;
};
#pragma pack(pop)


#endif
