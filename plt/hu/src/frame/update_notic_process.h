#ifndef _UPDATE_NOTIC_PROCESS_H_
#define _UPDATE_NOTIC_PROCESS_H_

#include "update_notic_task.h"
#include "update_notic_task_mgr.h"
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "my_define.h"
using namespace std;

struct UpdateSvrInfo{
	string m_strIp;
	TUINT16 m_uwPort;
};

class CUpdateNoticProcess
{
public:
    TINT32 Init(CUpdateNoticTaskQueue *pTaskQueue, CTseLogger *poServLog, TINT32 dwIpNum, UpdateSvrInfo *sUpdateSvrInfoList );
    static void *Start(void *pParam);
    TINT32 WorkRoutine();

public:
    TINT32 UpdateNotic_Process(SUpdateNoticTask *pstUpdateNoticTask);
    TINT32 SendUpdateNoticServer(SUpdateNoticTask *pstUpdateNoticTask, string strNotic);


public:
    static CTseLogger *m_poServLog;
    static CUpdateNoticTaskQueue *m_pTaskQueue;
	
private:
	TINT32 m_dwIpNum; //update svr 的机器数量 
	UpdateSvrInfo *m_sUpdateSvrInfoList; 

};

#endif
