#ifndef _DOC_EGG2_RELEASE_INSTALL_H_
#define _DOC_EGG2_RELEASE_INSTALL_H_


/**
   \page egg2_release_install Egg2安装
   
   \section depend 依赖环境
   \subsection req_linux linux 
   Linux内核版本 >= 2.6
   \subsection req_iconv iconv
   
   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>libiconv</td>
   </tr>
   <tr>
   <td>版本</td><td> >=1.13.1</td>
   </tr>
   <tr>
   <td>安装方式</td><td>源码编译</td>
   </tr>
   <tr>
   <td>下载</td><td><a href="http://www.gnu.org/s/libiconv/#downloading"> http://www.gnu.org/s/libiconv/#downloading </a></td>
   </tr>
   </table>
   
   \code
   tar zxvf libiconv-1.13.1.tar.gz
   cd libiconv-1.13.1
   ./configure
   make
   sudo make install
   \endcode
   
   \subsection req_cppunit cppunit
   
   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>cppunit</td>
   </tr>
   <tr>
   <td>版本</td><td> >=1.11</td>
   </tr>
   <tr>
   <td>安装方式</td><td>deb/rpm</td>
   </tr>
   <tr>
   <td>下载</td><td></td>
   </tr>
   </table>
   ubuntu:
   \code
   sudo apt-get install libcppunit-1.12-1 libcppunit-dev
   \endcode
   redhat:
   \code
   rpm -ivh cppunit-1.11.0-2.el4.i386.rpm
   rpm -ivh cppunit-devel-1.11.0-2.el4.i386.rpm
   \endcode

   \subsection req_glib glib 软件包

   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>glib</td>
   </tr>
   <tr>
   <td>版本</td><td> >=2.24</td>
   </tr>
   <tr>
   <td>安装方式</td><td>源码编译</td>
   </tr>
   <tr>
   <td>下载</td><td><a href="http://developer.gnome.org/glib/">http://developer.gnome.org/glib/</a></td>
   </tr>
   </table>

   \code
   tar zxvf glib-2.24.0.tar.gz
   cd glib-2.24.0
   ./configure
   make
   sudo make install
   \endcode


   \subsection req_openssl openssl 软件包
   \note  该软件包大多系统会自带，可用 ［ls /usr/lib /usr/local/lib |grep libssl］ 检查该软件是否已经安装\n\n

   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>openssl</td>
   </tr>
   <tr>
   <td>版本</td><td> >=0.9.2</td>
   </tr>
   <tr>
   <td>安装方式</td><td>源码编译</td>
   </tr>
   <tr>
   <td>下载</td><td><a href="http://www.openssl.org/">http://www.openssl.org/</a></td>
   </tr>
   </table>

   \code
   tar zxvf openssl-1.0.0d.tar.gz
   cd openssl-1.0.0d
   ./config
   make
   sudo make install
   \endcode


   \subsection req_bzip2 bzip2 软件包

   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>glib</td>
   </tr>
   <tr>
   <td>版本</td><td> >=1.0</td>
   </tr>
   <tr>
   <td>安装方式</td><td>源码编译</td>
   </tr>
   <tr>
   <td>下载</td><td><a href="http://www.digistar.com/bzip2/">http://www.digistar.com/bzip2/</a></td>
   </tr>
   </table>

   \code
   tar zxvf bzip2-1.0.1.tar.gz
   cd bzip2-1.0.1
   ./configure
   make
   sudo make install
   \endcode

   \subsection req_cws cws 软件包

   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>bzip2</td>
   </tr>
   <tr>
   <td>版本</td><td> ></td>
   </tr>
   <tr>
   <td>安装方式</td><td>源码编译</td>
   </tr>
   <tr>
   <td>下载</td><td>cvs download</td>
   </tr>
   </table>

   \code
   export CVSROOT=:pserver:username@222.73.218.46:2401/ImRoBot5
   cvs login
   cvs co cws

   cd cws
   ./configure
   make
   sudo make install
   \endcode

   \subsection req_egg EGG 软件包

    \li Download     export CVSROOT=:pserver:username@222.73.218.46:2401/ImRoBot5
                 cvs login
                 cvs co egg2

    \li Configure
		     cd $EGG_HOME
                     ./configure
   
    \li Compile      cd src
                     make
                     
    \li Install      make install
    
    \subsection
    EGG配置文件
    \arg 配置路径: \n
    在EGG2项目的conf文件夹下有一份egg的默认配置文件（egg.conf），在configure执行时可设置--with-conf=path来指定egg.conf的路径，如果不指定，egg.conf安装在$prefix/etc,如果$prefix本身也未指定该配置文件会被默认安装在/etc/egg2/下\n
    
    \arg analyzer配置格式(目前不支持注释功能)
    ANALYZER ANALYZER_name ANALYZER_lib_path\n\n
    例如:\n
    ANALYZER    ImCnLexAnalyzer   /usr/local/lib/libcwsplugin.so\n
    ANALYZER    ImCyLexAnalyzer   /usr/local/lib/libcwsplugin.so\n
    ANALYZER    ImCwsLexAnalyzer  /usr/local/lib/libcwsplugin.so\n
    ANALYZER    ImC2LexAnalyzer   /usr/local/lib/libcwsplugin.so\n
    
    
    \li 若启动egg的远程服务(fcgi) 请安装以下软件
    
    
    \subsection req_apache_httpd apache httpd软件包

   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>apache httpd</td>
   </tr>
   <tr>
   <td>版本</td><td> >=2.2</td>
   </tr>
   <tr>
   <td>安装方式</td><td>deb/rpm</td>
   </tr>
   <tr>
   <td>下载</td><td></td>
   </tr>
   </table>

   </table>
   ubuntu:
   \code
   sudo apt-get install apache2
   \endcode
   redhat:
   \code
   rpm -ivh httpd-2.2.3-11.el5_1.3.i386.rpm
   \endcode

   \subsection req_fcgi fcgi 软件包

   <table width="280" cellspacing="0" cellpadding="3" border="1">
   <tr>
   <td width="80">软件包</td><td>fcgi</td>
   </tr>
   <tr>
   <td>版本</td><td>>2.4.0</td>
   </tr>
   <tr>
   <td>安装方式</td><td>源码编译</td>
   </tr>
   <tr>
   <td>下载</td><td><a href="http://www.fastcgi.com/drupal/node/5">http://www.fastcgi.com/drupal/node/5</a></td>
   </tr>
   </table>

   \code
   tar zxvf fcgi-2.4.0.tar.gz
   cd fcgi-2.4.0
   ./configure
   make
   sudo make install
   \endcode

   \note 安装好依赖软件，按照apache的官方文档配置好fcgi后，cd  ${EGG_HOME}/bin/server/目录下，运行./build.sh后， 完成egg fastcgi的编译安装工作，egg.fcgi将被安装到apache fastcgi的默认路径(/var/www/cgi-bin/)下，若要更改fcgi的路径，则运行build.sh加入路径参数，./build.sh $FCGIPATH即可
*/




#endif //_DOC_EGG2_RELEASE_H_
