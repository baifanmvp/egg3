
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DLLNAME = 258,
     NAME_R = 259,
     NAME_C = 260,
     FSOCK_D = 261,
     FIP_D = 262,
     FPORT_D = 263,
     LOGPATH_D = 264,
     LOGLEVEL_D = 265,
     FSOCK = 266,
     FIP = 267,
     FPORT = 268,
     FIP_R = 269,
     FPORT_R = 270,
     CONNECT_R = 271,
     LOGFILE_R = 272,
     WORKDIR_R = 273,
     MEMEXE_R = 274,
     SYNCFREQ_R = 275,
     EXPORTEXE_R = 276,
     COUNTER_R = 277,
     MSERVERMAX_R = 278,
     NOWAIT_R = 279,
     FLISTEN_C = 280,
     LOGPATH_C = 281,
     LOGLEVEL_C = 282,
     BASEPATH_CORE = 283,
     LOGPATH_CORE = 284,
     LOGLEVEL_CORE = 285,
     BOOL = 286,
     NUMBER = 287,
     PATH = 288,
     IP = 289,
     EGGPATH = 290,
     NOTE = 291,
     TIME = 292,
     START = 293,
     END = 294,
     PARMERROR = 295
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 19 "eggCfg.bison"

char str[256];



/* Line 1676 of yacc.c  */
#line 98 "eggCfg.bison.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE cfglval;


