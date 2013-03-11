AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libegg2kset.la

libegg2kset_la_SOURCES	 = ./eggKeySet.c

libegg2kset_la_LIBADD = -legg2
libegg2kset_la_CPPFLAGS = $(AM_CPPFLAGS) $(glib_CFLAGS)  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DEGGRECOVERYLOG

libegg2kset_la_LDFLAGS = $(AM_LDFLAGS) 
libegg2kset_la_CFLAGS = $(AM_CFLAGS) 
includedir=${prefix}/include/egg2/kset/
nobase_include_HEADERS=eggKeySet.h

