#ifndef _EGG_TYPE_H_
#define _EGG_TYPE_H_

#include "./emacros.h"

E_BEGIN_DECLS

typedef char   echar;
typedef unsigned char uechar;
typedef short  eshort;
typedef long   elong;
typedef unsigned int esize;

typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned int     u32;
typedef unsigned long long u64;

typedef char             i8;
typedef short            i16;
typedef int              i32;
typedef long long        i64;

/*
#ifndef _EGG_LEGACY_TYPE_DEFINE
#define _EGG_LEGACY_TYPE_DEFINE

#endif //_EGG_LEGACY_TYPE_DEFINE
*/

#ifndef _EGG_POINTER_TYPE_DEFINE
#define _EGG_POINTER_TYPE_DEFINE
typedef void* epointer;
typedef const void* ecpointer;
#endif //_EGG_POINTER_TYPE_DEFINE


#ifndef _EGG_BOOLEAN_TYPE_DEFINE
#define _EGG_BOOLEAN_TYPE_DEFINE
typedef u16 EBOOL;
#endif //_EGG_BOOLEAN_TYPE_DEFINE

#ifndef _EGG_HANDLE_TYPE_DEFINE
#define _EGG_HANDLE_TYPE_DEFINE
#ifndef WIN32
typedef int EHANDLE;
#else
typedef HANDLE EHANDLE;
#endif
#endif //_EGG_HANDLE_TYPE_DEFINE


#ifndef _EGG_KEY_TYPE_DEFINE
#define _EGG_KEY_TYPE_DEFINE
#define _PTR     void*
#define _OFFSETPTR void*
#define _FUNCPTR   void*
#define _MODULEPTR void*
#define _OBJECTPTR void*
#define _IDPTR     void*
#define _ITFPTR    void*
#define _AND     ,
#define _NOARGS  void
#define _VOID    void
#define _EXFUN(name, arglist) name arglist
#endif //_EGG_KEY_TYPE_DEFINE



#define PUBLIC
#define PRIVATE  static
#define GLOBAL   extern
#define SESSION0 
#define SESSION1 static
#define LOCAL



typedef EBOOL (*EFnEqual)   (_PTR a,
                             _PTR b);

typedef EBOOL (*EFnSort)    (_PTR a,
                             esize size);

typedef EBOOL (*EFnAnalyze) (const _PTR a,
                             _PTR b);

typedef EBOOL (*EFnSerialize) (_PTR a);


E_END_DECLS

#endif //_EGG_TYPE_H_
