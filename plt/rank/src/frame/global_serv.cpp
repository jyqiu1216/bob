#include "rc4.h"
#include "statistic.h"
#include "game_info.h"
#include "aws_table_include.h"
#include "game_command.h"
#include "global_serv.h"
#include "conf_base.h"
#include "alliance_mapping.h"
#include "all_server.h"


// 静态变量定义
CTseLogger*	CGlobalServ::m_poServLog = NULL;
CTseLogger*	CGlobalServ::m_poDayReqLog = NULL;
CTseLogger* CGlobalServ::m_poRegLog = NULL;
CTseLogger* CGlobalServ::m_poStatLog = NULL;
CTseLogger* CGlobalServ::m_poFormatLog = NULL;
CConf*		CGlobalServ::m_poConf = NULL;
CTaskQueue* CGlobalServ::m_poTaskQueue = NULL;
CQueryNetIO*  CGlobalServ::m_poQueryNetIO = NULL;
CSearchNetIO* CGlobalServ::m_poSearchNetIO = NULL;
CTaskProcess* CGlobalServ::m_poTaskProcess = NULL;
CTseLogger* CGlobalServ::m_poJasonLog = NULL;

CZkRegConf*		CGlobalServ::m_poZkConf = NULL;
CZkRegClient*   CGlobalServ::m_poZkRegClient = NULL;
CDownMgr*       CGlobalServ::m_poDownMgr = NULL;

TUINT32		CGlobalServ::m_udwMailSeq = 0;
TUINT32		CGlobalServ::m_udwMailSeqStep = 1;

TUINT32		CGlobalServ::m_udwHsReqSeq = 1;
pthread_mutex_t CGlobalServ::m_mtxHsReqSeq = PTHREAD_MUTEX_INITIALIZER;

CNoticTaskQueue *CGlobalServ::m_poNoticTaskQueue = NULL;
CNoticProcess *CGlobalServ::m_poNoticProcess = NULL;

TUINT32 CGlobalServ::GenerateHsReqSeq()
{
    TUINT32 udwSeq = 0;
    pthread_mutex_lock(&m_mtxHsReqSeq);
    m_udwHsReqSeq++;
    if(m_udwHsReqSeq < 100000)
    {
        m_udwHsReqSeq = 100000;
    }
    udwSeq = m_udwHsReqSeq;
    pthread_mutex_unlock(&m_mtxHsReqSeq);
    return udwSeq;
}

int CGlobalServ::InitAwsTable(const TCHAR *pszProjectPrefix)
{
    TbAction::Init("../tblxml/action.xml", pszProjectPrefix);
    TbAl_assist::Init("../tblxml/al_assist.xml", pszProjectPrefix);
    TbAl_gift::Init("../tblxml/al_gift.xml", pszProjectPrefix);
    TbAl_help::Init("../tblxml/al_help.xml", pszProjectPrefix);
    TbAl_member::Init("../tblxml/al_member.xml", pszProjectPrefix);
    TbAl_store_consume::Init("../tblxml/al_store_consume.xml", pszProjectPrefix);
    TbAl_wall::Init("../tblxml/al_wall.xml", pszProjectPrefix);
    TbAlliance::Init("../tblxml/alliance.xml", pszProjectPrefix);
    TbAlliance_action::Init("../tblxml/alliance_action.xml", pszProjectPrefix);
    TbApns_token::Init("../tblxml/apns_token.xml", pszProjectPrefix);
    TbBackpack::Init("../tblxml/backpack.xml", pszProjectPrefix);
    TbBlack_account::Init("../tblxml/black_account.xml", pszProjectPrefix);
    TbBlacklist::Init("../tblxml/blacklist.xml", pszProjectPrefix);
    TbBookmark::Init("../tblxml/bookmark.xml", pszProjectPrefix);
    TbCity::Init("../tblxml/city.xml", pszProjectPrefix);
    TbDiplomacy::Init("../tblxml/diplomacy.xml", pszProjectPrefix);
    TbIap_transaction::Init("../tblxml/iap_transaction.xml", pszProjectPrefix);
    TbLogin::Init("../tblxml/login.xml", pszProjectPrefix);
    TbMail::Init("../tblxml/mail.xml", pszProjectPrefix);
    TbMail_operate::Init("../tblxml/mail_operate.xml", pszProjectPrefix);
    TbMail_user::Init("../tblxml/mail_user.xml", pszProjectPrefix);
    TbMap::Init("../tblxml/map.xml", pszProjectPrefix);
    TbMarch_action::Init("../tblxml/march_action.xml", pszProjectPrefix);
    TbParam::Init("../tblxml/param.xml", pszProjectPrefix);
    TbPlayer::Init("../tblxml/player.xml", pszProjectPrefix);
    TbQuest::Init("../tblxml/quest.xml", pszProjectPrefix);
    TbReport::Init("../tblxml/report.xml", pszProjectPrefix);
    TbReport_user::Init("../tblxml/report_user.xml", pszProjectPrefix);
    TbSvr_stat::Init("../tblxml/svr_stat.xml", pszProjectPrefix);
    TbTips::Init("../tblxml/tips.xml", pszProjectPrefix);
    TbUnique_name::Init("../tblxml/unique_name.xml", pszProjectPrefix);
    TbUser_stat::Init("../tblxml/user_stat.xml", pszProjectPrefix);
    TbAl_gift_reward::Init("../tblxml/al_gift_reward.xml", pszProjectPrefix);
    TbRally_history::Init("../tblxml/rally_history.xml", pszProjectPrefix);
    TbAltar_history::Init("../tblxml/altar_history.xml", pszProjectPrefix);
    TbTitle::Init("../tblxml/title.xml", pszProjectPrefix);

    return 0;
}

int CGlobalServ::Init()
{
    ostringstream oss;

    // 1. 初始化日志对象
    INIT_LOG_MODULE("../conf/serv_log.conf");
    config_ret = 0;
    DEFINE_LOG(serv_log, "serv_log");
    DEFINE_LOG(reg_log, "reg_log");
    DEFINE_LOG(req_log, "req_log");
    DEFINE_LOG(stat_log, "stat_log");
    m_poServLog = serv_log;
    m_poDayReqLog = req_log;
    m_poRegLog = reg_log;
    m_poStatLog = stat_log;


    // 2. 初始化配置文件
    m_poConf = new CConf;
    if(0 != m_poConf->Init("../conf/serv_info.conf"))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init conf failed!"));
        return -2;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init conf success!"));

    if(0 != CConfBase::Init("../conf/module.conf", m_poConf->m_szModule))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init project.conf failed!"));
        return -20;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init project.conf success!"));

    // 配置文件需要先读取，才能初始化表
    InitAwsTable(CConfBase::GetString("tbxml_project").c_str());

    //初始化zk相关配置
    m_poZkConf = new CZkRegConf;
    if (0 != m_poZkConf->Init("../conf/serv_info.conf", "../conf/module.conf"))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init zk_conf failed!"));
        return -3;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init zk_conf success!"));

    m_poZkRegClient = new CZkRegClient;
    if(0 != m_poZkRegClient->Init(m_poRegLog, m_poZkConf))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ Init reg_client_zk failed"));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ Init reg_client_zk succ"));


    // 初始化命令字信息
    CClientCmd *poClientCmd = CClientCmd::GetInstance();
    if(0 != poClientCmd->Init())
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init client cmd info failed!"));
        return -17;
    }

    // 3. 初始化队列信息
    m_poTaskQueue = new CTaskQueue();
    if(0 != m_poTaskQueue->Init(CConfBase::GetInt("queue_size")))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init task queue failed! [queue_size=%u]",
            CConfBase::GetInt("queue_size")));
        return -3;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init task_queue success!"));

    // 4. 初始化Session管理器
    if(0 != CSessionMgr::Instance()->Init(CConfBase::GetInt("queue_size"), m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init session_mgr failed!"));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init session_mgr success!"));

    // 5. 初始化网络IO
    m_poQueryNetIO = new CQueryNetIO;
    if(0 != m_poQueryNetIO->Init(m_poConf, m_poServLog, m_poTaskQueue))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init net_io failed!"));
        return -5;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init net_io success!"));

    // 6. 初始化网络IO
    m_poSearchNetIO = new CSearchNetIO;
    if(0 != m_poSearchNetIO->Init(m_poConf, m_poServLog, m_poRegLog, m_poTaskQueue))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init net_io failed!"));
        return -6;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init net_io success!"));

    //初始化zk下游管理器
    m_poDownMgr = CDownMgr::Instance();
    if(0 != m_poDownMgr->zk_Init(m_poRegLog, m_poSearchNetIO->m_pLongConn, m_poZkConf))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init zk_session_mgr failed!"));
        return -10;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init zk_session_mgr success!"));

    // 8. 初始化工作线程
    m_poTaskProcess = new CTaskProcess[CConfBase::GetInt("work_thread_num")];
    for(TUINT32 udwIdx = 0; udwIdx < CConfBase::GetInt("work_thread_num"); udwIdx++)
    {
        if(0 != m_poTaskProcess[udwIdx].Init(m_poConf, m_poSearchNetIO->m_pLongConn,
            m_poQueryNetIO->m_pLongConn, m_poTaskQueue, m_poServLog, m_poDayReqLog))
        {
            TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %uth task process failed!", udwIdx));
            return -8;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init task_process success!"));

    // 9. 初始化rc4算法管理器
    CEncryptCR4Mgr::get_instance();

    // 10. statistic
    CStatistic *pstStatistic = CStatistic::Instance();
    pstStatistic->Init(m_poStatLog, m_poConf);

    // 11. 初始化alliance mapping信息
    CAllianceMapping *poAllianceMapping = CAllianceMapping::GetInstance();
    if(0 != poAllianceMapping->Init("../data/alliance_mapping.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init alliance_mapping failed!"));
        return -15;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init alliance_mapping success!"));

    AllServer::init(m_poServLog, RANK_CONF_FILE, m_poConf->m_szModuleIp);
    AllServer* instance = AllServer::instance();
    instance->updateData();
    
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init rank all server success!"));

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
    TUINT32 udwIdx = 0;

    // 1. 创建工作线程
    for(udwIdx = 0; udwIdx < CConfBase::GetInt("work_thread_num"); udwIdx++)
    {
        if(pthread_create(&thread_id, NULL, CTaskProcess::Start, &m_poTaskProcess[udwIdx]) != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("create task process thread failed! [idx=%u] Error[%s]",
                udwIdx, strerror(errno)));
            return -1;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("create task_process thread success!"));
    

    //2. 创建zk下游管理线程
    if(pthread_create(&thread_id, NULL, CDownMgr::zk_StartPull, CDownMgr::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create down manager thread failed! Error[%s]", strerror(errno)));
        return -5;
    }
    if(pthread_create(&thread_id, NULL, CDownMgr::zk_StartCheck, CDownMgr::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create down manager thread failed! Error[%s]", strerror(errno)));
        return -5;
    }

    // 3. 创建网络线程-search
    if(pthread_create(&thread_id, NULL, CSearchNetIO::RoutineNetIO, m_poSearchNetIO) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create seach net io thread failed! Error[%s]",
            strerror(errno)));
        return -3;
    }
    TSE_LOG_INFO(m_poServLog, ("create net io thread success!"));

    // 4. 创建网络线程-query
    if(pthread_create(&thread_id, NULL, CQueryNetIO::RoutineNetIO, m_poQueryNetIO) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create query net io thread failed! Error[%s]",
            strerror(errno)));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("create net io thread success!"));

    // 5. 开启统计线程
    if(pthread_create(&thread_id, NULL, CStatistic::Start, CStatistic::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create statistic thread failed! Error[%s]",
            strerror(errno)));
        return -5;
    }
    TSE_LOG_INFO(m_poServLog, ("create statistic thread success!"));

    // 6.开启注册
    m_poZkRegClient->Start();

    TSE_LOG_INFO(m_poServLog, ("GlobalServ start all thread success!"));
    return 0;
}

int CGlobalServ::StopNet()
{
    // 停止query线程的接收请求端口
    m_poQueryNetIO->CloseListenSocket();
    return 0;
}
