#include "global_serv.h"
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "game_info.h"
#include "wild_info.h"
#include "document.h"
#include "backpack_info.h"
#include "conf_base.h"
#include "alliance_mapping.h"
#include "rating_user.h"
#include "iap_white_list.h"
#include "event_rule_info.h"
#include "user_dyeing_info.h"
#include "func_open.h"
#include "chest_lottery.h"
#include "lock_info.h"
#include "dragon_trail_control.h"
#include "global_gift.h"
#include "appsflyer_countries.h"
#include "blog_time.h"

// 0: 初始状态
// -1: stop
int g_flag = 0;

void sys_sig_kill(int signal)
{
	g_flag = -1;
}

void signal_kill(int signal)
{
	g_flag = -1;
	printf("recv kill signal[%d]\n", signal);
}

// 定义用户信号
TVOID InitSignal(){
	struct sigaction sa;
	sigset_t sset;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sys_sig_kill;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);

	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);

	sigemptyset(&sset);
	sigaddset(&sset, SIGSEGV);
	sigaddset(&sset, SIGBUS);
	sigaddset(&sset, SIGABRT);
	sigaddset(&sset, SIGILL);
	sigaddset(&sset, SIGCHLD);
	sigaddset(&sset, SIGFPE);
	sigprocmask(SIG_UNBLOCK, &sset, &sset);

	signal(SIGUSR1, signal_kill);
}


int main(int argc,char** argv)
{
	// 初始化信号量
	g_flag = 0;
	InitSignal();

	int ret_flag = 0;

	// 初始化
	ret_flag = CGlobalServ::Init();
	assert(ret_flag == 0);

	// 启动线程
	// note:批次号注册为0，上游通过这个批次号进行下游选取，只进行数据更新，不提供查询服务，
	ret_flag = CGlobalServ::Start();
	assert(ret_flag == 0);

	// 处理信号相关
	while (1)
	{
		// 收到信号则处理,否则一直等待
		if (g_flag == -1)
		{
			TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Receive stop signal!"));
			TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Stop new req, wait to exit ..."));

			//收到信号之后停止服务, 延时后退出
			CGlobalServ::StopNet();
			sleep(2);

			TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Stop success!"));
			break;
		}

		// 更新游戏相关配置
		if(access(UPDATE_DATA_FLAG_FILE, F_OK) == 0)
		{
			TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update game_svr conf"));
			if(CGameSvrInfo::Update(CGlobalServ::m_poServLog) == 0)
			{
				remove(UPDATE_DATA_FLAG_FILE);
			}
		}

        //更新game.json配置
        if (access(UPDATE_GAME_JSON_FLAG_FILE, F_OK) == 0 || access(UPDATE_SUB_GAME_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update game conf"));
            if (CGameInfo::Update("../data/game.json", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_GAME_JSON_FLAG_FILE);
                remove(UPDATE_SUB_GAME_JSON_FLAG_FILE);
            }
        }

        //更新lock_info.json
        if (access(UPDATE_LOCK_INFO_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update game conf"));
            if (CLockInfo::Update("../data/lock_info.json", CGlobalServ::m_poServLog) == 0)
            {
                TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Read lock_info.json succeed."));
                remove(UPDATE_LOCK_INFO_FLAG_FILE);
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("Read lock_info.json failed!!"));
            }
        }

        //更新alliance_mapping.json
        if(access(UPDATE_ALLIANCE_MAPPING_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update alliance mapping conf"));
            if(CAllianceMapping::Update("../data/alliance_mapping.json", CGlobalServ::m_poServLog) == 0)
            {
                TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Read alliance_mapping.json succeed."));
                remove(UPDATE_ALLIANCE_MAPPING_FLAG_FILE);
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("Read alliance_mapping.json failed!!"));
            }
        }

        //更新dragon_trail_control.json
        if (access(UPDATE_DRAGON_TRAIL_CONTROL_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update dragon trail control conf"));
            if (CDragonTrailControl::Update("../data/dragon_trail_control.json", CGlobalServ::m_poServLog) == 0)
            {
                TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Read dragon_trail_control.json succeed."));
                remove(UPDATE_DRAGON_TRAIL_CONTROL_FLAG_FILE);
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("Read dragon_trail_control.json failed!!"));
            }
        }

        //更新global_gift.json
        if (access(UPDATE_GLOBAL_GIFT_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update global gift control conf"));
            if (CGlobalGift::Update("../data/global_gift.json", CGlobalServ::m_poServLog) == 0)
            {
                TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Read global_gift.json succeed."));
                remove(UPDATE_GLOBAL_GIFT_FLAG_FILE);
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("Read global_gift.json failed!!"));
            }
        }

        //更新wild res json
        if (access(UPDATE_WILD_RES_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update wild res json"));
            if (CWildInfo::Update(CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_WILD_RES_JSON_FLAG_FILE);
            }
        }

        //更新黑名单列表
        if(access(UPDATE_BLACK_ACCOUNT_LIST_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update black list"));
            if(CGameSvrInfo::GetInstance()->LoadBlackAccountList(BLACK_ACCOUNT_LIST_FILE) == 0)
            {
                remove(UPDATE_BLACK_ACCOUNT_LIST_FLAG);
            }
        }

        //更新equip.json配置
        if (access(UPDATE_EQUIP_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update equip conf"));
            if (CBackpackInfo::Update("../data/equip.json", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_EQUIP_JSON_FLAG_FILE);
            }
        }
        
        /*
        //更新chest_lottery.json配置
        if (access(UPDATE_CHEST_LOTTERY_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update chest lottery conf"));
            if (CChestLottery::Update("../data/chest_lottery.json", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_CHEST_LOTTERY_JSON_FLAG_FILE);
            }
        }
        */

        //更新func open配置
        if(access(UPDATE_FUNC_OPEN_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update fucn open conf"));
            if(CFuncOpen::Update("../data/function_switch.json", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_FUNC_OPEN_JSON_FLAG_FILE);
            }
        }

        if (access(UPDATE_COUNTRIES_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update appsflyers conf"));
            if (CAppsflyerCountries::Update() == 0)
            {
                remove(UPDATE_COUNTRIES_FLAG);
            }
        }
        // ======================================================================================
        // 更新文案document
        // 1. 所有
        if (access(UPDATE_DOCUMENT_ALL_FLAG_FILE, F_OK) == 0 || access(UPDATE_SUB_DOCUMENT_ALL_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update all document"));
            if (CDocument::Update_All(CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_DOCUMENT_ALL_FLAG_FILE);
                remove(UPDATE_SUB_DOCUMENT_ALL_FLAG_FILE);
            }
        }

        // 2. 单个
        /*
        if(access(UPDATE_DOCUMENT_ENGLISH_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update english document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_ENGLISH_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_ENGLISH_FLAG_FILE);
            }
        }        
        */

        //更新project.conf配置
        if (access(DEFAULT_CONF_UPDATE_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update project conf"));
            if (CConfBase::Update() == 0)
            {
                remove(DEFAULT_CONF_UPDATE_FLAG);
            }
        }

        /*
        if(access(UPDATE_DOCUMENT_ARABIC_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update arabic document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_ARABIC_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_ARABIC_FLAG_FILE);
            }
        }
        if(access(UPDATE_DOCUMENT_CHINESE_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update chinese document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_CHINESE_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_CHINESE_FLAG_FILE);
            }
        }        
        if(access(UPDATE_DOCUMENT_FRENCH_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update french document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_FRENCH_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_FRENCH_FLAG_FILE);
            }
        }
        if(access(UPDATE_DOCUMENT_GERMAN_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update german document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_GERMAN_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_GERMAN_FLAG_FILE);
            }
        }        
        if(access(UPDATE_DOCUMENT_PORTUGAL_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update portugal document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_PORTUGAL_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_PORTUGAL_FLAG_FILE);
            }
        }
        if(access(UPDATE_DOCUMENT_RUSSIAN_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update russian document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_RUSSIAN_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_RUSSIAN_FLAG_FILE);
            }
        }        
        if(access(UPDATE_DOCUMENT_SPAIN_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update spain document"));
            if (CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_SPAIN_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_SPAIN_FLAG_FILE);
            }
        }
        */
        // ======================================================================================

        //更新rating_user@wave:20151209
        if (access(UPDATE_RATING_USER_INFO_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update rating user"));
            if (CRatingUserInfo::Update(CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_RATING_USER_INFO_FLAG_FILE);
            }
        }
        //更新iap_white_list.json
        if(access(UPDATE_IAP_WHITE_LIST_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update iap white list"));
            if(CIAPWhiteList::Update(CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_IAP_WHITE_LIST_FLAG_FILE);
            }
        }

        //更新user dyeing info
        if (access(UPDATE_USER_DYEING_INFO_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update user dyeing info"));
            if (CUserDyeingInfo::Update() == 0)
            {
                remove(UPDATE_USER_DYEING_INFO_FLAG);
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("update user dyeing info failed"));
            }
        }

        //更新event rule info
        if (access(UPDATE_EVENT_RULE_INFO_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update event rule info"));
            if (CEventRuleInfo::Update() == 0)
            {
                remove(UPDATE_EVENT_RULE_INFO_FLAG);
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("update event rule info failed"));
            }
        }
        //更新blog_time配置
        if (access(UPDATE_BLOG_TIME_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update blog time"));
            if (CBlogTime::Update("../data/blog_time.file", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_BLOG_TIME_FLAG_FILE);
            }
        }

		sleep(1);
	}

	return 0;
}

