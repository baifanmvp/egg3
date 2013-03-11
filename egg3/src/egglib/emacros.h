#ifndef _EGG_MACROS_H_
#define _EGG_MACROS_H_


#ifdef __cplusplus
#define E_BEGIN_DECLS extern "C" {
#define E_END_DECLS   }
#else
#define E_BEGIN_DECLS
#define E_END_DECLS
#endif

#ifdef WIN32
#define EGGAPI __stdcall
#else
#define EGGAPI
#endif

#ifdef EGG_TEST
#define EGG_TEST
#else
#define EGG_TEST //
#endif

#define MAKELONGLONG(a, b) ((u64)(((u32)((u64)(a) & 0xFFFFFFFF)) | ((u64)((u32)((u64)(b) & 0xFFFFFFFF))) << 32))
#define LOLONGLONG(ll)     ((u32)((u64)(ll) & 0xFFFFFFFF))
#define HILONGLONG(ll)     ((u32)((u64)(ll) >> 32))


#endif //_EGG_MACROS_H_

