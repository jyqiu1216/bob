#ifndef _GLOBAL_SERV_H_
#define _GLOBAL_SERV_H_

#include "std_header.h"
#include "task_process.h"
#include "down_mgr.h"
#include "cgi_socket.h"
#include "query_net_io.h"
#include "search_net_io.h"
#include "session_mgr.h"
#include <pthread.h>
#include "notic_task.h"
#include "notic_process.h"
#include "zkutil/zk_util.h"

/*
 * ��ʼ���͹���ȫ����Դ
 */
class CGlobalServ
{
public:
    static int Init();
    static int Uninit();

	static int InitAwsTable(const TCHAR *pszProjectPrefix);

    static int Start();

    static int StopNet();
    static TUINT32 GenerateHsReqSeq(); 

public:
    static CTseLogger           *m_poServLog;		// ������־
    static CTseLogger           *m_poDayReqLog;		// ������Ϣ
	static CTseLogger           *m_poRegLog;		// ע����־
	static CTseLogger			*m_poStatLog;		// ͳ����־
    static CTseLogger           *m_poFormatLog;
    static CTseLogger           *m_poJasonLog;

    static CConf                *m_poConf;			// rank��������Ϣ

    static CTaskQueue           *m_poTaskQueue;		// �������

    static CQueryNetIO          *m_poQueryNetIO;	// ����IO
	static CSearchNetIO         *m_poSearchNetIO;	// ����IO

    static CTaskProcess         *m_poTaskProcess;	// �����߳�

    static CNoticTaskQueue      *m_poNoticTaskQueue;
    static CNoticProcess        *m_poNoticProcess;

	static TUINT32				m_udwMailSeq;
	static TUINT32				m_udwMailSeqStep;

    //wave@20160712:zk_reg
    static CZkRegConf           *m_poZkConf;        //ZK 
    static CZkRegClient         *m_poZkRegClient;
    static CDownMgr             *m_poDownMgr;
	
private:
	static TUINT32				m_udwHsReqSeq;
	static pthread_mutex_t		m_mtxHsReqSeq;
};

#endif