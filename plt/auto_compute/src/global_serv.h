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

    static int InitAwsTable(const TCHAR *pszProjectPrefix);

public:
    static CTseLogger           *m_poServLog;		// ������־
    static CTseLogger           *m_poActionLog;		// ������Ϣ,�ϴ���־����
	static CTseLogger           *m_poRegLog;		// ������Ϣ,�ϴ���־����
	static CTseLogger			*m_poStatLog;		// ͳ����־
	static CTseLogger			*m_poReqLog;		// ��������־������
	static CTseLogger			*m_poFormatLog;		// ��ʽ����־
    static CTseLogger           *m_poWildLog;       // Ұ�ظ�����־
    static CTseLogger           *m_pAbnormalActionLog; //�쳣action(�ӳٽ�������)������log
    static CTseLogger *m_poJsonLog;

    static CConf                *m_poConf;			// ������Ϣ

    static CTaskQueue           *m_poTaskQueue;		// �������

	static CSearchNetIO         *m_poSearchNetIO;	// ����IO

    static CTaskProcess         *m_poTaskProcess;	// �����߳�

    static CNoticTaskQueue      *m_poNoticTaskQueue;
    static CNoticProcess        *m_poNoticProcess;

    static CDelayMsgProcess     *m_poMsgDelayUpdateProcess; //�ӳٸ��¹����߳�

	
	static CActionGet			*m_poActionPropcess; // action��ȡ�߳�

    static CZkRegConf           *m_poZkConf;        //ZK ����

public:
	static TUINT32				m_udwReportId;

private:
	static TUINT32				m_udwHsReqSeq;
	static pthread_mutex_t		m_mtxHsReqSeq;
};

#endif
