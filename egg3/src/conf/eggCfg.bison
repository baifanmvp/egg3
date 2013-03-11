
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eggConfig.h"
extern char dbName[64];
char subTabName[128];

void getDBname(char *t);
void getSubTabName(char *tabName);

extern int cfglex();
extern void cfgerror(void*, void*);

%}

%union
{
char str[256];
}
%parse-param {eggConfig* pConfig}
%name-prefix "cfg"

%token <str>DLLNAME NAME_R NAME_C

%token <str>FSOCK_D
            FIP_D
            FPORT_D
            LOGPATH_D
            LOGLEVEL_D
            FSOCK
            FIP
            FPORT
            FIP_R
            FPORT_R
            CONNECT_R
            LOGFILE_R
            WORKDIR_R
            MEMEXE_R
            SYNCFREQ_R
            EXPORTEXE_R
            COUNTER_R
            MSERVERMAX_R
            NOWAIT_R
            FLISTEN_C
            LOGPATH_C
            LOGLEVEL_C
            BASEPATH_CORE
            LOGPATH_CORE
            LOGLEVEL_CORE

%token <str>BOOL NUMBER PATH IP EGGPATH NOTE TIME


%token START
%token END
%token PARMERROR

%%
exp :  start | end |
       sockd |
       sock  |
       rws |
       cluster |
       core | 
       note  ;



cluster : NAME_C  { getDBname($1); 
                    eggConfig_set_cfg(pConfig, "dbName", dbName);
                    } |
          LOGPATH_C '=' PATH     {eggConfig_set_cfg(pConfig, $1, $3); };|
          LOGLEVEL_C '=' NUMBER     {eggConfig_set_cfg(pConfig, $1, $3); };|
          
          FLISTEN_C IP ':' NUMBER {eggConfig_set_cfg(pConfig, $1, $2);
                                   eggConfig_set_cfg(pConfig, $1, $4);
                                   } |
          path_ctunit;

path_ctunit : path_ctunit1 | path_ctunit2 | path_ctunit3 | path_ctunit4;

path_ctunit1: '[' NUMBER ',' NUMBER ']'  EGGPATH 
                {getSubTabName("eggpath");
                 eggConfig_set_cfg(pConfig, subTabName, $2);
                 eggConfig_set_cfg(pConfig, subTabName, $4);
                 eggConfig_set_cfg(pConfig, subTabName, $6);                                
                 }

path_ctunit2: '[' NUMBER ','  ']'  EGGPATH 
                {getSubTabName("eggpath");
                 eggConfig_set_cfg(pConfig, subTabName, $2);
                 eggConfig_set_cfg(pConfig, subTabName, "NULL");
                 eggConfig_set_cfg(pConfig, subTabName, $5);                                
                 }

path_ctunit3: '['  ',' NUMBER ']'  EGGPATH 
                {getSubTabName("eggpath");
                 eggConfig_set_cfg(pConfig, subTabName, "NULL");
                 eggConfig_set_cfg(pConfig, subTabName, $3);
                 eggConfig_set_cfg(pConfig, subTabName, $5);                                
                 }

path_ctunit4: '['  ','  ']'  EGGPATH 
                {getSubTabName("eggpath");
                 eggConfig_set_cfg(pConfig, subTabName, "NULL");
                 eggConfig_set_cfg(pConfig, subTabName, "NULL");
                 eggConfig_set_cfg(pConfig, subTabName, $4);                                
                 }


rws : NAME_R   { getDBname($1); eggConfig_set_cfg(pConfig, "dbName", dbName); } | 
      port_rwsunit | ip_rwsunit | path_rwsunit |
      con_rwsunit | log_rwsunit | wdir_rwsunit |
      memname_rwsunit | syncfreq_rwsunit |
      mservermax_rwsunit | nowait_rwsunit |
      export_rwsunit | counter_rwsunit;
port_rwsunit : FPORT_R '=' NUMBER     {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };




ip_rwsunit   : FIP_R '=' IP         {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };

path_rwsunit : EGGPATH              {getSubTabName("eggpath");
                                       eggConfig_set_cfg(pConfig, subTabName, $1); 
                                       };



con_rwsunit  : CONNECT_R '=' NUMBER {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };

log_rwsunit :  LOGFILE_R '=' PATH   {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };

wdir_rwsunit : WORKDIR_R '=' PATH      {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };

memname_rwsunit : MEMEXE_R '=' PATH   {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };

syncfreq_rwsunit : SYNCFREQ_R '=' NUMBER {getSubTabName($1);eggConfig_set_cfg(pConfig, subTabName, $3); }
                                           |SYNCFREQ_R '=' TIME  {getSubTabName($1); eggConfig_set_cfg(pConfig, subTabName, $3);}                                     


export_rwsunit : EXPORTEXE_R '=' PATH  {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };

counter_rwsunit : COUNTER_R '=' BOOL {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };

mservermax_rwsunit : MSERVERMAX_R '=' NUMBER {getSubTabName($1);
                                           eggConfig_set_cfg(pConfig, subTabName, $3); 
                                           };

nowait_rwsunit : NOWAIT_R '=' BOOL {getSubTabName($1);
                                       eggConfig_set_cfg(pConfig, subTabName, $3); 
                                       };




sockd :  unix_sockd | port_sockd | ip_sockd | logp_sockd | logl_sockd;

unix_sockd :     FSOCK_D '=' PATH       {eggConfig_set_cfg(pConfig, $1, $3); };
port_sockd :     FPORT_D '=' NUMBER     { eggConfig_set_cfg(pConfig, $1, $3); };
ip_sockd   :     FIP_D '=' IP           {eggConfig_set_cfg(pConfig, $1, $3); };
logp_sockd   :    LOGPATH_D '=' PATH     {eggConfig_set_cfg(pConfig, $1, $3); };
logl_sockd   :    LOGLEVEL_D '=' NUMBER     {eggConfig_set_cfg(pConfig, $1, $3); };


sock :  unix_sock | port_sock | ip_sock;

unix_sock :     FSOCK '=' PATH {eggConfig_set_cfg(pConfig, $1, $3); };
port_sock :     FPORT '=' NUMBER     {eggConfig_set_cfg(pConfig, $1, $3); };
ip_sock   :     FIP '=' IP         {eggConfig_set_cfg(pConfig, $1, $3); };

core : basepath_core | logpath_core | loglevel_core | analy;

basepath_core : BASEPATH_CORE '=' PATH     {eggConfig_set_cfg(pConfig, $1, $3);};

logpath_core : LOGPATH_CORE '=' PATH     {eggConfig_set_cfg(pConfig, $1, $3); };

loglevel_core : LOGLEVEL_CORE '=' NUMBER     {eggConfig_set_cfg(pConfig, $1, $3); };

analy : DLLNAME {eggConfig_set_cfg(pConfig, "analyzer", $1);
                 eggConfig_set_cfg(pConfig, "analyzer", "/usr/local/lib/libcwsplugin.la"); 
                 YYACCEPT;}| 
        DLLNAME PATH {eggConfig_set_cfg(pConfig, "analyzer", $1);
                      eggConfig_set_cfg(pConfig, "analyzer", $2);
                      }
        ; 


note : NOTE  {;};

start : START;
end :   END;
%%

extern FILE* cfgin;
extern int cfglineno;
extern int fileeof;
extern int fileline;
char dbName[64];

void cfgerror(void* pextra, void* s)
{
   if(fileeof != 0)
     printf("syntax error [line : %d]\n", fileline);

   return;
}

void getDBname(char *t)
{
   int len = strlen(t); 
   dbName[len-2] = '\0'; 
   memcpy(dbName, (char*)(t) + 1, len-2); 
   return ;
}


void getSubTabName(char *tabName)
{
   subTabName[0]='\0';
   strcat(subTabName, dbName);
   strcat(subTabName, ":");
   strcat(subTabName, tabName);
   return ;
}



                 


