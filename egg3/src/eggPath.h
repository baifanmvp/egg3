/**
 * eggPath.h
*/
#ifndef EGG_PATH_H_
#define EGG_PATH_H_

#include "EggDef.h"
#include "eggHandle.h"

E_BEGIN_DECLS

HEGGHANDLE eggPath_open(const char * path);

EBOOL eggPath_close(HEGGHANDLE hEggHandle );


E_END_DECLS

#endif //EGG_PATH_H_

