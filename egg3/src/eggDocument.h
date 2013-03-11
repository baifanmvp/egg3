#ifndef EGG_DOCUMENT_H_
#define EGG_DOCUMENT_H_

/**
   \file eggDocument.h
   \brief 数据单元 (API)
   \ingroup egg
*/

#include "./EggDef.h"
#include "eggField.h"
#include "./storage/eggFieldNode.h"
#include "./storage/eggDocNode.h"
#include "./uti/Utility.h"


E_BEGIN_DECLS

typedef struct eggDocument eggDocument;
typedef struct eggDocument EGGDOCUMENT;
typedef struct eggDocument* HEGGDOCUMENT;

HEGGDOCUMENT EGGAPI eggDocument_new();

EBOOL EGGAPI eggDocument_delete(HEGGDOCUMENT hEggDocument);

EBOOL EGGAPI eggDocument_add(HEGGDOCUMENT hEggDocument, HEGGFIELD hEggField);

HEGGFIELD EGGAPI eggDocument_get_field(HEGGDOCUMENT hEggDocument, const echar* lpNameField);

size32_t EGGAPI eggDocument_get_doc(HEGGDOCUMENT hEggDocument, const echar* lpNameField, echar** lppDoc);

EBOOL EGGAPI eggDocument_set_doc(HEGGDOCUMENT hEggDocument,
                                 const echar* lpNameField,
                                 const echar* lpDoc, size32_t nDocSize);

HEGGDOCNODE  eggDocument_serialization(HEGGDOCUMENT hEggDocument);

HEGGDOCUMENT  eggDocument_unserialization(HEGGDOCNODE hEggFieldNode);


EBOOL EGGAPI eggDocument_set_weight(HEGGDOCUMENT hEggDocument, int weight);

int EGGAPI eggDocument_get_weight(HEGGDOCUMENT hEggDocument);

HEGGFIELD EGGAPI eggDocument_remove_field_byname(HEGGDOCUMENT hEggDocument, const echar* lpNameField);

#define EGGDOCUMENT_IS_INVALID(hEggDocument) \
    (!hEggDocument? EGG_TRUE : EGG_FALSE)

/* for back-compliance */

#define eggDocument_removeField_byName(hEggDocument, lpNameField) eggDocument_remove_field_byname(hEggDocument, lpNameField) 

/* for back-compliance end */


E_END_DECLS

#endif //_EGG_DOCUMENT_H_

