#!/bin/bash

version=0
release=1
spec_dir=`pwd`
rpm_build_dir=${spec_dir}/rpmbuild

set -e

mkdir -p ${rpm_build_dir}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

cd ..

./configure --with-datapath=/var/lib/egg3/sysdata/ --with-conf=/etc/egg3/ --prefix=/usr --exec-prefix=/usr --bindir=/usr/bin --sbindir=/usr/sbin --sysconfdir=/etc --datadir=/usr/share --includedir=/usr/include --libdir=/usr/lib64 --libexecdir=/usr/libexec --mandir=/usr/share/man --infodir=/usr/share/info  && make dist-gzip && cp egg3-${version}-${release}.tar.gz ${rpm_build_dir}/SOURCES/


cd ${spec_dir}


rpmbuild --define "_topdir ${rpm_build_dir}" -ba egg.spec

cp ${rpm_build_dir}/RPMS/x86_64/egg3-debuginfo-${version}-${release}.x86_64.rpm .
cp ${rpm_build_dir}/RPMS/x86_64/egg3-${version}-${release}.x86_64.rpm .
cp ${rpm_build_dir}/SRPMS/egg3-${version}-${release}.src.rpm .

rm -r rpmbuild

