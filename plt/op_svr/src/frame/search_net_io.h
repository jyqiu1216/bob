#ifndef _SEARCH_NET_IO_H_
#define _SEARCH_NET_IO_H_

#include "std_header.h"
#include "aws_rsp_info.h"

class CSearchNetIO : public ITasksGroupCallBack
{
public:
    // ��ʼ��
    TINT32 Init(CConf *pobjConf, CTseLogger *pLog, CTaskQueue *poTaskQueue);

    // ����IO�߳���������
    static TVOID * RoutineNetIO(void *pParam);

    // ������Ϣ�ص�
    virtual void OnTasksFinishedCallBack(LTasksGroup *pTasksgrp);

    // ������Ϣ����
    virtual void OnUserRequest(LongConnHandle hSession, const TUCHAR *pData, TUINT32 uLen, BOOL &bWillResponse);

private:
    //  ��������socket
    SOCKET CreateListenSocket(TCHAR *pszListenHost, TUINT16 uwPort);

    // �رռ���socket
    TINT32 CloseListenSocket();

    // ��ȡip�Ͷ˿�
    TINT32 GetIp2PortByHandle(LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp);

public:
    // �����Ӷ���
    ILongConn               *m_pLongConn;
    // ��־����
    CTseLogger              *m_poServLog;
    // session����
    CTaskQueue              *m_poTaskQueue;
    // ���/�������
    CBaseProtocolPack       *m_pPackTool;
    CBaseProtocolUnpack     *m_pUnPackTool;
    // ����socket
    TINT32                  m_hListenSock;
    // ���к�
    TUINT32                 m_udwSeqno;
    // ����
    CConf					*m_poConf;

private:
    // ������Ӧ��Ϣ
    TINT32 OnEventResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 ParseEventResponse(TUCHAR *pszPack, TUINT32 udwPackLen, EventRspInfo* pAwsRspInfo);
};

#endif
