#ifndef _GLOBAL_SERV_H_
#define _GLOBAL_SERV_H_

#include "std_header.h"
#include "task_process.h"
#include "cgi_socket.h"
#include "query_net_io.h"
#include "search_net_io.h"
#include "session_mgr_t.h"
#include <pthread.h>
#include "notic_process.h"
#include "delay_msg_process.h"

#include "zkutil/zk_util.h"

#include "update_notic_task_mgr.h"
#include "update_notic_task.h"
#include "update_notic_process.h"
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
    static CTseLogger *m_poServLog;		    // ������־
    static CTseLogger *m_poDayReqLog;		// ������Ϣ
	static CTseLogger *m_poRegLog;		    // ע����־
	static CTseLogger *m_poStatLog;		    // ͳ����־
    static CTseLogger *m_poFormatLog;
    static CTseLogger *m_poJasonLog;

    static CConf *m_poConf;			        // ������Ϣ

    static CSessionMgr *m_poSessionMgr;     // session������
    static CTaskQueue *m_poTaskQueue;		// �������

    static CQueryNetIO *m_poQueryNetIO;	    // ����IO
	static CSearchNetIO *m_poSearchNetIO;	// ����IO

    static CTaskProcess *m_poTaskProcess;	// �����߳�

	static TUINT32 m_udwMailSeq;
	static TUINT32 m_udwMailSeqStep;

    static CNoticTaskQueue *m_poNoticTaskQueue;
    static CNoticProcess *m_poNoticProcess; 

    static CDelayMsgProcess *m_poMsgDelayUpdateProcess; //�ӳٸ��¹����߳�

    static CZkRegConf *m_poZkConf;          //ZK ����
    static CZkRegClient *m_poZkRegClient;

	//update_notic  add by andy 20160429
	static CUpdateNoticProcess *m_UpdateNoticProcessList;		 /// ����֪ͨ�߳��б�(����)
	static CUpdateNoticTaskQueue *m_poUpdateNoticQueue; //����֪ͨ��������
	static UpdateSvrInfo *m_sUpdateSvrInfoList; //ip port list

private:
	static TUINT32 m_udwHsReqSeq;
	static pthread_mutex_t m_mtxHsReqSeq;
};

#endif
