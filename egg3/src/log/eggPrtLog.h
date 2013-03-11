#ifndef EGGPRTLOG_H_
#define EGGPRTLOG_H_
#include "../EggDef.h"

enum EGGPRTLOGLEVEL {EGGPRTLOG_DEBUG, EGGPRTLOG_INFO, EGGPRTLOG_WARN, EGGPRTLOG_ERROR, EGGPRTLOG_CLAIM};

typedef struct eggPrtLog EGGPRTLOG;
typedef struct eggPrtLog *HEGGPRTLOG;

HEGGPRTLOG eggPrtLog_init(char *logfile);
int eggPrtLog_uninit(HEGGPRTLOG hLog);
int eggPrtLog_set_level(HEGGPRTLOG hLog,
                        enum EGGPRTLOGLEVEL loglevel);
int eggPrtLog_log_line(HEGGPRTLOG hLog,
                       enum EGGPRTLOGLEVEL level, char *who,
                       const char *format, ...);

extern HEGGPRTLOG g_eggPrtLog_handle;
EBOOL eggPrtLog_build();
EBOOL eggPrtLog_destroy();

#define eggPrtLog_debug(who, ...)     \
    eggPrtLog_log_line(g_eggPrtLog_handle,\
                       EGGPRTLOG_DEBUG, who, __VA_ARGS__)
#define eggPrtLog_info(who, ...)     \
    eggPrtLog_log_line(g_eggPrtLog_handle,\
                       EGGPRTLOG_INFO, who, __VA_ARGS__)
#define eggPrtLog_warn(who, ...)     \
    eggPrtLog_log_line(g_eggPrtLog_handle,\
                       EGGPRTLOG_WARN, who, __VA_ARGS__)
#define eggPrtLog_error(who, ...)     \
    eggPrtLog_log_line(g_eggPrtLog_handle,\
                       EGGPRTLOG_ERROR, who, __VA_ARGS__)
#define eggPrtLog_claim(who, ...)     \
    eggPrtLog_log_line(g_eggPrtLog_handle,\
                       EGGPRTLOG_CLAIM, who, __VA_ARGS__)

#endif
