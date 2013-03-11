#ifndef EGG_NETSERVER_H_
#define EGG_NETSERVER_H_
#include "../eggIndexReader.h"
#include "../eggIndexWriter.h"
#include "../eggIndexSearcher.h"
#include "../eggDirectory.h"
#include "../eggAnalyzer.h"
#include "./eggNetHttp.h"
#include "./eggNetPackage.h"
#include "./eggNetIndexList.h"
#include "../EggDef.h"

#define  EGG_NET_IVDHANDLE   0xffff
#define  EGG_NET_IVDOP   0xfffe
#define  EGG_NET_IVDIR   0xfffd

typedef void* HEGGNETSTREAM;
typedef struct eggNetServer EGGNETSERVER;
typedef struct eggNetServer* HEGGNETSERVER;

typedef EBOOL (*eggFnServerSend)(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);

typedef EBOOL (*eggFnServerRecv)(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);


struct eggNetServer
{
    HEGGNETSTREAM hInStream;
    HEGGNETSTREAM hOutStream;

    eggFnServerSend fnSend;
    eggFnServerRecv fnRecv;

    HEGGINDEXSEARCHER hSearcher;
    HEGGINDEXREADER hReader;
    HEGGINDEXWRITER hWriter;

    HEGGNETINDEXLIST head;
};

HEGGNETSERVER eggNetServer_new (HEGGNETSTREAM hInStream, HEGGNETSTREAM hOutStream, eggFnServerSend fnSend, eggFnServerRecv fnRecv);

EBOOL eggNetServer_delete (HEGGNETSERVER hNetServer);

EBOOL eggNetServer_recv(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);
    
EBOOL eggNetServer_send(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);

EBOOL eggNetServer_recv_cgi(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);

EBOOL eggNetServer_send_cgi(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);

HEGGNETPACKAGE eggNetServer_processing(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

EBOOL eggNetServer_destory_egg(HEGGNETSERVER hNetServer);

EBOOL eggNetServer_set_indexhandle(HEGGNETSERVER hNetServer, char *pEggName, char *pAnalyzerName);


#endif
