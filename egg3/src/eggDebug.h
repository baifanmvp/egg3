#ifndef _EGG_DEBUG_H_
#define _EGG_DEBUG_H_
#include <stdio.h>

#ifdef EGGDEBUG
#define eggDebug_printf(stream, format, args...)        \
  fprintf(stream, format, ##args)
#else
  #define eggDebug_printf(format,args...)
#endif

#endif
