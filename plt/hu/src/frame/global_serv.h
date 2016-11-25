#ifndef _GLOBAL_SERV_H_
#define _GLOBAL_SERV_H_

#include "std_header.h"
#include "task_process.h"
#include "cgi_socket.h"
#include "query_net_io.h"
#include "search_net_io.h"
#include "session_mgr_t.h"
#include <pthread.h>
#include "notic_process.h"
#include "delay_msg_process.h"

#include "zkutil/zk_util.h"

#include "update_notic_task_mgr.h"
#include "update_notic_task.h"
#include "update_notic_process.h"
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
    static CTseLogger *m_poServLog;		    // 运行日志
    static CTseLogger *m_poDayReqLog;		// 访问信息
	static CTseLogger *m_poRegLog;		    // 注册日志
	static CTseLogger *m_poStatLog;		    // 统计日志
    static CTseLogger *m_poFormatLog;
    static CTseLogger *m_poJasonLog;

    static CConf *m_poConf;			        // 配置信息

    static CSessionMgr *m_poSessionMgr;     // session管理器
    static CTaskQueue *m_poTaskQueue;		// 任务队列

    static CQueryNetIO *m_poQueryNetIO;	    // 网络IO
	static CSearchNetIO *m_poSearchNetIO;	// 网络IO

    static CTaskProcess *m_poTaskProcess;	// 工作线程

	static TUINT32 m_udwMailSeq;
	static TUINT32 m_udwMailSeqStep;

    static CNoticTaskQueue *m_poNoticTaskQueue;
    static CNoticProcess *m_poNoticProcess; 

    static CDelayMsgProcess *m_poMsgDelayUpdateProcess; //延迟更新工作线程

    static CZkRegConf *m_poZkConf;          //ZK 配置
    static CZkRegClient *m_poZkRegClient;

	//update_notic  add by andy 20160429
	static CUpdateNoticProcess *m_UpdateNoticProcessList;		 /// 更新通知线程列表(测试)
	static CUpdateNoticTaskQueue *m_poUpdateNoticQueue; //更新通知工作队列
	static UpdateSvrInfo *m_sUpdateSvrInfoList; //ip port list

private:
	static TUINT32 m_udwHsReqSeq;
	static pthread_mutex_t m_mtxHsReqSeq;
};

#endif
