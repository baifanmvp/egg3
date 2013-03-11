AUTOMAKE_OPTIONS=foreign

bin_PROGRAMS= rws-eggd eggRWSExportDoc eggRWSPackageSaver
dist_bin_SCRIPTS = rws-eggd-ctl rws-eggd-multi rws-eggd-dump-egg rws-eggd-store-egg rws-conf-deploy.pl

rws_eggd_SOURCES=	eggRWSIntServer.c		\
				eggRWSBakerCfg.c		\
				eggRWSBakerManager.c		\
				eggRWSCommon.c			\
				eggRWSMemDptor.c		\
				eggRWSMemManager.c		\
				eggRWSLog.c			\
				eggRWSMergePackage.c		\
				eggRWSPackBuf.c			\
				eggRWSBakerCfg.h		\
				eggRWSIntServer.h		\
				eggRWSMemManager.h		\
				eggRWSPackBuf.h			\
				eggRWSBakerManager.h		\
				eggRWSLog.h			\
				eggRWSMemServer.h		\
				eggRWSCommon.h			\
				eggRWSMemDptor.h		\
				eggRWSMergePackage.h


rws_eggd_LDADD=-legg3 -lcwsplugin -ldl   -lpthread -lgthread-2.0 -lrt $(glib_LIBS) 
rws_eggd_CPPFLAGS=-I/usr/local/include/ $(glib_CFLAGS)

eggRWSExportDoc_SOURCES=eggRWSPackBuf.c eggRWSExportDoc.c
eggRWSExportDoc_LDADD=-legg3 -lcwsplugin -ldl -lgthread-2.0 -lrt $(glib_LIBS)
eggRWSExportDoc_CPPFLAGS=-I/usr/local/include/ $(glib_CFLAGS)

eggRWSPackageSaver_SOURCES=eggRWSPackBuf.c eggRWSPackageSaver.c
eggRWSPackageSaver_LDADD=-legg3 -lcwsplugin -ldl -lgthread-2.0 -lrt  $(glib_LIBS)
eggRWSPackageSaver_CPPFLAGS=-I/usr/local/include/ $(glib_CFLAGS)

sysconfdir=$(egg_CONFPATH)
#sysconf_DATA=../../conf/eggInfoServer.cfg
