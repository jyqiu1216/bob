#ifndef _GLOBAL_SERV_H_
#define _GLOBAL_SERV_H_

#include "std_header.h"
#include "task_process.h"
#include "down_mgr.h"
#include "cgi_socket.h"
#include "query_net_io.h"
#include "search_net_io.h"
#include "session_mgr.h"
#include <pthread.h>
#include "notic_task.h"
#include "notic_process.h"
#include "zkutil/zk_util.h"

/*
 * 初始化和管理全局资源
 */
class CGlobalServ
{
public:
    static int Init();
    static int Uninit();

	static int InitAwsTable(const TCHAR *pszProjectPrefix);

    static int Start();

    static int StopNet();
    static TUINT32 GenerateHsReqSeq(); 

public:
    static CTseLogger           *m_poServLog;		// 运行日志
    static CTseLogger           *m_poDayReqLog;		// 访问信息
	static CTseLogger           *m_poRegLog;		// 注册日志
	static CTseLogger			*m_poStatLog;		// 统计日志
    static CTseLogger           *m_poFormatLog;
    static CTseLogger           *m_poJasonLog;

    static CConf                *m_poConf;			// rank的配置信息

    static CTaskQueue           *m_poTaskQueue;		// 任务队列

    static CQueryNetIO          *m_poQueryNetIO;	// 网络IO
	static CSearchNetIO         *m_poSearchNetIO;	// 网络IO

    static CTaskProcess         *m_poTaskProcess;	// 工作线程

    static CNoticTaskQueue      *m_poNoticTaskQueue;
    static CNoticProcess        *m_poNoticProcess;

	static TUINT32				m_udwMailSeq;
	static TUINT32				m_udwMailSeqStep;

    //wave@20160712:zk_reg
    static CZkRegConf           *m_poZkConf;        //ZK 
    static CZkRegClient         *m_poZkRegClient;
    static CDownMgr             *m_poDownMgr;
	
private:
	static TUINT32				m_udwHsReqSeq;
	static pthread_mutex_t		m_mtxHsReqSeq;
};

#endif
