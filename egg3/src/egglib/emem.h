#ifndef _EGG_MEMORY_H_
#define _EGG_MEMORY_H_

#include "etype.h"
#include "../EggDef.h"

E_BEGIN_DECLS

/*!
  \def EGG_CMP_EQUAL
  \brief 比较相等
*/
#define EGG_CMP_EQUAL   EGG_TRUE

/*!
  \def EGG_CMP_LESS
  \brief 比较小于
*/
#define EGG_CMP_LESS    0xFFFF

/*!
  \def EGG_CMP_GREATER
  \brief 比较大于
*/
#define EGG_CMP_GREATER 0x0002


extern EBOOL EMemCmp(const void* src, const void* dest, size32_t nLen);

extern EBOOL EMemMove(void* dest, const void* src, size32_t nLen);

extern EBOOL EMemCpy(void* dest, const void* src, size32_t nLen);


E_END_DECLS

#endif //_EGG_MEMORY_H_
