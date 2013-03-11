AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libegg3.la

libegg3_la_SOURCES = ./egglib/emem.c\
./egglib/eggAllocFree.c\
./storage/ViewStream.c\
./storage/File.c\
./storage/Cluster.c\
./storage/FileLock.c\
./storage/eggRecoveryLog.c\
./storage/eggIdNode.c\
./storage/eggIdTable.c\
./index/eggIndexView.c\
./index/eggIndexNodeView.c\
./index/eggIndexRecord.c\
./index/eggListView.c\
./index/eggIdView.c\
./index/eggDocView.c\
./index/eggIndexCache.c\
./index/eggFieldView.c\
./index/eggFieldWeight.c\
./search/eggQuery.c\
./search/eggScoreDoc.c\
./search/eggTopCollector.c\
./search/eggFieldKey.c\
./search/eggSearchIter.c\
./analysis/eggAnalyzer.c\
./similarity/eggSimilarScore.c\
./interface/eggDirectory.c\
./interface/eggHttp.c\
./interface/eggServiceClient.c\
./interface/eggServiceClientConfig.c\
./interface/eggCluster.c\
./interface/eggDocument.c\
./interface/eggIndexWriter.c\
./interface/eggIndexWriterLocal.c\
./interface/eggIndexWriterRemote.c\
./interface/eggIndexWriterServiceClient.c\
./interface/eggIndexWriterCluster.c\
./interface/eggIndexReader.c\
./interface/eggIndexReaderLocal.c\
./interface/eggIndexReaderRemote.c\
./interface/eggIndexReaderServiceClient.c\
./interface/eggIndexReaderCluster.c\
./interface/eggIndexSearcher.c\
./interface/eggIndexSearcherLocal.c\
./interface/eggIndexSearcherRemote.c\
./interface/eggIndexSearcherServiceClient.c\
./interface/eggIndexSearcherCluster.c\
./interface/eggField.c\
./interface/eggPath.c\
./net/eggNetSocket.c\
./net/eggNetServer.c\
./net/eggNetIndexList.c\
./net/eggNetHttp.c\
./net/eggNetServiceClient.c\
./net/eggNetPackage.c\
./net/eggSpanUnit.c\
./net/eggClientInfoServ.c\
./common/eggLibLoad.c\
./common/eggSySRecorder.c\
./log/eggPrtLog.c\
./uti/Utility.c\
./uti/VariantInteger.c\
./uti/eggUtiMd5.c\
./uti/eggThreadPool.c\
./uti/eggReg.c\
./conf/eggConfig.c\
./conf/eggCfg.bison.c\
./conf/eggCfg.flex.c\
./cluster/eggClusterCommon.c

libegg3_la_LIBADD = -lcwsplugin $(glib_LIBS)  -lbz2   -lpthread -ldl -lcws -lfl
libegg3_la_CPPFLAGS = $(AM_CPPFLAGS) $(glib_CFLAGS)  -D_GNU_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DEGGRECOVERYLOG -shared

libegg3_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
libegg3_la_CFLAGS = $(AM_CFLAGS)
includedir=${prefix}/include/${egg_version}/

nobase_include_HEADERS = Egg3.h\
eggDebug.h\
eggAnalyzer.h\
eggSySRecorder.h\
eggDid.h\
eggQuery.h\
eggIndexSearcher.h\
eggTopCollector.h\
eggFieldKey.h\
EggDef.h\
eggHandle.h\
eggPath.h\
eggDirectory.h\
eggHttp.h\
eggCluster.h\
eggIndexReader.h\
eggIndexWriter.h\
eggScoreDoc.h\
eggSearchIter.h\
eggServiceClient.h\
eggIndexCache.h\
index/eggIndexView.h\
index/eggDocView.h\
index/eggIndexNodeView.h\
index/eggIndexRecord.h\
index/eggListView.h\
index/eggIdView.h\
index/eggFieldView.h\
index/eggFieldWeight.h\
interface/eggIndexReaderCluster.h\
interface/eggIndexReaderLocal.h\
interface/eggIndexReaderRemote.h\
interface/eggIndexReaderServiceClient.h\
interface/eggIndexSearcherCluster.h\
interface/eggIndexSearcherLocal.h\
interface/eggIndexSearcherRemote.h\
interface/eggIndexSearcherServiceClient.h\
interface/eggIndexWriterCluster.h\
interface/eggIndexWriterLocal.h\
interface/eggIndexWriterRemote.h\
interface/eggIndexWriterServiceClient.h\
interface/eggServiceClientConfig.h\
storage/File.h\
storage/FileLock.h\
storage/Cluster.h\
storage/ViewStream.h\
storage/eggDocNode.h\
storage/eggIdNode.h\
storage/eggFieldNode.h\
storage/eggIdTable.h\
storage/eggRecoveryLog.h\
similarity/eggSimilarScore.h\
egglib/emacros.h\
egglib/etype.h\
egglib/emem.h\
egglib/eggAllocFree.h\
EggError.h\
uti/eggThreadPool.h\
uti/Utility.h\
uti/eggUtiMd5.h\
eggDocument.h\
eggField.h\
net/eggNetHttp.h\
net/eggNetPackage.h\
net/eggNetSocket.h\
net/eggNetServer.h\
net/eggNetIndexList.h\
net/eggNetType.h\
net/eggSpanUnit.h\
net/eggClientInfoServ.h\
conf/eggConfig.h\
conf/eggCfg.bison.h\
net/eggNetServiceClient.h\
uti/VariantInteger.h\
uti/eggReg.h\
log/eggPrtLog.h\
cluster/eggClusterCommon.h\
common/eggLibLoad.h


sysconfdir=$(egg_CONFPATH)
dist_sysconf_DATA=../conf/egg.cfg

install-data-local:
	mkdir -p $(DESTDIR)$(egg_SYSDATA)
	chmod 777 $(DESTDIR)$(egg_SYSDATA)


EXTRA_DIST=egg3.pc.in conf/eggCfg.flex conf/eggCfg.bison 
pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = egg3.pc

egg3.pc: egg3.pc.in
	sed -e "s:@PREFIX@:${prefix}:"\
		  -e "s:@EXEC_PREFIX@:${exec_prefix}:"\
	      -e "s:@LIBDIR@:${libdir}:"\
		  -e "s:@INCLUDEDIR@:${includedir}:"\
	      $< >$@
