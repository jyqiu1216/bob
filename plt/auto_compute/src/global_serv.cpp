#include "global_serv.h"
#include "statistic.h"
#include "game_info.h"
#include "wild_info.h"
#include "svr_list_conf.h"
#include "document.h"
#include "svr_list_conf.h"
#include "backpack_info.h"
#include "conf_base.h"
#include "game_svr.h"
#include "user_dyeing_info.h"
#include "event_rule_info.h"
#include "delay_msg_task_mgr.h"
#include "func_open.h"
#include "jsonlog.h"
#include "special_map.h"

// ��̬��������
CTseLogger*	CGlobalServ::m_poServLog = NULL;
CTseLogger*	CGlobalServ::m_poActionLog = NULL;
CTseLogger* CGlobalServ::m_poRegLog = NULL;
CTseLogger* CGlobalServ::m_poStatLog = NULL;
CTseLogger* CGlobalServ::m_poReqLog = NULL;
CTseLogger* CGlobalServ::m_poFormatLog = NULL;
CTseLogger* CGlobalServ::m_poWildLog = NULL;
CTseLogger* CGlobalServ::m_pAbnormalActionLog = NULL;
CZkRegConf*		CGlobalServ::m_poZkConf = NULL;
CTseLogger *CGlobalServ::m_poJsonLog = NULL;

CConf*		CGlobalServ::m_poConf = NULL;
CTaskQueue* CGlobalServ::m_poTaskQueue = NULL;
CActionGet* CGlobalServ::m_poActionPropcess = NULL;
CSearchNetIO* CGlobalServ::m_poSearchNetIO = NULL;
CTaskProcess* CGlobalServ::m_poTaskProcess = NULL;

CDelayMsgProcess* CGlobalServ::m_poMsgDelayUpdateProcess = NULL;

TUINT32		CGlobalServ::m_udwReportId = 0;
TUINT32		CGlobalServ::m_udwHsReqSeq = 1;
pthread_mutex_t CGlobalServ::m_mtxHsReqSeq = PTHREAD_MUTEX_INITIALIZER;

CNoticTaskQueue *CGlobalServ::m_poNoticTaskQueue = NULL;
CNoticProcess *CGlobalServ::m_poNoticProcess = NULL;

TUINT32 CGlobalServ::GenerateHsReqSeq()
{
    pthread_mutex_lock(&m_mtxHsReqSeq);
    m_udwHsReqSeq++;
    if(m_udwHsReqSeq < 1000)
    {
        m_udwHsReqSeq = 1000;
    }
    TUINT32 udwSeq = m_udwHsReqSeq;
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
    TbEquip::Init("../tblxml/equip.xml", pszProjectPrefix);
    TbTitle::Init("../tblxml/title.xml", pszProjectPrefix);
    TbEvent_tips::Init("../tblxml/event_tips.xml", pszProjectPrefix);
    TbBounty::Init("../tblxml/bounty.xml", pszProjectPrefix);
    TbBroadcast::Init("../tblxml/broadcast.xml", pszProjectPrefix);
    TbAl_invite_record::Init("../tblxml/al_invite_record.xml", pszProjectPrefix);
    TbSvr_al::Init("../tblxml/svr_al.xml", pszProjectPrefix);
    TbThrone::Init("../tblxml/throne.xml", pszProjectPrefix);
    TbTask::Init("../tblxml/task.xml", pszProjectPrefix);
    TbReward_window::Init("../tblxml/reward_window.xml", pszProjectPrefix);
    TbIdol::Init("../tblxml/idol.xml", pszProjectPrefix);
    return 0;
}

int CGlobalServ::Init()
{
    ostringstream oss;

    // 1. ��ʼ����־����
    INIT_LOG_MODULE("../conf/serv_log.conf");
    config_ret = 0;
    DEFINE_LOG(serv_log, "serv_log");
    DEFINE_LOG(reg_log, "reg_log");
    DEFINE_LOG(stat_log, "stat_log");
    DEFINE_LOG(action_log, "action_log");
    DEFINE_LOG(req_log, "req_log");
    DEFINE_LOG(format_log, "format_log");
    DEFINE_LOG(wildres_log, "wildres_log");
    DEFINE_LOG(abnormal_action_log, "abnormal_action_log");
    DEFINE_LOG(json_log, "json_log");
    m_poServLog = serv_log;
    m_poActionLog = action_log;
    m_poRegLog = reg_log;
    m_poStatLog = stat_log;
    m_poReqLog = req_log;
    m_poFormatLog = format_log;
    m_poWildLog = wildres_log;
    m_pAbnormalActionLog = abnormal_action_log;
    m_poJsonLog = json_log;

    // 2. ��ʼ�������ļ�
    m_poConf = new CConf;
    if(0 != m_poConf->Init("../conf/serv_info.conf"))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init conf failed!"));
        return -2;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init conf success!"));

    if (0 != CConfBase::Init("../conf/module.conf", m_poConf->m_szModuleName))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init project.conf failed!"));
        return -20;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init project.conf success!"));

    // 2.1 ��ʼ��tbl
    InitAwsTable(CConfBase::GetString("tbxml_project").c_str());

    // 2.2 ��ʼ��zk�����ļ�
    m_poZkConf = new CZkRegConf;
    if (0 != m_poZkConf->Init("../conf/serv_info.conf", "../conf/module.conf"))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init zk_conf failed!"));
        return -2;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init zk_conf success!"));

    // ������Ϸ��̬�ļ�
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    if(0 != poGameInfo->Init("../data/game.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init game_info failed!"));
        return -12;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init game_info success!"));

    if(0 != CGameSvrInfo::Update(m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init game_svr failed!"));
        return -13;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init game_svr success!"));

    // ��ʼ���İ�����
    CDocument *poDocument = CDocument::GetInstance();
    if(0 != poDocument->Init_All(m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init all document failed!"));
        return -19;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init document success!"));


    // ����action_table.conf�ļ�����������action_table.conf
    if(0 != CActionTableConf::GetInstance()->Init("../conf/action_table.conf", CGlobalServ::m_poConf->m_szModuleIp))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init action_table.conf failed!"));
        return -14;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init action_table.conf success!"));
    // ����wild res json
    if(0 != CWildInfo::Update(m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init wild res json failed!"));
        return -14;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init wild res json success!"));

    // 3. ��ʼ��������Ϣ
    m_poTaskQueue = new CTaskQueue();
    if(0 != m_poTaskQueue->Init(CConfBase::GetInt("queue_size")))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init task queue failed! [queue_size=%d]",
            CConfBase::GetInt("queue_size")));
        return -3;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init task_queue success!"));

    // 4. ��ʼ��Session������
    if(0 != CSessionMgr::Instance()->Init(CConfBase::GetInt("queue_size"), m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init session_mgr failed!"));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init session_mgr success!"));

    // 5. ��ʼ��action�߳�
    m_poActionPropcess = new CActionGet;
    if(0 != m_poActionPropcess->Init(m_poConf, m_poZkConf, m_poTaskQueue, m_poActionLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init action_process failed!"));
        return -5;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init action_process success!"));

    // 6. ��ʼ������IO
    m_poSearchNetIO = new CSearchNetIO;
    if(0 != m_poSearchNetIO->Init(m_poConf, m_poServLog, m_poRegLog, m_poTaskQueue))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init net_io failed!"));
        return -6;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init net_io success!"));
    
    //��ʼ��zk���ι�����
    if (0 != CDownMgr::Instance()->zk_Init(m_poRegLog, m_poSearchNetIO->m_pLongConn, m_poZkConf))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init session_mgr failed!"));
        return -7;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init zk_session_mgr success!"));

    // 8. ��ʼ�������߳�
    m_poTaskProcess = new CTaskProcess[CConfBase::GetInt("work_thread_num")];
    TSE_LOG_DEBUG(m_poServLog, ("CGlobalServ:sizeof(LTasksGroup)=%u", sizeof(LTasksGroup)));
    for(TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("work_thread_num"); dwIdx++)
    {
        if(0 != m_poTaskProcess[dwIdx].Init(m_poConf, m_poSearchNetIO->m_pLongConn,
            m_poSearchNetIO->m_pLongConn, m_poTaskQueue, m_poServLog, m_poActionLog))
        {
            TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %uth task process failed!", dwIdx));
            return -8;
        }
    }
    TSE_LOG_DEBUG(m_poServLog, ("CGlobalServ:sizeof(LTasksGroup)=%u", sizeof(LTasksGroup)));
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init task_process success!"));

    // �ӳ�msg����
    if(0 != CDelayMsgTaskMgr::Instance()->Init(CConfBase::GetInt("DelayMsgQueueSize"), m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init delay_msg_task_mgr failed!"));
        return -9;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init delay_msg_task_mgr success!"));

    m_poMsgDelayUpdateProcess = new CDelayMsgProcess[CConfBase::GetInt("DelayMsgWorkThreadNum")];
    for(TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("DelayMsgWorkThreadNum"); dwIdx++)
    {
        if(0 != m_poMsgDelayUpdateProcess[dwIdx].Init(CDelayMsgTaskMgr::Instance()->GetCommQueue(), m_poServLog))
        {
            TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %dth delay msg task process failed!", dwIdx));
            return -10;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init delay_msg_task_process success!"));

    // 10. statistic
    CStatistic *pstStatistic = CStatistic::Instance();
    pstStatistic->Init(m_poStatLog, m_poConf);

    //11 ��ʼ��equip.json
    CBackpackInfo *poBackpackInfo = CBackpackInfo::GetInstance();
    if(0 != poBackpackInfo->Init("../data/equip.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init equip info failed!"));
        return -11;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init equip_info success!"));

    // ��ʼ��function_switch.json
    CFuncOpen *poFuncOpen = CFuncOpen::GetInstance();
    if(0 != poFuncOpen->Init("../data/function_switch.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init func open info failed!"));
        return -11;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init func open success!"));


    // ����user dyeing info
    if (0 != CUserDyeingInfo::Update())
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init user dyeing info failed!"));
        return -20;
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("GlobalServ init user dyeing info success!"));
    }

    // ����event_rule
    if (0 != CEventRuleInfo::Update())
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init event rule info failed!"));
        return -21;
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("GlobalServ init event rule info success!"));
    }

    // ************************************************************************************* //
    
    // ��ʼ�����Ͷ���(��Ϊnotictask�Ļ������)
    m_poNoticTaskQueue = new CNoticTaskQueue();
    if (0 != m_poNoticTaskQueue->Init(CConfBase::GetInt("notic_task_queue_size")))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init notic_task_queue failed! [queue_size=%u]", \
            CConfBase::GetInt("notic_task_queue_size")));
        return -20;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init notic_task_queue success!"));

    // ��ʼ�����Ͷ��й�����(Ԥ������notictask���ڴ�)
    if (0 != CNoticTaskMgr::GetInstance()->Init(CConfBase::GetInt("notic_task_queue_size"), m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init notic_task_mgr failed!"));
        return -21;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init notic_task_mgr success!"));

    // ��ʼ�����ʹ����߳�
    m_poNoticProcess = new CNoticProcess[CConfBase::GetInt("notic_task_thread_num")];
    for (TUINT32 dwIdx = 0; dwIdx < (TUINT32)CConfBase::GetInt("notic_task_thread_num"); dwIdx++)
    {
        if (0 != m_poNoticProcess[dwIdx].Init(m_poNoticTaskQueue, m_poServLog))
        {
            TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %uth notic_task_process failed!", dwIdx));
            return -22;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init notic_task_process success!"));

    //��ʼ��json_log
    CJsonLog::Init(m_poJsonLog);

    //���������
    CSpecailMap::Update();

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

    // 1. ���������߳�
    for(TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("work_thread_num"); dwIdx++)
    {
        if(pthread_create(&thread_id, NULL, CTaskProcess::Start, &m_poTaskProcess[dwIdx]) != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("create task process thread failed! [idx=%u] Error[%s]",
                dwIdx, strerror(errno)));
            return -1;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("create task_process thread success!"));

    //2.1����zk���ι����߳�
    if (pthread_create(&thread_id, NULL, CDownMgr::zk_StartPull, CDownMgr::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create down manager thread failed! Error[%s]", strerror(errno)));
        return -2;
    }
    if (pthread_create(&thread_id, NULL, CDownMgr::zk_StartCheck, CDownMgr::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create down manager thread failed! Error[%s]", strerror(errno)));
        return -2;
    }

    // 3. ���������߳�-search
    if(pthread_create(&thread_id, NULL, CSearchNetIO::RoutineNetIO, m_poSearchNetIO) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create seach net io thread failed! Error[%s]",
            strerror(errno)));
        return -3;
    }
    TSE_LOG_INFO(m_poServLog, ("create seach net io thread success!"));

    // 4. ����action_get�߳�
    if(pthread_create(&thread_id, NULL, CActionGet::Start, m_poActionPropcess) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create action net io thread failed! Error[%s]",
            strerror(errno)));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("create action net io thread success!"));

    // 1.2 �������ʹ����߳�
    for(TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("notic_task_thread_num"); ++dwIdx)
    {
        if (pthread_create(&thread_id, NULL, CNoticProcess::Start, &m_poNoticProcess[dwIdx]) != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("create notic_task_process thread failed! [idx=%ld] Error[%s]", \
                                        dwIdx, strerror(errno)));
            return -1;
        }
        TSE_LOG_INFO(m_poServLog, ("create notic_task_process thread success!"));
    }

    // 1.3 �����ӳ�MSG���͹����߳�
    for(TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("DelayMsgWorkThreadNum"); ++dwIdx)
    {
        if(pthread_create(&thread_id, NULL, CDelayMsgProcess::Start, &m_poMsgDelayUpdateProcess[dwIdx]) != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("create delay msg task process thread failed! [idx=%u] Error[%s]",
                dwIdx, strerror(errno)));
            return -1;
        }
        TSE_LOG_INFO(m_poServLog, ("create delay_update_process thread success!"));
    }

    // 5. ����ͳ���߳�
    if(pthread_create(&thread_id, NULL, CStatistic::Start, CStatistic::Instance()) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("create statistic thread failed! Error[%s]",
            strerror(errno)));
        return -5;
    }
    TSE_LOG_INFO(m_poServLog, ("create statistic thread success!"));

    TSE_LOG_INFO(m_poServLog, ("GlobalServ start all thread success!"));
    return 0;
}

int CGlobalServ::StopNet()
{
    // ֹͣaction��ѯ�̵߳Ĳ�ѯ
    m_poActionPropcess->StopQuery();

    return 0;
}
