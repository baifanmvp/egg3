/**
   \file eggFieldKey.h
   \brief 域键值对
   \ingroup egg
*/
#ifndef EGGFIELDKEY_H_
#define EGGFIELDKEY_H_
#include "./EggDef.h"

E_BEGIN_DECLS

struct eggFieldKey {
    char *fieldName;
    size32_t keySz;
    char *key;
    struct eggFieldKey *next;
};
typedef struct eggFieldKey EGGFIELDKEY;
typedef struct eggFieldKey *HEGGFIELDKEY;
HEGGFIELDKEY eggFieldKey_new(char *fieldName, char *key, size32_t keySz);
HEGGFIELDKEY eggFieldKey_append(HEGGFIELDKEY head, HEGGFIELDKEY tail);
HEGGFIELDKEY eggFieldKey_dup(HEGGFIELDKEY hEggFieldKey);
EBOOL eggFieldKey_equal(HEGGFIELDKEY hEggFieldKey, HEGGFIELDKEY hEggFieldKey2);
void eggFieldKey_del(HEGGFIELDKEY hEggFieldKey);
char * eggFieldKey_serialise(HEGGFIELDKEY hEggFieldKey, size32_t *sz);
HEGGFIELDKEY eggFieldKey_unserialise( char *hEggFieldKey);

E_END_DECLS

#endif  /* EGGFIELDKEY_H_ */
