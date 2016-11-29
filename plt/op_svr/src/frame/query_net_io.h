#ifndef _QUERY_NET_IO_H_
#define _QUERY_NET_IO_H_

#include "std_header.h"

class CQueryNetIO : public ITasksGroupCallBack
{
public:
    // ��ʼ��
    TINT32 Init(CTseLogger *pLog, CTaskQueue *poTaskQueue);

    // ����IO�߳���������
    static TVOID * RoutineNetIO(void *pParam);

    // ������Ϣ�ص�
    virtual void OnTasksFinishedCallBack(LTasksGroup *pTasksgrp);

    // ������Ϣ����
    virtual void OnUserRequest(LongConnHandle hSession, const TUCHAR *pData, TUINT32 uLen, BOOL &bWillResponse);

public:
    //  ��������socket
    SOCKET CreateListenSocket(TCHAR *pszListenHost, TUINT16 uwPort);

    // �رռ���socket
    TINT32 CloseListenSocket();

    // ��ȡip�Ͷ˿�
    TINT32 GetIp2PortByHandle(LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp);

private:
    // �ڽ�������ʱ�ͷ�������ʱ,��front���ش���
    TINT32 SendHttpBackErr(LongConnHandle stHandle, TUINT32 udwSeqno);

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
};

#endif