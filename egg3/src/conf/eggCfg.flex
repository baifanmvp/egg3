%option yylineno noyywrap
%option prefix="cfg"
%{
#define _GNU_SOURCE
     
#include <stdio.h>
#include <stdlib.h>
#include "eggCfg.bison.h"

int parmnum = 0;
int fileeof=1;
int fileline=0;

#undef YY_INPUT


#define YY_INPUT(buf, retval, maxlen)           \
    {                                           \
    while(1)                                    \
    {                                           \
        char* ret = fgets(buf, maxlen, yyin);   \
        fileline++;                             \
        if(ret)                                 \
        {                                       \
            if (buf[0] == '\n')                 \
                continue;                       \
            retval=strlen(buf);                 \
        }                                       \
        else                                    \
        {                                       \
            retval = 0;                         \
            fileeof = 0;                        \
        }                                       \
        break;                                  \
    }                                           \
}

%}



%e 1200
IP        ([0-2]?[0-9]?[0-9]\.){3}[0-2]?[0-9]?[0-9]
FILEPATH  \/+([^\/\n\t#]+\/*)*

NOTE      #.+
NAME      [a-zA-Z0-9_]+
NUMBER    [-]?[0-9]+
TIME      {NUMBER}[mhdMHD]?  

LOCALPATH   (file:\/\/)+{FILEPATH}
SOCKETPATH  ((socket)|(tcp)):\/\/{IP}:{NUMBER}{FILEPATH} 
UNIXPATH    unixsock:\/\/{FILEPATH}:{FILEPATH}
CLUSTERPATH cluster:\/\/{IP}:{NUMBER}\/{NAME}
RWSPATH     rws:\/\/{IP}:{NUMBER}\/({NAME})?
BOOL        [Yy]|[Nn]|[Yy][Ee][Ss]|[Nn][Oo]
       

%x CORE ANA SOCK SOCKD RWS RWSUNIT CLUSTER CTUNIT


%%
<CORE>\<ANALYZER\>           {strcpy(cfglval.str, yytext); BEGIN ANA;return START;}
<CORE>{FILEPATH}            {strcpy(cfglval.str, yytext); return PATH;}
<CORE>{NUMBER}              {strcpy(cfglval.str, yytext); return NUMBER;}

<CORE>basepath               {strcpy(cfglval.str, yytext); return BASEPATH_CORE;}
<CORE>logpath                {strcpy(cfglval.str, yytext); return LOGPATH_CORE;}
<CORE>loglevel                {strcpy(cfglval.str, yytext); return LOGLEVEL_CORE;}
<CORE>"="                    {strcpy(cfglval.str, yytext); return yytext[0];}
<CORE>\<\/CORECFG\>          {strcpy(cfglval.str, yytext); BEGIN INITIAL;return END;}

<CORE>{NOTE}                 {return NOTE;}
<CORE>\n                     {return 0;};
<CORE>[ \t]                      ;
<ANA>\<\/ANALYZER\>       {strcpy(cfglval.str, yytext); BEGIN CORE;return END;}
<ANA>{NAME}               {strcpy(cfglval.str, yytext); return DLLNAME;}
<ANA>{FILEPATH}           {strcpy(cfglval.str, yytext);  return PATH;}
<ANA>{NOTE}               {return NOTE;}
<ANA>\n                   {return 0;};
<ANA>[ \t]                      ;

<SOCKD>\<\/SOCKD\>                                      {strcpy(cfglval.str, yytext); BEGIN INITIAL; return END;}
<SOCKD>socket                                           {strcpy(cfglval.str, yytext); return FSOCK_D;}
<SOCKD>ip                                               {strcpy(cfglval.str, yytext); return FIP_D;}
<SOCKD>port                                             {strcpy(cfglval.str, yytext); return FPORT_D;}
<SOCKD>logpath                                          {strcpy(cfglval.str, yytext); return LOGPATH_D;}
<SOCKD>loglevel                                         {strcpy(cfglval.str, yytext); return LOGLEVEL_D;}

<SOCKD>{FILEPATH}                                       {strcpy(cfglval.str, yytext); return PATH;}
<SOCKD>{NUMBER}                                         {strcpy(cfglval.str, yytext); return NUMBER;}
<SOCKD>{IP}                                             {strcpy(cfglval.str, yytext); return IP;}
<SOCKD>"="                                              {strcpy(cfglval.str, yytext); return yytext[0];}
<SOCKD>{NOTE}                                           {strcpy(cfglval.str, yytext); return NOTE;}
<SOCKD>\n                                               {return 0;};


<SOCK>\<\/SOCK\>                                        {strcpy(cfglval.str, yytext); BEGIN INITIAL;return END;}
<SOCK>socket                                            {strcpy(cfglval.str, yytext); return FSOCK;}
<SOCK>ip                                                {strcpy(cfglval.str, yytext); return FIP;}
<SOCK>port                                              {strcpy(cfglval.str, yytext); return FPORT;}
<SOCK>{FILEPATH}                                        {strcpy(cfglval.str, yytext); return PATH;}
<SOCK>{NUMBER}                                          {strcpy(cfglval.str, yytext); return NUMBER;}
<SOCK>{IP}                                              {strcpy(cfglval.str, yytext); return IP;}
<SOCK>"="                                               {strcpy(cfglval.str, yytext); return yytext[0];}
<SOCK>{NOTE}                                            {strcpy(cfglval.str, yytext); return NOTE;}
<SOCK>\n                                                {return 0;};


<RWS>\<\/RWS\>                                        {strcpy(cfglval.str, yytext); BEGIN INITIAL;return END;}
<RWS>\<{NAME}\>                                       {strcpy(cfglval.str, yytext); BEGIN RWSUNIT;return NAME_R;}
<RWS>{NOTE}                                           {strcpy(cfglval.str, yytext); return NOTE;}
<RWS>\n                                               {return 0;};

<RWSUNIT>\<\/{NAME}\>                                     {strcpy(cfglval.str, yytext); BEGIN RWS;return END;}
<RWSUNIT>{LOCALPATH}                                      {strcpy(cfglval.str, yytext); return EGGPATH;}
<RWSUNIT>{SOCKETPATH}                                     {strcpy(cfglval.str, yytext); return EGGPATH;}
<RWSUNIT>{UNIXPATH}                                       {strcpy(cfglval.str, yytext); return EGGPATH;}
<RWSUNIT>{CLUSTERPATH}                                    {strcpy(cfglval.str, yytext); return EGGPATH;} 
<RWSUNIT>{RWSPATH}                                        {strcpy(cfglval.str, yytext); return EGGPATH;}


<RWSUNIT>ip                                               {strcpy(cfglval.str, yytext); return FIP_R;}
<RWSUNIT>port                                             {strcpy(cfglval.str, yytext); return FPORT_R;}
<RWSUNIT>{IP}                                             {strcpy(cfglval.str, yytext); return IP;}
<RWSUNIT>{NUMBER}                                         {strcpy(cfglval.str, yytext); return NUMBER;}
<RWSUNIT>{FILEPATH}                                       {strcpy(cfglval.str, yytext); return PATH;}
<RWSUNIT>"="                                              {strcpy(cfglval.str, yytext); return yytext[0];}
<RWSUNIT>{BOOL}                                           {strcpy(cfglval.str, yytext); return BOOL;}
<RWSUNIT>{TIME}                                           {strcpy(cfglval.str, yytext); return TIME;}


<RWSUNIT>connectthreadnum                                 {strcpy(cfglval.str, yytext); return CONNECT_R;}
<RWSUNIT>logfile                                          {strcpy(cfglval.str, yytext); return LOGFILE_R;}
<RWSUNIT>workdir                                          {strcpy(cfglval.str, yytext); return WORKDIR_R;}
<RWSUNIT>memserverexename                                 {strcpy(cfglval.str, yytext); return MEMEXE_R;}
<RWSUNIT>memserverage                                     {strcpy(cfglval.str, yytext); return SYNCFREQ_R;}
<RWSUNIT>docexportexename                                 {strcpy(cfglval.str, yytext); return EXPORTEXE_R;}
<RWSUNIT>counter                                          {strcpy(cfglval.str, yytext); return COUNTER_R;}
<RWSUNIT>nummemservermax                                  {strcpy(cfglval.str, yytext); return MSERVERMAX_R;}  
<RWSUNIT>nowaitcleanupmemserver                           {strcpy(cfglval.str, yytext); return NOWAIT_R;}  

<RWSUNIT>{NOTE}                                           {strcpy(cfglval.str, yytext); return NOTE;}
<RWSUNIT>\n                                               {return 0;};



<CLUSTER>\<\/CLUSTER\>                                    {strcpy(cfglval.str, yytext); BEGIN INITIAL;return END;}
<CLUSTER>\<{NAME}\>                                       {strcpy(cfglval.str, yytext); BEGIN CTUNIT;return NAME_C;}


<CLUSTER>listen                                           {strcpy(cfglval.str, yytext); return FLISTEN_C;}
<CLUSTER>{IP}                                             {strcpy(cfglval.str, yytext); return IP;}
<CLUSTER>{NUMBER}                                         {strcpy(cfglval.str, yytext); return NUMBER;}
<CLUSTER>":"                                              {strcpy(cfglval.str, yytext); return yytext[0];}
<CLUSTER>"="                                              {strcpy(cfglval.str, yytext); return yytext[0];}
<CLUSTER>logpath                                          {strcpy(cfglval.str, yytext); return LOGPATH_C;}
<CLUSTER>loglevel                                         {strcpy(cfglval.str, yytext); return LOGLEVEL_C;}

<CLUSTER>{FILEPATH}                                       {strcpy(cfglval.str, yytext); return PATH;}



<CLUSTER>{NOTE}                                           {strcpy(cfglval.str, yytext); return NOTE;}
<CLUSTER>\n                                               {return 0;};


<CTUNIT>\<\/{NAME}\>                                      {strcpy(cfglval.str, yytext); BEGIN CLUSTER;return END;}

<CTUNIT>{NUMBER}                                          {strcpy(cfglval.str, yytext); return NUMBER;} 
<CTUNIT>"["                                               {strcpy(cfglval.str, yytext); return yytext[0];}
<CTUNIT>"]"                                               {strcpy(cfglval.str, yytext); return yytext[0];}
<CTUNIT>","                                               {strcpy(cfglval.str, yytext); return yytext[0];}


<CTUNIT>{LOCALPATH}                                      {strcpy(cfglval.str, yytext); return EGGPATH;}
<CTUNIT>{SOCKETPATH}                                     {strcpy(cfglval.str, yytext); return EGGPATH;}
<CTUNIT>{UNIXPATH}                                       {strcpy(cfglval.str, yytext); return EGGPATH;}
<CTUNIT>{CLUSTERPATH}                                    {strcpy(cfglval.str, yytext); return EGGPATH;} 
<CTUNIT>{RWSPATH}                                        {strcpy(cfglval.str, yytext); return EGGPATH;}

<CTUNIT>{NOTE}                                           {strcpy(cfglval.str, yytext); return NOTE;}
<CTUNIT>\n                                               {return 0;};


\<CORECFG\> {strcpy(cfglval.str, yytext); BEGIN CORE;return START;}
\<RWS\>    {strcpy(cfglval.str, yytext); BEGIN RWS;return START;}
\<SOCKD\>    {strcpy(cfglval.str, yytext); BEGIN SOCKD;return START;}
\<SOCK\>     {strcpy(cfglval.str, yytext); BEGIN SOCK;return START;}
\<CLUSTER\>     {strcpy(cfglval.str, yytext); BEGIN CLUSTER;return START;}
\n|[ \t]            {return 0;};
%%


