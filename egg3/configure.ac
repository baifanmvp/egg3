#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT(egg3, 0-1, [fan.bai@ape-tech.com])
AC_CONFIG_SRCDIR([src/index/eggIndexView.c])
AM_INIT_AUTOMAKE()

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
# Checks for libraries.
# Checks for header files.
AC_CHECK_HEADERS(sys/epoll.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

AC_ARG_WITH(conf,
            [AS_HELP_STRING([--with-conf], [enable confpath set])],
            [
               if test -d $withval; then
               eggconfigpath=$withval   
               AC_DEFINE_UNQUOTED(EGG_CONFIG_PATH, ["${eggconfigpath}/egg.cfg"], [""])
               AC_SUBST(egg_CONFPATH, [${eggconfigpath}], [""])

               else
                  eggconfigpath=""
                  echo  "[ERROR] egg.cfg dir is not exist! "
                  exit -1
               fi
            ],
            [
               if test  $prefix = "NONE"; then
                  eggconfigpath="/etc/egg3/"
               else
                  eggconfigpath="$prefix/etc/egg3/"              
               fi
               
#               AC_DEFINE(EGG_CONFIG_PATH, ["${eggconfigpath}"], [""])
                #上面那种不能转义变量名 
                AC_DEFINE_UNQUOTED(EGG_CONFIG_PATH, ["${eggconfigpath}/egg.cfg"], [""])
                AC_SUBST(egg_CONFPATH, [${eggconfigpath}], [""])

            ])




AC_ARG_WITH(datapath,
            [AS_HELP_STRING([--with-datapath], [enable datapath set])],
            [
               if test -d $withval; then
               eggdatapath=$withval   
               AC_DEFINE_UNQUOTED(EGG_SYSRECORD_PATH, ["${eggdatapath}"], [""])
               AC_SUBST(egg_SYSDATA, [${eggdatapath}], [""])

               else
                  eggconfigpath=""
                  echo  "[ERROR] egg sysrecord dir is not exist! "
                  exit -1
               fi
            ],
            [
               
                   eggsysdata="/var/lib/egg3/sysdata/"              
               
#               AC_DEFINE(EGG_CONFIG_PATH, ["${eggconfigpath}"], [""])
                #上面那种不能转义变量名 
                AC_DEFINE_UNQUOTED(EGG_SYSRECORD_PATH, ["${eggsysdata}"], [""])
                AC_SUBST(egg_SYSDATA, [${eggsysdata}], [""])

            ])


AC_ARG_WITH(debug, 
                        [AS_HELP_STRING([--with-debug], [enable debugging output])], 
                        [
                            if test $withval = "yes"; then

                            AC_SUBST(AM_CFLAGS, ["-g3 -O0 -Wall"])
                            AC_SUBST(AM_CXXFLAGS, ["-g3 -O0 -Wall"])
                            AC_DEFINE([_VERSION_DEBUG],[1], [""])

                            PKG_CHECK_MODULES(  [eggUtest],
                                                                  [cppunit],
                                                                  [],
                                                                  [AC_MSG_ERROR(["Error !(egg2Utest) libcppunit not find !"])])    

                             PKG_CHECK_MODULES(  [eggUtest],
                                                                  [egg3],
                                                                  [],
                                                                  [AC_MSG_ERROR(["Error !(egg2Utest) libegg3 not find !"])])                                      
                            else                                                                                                                          
                            AC_SUBST(AM_CFLAGS, [" -O2 "])
                            AC_SUBST(AM_CXXFLAGS, ["-O2"])
                            AC_DEFINE([_VERSION_RELEARE],[1], [""])

                            fi

                        ] )

PKG_CHECK_MODULES([glib], [glib-2.0], [],
    AC_MSG_ERROR(["Error! You need to have glib2.0 installed.(glib2.0-dev)"])
    ])
    
#AC_CHECK_LIB([iconv], [iconv_open], [], [
#    AC_MSG_ERROR(["Error! You need to have libiconv installed."]) ])
AC_CHECK_LIB([cwsplugin], [ImToken_new], [:], [
    AC_MSG_ERROR(["Error! You need to have libcwsplugin installed."]) ], [$glib_LIBS -lglib-2.0 -lgthread-2.0])
AC_CHECK_LIB([bz2], [BZ2_bzBuffToBuffCompress], [], [
    AC_MSG_ERROR(["Error! You need to have libbz2 installed."]) ])
    
#AC_CHECK_LIB([g2c], [s_wsfe], [], [
#    AC_MSG_ERROR(["Error! You need to have libg2c installed."]) ])


AC_DEFINE([_SYS_BIT], [65], [""])

echo "---------"$eggconfigpath"---------"
egg_version="egg3"



AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(egg_version)

_SYS_BIT=`getconf LONG_BIT`
# Checks for library functions.
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 bin/Makefile
                 server/Makefile
                 server/RWS/Makefile
                 utest/Makefile
                 ])
AC_OUTPUT
