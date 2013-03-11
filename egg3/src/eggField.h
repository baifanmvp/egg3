
/**
   \file eggField.h
   \brief 数据单元字段 (API)
   \ingroup egg
*/

#ifndef EGG_FIELD_H_
#define EGG_FIELD_H_
#include "./EggDef.h"
#include "./storage/eggFieldNode.h"
#include "./uti/Utility.h"
#include "./eggAnalyzer.h"
E_BEGIN_DECLS

typedef struct eggField EGGFIELD;
typedef struct eggField* HEGGFIELD;


/** \param type Ref EggDef.h
 */
HEGGFIELD EGGAPI eggField_new(const echar* lpName,
                            const echar* lpValue, size32_t nSize,
                              type_t type, ...);

EBOOL EGGAPI eggField_delete(HEGGFIELD hEggField);


EBOOL  eggField_dup(HEGGFIELD hEggField,
                           const echar* lpName, size32_t nNameSize,
                           const echar* lpValue, size32_t nValSize,
                          type_t type, char* analyzerName);

echar* EGGAPI eggField_get_value(HEGGFIELD hEggField, size32_t* lpnSize);

EBOOL EGGAPI eggField_set_value(HEGGFIELD hEggField, const echar* lpValue, size32_t nSize);

type_t EGGAPI eggField_get_type(HEGGFIELD hEggField);

echar* EGGAPI eggField_get_name(HEGGFIELD hEggField);

HEGGFIELD EGGAPI eggField_get_next(HEGGFIELD hEggField);

EBOOL EGGAPI eggField_set_next(HEGGFIELD hEggField, HEGGFIELD hOtherField);

size32_t EGGAPI eggField_get_size(HEGGFIELD hEggField);


EBOOL EGGAPI eggField_set_dictname(HEGGFIELD hEggField, const echar* dictName);


char* EGGAPI eggField_get_dictname(HEGGFIELD hEggField);


EBOOL eggField_compress(HEGGFIELD hEggField);

EBOOL eggField_decompress(HEGGFIELD hEggField);

size32_t eggField_get_serializationsize(HEGGFIELD hEggField);

HEGGFIELDNODE eggField_serialization(HEGGFIELD hEggField);

HEGGFIELD eggField_unserialization(HEGGFIELDNODE hEggFieldNode);

echar* EGGAPI eggField_get_analyzername(HEGGFIELD hEggField);

EBOOL EGGAPI eggField_set_analyzername(HEGGFIELD hEggField, const echar* analyzerName);

EBOOL EGGAPI eggField_append_value(HEGGFIELD hEggField, const echar* lpValue, size32_t nSize);

EBOOL  EGGAPI eggField_set_mask(HEGGFIELD hEggField, size16_t mask);

size16_t EGGAPI eggField_get_mask(HEGGFIELD hEggField);



/* for back-compliance */

#define eggField_set_dictName(hEggField, dictName) eggField_set_dictname(hEggField, dictName) 
#define eggField_set_analyzerName(hEggField, analyzerName) eggField_set_analyzername(hEggField, analyzerName) 
#define eggField_serialSz(hEggField) eggField_get_serializationsize(hEggField) 
#define eggField_get_dictName(hEggField) eggField_get_dictname(hEggField) 
#define eggField_get_analyzerName(hEggField) eggField_get_analyzername(hEggField) 

/* for back-compliance end */



E_END_DECLS

#endif
