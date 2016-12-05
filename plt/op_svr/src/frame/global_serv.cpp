#include "global_serv.h"
#include "statistic.h"
#include "curl/curl.h"
#include "aws_table_include.h"
#include "conf_base.h"
#include <string>
#include <sstream>
#include "warning_mgr.h"
#include "game_command.h"

// ��̬��������
CTseLogger* CGlobalServ::m_poServLog    = NULL;
CTseLogger* CGlobalServ::m_poReqLog     = NULL;
CTseLogger* CGlobalServ::m_poRegLog     = NULL;
CTseLogger* CGlobalServ::m_poStatLog    = NULL;

CConf* CGlobalServ::m_poConf = NULL;

CSessionMgr* CGlobalServ::m_poSessionMgr = NULL;
CTaskQueue* CGlobalServ::m_poTaskQueue = NULL;

CQueryNetIO*  CGlobalServ::m_poQueryNetIO = NULL;
CSearchNetIO* CGlobalServ::m_poSearchNetIO = NULL;
CTaskProcess* CGlobalServ::m_poTaskProcess = NULL;
CDownMgr* CGlobalServ::m_poDownMgr = NULL;

CZkRegConf* CGlobalServ::m_poZkConf = NULL;
CZkRegClient* CGlobalServ::m_poZkRegClient = NULL;



TUINT32 CGlobalServ::m_udwDownReqSeq = 1;
pthread_mutex_t CGlobalServ::m_mtxDownReqSeq = PTHREAD_MUTEX_INITIALIZER;


TUINT32 CGlobalServ::GenerateHsReqSeq()
{
    TUINT32 udwSeq = 0;
    pthread_mutex_lock(&m_mtxDownReqSeq);
    m_udwDownReqSeq++;
    if (m_udwDownReqSeq < 100000)
    {
        m_udwDownReqSeq = 100000;
    }
    udwSeq = m_udwDownReqSeq;
    pthread_mutex_unlock(&m_mtxDownReqSeq);
    return udwSeq;
}

int CGlobalServ::InitAwsTable(const TCHAR *pszProjectPrefix)
{
    TbAl_member::Init("../tblxml/al_member.xml", pszProjectPrefix);
}

int CGlobalServ::Init()
{
    // ��ʼ����־����
    INIT_LOG_MODULE("../conf/serv_log.conf");
    config_ret = 0;
    DEFINE_LOG(serv_log, "serv_log");
    DEFINE_LOG(reg_log, "reg_log");
    DEFINE_LOG(req_log, "req_log");
    DEFINE_LOG(stat_log, "stat_log");

    m_poServLog = serv_log;
    m_poReqLog  = req_log;
    m_poRegLog  = reg_log;
    m_poStatLog = stat_log;

    // ---------------------����tool��ʼ��--------------------------------
    // curl
    curl_global_init(CURL_GLOBAL_ALL);

    // ----------------------��ʼ�������ļ�--------------------------------
    m_poConf = new CConf();
    if (0 != m_poConf->Init("../conf/serv_info.conf"))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init conf failed!"));
        return -1;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init conf success!"));

    if (0 != CConfBase::Init("../conf/module.conf", m_poConf->m_szModuleName))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init project.conf failed!"));
        return -2;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init project.conf success!"));

    // ��ʼ��zk�������
    m_poZkConf = new CZkRegConf();
    if (0 != m_poZkConf->Init("../conf/serv_info.conf", "../conf/module.conf"))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init zk_conf failed!"));
        return -3;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init zk_conf success!"));

    //��ʼ��aws_tables
    InitAwsTable(CConfBase::GetString("tbxml_project").c_str());
    TSE_LOG_INFO(m_poServLog, ("GlobalServ inittable success!"));

    // ��ʼ��ZK�ͻ���
    m_poZkRegClient = new CZkRegClient();
    if (0 != m_poZkRegClient->Init(m_poRegLog, m_poZkConf))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ Init zk_reg_client failed"));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ Init zk_reg_client succ"));

    // ��ʼ����������Ϣ
    CClientCmd *poClientCmd = CClientCmd::GetInstance();
    if (0 != poClientCmd->Init())
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init client cmd info failed!"));
        return -17;
    }

    // ----------------------��ʼ���������--------------------------------
    // ��ʼ��������Ϣ
    m_poTaskQueue = new CTaskQueue();
    if (0 != m_poTaskQueue->Init(CConfBase::GetInt("queue_size")))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init task queue failed! [queue_size=%u]",
            CConfBase::GetInt("queue_size")));
        return -5;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init task_queue success!"));

    // ��ʼ��Session������
    m_poSessionMgr = CSessionMgr::Instance();
    if (0 != m_poSessionMgr->Init(CConfBase::GetInt("queue_size"), m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init session_mgr failed!"));
        return -6;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init session_mgr success!"));

    // ----------------------��ʼ���߳����--------------------------------
    // ��ʼ������IO����������
    m_poQueryNetIO = new CQueryNetIO();
    if (0 != m_poQueryNetIO->Init(m_poServLog, m_poTaskQueue))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init query_net_io failed!"));
        return -7;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init query_net_io success!"));

    // ��ʼ������IO����������
    m_poSearchNetIO = new CSearchNetIO();
    if (0 != m_poSearchNetIO->Init(m_poConf, m_poServLog, m_poTaskQueue))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init search_net_io failed!"));
        return -8;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init search_net_io success!"));

    // ��ʼ��zk���ι����߳�
    m_poDownMgr = CDownMgr::Instance();
    if (0 != m_poDownMgr->zk_Init(m_poRegLog, m_poSearchNetIO->m_pLongConn, m_poZkConf))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init down_mgr failed!"));
        return -9;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init down_mgr success!"));

    // ��ʼ�������߳�
    m_poTaskProcess = new CTaskProcess[CConfBase::GetInt("work_thread_num")];
    for (TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("work_thread_num"); dwIdx++)
    {
        if (0 != m_poTaskProcess[dwIdx].Init(m_poConf, m_poSearchNetIO->m_pLongConn,
            m_poQueryNetIO->m_pLongConn, m_poTaskQueue, m_poServLog, m_poReqLog))
        {
            TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %uth task process failed!", dwIdx));
            return -10;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init task_process success!"));

    // ��ʼ���澯�ϱ�
    CWarningMgr::GetInstance()->Init(m_poServLog);
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init warning mgr success!"));
    // statistic
    CStatistic *pstStatistic = CStatistic::Instance();
    pstStatistic->Init(m_poStatLog, m_poConf);

    // ----------------------end--------------------------------
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init all success!"));

    return 0;
}

int CGlobalServ::Uninit()
{
    // do nothing
    return 0;
}

int CGlobalServ::Start()
{
    pthread_t thread_id = 0;

    // ����ͳ���߳�
    if (pthread_create(&thread_id, NULL, CStatistic::Start, CStatistic::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create statistic thread failed! Error[%s]",
            strerror(errno)));
        return -1;
    }
    TSE_LOG_INFO(m_poServLog, ("create statistic thread success!"));

    // ���������߳�
    for (TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("work_thread_num"); dwIdx++)
    {
        if (pthread_create(&thread_id, NULL, CTaskProcess::Start, &m_poTaskProcess[dwIdx]) != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("create task process thread failed! [idx=%u] Error[%s]",
                dwIdx, strerror(errno)));
            return -2;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("create task_process thread success!"));

    // ����zk���ι����߳�
    if (pthread_create(&thread_id, NULL, CDownMgr::zk_StartPull, CDownMgr::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create down manager thread failed! Error[%s]", strerror(errno)));
        return -3;
    }
    if (pthread_create(&thread_id, NULL, CDownMgr::zk_StartCheck, CDownMgr::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create down manager thread failed! Error[%s]", strerror(errno)));
        return -3;
    }

    // ���������߳�-����
    if (pthread_create(&thread_id, NULL, CSearchNetIO::RoutineNetIO, m_poSearchNetIO) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create search_net_io thread failed! Error[%s]",
            strerror(errno)));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("create search_net_io thread success!"));

    // ���������߳�-���Ρ������߳��ṩ���������������ʱ����
    sleep(2);
    if (pthread_create(&thread_id, NULL, CQueryNetIO::RoutineNetIO, m_poQueryNetIO) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create query_net_io thread failed! Error[%s]",
            strerror(errno)));
        return -5;
    }
    TSE_LOG_INFO(m_poServLog, ("create query_net_io thread success!"));


    // ����ע���߳�
    m_poZkRegClient->Start();

    TSE_LOG_INFO(m_poServLog, ("GlobalServ start all thread success!"));
    return 0;
}

int CGlobalServ::StopNet()
{
    // ֹͣquery�̵߳Ľ�������˿�
    m_poQueryNetIO->CloseListenSocket();

    curl_global_cleanup();
    return 0;
}
