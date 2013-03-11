#ifndef EGG2_RECOVERY_H
#define EGG2_RECOVERY_H
/**
  \page recovery_module 日志模块

   \section section_eggrecoverymodule eggRecovery模块
   \subsection eggRecovery_introduction 1. Introduction
      为使程序意外退出时恢复至数据一致性状态。
   \subsection eggRecovery_impl 2. Implementation

   有2个文件: egg.rlog, egg.rlog.info. egg.rlog为日志记录，egg.rlog.info为相应索引。

   采用WAL(write-ahead-logging)方式。

   写一条记录：先写入原有记录,undo record，再写入改写值, redo record.

   恢复: 写入redo  record, 写入未提交的undo record.

   每一Log record类型：commit, undo, redo.

   每一Log record 以在文件的位置最为唯一性标识 Lsn (log sequence number).

   每一Log record 以链表的形式形成一次act.

   act 是原子的。一个正常的act结束是commit类型的Log record.

   egg.rlog文件数据结构:
   \code
   typedef struct {
   uint64_t prevLsn;
   uint64_t undoNextLsn;
   uint32_t logType;
   uint32_t actId;
   uint64_t filePosition;
   uint64_t size;
   char *fileName;
   char *data;
   } eggLogRecord;
   \endcode
   每次数据恢复从含LOGFLAG_CHECKPOINT标志的Log record开始。

   当Log record 含LOGFLAG_WRITTEN标志，表示已经写入磁盘。

   egg.rlog.info文件数据结构:
   \code
   typedef struct {
   uint64_t Lsn;
   uint64_t flag;
   } eggLogInfo;
   enum eggLogInfoFlag { LOGFLAG_WRITTEN = 'W', LOGFLAG_CHECKPOINT = 'K' };
   \endcode
   每次数据修改， 成为act，需分配一个actId，并生成相应Log record.

   数据结构如下:
   \code
   struct ActInfo {
   uint64_t actId;
   uint64_t lastLsn;
   uint64_t undoNextLsn;
   };
   \endcode
   \subsection eggRecovery_interface 3. Interface
   初始化
   \code
   HEGGRECOVERYHANDLE eggRecoveryLog_init(char *baseName);
   \endcode
   清理
   \code
   int eggRecoveryLog_destroy(HEGGRECOVERYHANDLE pEggRecoveryHandle);
   \endcode
   开始1个act
   \code
   ActInfo *eggRecoveryLog_beginAct(HEGGRECOVERYHANDLE pEggRecoveryHandle);
   \endcode
   act的1次修改
   \code
   int eggRecoveryLog_writeLog(HEGGRECOVERYHANDLE pEggRecoveryHandle,
                            ActInfo *actInfo, uint32_t logType,
                            char *fileName, uint64_t filePosition,
                            void *data, uint64_t size);
   \endcode
   act的1次修改,确定已经写入磁盘
   \code
   int eggRecoveryLog_writeLogCon(HEGGRECOVERYHANDLE pEggRecoveryHandle,
                               ActInfo *actInfo, uint32_t logType,
                               char *fileName, uint64_t filePosition,
                               void *data, uint64_t size);
   \endcode
   结束1个act
   \code
   int eggRecoveryLog_endAct(HEGGRECOVERYHANDLE pEggRecoveryHandle, ActInfo *actInfo);
   \endcode
   恢复同时写入LOGFLAG_CHECKPOINT
   \code
   int eggRecoveryLog_makeCheckpoint(HEGGRECOVERYHANDLE pEggRecoveryHandle);
   \endcode
   恢复同时删除以前记录
   \code
   int eggRecoveryLog_makeCleanCheckpoint(HEGGRECOVERYHANDLE pEggRecoveryHandle);
   \endcode
   
   \subsection eggRecovery_ref 4. Reference
        \li ARIES: A Transaction Recovery Method Supporting Fine-Granularity Locking
                  and Partial Rollbacks Using Write-Ahead Logging
        \li ARIES/lM: An Efficient and High Concurrency index Management Method
                  Using Write-Ahead Logging


*/
#endif  /*  EGG2_RECOVERY_H */
