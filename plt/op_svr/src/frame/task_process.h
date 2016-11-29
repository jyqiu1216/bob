#ifndef _SSC_TASK_PROCESS_H_
#define _SSC_TASK_PROCESS_H_

#include "std_header.h"

class CTaskProcess
{
public:
    // ��ʼ��
    TINT32 Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn,
        CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDbLog);
    // �߳���ں���
    static void *Start(void *pParam);
    // ���ԴӶ������ȡ����
    TINT32 WorkRoutine();

public:
    // function  ===>����client������(����ʱ��Ҫ��֤session���ͷźͿͻ��˵ķ���)
    //TINT32 ProcessClientRequest(SSession *pstSession);


private:
    //TINT32 SendBackResult(SSession *pstSession);

public:
    // �����ļ�����
    CConf               *m_poConf;
    ILongConn           *m_poSearchLongConn;
    ILongConn           *m_poQueryLongConn;

    // ��־����
    CTseLogger          *m_poServLog;
    CTseLogger          *m_poClientReqLog;

    // �������
    CTaskQueue          *m_pTaskQueue;

    // ���/�����
    CBaseProtocolPack   *m_pPackTool[MAX_AWS_REQ_TASK_NUM];
    CBaseProtocolUnpack *m_pUnPackTool;

private:
    //�м�ʹ�õı���////////////////////////////////////
    TUCHAR              m_szReqBuf[MAX_REQ_BUF_LEN];
};

#endif
