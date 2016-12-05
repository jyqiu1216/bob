#ifndef _SEARCH_NET_IO_H_
#define _SEARCH_NET_IO_H_

#include "std_header.h"
#include "aws_rsp_info.h"

class CSearchNetIO : public ITasksGroupCallBack
{
public:
    // 初始化
    TINT32 Init(CConf *pobjConf, CTseLogger *pLog, CTaskQueue *poTaskQueue);

    // 网络IO线程驱动函数
    static TVOID * RoutineNetIO(void *pParam);

    // 网络消息回调
    virtual void OnTasksFinishedCallBack(LTasksGroup *pTasksgrp);

    // 网络消息请求
    virtual void OnUserRequest(LongConnHandle hSession, const TUCHAR *pData, TUINT32 uLen, BOOL &bWillResponse);

private:
    //  创建监听socket
    SOCKET CreateListenSocket(TCHAR *pszListenHost, TUINT16 uwPort);

    // 关闭监听socket
    TINT32 CloseListenSocket();

    // 获取ip和端口
    TINT32 GetIp2PortByHandle(LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp);

public:
    // 长连接对象
    ILongConn               *m_pLongConn;
    // 日志对象
    CTseLogger              *m_poServLog;
    // session队列
    CTaskQueue              *m_poTaskQueue;
    // 打包/解包工具
    CBaseProtocolPack       *m_pPackTool;
    CBaseProtocolUnpack     *m_pUnPackTool;
    // 监听socket
    TINT32                  m_hListenSock;
    // 序列号
    TUINT32                 m_udwSeqno;
    // 配置
    CConf					*m_poConf;

private:
    // 处理响应消息
    TINT32 OnEventResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 ParseEventResponse(TUCHAR *pszPack, TUINT32 udwPackLen, EventRspInfo* pAwsRspInfo);
};

#endif
