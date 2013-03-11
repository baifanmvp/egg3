#ifndef EGG_CLIENTINFOSERV_H
#define EGG_CLIENTINFOSERV_H
#include "../EggDef.h"
#include "eggSpanUnit.h"

E_BEGIN_DECLS

struct eggInfoServ;
typedef struct eggInfoServ eggInfoServ;
typedef struct eggInfoServ *HEGGINFOSERV;

PUBLIC HEGGINFOSERV eggInfoServ_open(char* host, short port);

PUBLIC EBOOL eggInfoServ_close(HEGGINFOSERV hInfoServ);

PUBLIC HEGGSPANUNIT eggInfoServ_inquire(HEGGINFOSERV hInfoServ,
                                        HEGGSPANUNIT inquire,
                                        count_t *outCount);


E_END_DECLS



#endif
