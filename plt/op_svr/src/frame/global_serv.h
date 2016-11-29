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
 * ��ʼ���͹���ȫ����Դ
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
    // ����
    static CConf                *m_poConf;          // ������Ϣ
    static CZkRegConf           *m_poZkConf;        // ZK����

    // ע��
    static CZkRegClient         *m_poZkRegClient;   // ע��ͻ���

    // ����


    // ��־
    static CTseLogger           *m_poServLog;       // ������־
    static CTseLogger           *m_poReqLog;        // ������Ϣ
    static CTseLogger           *m_poRegLog;        // ע����־
    static CTseLogger           *m_poStatLog;       // ͳ����־

    // ����
    static CSessionMgr          *m_poSessionMgr;    // Session������
    static CTaskQueue           *m_poTaskQueue;     // �������

    // �߳�
    static CQueryNetIO          *m_poQueryNetIO;    // ����IO
    static CSearchNetIO         *m_poSearchNetIO;   // ����IO
    static CTaskProcess         *m_poTaskProcess;   // �����߳�
    static CDownMgr             *m_poDownMgr;       // ���ι�����

private:
    // �̰߳�ȫ�����кţ��������������󽻻�
    static TUINT32              m_udwDownReqSeq;
    static pthread_mutex_t      m_mtxDownReqSeq;
};

#endif
