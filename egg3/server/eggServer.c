
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "egg2/Egg2.h"

#include "fcgi_config.h"
#include "fcgiapp.h"
#include "fcgi_stdio.h"
#include <stdio.h>
#include <assert.h>

extern char** environ;
static FCGX_Stream *in, *out, *err;
static FCGX_ParamArray envp;

typedef struct eggServerConfig EGGSERVERCONFIG;
typedef struct eggServerConfig* HEGGSERVERCONFIG;

struct eggServerConfig
{
    char* path;
    char* analyzer;
};



EBOOL recv_fcgi(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    char* lp_str = (char*)ePointer;
    size32_t org = size;
/*
    while(size)
    {
*/
      //        int n_recv = FCGX_GetStr(lp_str, size, in);        
      int n_recv = fread(lp_str, 1, size, stdin);        
      if(n_recv != size)
        {
          if (feof(stdin))
	    {
                fprintf(stderr, "fcgi stdin EOF : received %d need %d\n",
                        (int) n_recv, (int)(org-n_recv));
	    }
	  else
	    {
                fprintf(stderr, "fcgi stdin ERR : received %d need %d\n",
                        (int) n_recv, (int)(org-n_recv));
	    }
            return EGG_FALSE;
        }
/*
        else
        {
            size -= n_recv;
            lp_str += n_recv;
        }
    }
*/

    return EGG_TRUE;
}



EBOOL send_fcgi(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    char* lp_str = (char*)ePointer;

/*
    while(size)
    {
*/       
      //int n_send = FCGX_PutStr(lp_str, size, out);
      int n_send = fwrite(lp_str, 1, size, stdout);
        if(n_send != size)
        {
	    fprintf(stderr, "fcgi stdout ERR: write %d need %d\n",
			(int)n_send, (int)(size-n_send));
            return EGG_FALSE;
        }
/*
        else
        {
            size -= n_send;
            lp_str += n_send;
        }
    }
*/

    fflush(stdout);

    return EGG_TRUE;
}
extern HEGGNETINDEXLIST g_eggNet_list ;

int main(int argc ,char* argv[])
{
    int rc = 0;
    HEGGNETSERVER lp_server = eggNetServer_new (EGG_NULL, EGG_NULL, eggNetServer_send_cgi, eggNetServer_recv_cgi);
    g_eggNet_list = eggNetIndexList_new();
    
    count_t count = 0;
    char directory[128];
    char analyzer[128];
    
    while ((rc = FCGI_Accept()) >= 0)
    {
      char* p_content_length = getenv("CONTENT_LENGTH");//FCGX_GetParam("CONTENT_LENGTH", envp);
        int n_content_len;
        HEGGNETPACKAGE lp_recv_package = EGG_NULL;
        HEGGNETPACKAGE lp_send_package = EGG_NULL;
        
        if (p_content_length)
        {
            char *p;
            char* lp_analyzer_name = EGG_NULL;
            char *lp_directory_path = EGG_NULL;
            
            char *queryString = getenv("QUERY_STRING");//FCGX_GetParam("QUERY_STRING", envp);
            if(!queryString)
            {
                continue;
            }
            
            if (!(lp_directory_path = strstr(queryString, "EGG_DIR_PATH=")))
            {
                printf("queryString %s\n", queryString);
                continue;
            }
            lp_directory_path += strlen("EGG_DIR_PATH=");
        
            n_content_len = atoi(p_content_length);
            lp_recv_package = (HEGGNETPACKAGE)malloc(n_content_len);
            //fread(lp_recv_package, n_content_len, 1, stdin);
            if (recv_fcgi(lp_server, lp_recv_package, n_content_len, 0) == EGG_FALSE)
            {
                fprintf(stderr, "recv_fcgi error CONTENT_LENGTH=%s\n", p_content_length);
                continue;
                
            }
//                eggNetServer_recv(lp_server, lp_recv_package, n_content_len, 0);
            //printf("1queryString %s\n", queryString);
            
            if(eggNetServer_set_indexhandle(lp_server, lp_directory_path, NULL) == EGG_FALSE)
            {
                EBOOL ret = EGG_NET_IVDIR;
                lp_send_package = eggNetPackage_new(0);
                lp_send_package = eggNetPackage_add(lp_send_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
            }
            else
            {
                lp_send_package = eggNetServer_processing(lp_server, lp_recv_package);
            }
            
            printf("Content-type: text/html\r\nContent-Length: %d\r\n\r\n", eggNetPackage_get_packagesize(lp_send_package));
            send_fcgi(lp_server, lp_send_package, eggNetPackage_get_packagesize(lp_send_package), 0);
            /* fwrite(lp_send_package, eggNetPackage_get_packagesize(lp_send_package), 1, stdout); */
            /*           fflush(stdout); */
                      //eggNetServer_send(lp_server, lp_send_package, eggNetPackage_get_packagesize(lp_send_package), 0);
            free(lp_recv_package);
            free(lp_send_package);
        }
        else
        {
            printf("Content-type: text/html\r\n"
		   "\r\n"
		   "<title>FastCGI echo</title>"
		   "<h1>FastCGI echo</h1>\n"
		   "no data !, Request number %d,  Process ID: %d <p>\n",  ++count, getpid());

        }
//        FCGI_Finish();
    }
    
    eggNetServer_delete (lp_server);

    return 0;
}
