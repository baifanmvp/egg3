%define eggconfigpath /etc/egg3/
%define eggdatapath /var/lib/egg3/sysdata/

Summary: A library that index and search document.
Name: egg3
Version: 0
Release: 1
Group: Development/Libraries
Source: egg3-%{version}-%{release}.tar.gz
#Patch:
AutoReqProv: no
Requires: cws, cwsplugin, glib2-devel >= 2.16.0, bzip2-devel
Provides: egg
License: GPL
BuildRoot: /var/tmp/%{name}-buildroot

%description
egg is a library of Documents-oriented indexer and searcher.



%prep
%setup -n %{name}-%{version}-%{release}

%build
CFLAGS="%optflags -g3 -O0" CXXFLAGS="%optflags -g3 -O0" ./configure \
    --with-datapath=%{eggdatapath}				\
	    --with-conf=%{eggconfigpath}				\
	    --prefix=%{_prefix}						\
	    --exec-prefix=%{_exec_prefix}			 	\
	    --bindir=%{_bindir}						\
	    --sbindir=%{_sbindir}					\
	    --sysconfdir=%{_sysconfdir}					\
	    --datadir=%{_datadir}					\
            --includedir=%{_includedir}					\
            --libdir=%{_libdir}						\
	    --libexecdir=%{_libexecdir}					\
	    --mandir=%{_mandir}						\
	    --infodir=%{_infodir}
cd src
make

%install
cd src
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

cd ..

install -d $RPM_BUILD_ROOT%{eggconfigpath}
sed -e s:@LIBDIR@:%{_libdir}: conf/egg.cfg.in >conf/egg.cfg.out
install -m644 conf/egg.cfg.out $RPM_BUILD_ROOT%{eggconfigpath}/egg.cfg

install -m777 -d $RPM_BUILD_ROOT%{eggdatapath}


cd bin
make DESTDIR=$RPM_BUILD_ROOT install
cd ..
cd server
make DESTDIR=$RPM_BUILD_ROOT install

cd ..
install -m644 conf/eggd.cfg $RPM_BUILD_ROOT%{eggconfigpath}/eggd.cfg
install -m644 conf/cluster-eggd.cfg $RPM_BUILD_ROOT%{eggconfigpath}/cluster-eggd.cfg
install -m644 conf/rws-eggd.cfg $RPM_BUILD_ROOT%{eggconfigpath}/rws-eggd.cfg


install -d $RPM_BUILD_ROOT/etc/init.d
install -d $RPM_BUILD_ROOT/etc/rc.d/init.d
install -m755 redhat/eggd.init $RPM_BUILD_ROOT/etc/init.d/eggd
install -m755 redhat/eggd.init $RPM_BUILD_ROOT/etc/rc.d/init.d/eggd


%clean
#rm -rf $RPM_BUILD_ROOT

%post

%preun

%postun


%files
%defattr(-,root,root)
%doc README COPYING ChangeLog

%{_libdir}/libegg3.a
%{_libdir}/libegg3.la
%{_libdir}/libegg3.so
%{_libdir}/libegg3.so.0
%{_libdir}/libegg3.so.0.0.0
%{_libdir}/pkgconfig/egg3.pc
%{_includedir}/egg3/Egg3.h
%{_includedir}/egg3/EggDef.h
%{_includedir}/egg3/EggError.h
%{_includedir}/egg3/cluster/eggClusterCommon.h
%{_includedir}/egg3/common/eggLibLoad.h
%{_includedir}/egg3/conf/eggCfg.bison.h
%{_includedir}/egg3/conf/eggConfig.h
%{_includedir}/egg3/eggAnalyzer.h
%{_includedir}/egg3/eggCluster.h
%{_includedir}/egg3/eggDebug.h
%{_includedir}/egg3/eggDid.h
%{_includedir}/egg3/eggDirectory.h
%{_includedir}/egg3/eggDocument.h
%{_includedir}/egg3/eggField.h
%{_includedir}/egg3/eggFieldKey.h
%{_includedir}/egg3/eggHandle.h
%{_includedir}/egg3/eggHttp.h
%{_includedir}/egg3/eggIndexCache.h
%{_includedir}/egg3/eggIndexReader.h
%{_includedir}/egg3/eggIndexSearcher.h
%{_includedir}/egg3/eggIndexWriter.h
%{_includedir}/egg3/eggPath.h
%{_includedir}/egg3/eggQuery.h
%{_includedir}/egg3/eggScoreDoc.h
%{_includedir}/egg3/eggSearchIter.h
%{_includedir}/egg3/eggServiceClient.h
%{_includedir}/egg3/eggSySRecorder.h
%{_includedir}/egg3/eggTopCollector.h
%{_includedir}/egg3/egglib/eggAllocFree.h
%{_includedir}/egg3/egglib/emacros.h
%{_includedir}/egg3/egglib/emem.h
%{_includedir}/egg3/egglib/etype.h
%{_includedir}/egg3/index/eggDocView.h
%{_includedir}/egg3/index/eggFieldView.h
%{_includedir}/egg3/index/eggFieldWeight.h
%{_includedir}/egg3/index/eggIdView.h
%{_includedir}/egg3/index/eggIndexNodeView.h
%{_includedir}/egg3/index/eggIndexRecord.h
%{_includedir}/egg3/index/eggIndexView.h
%{_includedir}/egg3/index/eggListView.h
%{_includedir}/egg3/interface/eggIndexReaderCluster.h
%{_includedir}/egg3/interface/eggIndexReaderLocal.h
%{_includedir}/egg3/interface/eggIndexReaderRemote.h
%{_includedir}/egg3/interface/eggIndexReaderServiceClient.h
%{_includedir}/egg3/interface/eggIndexSearcherCluster.h
%{_includedir}/egg3/interface/eggIndexSearcherLocal.h
%{_includedir}/egg3/interface/eggIndexSearcherRemote.h
%{_includedir}/egg3/interface/eggIndexSearcherServiceClient.h
%{_includedir}/egg3/interface/eggIndexWriterCluster.h
%{_includedir}/egg3/interface/eggIndexWriterLocal.h
%{_includedir}/egg3/interface/eggIndexWriterRemote.h
%{_includedir}/egg3/interface/eggIndexWriterServiceClient.h
%{_includedir}/egg3/interface/eggServiceClientConfig.h
%{_includedir}/egg3/log/eggPrtLog.h
%{_includedir}/egg3/net/eggClientInfoServ.h
%{_includedir}/egg3/net/eggNetHttp.h
%{_includedir}/egg3/net/eggNetIndexList.h
%{_includedir}/egg3/net/eggNetPackage.h
%{_includedir}/egg3/net/eggNetServer.h
%{_includedir}/egg3/net/eggNetServiceClient.h
%{_includedir}/egg3/net/eggNetSocket.h
%{_includedir}/egg3/net/eggNetType.h
%{_includedir}/egg3/net/eggSpanUnit.h
%{_includedir}/egg3/similarity/eggSimilarScore.h
%{_includedir}/egg3/storage/Cluster.h
%{_includedir}/egg3/storage/File.h
%{_includedir}/egg3/storage/FileLock.h
%{_includedir}/egg3/storage/ViewStream.h
%{_includedir}/egg3/storage/eggDocNode.h
%{_includedir}/egg3/storage/eggFieldNode.h
%{_includedir}/egg3/storage/eggIdNode.h
%{_includedir}/egg3/storage/eggIdTable.h
%{_includedir}/egg3/storage/eggRecoveryLog.h
%{_includedir}/egg3/uti/eggReg.h
%{_includedir}/egg3/uti/eggThreadPool.h
%{_includedir}/egg3/uti/eggUtiMd5.h
%{_includedir}/egg3/uti/Utility.h
%{_includedir}/egg3/uti/VariantInteger.h


%attr(0644,root,root) %config(noreplace) %{eggconfigpath}/egg.cfg

%attr(0777,root,root) %dir %{eggdatapath}


%attr(0644,root,root) %config(noreplace) %{eggconfigpath}/eggd.cfg
%attr(0644,root,root) %config(noreplace) %{eggconfigpath}/cluster-eggd.cfg
%attr(0644,root,root) %config(noreplace) %{eggconfigpath}/rws-eggd.cfg



%attr(0755,root,root) %config /etc/rc.d/init.d/eggd
%attr(0755,root,root) %config /etc/init.d/eggd



%{_bindir}/cluster-eggd
%{_bindir}/eggBtreeCheck
%{_bindir}/eggBtreeFix
%{_bindir}/eggd
%{_bindir}/eggDocExport
%{_bindir}/eggExportKey
%{_bindir}/eggIdsChange
%{_bindir}/eggIdsMove
%{_bindir}/eggIntegrateIds
%{_bindir}/eggMemServer
%{_bindir}/eggRecordOff
%{_bindir}/eggRecoveryLogInspect
%{_bindir}/eggRWSExportDoc
%{_bindir}/eggRWSPackageSaver
%{_bindir}/eggSplit
%{_bindir}/eggSplitCalc
%{_bindir}/rws-eggd
%{_bindir}/rws-eggd-ctl
%{_bindir}/rws-eggd-multi
%{_bindir}/eggRecoveryLogClean
%{_bindir}/rws-eggd-dump-egg
%{_bindir}/rws-eggd-store-egg



%config

%changelog
