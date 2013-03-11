#ifndef _DOC_EGG2_RWS_TUTORIAL_H_
#define _DOC_EGG2_RWS_TUTORIAL_H_
/**
   \page egg2_rws_tutorial egg2 读写分离配置 教程
   \section section_introduce 概述

  RWS是一个读写分离服务，它管理多个数据相同的eggDB（eggDB的数据访问方式可以是cluster，local和socket三种类型），负责分流read-respon到空闲eggDB和定期同步write-respon数据到每个eggDB。RWS需要egg基库（版本1.1以上）支持，目前不支持删除和修改，查询只支持docid全返回模式。

   \section section_config 配置流程

    \subsection section_editconfig 填写配置文件
    配置模板文件（eggRWSIntServer.cfg）经过make install会安装到/etc/egg2/目录下

    下面两图就是配置模板文件
    \image html egg2-rwsconfig.png \n
    \li port和ip指的是RWS服务（eggRWSIntServer）的启动端口和地址

    \li 接下来的7行路径是7个eggDB的路径，cluster，local和socket三种类型的路径可以混搭，路径写法请参看以前的教程
    \li connectthreadnum 是eggRWSIntServer启动是处理连接请求的线程数量，默认填0

    \li logfile是log文件生成的路径名字

    \li workdir是最新写入数据的临时存放路径（定期dump数据到各eggdb）

    \li memserverexename填eggMemServer的路径位置（eggMemServer管理还没同步到eggdb的最新数据的索引）

    \li memserverage代表是数据同步周期，1代表1分钟 1h代表一小时 1d代表一天

    \li docexportexename只的是同步是export程序的路径

    \li counter监控各个eggDB的状态，在stdout里打出，yes是监控，no是不监控

    
    \subsection section_starteggdb 启动eggDB

    启动eggDB各自的server，server的方式和eggRWSIntServer.cfg里的eggDB路径，ip，port要保持一致。我上面的配置文件写的是tcp的socket方式，所以启动的服务程序是eggServiceServer

    \image html egg2-socketserver.png \n


    \subsection section_starteggrwsintserver 启动eggRWSIntServer

   待所有eggDB启动完成以后，启动eggRWSIntServer
    \image html egg2-rwsserver.png \n

    \subsection section_connecttoeggrwsintserver eggclient去连接eggRWSIntServer

   采用egg的client去连接eggRWSIntServer进行数据操作，因为eggRWSIntServer本身走tcp方式通信，所以路径是socket格式

    \image html egg2-rwsclient.png \n

     

   
*/



#endif //_DOC_EGG2_RWS_TUTORIAL_H_

