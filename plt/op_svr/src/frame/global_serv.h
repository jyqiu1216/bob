#ifndef _GLOBAL_SERV_H_
#define _GLOBAL_SERV_H_

#include "std_header.h"
#include "task_process.h"
#include "cgi_socket.h"
#include "query_net_io.h"
#include "search_net_io.h"
#include "session_mgr.h"
#include <pthread.h>
#include "notic_process.h"
#include "delay_msg_process.h"
#include "zk_util.h"

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

public:
    // 配置
    static CConf                *m_poConf;          // 配置信息
    static CZkRegConf           *m_poZkConf;        // ZK配置

    // 注册
    static CZkRegClient         *m_poZkRegClient;   // 注册客户端

    // 数据


    // 日志
    static CTseLogger           *m_poServLog;       // 运行日志
    static CTseLogger           *m_poReqLog;        // 访问信息
    static CTseLogger           *m_poRegLog;        // 注册日志
    static CTseLogger           *m_poStatLog;       // 统计日志

    // 队列
    static CSessionMgr          *m_poSessionMgr;    // Session管理器
    static CTaskQueue           *m_poTaskQueue;     // 任务队列

    // 线程
    static CQueryNetIO          *m_poQueryNetIO;    // 网络IO
    static CSearchNetIO         *m_poSearchNetIO;   // 网络IO
    static CTaskProcess         *m_poTaskProcess;   // 工作线程
    static CDownMgr             *m_poDownMgr;       // 下游管理器

private:
    // 线程安全的序列号，用于上下游请求交互
    static TUINT32              m_udwDownReqSeq;
    static pthread_mutex_t      m_mtxDownReqSeq;
};

#endif
