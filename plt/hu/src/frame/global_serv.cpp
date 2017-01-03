#include "global_serv.h"
#include "rc4.h"
#include "statistic.h"
#include "game_info.h"
#include "curl/curl.h"
#include "aws_table_include.h"
#include "game_command.h"
#include "hu_work.h"
#include "wild_info.h"
#include "document.h"
#include "backpack_info.h"
#include "conf_base.h"
#include "alliance_mapping.h"
#include "rating_user.h"
#include "iap_white_list.h"
#include "user_dyeing_info.h"
#include "event_rule_info.h"
#include "delay_msg_task_mgr.h"
#include "warning_mgr.h"
#include "func_open.h"
#include "chest_lottery.h"
#include "lock_info.h"
#include <string>
#include <sstream>
#include "dragon_trail_control.h"
#include "global_gift.h"
#include "output_conf.h"
#include "blog_time.h"

#include "update_notic_task_mgr.h"
#include "update_notic_task.h"
#include "update_notic_process.h"
#include "appsflyer_countries.h"

// 静态变量定义
CTseLogger*	CGlobalServ::m_poServLog		= NULL;
CTseLogger*	CGlobalServ::m_poDayReqLog		= NULL;
CTseLogger* CGlobalServ::m_poRegLog			= NULL;
CTseLogger* CGlobalServ::m_poStatLog		= NULL;
CTseLogger* CGlobalServ::m_poFormatLog		= NULL;
CConf*		CGlobalServ::m_poConf			= NULL;
CTaskQueue* CGlobalServ::m_poTaskQueue		= NULL;
CQueryNetIO*  CGlobalServ::m_poQueryNetIO	= NULL;
CSearchNetIO* CGlobalServ::m_poSearchNetIO	= NULL;
CTaskProcess* CGlobalServ::m_poTaskProcess	= NULL;
CTseLogger* CGlobalServ::m_poJasonLog		= NULL;
CZkRegConf*		CGlobalServ::m_poZkConf     = NULL;
CSessionMgr*    CGlobalServ::m_poSessionMgr = NULL;
CZkRegClient *CGlobalServ::m_poZkRegClient = NULL;

//update_notic add by andy
CUpdateNoticProcess* CGlobalServ::m_UpdateNoticProcessList 	= NULL;		 /// 更新通知线程列表(测试)
CUpdateNoticTaskQueue* CGlobalServ::m_poUpdateNoticQueue 	= NULL; 		//更新通知工作队列
UpdateSvrInfo* CGlobalServ::m_sUpdateSvrInfoList			= NULL;


TUINT32		CGlobalServ::m_udwMailSeq		= 0;
TUINT32		CGlobalServ::m_udwMailSeqStep	= 1;

CDelayMsgProcess* CGlobalServ::m_poMsgDelayUpdateProcess = NULL;

TUINT32		CGlobalServ::m_udwHsReqSeq		= 1;
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
    TbEquip::Init("../tblxml/equip.xml", pszProjectPrefix);
    TbTitle::Init("../tblxml/title.xml", pszProjectPrefix);
    TbEvent_tips::Init("../tblxml/event_tips.xml", pszProjectPrefix);
    TbBounty::Init("../tblxml/bounty.xml", pszProjectPrefix);
    TbBroadcast::Init("../tblxml/broadcast.xml", pszProjectPrefix);
    TbAl_invite_record::Init("../tblxml/al_invite_record.xml", pszProjectPrefix);
    TbSvr_al::Init("../tblxml/svr_al.xml", pszProjectPrefix);
    TbTask::Init("../tblxml/svr_al.xml", pszProjectPrefix);
    TbThrone::Init("../tblxml/throne.xml", pszProjectPrefix);
    TbTask::Init("../tblxml/task.xml", pszProjectPrefix);
    TbReward_window::Init("../tblxml/reward_window.xml", pszProjectPrefix);
    TbRandom_reward::Init("../tblxml/random_reward.xml", pszProjectPrefix);
    TbData_output::Init("../tblxml/data_output.xml", pszProjectPrefix);
    TbIdol::Init("../tblxml/idol.xml", pszProjectPrefix);
    TbLord_image::Init("../tblxml/lord_image.xml", pszProjectPrefix);
    TbDecoration::Init("../tblxml/decoration.xml", pszProjectPrefix);
    TbClear_al_gift::Init("../tblxml/clear_al_gift.xml", pszProjectPrefix);

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
    DEFINE_LOG(format_log, "format_log");
    DEFINE_LOG(jason_log, "jason_log");
	m_poServLog = serv_log;
	m_poDayReqLog = req_log;
	m_poRegLog = reg_log;
	m_poStatLog = stat_log;
    m_poFormatLog = format_log;
    m_poJasonLog = jason_log;

	// 2. 初始化配置文件
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

    //初始化zk相关配置
    m_poZkConf = new CZkRegConf;
    if (0 != m_poZkConf->Init("../conf/serv_info.conf", "../conf/module.conf"))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init zk_conf failed!"));
        return -2;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init zk_conf success!"));

    //初始化aws_tables
    InitAwsTable(CConfBase::GetString("tbxml_project").c_str());
	TSE_LOG_INFO(m_poServLog, ("GlobalServ inittable success!"));

	CGameInfo *poGameInfo = CGameInfo::GetInstance();
	if(0 != poGameInfo->Init("../data/game.json", m_poServLog))
	{
		TSE_LOG_ERROR(m_poServLog, ("GlobalServ init game_info failed!"));
		return -12;
	}
	TSE_LOG_INFO(m_poServLog, ("GlobalServ init game_info success!"));

    CLockInfo *poLockInfo = CLockInfo::GetInstance();
    if (0 != poLockInfo->Init("../data/lock_info.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init lock_info failed!"));
        return -12;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init lock_info success!"));

    // 初始化alliance mapping信息
    CAllianceMapping *poAllianceMapping = CAllianceMapping::GetInstance();
    if(0 != poAllianceMapping->Init("../data/alliance_mapping.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init alliance_mapping failed!"));
        return -15;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init alliance_mapping success!"));

    // 初始化dragon trail control信息
    CDragonTrailControl *poDragonTrailControl = CDragonTrailControl::GetInstance();
    if (0 != poDragonTrailControl->Init("../data/dragon_trail_control.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init dragon_trail_control failed!"));
        return -24;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init dragon_trail_control success!"));

    // 初始化global gift信息
    CGlobalGift *poGlobalGift = CGlobalGift::GetInstance();
    if (0 != poGlobalGift->Init("../data/global_gift.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init global_gift failed!"));
        return -25;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init global_gift success!"));
    
	if(0 != CGameSvrInfo::Update(m_poServLog))
	{
		TSE_LOG_ERROR(m_poServLog, ("GlobalServ init game_svr failed!"));
		return -13;
	}
    // 加载wild res json
    if (0 != CWildInfo::Update(m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init wild res json failed!"));
        return -14;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init wild res json success!"));

    // 初始化文案配置
    CDocument *poDocument = CDocument::GetInstance();
    if(0 != poDocument->Init_All(m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init all document failed!"));
        return -19;
    }
	TSE_LOG_INFO(m_poServLog, ("GlobalServ init document success!"));

    //初始化博客时间戳列表
    CBlogTime *poBlogTime = CBlogTime::GetInstance();
    if (0 != poBlogTime->Init("../data/blog_time.file", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init blog_time failed!"));
        return -16;
    }

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
    m_poSessionMgr = new CSessionMgr();
    if (0 != m_poSessionMgr->Init(CConfBase::GetInt("queue_size"), m_poServLog))
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
    if (0 != CDownMgr::Instance()->zk_Init(m_poRegLog, m_poSearchNetIO->m_pLongConn, m_poZkConf))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init session_mgr failed!"));
        return -7;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init zk_session_mgr success!"));

    m_poZkRegClient = new CZkRegClient;
    if (0 != m_poZkRegClient->Init(m_poRegLog, m_poZkConf))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ Init reg_client_zk failed"));
        return -4;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ Init reg_client_zk succ"));

	// 8. 初始化工作线程
    m_poTaskProcess = new CTaskProcess[CConfBase::GetInt("work_thread_num")];
    for (TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("work_thread_num"); dwIdx++)
	{
		if(0 != m_poTaskProcess[dwIdx].Init(m_poConf, m_poSearchNetIO->m_pLongConn, 
			m_poQueryNetIO->m_pLongConn, m_poTaskQueue, m_poServLog, m_poDayReqLog))
		{
			TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %uth task process failed!", dwIdx));
			return -8;
		}
	}
	TSE_LOG_INFO(m_poServLog, ("GlobalServ init task_process success!"));

    // 延迟msg发送
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
            TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %uth delay msg task process failed!", dwIdx));
            return -10;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init delay_msg_task_process success!"));


	// 9. 初始化rc4算法管理器
    CEncryptCR4Mgr::get_instance();

	// 10. statistic
	CStatistic *pstStatistic = CStatistic::Instance();
	pstStatistic->Init(m_poStatLog, m_poConf);

	TSE_LOG_INFO(m_poServLog, ("GlobalServ init all success!"));

    //10. 初始化curl
    curl_global_init(CURL_GLOBAL_ALL);

    //11 初始化equip.json
    CBackpackInfo *poBackpackInfo = CBackpackInfo::GetInstance();
    if (0 != poBackpackInfo->Init("../data/equip.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("equip.json init equip info failed!"));
        return -11;
    }
    TSE_LOG_INFO(m_poServLog, ("equip.json init equip_info success!"));

    //12 加载Rating_user.json
    CRatingUserInfo *poRatingUserInfo = CRatingUserInfo::GetInstance();
    if (0 != poRatingUserInfo->Init(m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init rating_user failed!"));
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("GlobalServ init rating_user success!"));
    }
    //12 加载iap_white_list.json
    CIAPWhiteList *poIAPWhiteList = CIAPWhiteList::GetInstance();
    if(0 != poIAPWhiteList->Init(m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init aip white list failed!"));
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("GlobalServ init aip white list success!"));
    }
    
    // 加载user dyeing info
    if (0 != CUserDyeingInfo::Update())
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init user dyeing info failed!"));
        return -20;
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("GlobalServ init user dyeing info success!"));
    }

    // 加载event_rule
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

    // 初始化推送队列(作为notictask的缓冲队列)
    m_poNoticTaskQueue = new CNoticTaskQueue();
    if (0 != m_poNoticTaskQueue->Init(CConfBase::GetInt("notic_task_queue_size")))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init notic_task_queue failed! [queue_size=%u]", \
                                    CConfBase::GetInt("notic_task_queue_size")));
        return -20;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init notic_task_queue success!"));

    // 初始化推送队列管理器(预先申请notictask的内存)
    if (0 != CNoticTaskMgr::GetInstance()->Init(CConfBase::GetInt("notic_task_queue_size"), m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init notic_task_mgr failed!"));
        return -21;
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init notic_task_mgr success!"));

    // 初始化推送处理线程
    m_poNoticProcess = new CNoticProcess[CConfBase::GetInt("notic_task_thread_num")];
    for (TINT32 dwIdx = 0; dwIdx < CConfBase::GetInt("notic_task_thread_num"); dwIdx++)
    {
        if (0 != m_poNoticProcess[dwIdx].Init(m_poNoticTaskQueue, m_poServLog))
        {
            TSE_LOG_ERROR(m_poServLog, ("GlobalServ init %uth notic_task_process failed!", dwIdx));
            return -22;
        }
    }
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init notic_task_process success!"));

    // 初始化chest_lottery.json
    CChestLottery *poChestLottery = CChestLottery::GetInstance();
    if (0 != poChestLottery->Init("../data/chest_lottery.json", m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("chest_lottery.json init chest lottery info failed!"));
        return -23;
    }
    TSE_LOG_INFO(m_poServLog, ("chest_lottery.json init chest_lottery info success!"));


    // ************************************************************************************* //
    // 初始化告警上报
    CWarningMgr::GetInstance()->Init(m_poServLog);
    TSE_LOG_INFO(m_poServLog, ("GlobalServ init warning mgr success!"));

    // 加载func open
    CFuncOpen *poFuncOpen = CFuncOpen::GetInstance();
    if(0 != poFuncOpen->Init("../data/function_switch.json",m_poServLog))
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init func open failed!"));
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("GlobalServ init func open success!"));
    }

    // 加载appsflyer_countries
    if (0 != CAppsflyerCountries::Update())
    {
        TSE_LOG_ERROR(m_poServLog, ("GlobalServ init appsflyers failed!"));
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("GlobalServ init appsflyers success!"));
    }

    TSE_LOG_INFO(m_poServLog, ("GlobalServ init all success!"));

/* 
	send update_notic add by andy 20160429	 Begin*******************************************
*/

	////update_notic_mgr
	//if( S_OK != CUpdateNoticTaskMgr::GetInstance()->Init( CConfBase::GetInt("update_svr_task_queue_size"), m_poServLog))
	//{
	//	TSE_LOG_ERROR(m_poServLog, ("GlobalServ Init update_notic_mgr failed"));
	//	return S_FAIL;
	//}
	//TSE_LOG_INFO( m_poServLog, ( "GlobalServ Init update_notic_mgr succ" ) );

	////update_notic_work_queue
	//m_poUpdateNoticQueue = new CUpdateNoticTaskQueue;
	//if (0 != m_poUpdateNoticQueue->Init(CConfBase::GetInt("update_svr_task_queue_size")))
	//{
	//	TSE_LOG_ERROR(m_poServLog, ("GlobalServ Init update_notic_work_queue failed with queue_size=%d", CConfBase::GetInt("update_notic_task_queue_size")));
	//	return S_FAIL;
	//}
	//TSE_LOG_INFO(m_poServLog, ("GlobalServ Init update_notic_work_queue succ"));

	////update_notic_work_process
	//m_UpdateNoticProcessList = new CUpdateNoticProcess[CConfBase::GetInt("update_svr_task_thread_num")];
	//TINT32 dwIpNum = CConfBase::GetInt("num", "update_svr");
	//TSE_LOG_INFO(m_poServLog, ("GlobalServ Init update_notic_work_process update_notic_server_num [num=%d]", dwIpNum));
	//m_sUpdateSvrInfoList = new UpdateSvrInfo[ dwIpNum ];
	//std::stringstream ss;
	//std::string strTmp;
	//for(int i=0; i < dwIpNum; i++)
	//{
	//	ss << i;
	//	ss >> strTmp;
 //       string strIP = CConfBase::GetString("ip_" + strTmp, "update_svr");
 //       TINT32 dwPort = CConfBase::GetInt("port_" + strTmp, "update_svr");
	//	TSE_LOG_INFO(m_poServLog, ("GlobalServ Init update_notic_work_process get [update_notic_sever_%d=%s] [update_notic_sever_port_%d=%d]", \
	//					i, strIP.c_str(), i, dwPort));
	//	m_sUpdateSvrInfoList[i].m_strIp = strIP;
	//	m_sUpdateSvrInfoList[i].m_uwPort= dwPort;
	//}

	//for( TINT32 dwJ = 0; dwJ < CConfBase::GetInt("update_svr_task_thread_num"); ++dwJ	)
	//{
	//	if( S_OK != m_UpdateNoticProcessList[dwJ].Init( m_poUpdateNoticQueue, m_poServLog, dwIpNum, m_sUpdateSvrInfoList))
	//	{
	//		TSE_LOG_ERROR(m_poServLog, ("GlobalServ Init update_notic_work_process failed"));
	//		return S_FAIL;
	//	}
	//	break;
	//}
	//TSE_LOG_INFO(m_poServLog, ("GlobalServ Init update_notic_work_process succ"));

/* 
	send update_notic add by andy 20160429	 End**********************************************
*/	

    // 12
    COutputConf *pOutputConf = COutputConf::GetInstace();
    assert(pOutputConf);

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
    
	// 1. 创建工作线程
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

    //2.1创建zk下游管理线程
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

	// 3. 创建网络线程-search
	if (pthread_create(&thread_id, NULL, CSearchNetIO::RoutineNetIO, m_poSearchNetIO) != 0)
	{
		TSE_LOG_ERROR(m_poServLog, ("create seach net io thread failed! Error[%s]",
			strerror(errno)));
		return -3;
	}
	TSE_LOG_INFO(m_poServLog, ("create net io thread success!"));

	// 4. 创建网络线程-query
	if (pthread_create(&thread_id, NULL, CQueryNetIO::RoutineNetIO, m_poQueryNetIO) != 0)
	{
		TSE_LOG_ERROR(m_poServLog, ("create query net io thread failed! Error[%s]",
			strerror(errno)));
		return -4;
	}
	TSE_LOG_INFO(m_poServLog, ("create net io thread success!"));


    // 1.2 创建推送处理线程
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

    // 1.3 创建延迟MSG发送工作线程
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

	// 5. 开启统计线程
	if (pthread_create(&thread_id, NULL, CStatistic::Start, CStatistic::Instance()) != 0) 
	{
		TSE_LOG_ERROR(m_poServLog, ("create statistic thread failed! Error[%s]",
			strerror(errno)));
		return -5;
	}
	TSE_LOG_INFO(m_poServLog, ("create statistic thread success!"));


/* 
	send update_notic add by andy 20160429	 Begin*******************************************
*/
	
	//for( TINT32 dwj =0; dwj < CConfBase::GetInt("update_svr_task_thread_num"); ++dwj )
	//{
	//	if( 0 != pthread_create( &thread_id, NULL, CUpdateNoticProcess::Start, &( m_UpdateNoticProcessList[dwj])) )
	//	{
	//		TSE_LOG_ERROR( m_poServLog, ( "Create update_notic thread failed, idex[%d], err msg[%s]", dwj, strerror(errno) ) );
	//		return S_FAIL;
	//	}
	//}
	//TSE_LOG_INFO(m_poServLog, ("create update_notic thread success! [thread_num=%d]", CConfBase::GetInt("update_notic_task_thread_num")));
/* 
	send update_notic add by andy 20160429	 End*****************************************
*/

    // 启动注册线程
    if (CConfBase::GetInt("tcp_mode"))
    {
        m_poZkRegClient->Start();
    }
    
	TSE_LOG_INFO(m_poServLog, ("GlobalServ start all thread success!"));
	return 0;
}

int CGlobalServ::StopNet()
{
    m_poZkRegClient->Stop();

	// 停止query线程的接收请求端口
	m_poQueryNetIO->CloseListenSocket();

    curl_global_cleanup();
	return 0;
}

