#include "global_serv.h"
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include "svr_list_conf.h"
#include "game_info.h"
#include "wild_info.h"
#include "document.h"
#include "backpack_info.h"
#include "conf_base.h"
#include "game_svr.h"
#include "user_dyeing_info.h"
#include "event_rule_info.h"
#include "func_open.h"

// 0: ��ʼ״̬
// -1: stop
int g_flag = 0;
void signal_kill(int signal)
{
    g_flag = -1;
}

int main(int argc, char** argv)
{
    g_flag = 0;
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR1, signal_kill);

    int ret_flag = 0;

    // ��ʼ��
    ret_flag = CGlobalServ::Init();
    assert(ret_flag == 0);

    if(argc > 1)
    {
        CGlobalServ::m_udwReportId = strtoul(argv[1], NULL, 10);
    }
    else
    {
        CGlobalServ::m_udwReportId = 0;
    }

    // �����߳�
    // note:���κ�ע��Ϊ0������ͨ��������κŽ�������ѡȡ��ֻ�������ݸ��£����ṩ��ѯ����
    ret_flag = CGlobalServ::Start();
    assert(ret_flag == 0);

    // �����ź����
    while(1)
    {
        // �յ��ź�����,����һֱ�ȴ�
        if(g_flag == -1)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Receive stop signal!"));
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Stop new req, wait to exit ..."));

            //�յ��ź�֮��ֹͣ����, ��ʱ���˳�
            CGlobalServ::StopNet();
            sleep(5);

            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("Stop success!"));
            break;
        }

        // ����svr_list_conf����
        if(access(UPDATE_SVR_LIST_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update action_table conf"));
            if(CActionTableConf::Update("../conf/action_table.conf", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_SVR_LIST_FLAG_FILE);
            }
        }

        // ������Ϸ�������
        if(access(UPDATE_DATA_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update game_svr conf"));
            if(CGameSvrInfo::Update(CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_DATA_FLAG_FILE);
            }
        }

        //����game.json����
        if(access(UPDATE_GAME_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update game conf"));
            if(CGameInfo::Update("../data/game.json", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_GAME_JSON_FLAG_FILE);
            }
        }

        //����wild res json
        if(access(UPDATE_WILD_RES_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update wild res json"));
            if(CWildInfo::Update(CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_WILD_RES_JSON_FLAG_FILE);
            }
        }

        //����equip.json����
        if(access(UPDATE_EQUIP_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update equip conf"));
            if(CBackpackInfo::Update("../data/equip.json", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_EQUIP_JSON_FLAG_FILE);
            }
        }
        //����function_switch.json����
        if(access(UPDATE_FUNC_OPEN_JSON_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update func open conf"));
            if(CFuncOpen::Update("../data/function_switch.json", CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_FUNC_OPEN_JSON_FLAG_FILE);
            }
        }

        // ======================================================================================
        // �����İ�document
        // 1. ����
        if(access(UPDATE_DOCUMENT_ALL_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update all document"));
            if(CDocument::Update_All(CGlobalServ::m_poServLog) == 0)
            {
                remove(UPDATE_DOCUMENT_ALL_FLAG_FILE);
            }
        }

        // 2. ����
        /*
        if(access(UPDATE_DOCUMENT_ENGLISH_FLAG_FILE, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update english document"));
            if(CDocument::Update(CGlobalServ::m_poServLog, DOCUMENT_ENGLISH_FILE) == 0)
            {
                remove(UPDATE_DOCUMENT_ENGLISH_FLAG_FILE);
            }
        }
        */

        //����project.conf����
        if(access(DEFAULT_CONF_UPDATE_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_poServLog, ("beg to update module conf"));
            if(CConfBase::Update() == 0)
            {
                remove(DEFAULT_CONF_UPDATE_FLAG);
            }
            else
            {
                TSE_LOG_INFO(CGlobalServ::m_poServLog, ("update module.conf fail!"));
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

        //����user dyeing info
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

        //����event rule info
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
        sleep(1);
    }

    return 0;
}

