#ifndef EGGRWSLOG_H_
#define EGGRWSLOG_H_

enum EGGRWSLOGLEVEL {EGGRWSLOG_INFO, EGGRWSLOG_WARN, EGGRWSLOG_ERROR, EGGRWSLOG_CLAIM};
typedef struct eggRWSLog EGGRWSLOG;
typedef struct eggRWSLog *HEGGRWSLOG;

HEGGRWSLOG eggRWSLog_init(char *logfile);
int eggRWSLog_uninit(HEGGRWSLOG hLog);

int eggRWSLog_setLevel(HEGGRWSLOG hLog, enum EGGRWSLOGLEVEL loglevel);
int eggRWSLog_log_line(HEGGRWSLOG hLog, enum EGGRWSLOGLEVEL level, char *who,
                       const char *format, ...);

#endif
