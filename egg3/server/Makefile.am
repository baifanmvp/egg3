AUTOMAKE_OPTIONS=foreign

SUBDIRS = RWS

bin_PROGRAMS=   cluster-eggd  eggd eggMemServer


eggd_SOURCES=eggServiceServer.c eggServiceServerConfig.c eggServiceServerConfig.h
eggd_LDADD=-lcwsplugin -legg3 -ldl   -lpthread -lgthread-2.0 -lrt $(glib_LIBS)
eggd_CPPFLAGS=-I/usr/local/include/ $(glib_CFLAGS)

cluster_eggd_SOURCES=eggInfoServer.c
cluster_eggd_LDADD=-legg3 $(glib_LIBS)
cluster_eggd_CPPFLAGS=-I/usr/local/include/ $(glib_CFLAGS)


eggMemServer_SOURCES=eggMemServer.c eggServiceServerConfig.c
eggMemServer_LDADD= $(glib_LIBS) -lcwsplugin -legg3 -ldl   -lgthread-2.0 -lrt
eggMemServer_CPPFLAGS=-I/usr/local/include/ $(glib_CFLAGS)

sysconfdir=$(egg_CONFPATH)
#sysconf_DATA=../../conf/eggInfoServer.cfg
 
