#ifndef _NOTIC_PROCESS_H_
#define _NOTIC_PROCESS_H_

#include "notic_task.h"
#include "notic_task_mgr.h"
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "my_define.h"
using namespace std;

class CNoticProcess
{
public:
    TINT32 Init(CNoticTaskQueue *pTaskQueue, CTseLogger *poServLog);
    static void *Start(void *pParam);
    TINT32 WorkRoutine();

public:
    TINT32 Notic_Process(SNoticTask *pstNoticTask);
    TINT32 SendNoticServer(string strNotic);


public:
    static CTseLogger *m_poServLog;
    static CNoticTaskQueue *m_pTaskQueue;

    TINT32 m_dwSocket;
    vector<struct sockaddr_in *> m_vserver_addr_list;

    /*
    struct sockaddr_in m_server_addr;
    */
    
    TUCHAR m_szBuf[MAX_NETIO_PACKAGE_BUF_LEN];
    TUINT32 m_udwBufLen;


};

#endif
