#ifndef _SSC_TASK_PROCESS_H_
#define _SSC_TASK_PROCESS_H_

#include "std_header.h"
#include "game_command.h"
#include "des.h"
#include "key_value.h"
#include "json_result_generator.h"

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
    TINT32 ProcessClientRequest(SSession *pstSession);
    // function  ===> normal/special流程的选取
    TINT32 ProcessCommand(SSession *pstSession);

    // function  ===> normal命令字处理的主流程(需要保证session的释放和客户端的返回)
    // in_value  ===> pstSession: 业务session
    //           ===> pBaseCmd: 命令字的处理类的基类,指向具体的命令处理类
    TVOID NormalMainProdure(SSession *pstSession, const TINT32 dwCmdType);
    // function  ===> special命令字处理的主流程(需要保证session的释放和客户端的返回)
    // in_value  ===> pstSession: 业务session
    //           ===> pBaseCmd: 命令字的处理类的基类,指向具体的命令处理类
    TVOID SpecialMainProdure(SSession *pstSession, const TINT32 dwCmdType);

private:
   TINT32 SendBackResult(SSession *pstSession);
   TINT32 SendBackResult_Http(SSession *pstSession);
   TINT32 SendBackResult_Binary(SSession *pstSession);

   TVOID ResetSessionTmpParam(SSession *pstSession);
   TVOID PrintLog(SSession *pstSession);
   TVOID GetStatistics(SSession *pstSession);

private:
    /*****************************************
    加密解密相关
    *****************************************/
    bool EncryptUrl(char *pszIn, char *pszOut, int &dwOutLen, float fVersion);
    bool DecryptUrl(char *pszIn, char *pszOut, int &dwOutLen, float fVersion);
    CDes				*m_pobjEncrypt;
    TCHAR				m_szEncryptBuf[MAX_HTTP_REQ_LEN];
    TCHAR				m_szEncryptUrl[MAX_HTTP_REQ_LEN];

public:
    // 配置文件对象
    CConf               *m_poConf;
    ILongConn           *m_poSearchLongConn;
    ILongConn           *m_poQueryLongConn;

    // 日志对象
    CTseLogger          *m_poServLog;
    CTseLogger          *m_poClientReqLog;
    CTseLogger			*m_poDayLog;

    // 任务队列
    CTaskQueue          *m_pTaskQueue;
    // Json生成器
    CJsonResultGenerator m_oJsonResultGenerator;

    // 打包/解包器
    CBaseProtocolPack   *m_pPackTool[MAX_AWS_REQ_TASK_NUM];
    CBaseProtocolUnpack *m_pUnPackTool;

private:
    //中间使用的变量////////////////////////////////////
    // http的k-v对
    RequestParam		m_stHttpParam;
    TCHAR				*m_pTmpBuf;

    TUCHAR              m_szReqBuf[MAX_REQ_BUF_LEN];
};

#endif
