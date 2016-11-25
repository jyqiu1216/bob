#ifndef _GLOBAL_SERV_H_
#define _GLOBAL_SERV_H_

#include "std_header.h"
#include "task_process.h"
#include "cgi_socket.h"
#include "search_net_io.h"
#include "session_mgr.h"
#include "action_get.h"
#include <pthread.h>
#include "aws_table_include.h"
#include "notic_process.h"
#include "delay_msg_process.h"
#include "zkutil/zk_util.h"

/*
 * 初始化和管理全局资源
 */
class CGlobalServ
{
public:
    static int Init();
    static int Uninit();

    static int Start();

    static int StopNet();
    static TUINT32 GenerateHsReqSeq();

    static int InitAwsTable(const TCHAR *pszProjectPrefix);

public:
    static CTseLogger           *m_poServLog;		// 运行日志
    static CTseLogger           *m_poActionLog;		// 访问信息,上传日志中心
	static CTseLogger           *m_poRegLog;		// 访问信息,上传日志中心
	static CTseLogger			*m_poStatLog;		// 统计日志
	static CTseLogger			*m_poReqLog;		// 请求处理日志结果输出
	static CTseLogger			*m_poFormatLog;		// 格式化日志
    static CTseLogger           *m_poWildLog;       // 野地更新日志
    static CTseLogger           *m_pAbnormalActionLog; //异常action(延迟结束处理)单独打log
    static CTseLogger *m_poJsonLog;

    static CConf                *m_poConf;			// 配置信息

    static CTaskQueue           *m_poTaskQueue;		// 任务队列

	static CSearchNetIO         *m_poSearchNetIO;	// 网络IO

    static CTaskProcess         *m_poTaskProcess;	// 工作线程

    static CNoticTaskQueue      *m_poNoticTaskQueue;
    static CNoticProcess        *m_poNoticProcess;

    static CDelayMsgProcess     *m_poMsgDelayUpdateProcess; //延迟更新工作线程

	
	static CActionGet			*m_poActionPropcess; // action获取线程

    static CZkRegConf           *m_poZkConf;        //ZK 配置

public:
	static TUINT32				m_udwReportId;

private:
	static TUINT32				m_udwHsReqSeq;
	static pthread_mutex_t		m_mtxHsReqSeq;
};

#endif
