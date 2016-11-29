#ifndef _SSC_TASK_PROCESS_H_
#define _SSC_TASK_PROCESS_H_

#include "std_header.h"

class CTaskProcess
{
public:
    // 初始化
    TINT32 Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn,
        CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDbLog);
    // 线程入口函数
    static void *Start(void *pParam);
    // 尝试从队列里获取任务
    TINT32 WorkRoutine();

public:
    // function  ===>处理client的请求(出错时需要保证session的释放和客户端的返回)
    //TINT32 ProcessClientRequest(SSession *pstSession);


private:
    //TINT32 SendBackResult(SSession *pstSession);

public:
    // 配置文件对象
    CConf               *m_poConf;
    ILongConn           *m_poSearchLongConn;
    ILongConn           *m_poQueryLongConn;

    // 日志对象
    CTseLogger          *m_poServLog;
    CTseLogger          *m_poClientReqLog;

    // 任务队列
    CTaskQueue          *m_pTaskQueue;

    // 打包/解包器
    CBaseProtocolPack   *m_pPackTool[MAX_AWS_REQ_TASK_NUM];
    CBaseProtocolUnpack *m_pUnPackTool;

private:
    //中间使用的变量////////////////////////////////////
    TUCHAR              m_szReqBuf[MAX_REQ_BUF_LEN];
};

#endif
