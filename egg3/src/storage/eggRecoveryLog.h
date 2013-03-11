#ifndef EGGRECOVERYLOG_H_
#define EGGRECOVERYLOG_H_

#include <stdint.h>

typedef struct ActInfo ActInfo;
enum eggLogType {EGGRECOVERYLOG_COMMIT = 'C',
                 EGGRECOVERYLOG_UNDO = 'U',
                 EGGRECOVERYLOG_UNDOBTREE = 'V',
                 EGGRECOVERYLOG_REDO = 'R',
                 EGGRECOVERYLOG_REDOBTREE = 'S',
};
typedef struct eggRecoveryHandle EGGRECOVERYHANDLE;
typedef struct eggRecoveryHandle *HEGGRECOVERYHANDLE;
HEGGRECOVERYHANDLE eggRecoveryLog_init(char *baseName);
int eggRecoveryLog_destroy(HEGGRECOVERYHANDLE pEggRecoveryHandle);

ActInfo *eggRecoveryLog_beginact(HEGGRECOVERYHANDLE pEggRecoveryHandle);
int eggRecoveryLog_endact(HEGGRECOVERYHANDLE pEggRecoveryHandle, ActInfo *actInfo);
int eggRecoveryLog_writelog(HEGGRECOVERYHANDLE pEggRecoveryHandle,
                            ActInfo *actInfo, uint32_t logType,
                            char *fileName, uint64_t filePosition,
                            void *data, uint64_t size);
int eggRecoveryLog_writelog_con(HEGGRECOVERYHANDLE pEggRecoveryHandle,
                               ActInfo *actInfo, uint32_t logType,
                               char *fileName, uint64_t filePosition,
                               void *data, uint64_t size);
int eggRecoveryLog_make_checkpoint(HEGGRECOVERYHANDLE pEggRecoveryHandle);
int eggRecoveryLog_makeclean_checkpoint(HEGGRECOVERYHANDLE pEggRecoveryHandle);

#endif /*EGGRECOVERYLOG_H_*/
