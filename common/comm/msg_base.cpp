#include "msg_base.h"
#include "game_info.h"
#include "time_utils.h"
#include "common_func.h"
#include "document.h"
#include "http_utils.h"
#include "notic_task_mgr.h"
#include "delay_msg_task_mgr.h"
#include "notic_process.h"

TINT32 CMsgBase::SendDelaySystemMsg(const TCHAR *pszMsg)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    CDelayMsgTaskQueue *pCommQueue = CDelayMsgTaskMgr::Instance()->GetCommQueue();

    if(NULL == pszMsg || NULL == pCommQueue)
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("SendDelaySystemMsg: param err! "));
        return -1;
    }

    TSE_LOG_DEBUG(poGameInfo->m_poLog, ("SendDelaySystemMsg: [pszMsg=%s]", pszMsg));

    SDelayMsgTask *pTask = NULL;
    if(CDelayMsgTaskMgr::Instance()->WaitTillDelayTask(&pTask) != 0)
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("Get new delay msg failed! "));
        return -2;
    }

    pTask->Reset();
    snprintf(pTask->m_szMsg, MAX_LEN_DELAY_MSG, "%s", pszMsg);

    pCommQueue->WaitTillPush(pTask);

    return 0;
}

TVOID CMsgBase::SendHelpMail(TINT64 ddwTargetUid, TINT32 dwDocId, TUINT32 udwSid, const TCHAR *szContent)
{
    const TINT32 k_tmp_len = 1024;
    TCHAR szScriptStr[k_tmp_len];
    const TCHAR *pszTitle = "Customer Service";

    sprintf(szScriptStr, "./send_operate_mail.sh \"%u\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\"", udwSid, EN_MAIL_SEND_TYPE_SUPPORT_TO_PLAYER, ddwTargetUid, pszTitle, dwDocId, szContent);
    CMsgBase::SendDelaySystemMsg(szScriptStr);
}

TVOID CMsgBase::SendOperateMail(TINT64 ddwTargetUid, TINT32 dwDocId, TUINT32 udwSid, const TCHAR *szContent)
{
    const TINT32 k_tmp_len = 1024;
    TCHAR szScriptStr[k_tmp_len];
    const TCHAR *pszTitle = "System";

//#"$1"  sid
//#"$2"  send type
//#"$3"  tid
//#"$4"  title
//#"$5"  docid
//#"$6"  content
//#"$7"  extra content
//#"$8"  displayclass sender
//#"$9"  reward
//#详细见接口文档


    sprintf(szScriptStr, "./send_operate_mail.sh \"%u\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\"", udwSid, EN_MAIL_SEND_TYPE_SYSTEM_TO_PLAYERS, ddwTargetUid, pszTitle, dwDocId, szContent);
    CMsgBase::SendDelaySystemMsg(szScriptStr);
}

TVOID CMsgBase::SendOperateMail(TINT64 ddwTargetUid, TINT32 dwDocId, TUINT32 udwSid, TINT32 dwDisplayClass, const TCHAR *szContent, const TCHAR* szExtraContent, const TCHAR* szReward)
{
    const TINT32 k_tmp_len = 2048;
    TCHAR szScriptStr[k_tmp_len];
    const TCHAR *pszTitle = "System";

    //#"$1"  sid
    //#"$2"  send type
    //#"$3"  tid
    //#"$4"  title
    //#"$5"  docid
    //#"$6"  content
    //#"$7"  extra content
    //#"$8"  displayclass sender
    //#"$9"  reward
    //#详细见接口文档
    TCHAR szJsonBuff[4096];
    CHttpUtils::url_encode(szExtraContent, szJsonBuff, 4096);
    szJsonBuff[4096] = '\0';
    sprintf(szScriptStr, "./send_operate_mail.sh \"%u\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\" \"%s\" \"%d\" \"%s\"",
        udwSid, EN_MAIL_SEND_TYPE_SYSTEM_TO_PLAYERS, ddwTargetUid, pszTitle, dwDocId, szContent,
        szJsonBuff, dwDisplayClass, szReward);
    szScriptStr[k_tmp_len - 1] = '\0';
    CMsgBase::SendDelaySystemMsg(szScriptStr);
}

TVOID CMsgBase::SendOperateAlMail(TINT64 ddwTargetAid, TINT32 dwDocId, TUINT32 udwSid, const TCHAR *szContent)
{
    const TINT32 k_tmp_len = 1024;
    TCHAR szScriptStr[k_tmp_len];
    const TCHAR *pszTitle = "System";

    //#"$1"  sid
    //#"$2"  send type
    //#"$3"  tid
    //#"$4"  title
    //#"$5"  docid
    //#"$6"  content
    //#"$7"  extra content
    //#"$8"  displayclass sender
    //#"$9"  reward
    //#详细见接口文档

    sprintf(szScriptStr, "./send_operate_mail.sh \"%u\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\"", udwSid, EN_MAIL_SEND_TYPE_SYSTEM_TO_ALLIANCE, ddwTargetAid, pszTitle, dwDocId, szContent);
    CMsgBase::SendDelaySystemMsg(szScriptStr);
}

TVOID CMsgBase::SendOperateAlMail(TINT64 ddwTargetAid, TINT32 dwDocId, TUINT32 udwSid, TINT32 dwDisplayClass, const TCHAR *szContent, const TCHAR* szExtraContent, const TCHAR* szReward)
{
    const TINT32 k_tmp_len = 1024;
    TCHAR szScriptStr[k_tmp_len];
    const TCHAR *pszTitle = "System";

    //#"$1"  sid
    //#"$2"  send type
    //#"$3"  tid
    //#"$4"  title
    //#"$5"  docid
    //#"$6"  content
    //#"$7"  extra content
    //#"$8"  displayclass sender
    //#"$9"  reward
    //#详细见接口文档


    TCHAR szJsonBuff[4096];
    CHttpUtils::url_encode(szExtraContent, szJsonBuff, 4096);
    sprintf(szScriptStr, "./send_operate_mail.sh \"%u\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\" \"%s\" \"%d\" \"%s\"",
        udwSid, EN_MAIL_SEND_TYPE_SYSTEM_TO_ALLIANCE, ddwTargetAid, pszTitle, dwDocId, szContent,
        szJsonBuff, dwDisplayClass, szReward);
    CMsgBase::SendDelaySystemMsg(szScriptStr);
}

TINT32 CMsgBase::SendEncourageMail(TbUser_stat *ptbStat, TINT64 ddwSid, TUINT32 udwMailDocId, TUINT32 udwSPos /*= 0*/, string strContent /*= ""*/)
{
    const TINT32 k_tmp_len = 1024;
    TCHAR szScriptStr[k_tmp_len];
    const TCHAR *pszSUname = "Blaze of Battle Stuido";

    if (ptbStat->m_nUid == 0)
    {
        return 0;
    }

    //判断是否已经发送过邮件
    if(BITTEST(ptbStat->m_bMail_flag[0].m_bitFlag, udwMailDocId))
    {
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendEncourageMail::mail has been send [uid=%ld][mail_doc_id=%u].", ptbStat->m_nUid, udwMailDocId));
        return 0;
    }

    //如果是第一次创建联盟，则第一次加入联盟也完成
    if(EN_MAIL_ID__FIRST_CREATE_ALLIANCE == udwMailDocId)
    {
        BITSET(ptbStat->m_bMail_flag[0].m_bitFlag ,EN_MAIL_ID__FIRST_JOIN_ALLIANCE);
    }

    BITSET(ptbStat->m_bMail_flag[0].m_bitFlag, udwMailDocId);
    ptbStat->SetFlag(TbUSER_STAT_FIELD_MAIL_FLAG);

    // send mail
    sprintf(szScriptStr, "./send_operate_mail.sh \"%ld\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\"", ddwSid, EN_MAIL_SEND_TYPE_SYSTEM_TO_PLAYERS, ptbStat->m_nUid, pszSUname, udwMailDocId, strContent.c_str());

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendEncourageMail::cmd=%s", szScriptStr));

    CMsgBase::SendDelaySystemMsg(szScriptStr);

    return 0;
}

TVOID CMsgBase::SendNotificationPerson(string strProject, TUINT32 udwSid, TINT64 ddwTargetUid, SNoticInfo &rstNoticInfo)
{
    if(0 == ddwTargetUid)
    {
        return;
    }

    //wave@20160425: 屏蔽历史上多余的notification——产品需求
    if(rstNoticInfo.m_dwNoticId >= EN_NOTI_ID__END)
    {
        return;
    }

    //wave@20160929: 暂时统一为英文
    rstNoticInfo.m_strLang = "english";

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendNotificationPerson: [lang=%s]", \
                                                      rstNoticInfo.m_strLang.c_str()));
    CDocument *pDocument = CDocument::GetInstance();
    
    TBOOL bIsExistDocument = pDocument->IsSupportLang(rstNoticInfo.m_strLang);
    if(FALSE == bIsExistDocument)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendNotificationPerson: do not have this language document"));
        return;
    }

    const Json::Value &stDocumentJson = pDocument->GetDocumentJsonByLang(rstNoticInfo.m_strLang);

    TINT32 dwRet = 0;
    string strNoticContent = "";
    strNoticContent = GetNoticDocumentByNoticId(rstNoticInfo.m_dwNoticId, stDocumentJson);

    string szName = "";
    switch(rstNoticInfo.m_dwNoticId)
    {
    case EN_NOTI_ID__AL_GIFT:
        break;
    case EN_NOTI_ID__MAIL:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        break;
    case EN_NOTI_ID__TRAIN_SUCC:
    case EN_NOTI_ID__HEAL_SUCC:
    case EN_NOTI_ID__REVIVE_SUCC:
    case EN_NOTI_ID__FORT_TRAIN_SUCC:
    case EN_NOTI_ID__FORT_REPAIR_SUCC:
        for (TUINT32 udwIdx = 0; udwIdx < stDocumentJson[rstNoticInfo.m_strKeyName].size(); ++udwIdx)
        {
            if (stDocumentJson[rstNoticInfo.m_strKeyName][udwIdx]["id"].asString() == rstNoticInfo.m_strValueName)
            {
                szName = stDocumentJson[rstNoticInfo.m_strKeyName][udwIdx]["name"].asString();
                break;
            }
        }
        if (szName == "")
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendNotificationPerson: [doc_size=%d][doc_str=%s][doc_key=%s]", \
                stDocumentJson[rstNoticInfo.m_strKeyName].size(),
                rstNoticInfo.m_strKeyName.c_str(),
                rstNoticInfo.m_strValueName.c_str()
                ));

            dwRet = -1;
        }
        StringReplace(strNoticContent, "STRING0", szName);
        break;
    case EN_NOTI_ID__BUILDING_CAN_FREE:
    case EN_NOTI_ID__RESEARCH_CAN_FREE:
    case EN_NOTI_ID__REMOVE_CAN_FREE:
    case EN_NOTI_ID__BUILDING_UPGRADE_OK:
    case EN_NOTI_ID__BUILDING_REMOVE_OK:
    case EN_NOTI_ID__RESEARCH_UPGRADE_OK:
    case EN_NOTI_ID__FORGE_EQUIP_SUCC:
    case EN_NOTI_ID__HERO_ATTACK_SUCC:
    case EN_NOTI_ID__FORGE_CAN_FREE:
    case EN_NOTI_ID__HERO_ATTACK_SUCC_AND_AL_GIFT:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strValueName);
        StringReplace(strNoticContent, "STRING1", rstNoticInfo.m_strKeyName);
        break;
    case EN_NOTI_ID__QUEST_COMPLETE:
    case EN_NOTI_ID__MYSTERY_GIFT:
    case EN_NOTI_ID__NEW_AGE:
    case EN_NOTI_ID__BONUTY_FINISH:
        break;
    case EN_NOTI_ID__TRANSPORT_SUCC:
    case EN_NOTI_ID__REINFORCE_SUCC:
    case EN_NOTI_ID__ATTACK_SUCC:
    case EN_NOTI_ID__ATTACK_FAIL:
    case EN_NOTI_ID__SCOUT_SUCC:
    case EN_NOTI_ID__RALLY_ATTACK_SUCC:
    case EN_NOTI_ID__RALLY_ATTACK_FAIL:
    case EN_NOTI_ID__RALLY_DEFENSE_SUCC:
    case EN_NOTI_ID__RALLY_DEFENSE_FAIL:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strTName);
        break;
    case EN_NOTI_ID__BE_TRANSPORT_SUCC:
    case EN_NOTI_ID__BE_REINFORCE_SUCC:
    case EN_NOTI_ID__DEFENSE_FAIL:
    case EN_NOTI_ID__DEFENSE_SUCC:
    case EN_NOTI_ID__BE_SCOUT:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        break;
    case EN_NOTI_ID__COMING_ATTACK_7:
    case EN_NOTI_ID__OCCUPY_COMING_ATTACK_7:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        StringReplace(strNoticContent, "STRING1", rstNoticInfo.m_strCostTime);
        break;
    case EN_NOTI_ID__LOAD_SUCC:
        break;
    case EN_NOTI_ID__HERO_BE_RELEASED:
        break;
    case EN_NOTI_ID__HERO_BE_KILLED:
    case EN_NOTI_ID__HERO_BE_CAPTURED:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        break;
    case EN_NOTI_ID__HAVE_TRADE_COMING:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        break;
    case EN_NOTI_ID__AL_INVITE:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        break;
    case EN_NOTI_ID__REINFORCE_THRONE_SUCC:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        break;
    case EN_NOTI_ID__EVENT_REWARD:
        break;
    case EN_NOTI_ID__COLLECT_TAX:
    case EN_NOTI_ID__PAY_TAX:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strValueName);
        break;
    case EN_NOTI_ID__HERO_RETURN:
    case EN_NOTI_ID__TROOP_RETURN:
        break;
    case EN_NOTI_ID__PEACETIME_END:
    case EN_NOTI_ID__NEW_PLAYER_END:
    case EN_NOTI_ID__VIP_EXPIRED:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strCostTime);
        break;
    case EN_NOTI_ID__BUFF_EXPIRED:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strKeyName);
        StringReplace(strNoticContent, "STRING1", rstNoticInfo.m_strCostTime);
        break;
    case EN_NOTI_ID__HERO_UNLOCKED:
    case EN_NOTI_ID__HERO_ENERGY_RECOVERED:
    case EN_NOTI_ID__KNIGHT_BE_DISMISSED:
    case EN_NOTI_ID__AL_REQUEST:
        break;
    case EN_NOTI_ID__CAMP_PROTECT_EXPIRE:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strTPosX);
        StringReplace(strNoticContent, "STRING1", rstNoticInfo.m_strTPosY);
        break;
    case EN_NOTI_ID__SCOUT_BE_PREVENTED:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strTName);
        StringReplace(strNoticContent, "STRING1", rstNoticInfo.m_strTPosX);
        StringReplace(strNoticContent, "STRING2", rstNoticInfo.m_strTPosY);
        break;
    case EN_NOTI_ID__PREVENT_SCOUT:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        break;
    default:
        dwRet = -1;
        break;
    }

    if(dwRet < 0)
    {
        return;
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendNotificationPerson: [strNoticContent=%s] [ddwTargetUid=%ld]", \
        strNoticContent.c_str(), \
        ddwTargetUid));

    ostringstream oss;
    oss.str("");
    oss << udwSid;
    string strSid = oss.str();
    oss.str("");
    oss << ddwTargetUid;
    string strUid = oss.str();
 
    if(0 < CNoticTaskMgr::GetInstance()->GetEmptySessionSize())
    {
        SNoticTask *pNoticTask = NULL;
        CNoticTaskMgr::GetInstance()->WaitTillNoticTask(&pNoticTask);

        pNoticTask->m_strTitle = "";
        pNoticTask->m_ddwUid = ddwTargetUid;
        pNoticTask->m_ddwAid = 0;
        pNoticTask->m_ddwSid = (TINT64)udwSid;
        pNoticTask->m_ddwNotiType = CMsgBase::GetNotiType(rstNoticInfo.m_dwNoticId, stDocumentJson);
        pNoticTask->m_strContent = strNoticContent;
        pNoticTask->m_strProject = strProject;
        pNoticTask->m_ddwNoticFlag = 0;
        pNoticTask->m_strSound = CMsgBase::GetNotiSound(pNoticTask->m_ddwNotiType, stDocumentJson);

            
        if(NULL == CNoticProcess::m_pTaskQueue)
        {
            // 输出错误日志
            TSE_LOG_ERROR(CNoticProcess::m_poServLog, ("SendNotificationPerson: notic_task_queue not init"));
        }
        else
        {            
            TSE_LOG_DEBUG(CNoticProcess::m_poServLog, ("SendNotificationPerson: push notic_task_task in queue"));
            CNoticProcess::m_pTaskQueue->WaitTillPush(pNoticTask);
            TSE_LOG_DEBUG(CNoticProcess::m_poServLog, ("SendNotificationPerson: push notic_task_task in queue [size=%d]", 
                CNoticProcess::m_pTaskQueue->Size()));
        }
    }
    else
    {   
        // 输出错误日志
        TSE_LOG_ERROR(CNoticProcess::m_poServLog, ("SendNotificationPerson: notic_task_queue not space"));
    }
    return;
}

TVOID CMsgBase::SendNotificationAlliance(string strProject, TUINT32 udwSid, TINT64 ddwSuid, TINT64 ddwAid, SNoticInfo &rstNoticInfo)
{
    if(0 == ddwSuid
        || 0 == ddwAid)
    {
        return;
    }

    //wave@20160929: 暂时统一为英文
    rstNoticInfo.m_strLang = "english";

    CDocument *pDocument = CDocument::GetInstance();

    TBOOL bIsExistDocument = pDocument->IsSupportLang(rstNoticInfo.m_strLang);
    if(FALSE == bIsExistDocument)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendNotificationAlliance: do not have this language document"));
        return;
    }

    const Json::Value &stDocumentJson = pDocument->GetDocumentJsonByLang(rstNoticInfo.m_strLang);

    TINT32 dwRet = 0;
    string strNoticContent = "";
    strNoticContent = GetNoticDocumentByNoticId(rstNoticInfo.m_dwNoticId, stDocumentJson);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendNotificationAlliance: [strNoticContent=%s]", \
                                                     strNoticContent.c_str()));

    switch(rstNoticInfo.m_dwNoticId)
    {
    case EN_NOTI_ID__THRONE_BE_CAPTURED:
    case EN_NOTI_ID__PROVINCE_BE_CAPTURED:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        StringReplace(strNoticContent, "STRING1", rstNoticInfo.m_strTName);
        break;
    case EN_NOTI_ID__OCCUPY_THRONE_SUCC:
    case EN_NOTI_ID__OCCUPY_PROVINCE_SUCC:
        break;
    case EN_NOTI_ID__AL_GIFT:
        break;
    case EN_NOTI_ID__RALLYWAR_START:
    case EN_NOTI_ID__RALLYWAR_DEFENSE:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strSName);
        StringReplace(strNoticContent, "STRING1", rstNoticInfo.m_strTName);
        StringReplace(strNoticContent, "STRING2", rstNoticInfo.m_strCostTime);
        break;
    case EN_NOTI_ID__RALLY_ATTACK_SUCC:
    case EN_NOTI_ID__RALLY_ATTACK_FAIL:
    case EN_NOTI_ID__RALLY_DEFENSE_SUCC:
    case EN_NOTI_ID__RALLY_DEFENSE_FAIL:
        StringReplace(strNoticContent, "STRING0", rstNoticInfo.m_strTName);
        break;
    case EN_NOTI_ID__IDOL_CAN_BE_ATTACKED:
    case EN_NOTI_ID__OCCUPY_IDOL:
    case EN_NOTI_ID__OCCUPY_THRONE:
    case EN_NOTI_ID__THRONE_BE_ROBED:
    case EN_NOTI_ID__THRONE_PEACE_TIME_END_IN_ONE_HOUR:
        break;
    default:
        dwRet = -1;
        break;
    }

    if(dwRet < 0)
    {
        return;
    }

    ostringstream oss;
    oss.str("");
    oss << udwSid;
    string strSid = oss.str();
    oss.str("");
    oss << ddwSuid;
    string strUid = oss.str();
    oss.str("");
    oss << ddwAid;
    string strAid = oss.str();

    if(0 < CNoticTaskMgr::GetInstance()->GetEmptySessionSize())
    {
        SNoticTask *pNoticTask = NULL;
        CNoticTaskMgr::GetInstance()->WaitTillNoticTask(&pNoticTask);
        
        pNoticTask->m_strTitle = "";
        pNoticTask->m_ddwUid = ddwSuid;
        pNoticTask->m_ddwAid = ddwAid;
        pNoticTask->m_ddwSid = (TINT64)udwSid;
        pNoticTask->m_ddwNotiType = CMsgBase::GetNotiType(rstNoticInfo.m_dwNoticId, stDocumentJson);
        pNoticTask->m_strContent = strNoticContent;
        pNoticTask->m_strProject = strProject;
        pNoticTask->m_ddwNoticFlag = 1;
        pNoticTask->m_strSound = CMsgBase::GetNotiSound(pNoticTask->m_ddwNotiType, stDocumentJson);
            
        if (NULL == CNoticProcess::m_pTaskQueue)
        {
            // 输出错误日志
            TSE_LOG_ERROR(CNoticProcess::m_poServLog, ("SendNotificationAlliance: notic_task_queue not init"));
        }
        else
        {            
            TSE_LOG_DEBUG(CNoticProcess::m_poServLog, ("SendNotificationAlliance: push log_report_task in queue"));
            CNoticProcess::m_pTaskQueue->WaitTillPush(pNoticTask);
            TSE_LOG_DEBUG(CNoticProcess::m_poServLog, ("SendNotificationAlliance: push log_report_task in queue [size=%d]", 
                CNoticProcess::m_pTaskQueue->Size()));
        }
    }
    else
    {   
        // 输出错误日志
        TSE_LOG_ERROR(CNoticProcess::m_poServLog, ("SendNotificationAlliance: notic_task_queue not space"));
    }
    return;
}

TVOID CMsgBase::CheckMap(TINT64 ddwUid, TUINT32 udwSid)
{
    TCHAR szMsg[256];

    sprintf(szMsg, "./check_map.sh \"%ld\" \"%u\"", ddwUid, udwSid);

    CMsgBase::SendDelaySystemMsg(szMsg);
}

string CMsgBase::GetNoticDocumentByNoticId(TINT32 dwNoticId, const Json::Value &rstDocumentJson)
{
    return rstDocumentJson["doc_notification"][CCommonFunc::NumToString(dwNoticId)]["name"].asString();
}

TINT32 CMsgBase::GetNotiType(TINT32 dwNoticId, const Json::Value &rstDocumentJson)
{
    return rstDocumentJson["doc_notification"][CCommonFunc::NumToString(dwNoticId)]["type"].asInt();
}

string CMsgBase::GetNotiSound(TINT32 dwType, const Json::Value &rstDocumentJson)
{
    return rstDocumentJson["doc_notification_type"][CCommonFunc::NumToString(dwType)]["svr_ios_music"].asString();
}

TVOID CMsgBase::StringReplace(string &strSrc, const string &strFind, const string &strReplace)
{
    string::size_type pos = 0;
    string::size_type FindSize = strFind.size();
    string::size_type ReplaceSize = strReplace.size();
    while((pos = strSrc.find(strFind, pos)) != string::npos)
    {
        strSrc.replace(pos, FindSize, strReplace);
        pos += ReplaceSize;
    }
}

TVOID CMsgBase::AddAlRank(TbAlliance* ptbAlliance)
{
    TCHAR szMsg[1024];

    sprintf(szMsg, "./http_to_rank.sh \"add_al_info&key0=%ld&key1=%ld&key2=%ld&key3=%s&key4=%s&key5=%ld&key6=%ld\"",
        ptbAlliance->m_nSid,
        ptbAlliance->m_nAid,
        ptbAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET,
        ptbAlliance->m_sName.c_str(),
        ptbAlliance->m_sAl_nick_name.c_str(),
        ptbAlliance->m_nMight,
        ptbAlliance->m_nGift_point);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    //system(szMsg);
    return;
}

TVOID CMsgBase::DelAlRank(TbAlliance* ptbAlliance)
{
    TCHAR szMsg[1024];

    sprintf(szMsg, "./http_to_rank.sh \"del_al_info&key0=%ld&key1=%ld\"",
                    ptbAlliance->m_nSid,
                    ptbAlliance->m_nAid);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    //system(szMsg);
    return;
}


TVOID CMsgBase::ChangeAlLang(TbAlliance* ptbAlliance)
{
    TCHAR szMsg[1024];

    sprintf(szMsg, "./http_to_rank.sh \"change_al_info&key0=%ld&key1=%ld&key2=%d&key3=%ld\"",
        ptbAlliance->m_nSid,
        ptbAlliance->m_nAid,
        0,
        ptbAlliance->m_nLanguage);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    //system(szMsg);
    return;
}

TVOID CMsgBase::ChangeAlPolicy(TbAlliance* ptbAlliance)
{
    TCHAR szMsg[1024];

    sprintf(szMsg, "./http_to_rank.sh \"change_al_info&key0=%ld&key1=%ld&key2=%d&key3=%ld\"",
        ptbAlliance->m_nSid,
        ptbAlliance->m_nAid,
        1,
        ptbAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    //system(szMsg);
    return;
}

TVOID CMsgBase::ChangeAlName(TbAlliance* ptbAlliance)
{
    TCHAR szMsg[1024];

    sprintf(szMsg, "./http_to_rank.sh \"change_al_info&key0=%ld&key1=%ld&key2=%d&key3=%s\"",
        ptbAlliance->m_nSid,
        ptbAlliance->m_nAid,
        2,
        ptbAlliance->m_sName.c_str());

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    //system(szMsg);
    return;
}

TVOID CMsgBase::ChangeAlNick(TbAlliance* ptbAlliance)
{
    TCHAR szMsg[1024];

    sprintf(szMsg, "./http_to_rank.sh \"change_al_info&key0=%ld&key1=%ld&key2=%d&key3=%s\"",
        ptbAlliance->m_nSid,
        ptbAlliance->m_nAid,
        3,
        ptbAlliance->m_sAl_nick_name.c_str());

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    //system(szMsg);
    return;
}

string CMsgBase::GetEventMailContentByEventId(TINT32 dwMailId, const Json::Value &rstDocumentJson)
{
    ostringstream oss;
    oss.str("");
    oss << dwMailId;
    string strMailId = oss.str();
    string strContent = "";

    const Json::Value &oMailJson = rstDocumentJson["doc_mail"];
    
    if(!oMailJson.isMember(strMailId))
    {
        return strContent;
    }

    strContent = oMailJson[strMailId]["content"].asString();

    return strContent;
}

string CMsgBase::GetEventMailTitleByEventId(TINT32 dwMailId, const Json::Value &rstDocumentJson)
{
    ostringstream oss;
    oss.str("");
    oss << dwMailId;
    string strMailId = oss.str();
    string strContent = "";


    const Json::Value &oMailJson = rstDocumentJson["doc_mail"];

    if(!oMailJson.isMember(strMailId))
    {
        return strContent;
    }

    strContent = oMailJson[strMailId]["title"].asString();

    return strContent;
}

string CMsgBase::GetItemNameByItemId(TUINT32 udwItenId, const Json::Value &rstDocumentJson)
{
    ostringstream oss;
    oss.str("");
    oss << udwItenId;
    string strItemId = oss.str();
    string strContent = "";

    const Json::Value &oItemJson = rstDocumentJson["doc_item"];

    if(!oItemJson.isMember(strItemId))
    {
        return strContent;
    }

    strContent = oItemJson[strItemId]["name"].asString();

    return strContent;
}

string CMsgBase::GetSvrNameBySvrId(string szSvrId, const Json::Value &rstDocumentJson)
{
    const Json::Value &oSvrJson = rstDocumentJson["doc_world"];

    if (!oSvrJson.isMember(szSvrId))
    {
        return "World" + szSvrId;
    }
    return oSvrJson[szSvrId]["svr_name"].asString();
}

string CMsgBase::GetBuffInfoName( TINT32 dwBuffId )
{
    const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang("english");
    return stDocumentJson["doc_buff_info"][CCommonFunc::NumToString(dwBuffId)]["name"].asString();
}

TVOID CMsgBase::RefreshUserInfo(TINT64 ddwUid)
{
    if (ddwUid == 0)
    {
        return;
    }

    TCHAR szMsg[1024];

    sprintf(szMsg, "./refresh_user_info.sh %ld %u",
        ddwUid, CTimeUtils::GetUnixTime());

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("RefreshUserInfo: delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    return;
}

TVOID CMsgBase::ClearNoPlayerMap(TINT64 ddwUid, TINT64 ddwSid, TINT64 ddwPos)
{
    if (ddwUid == 0 || ddwPos == 0)
    {
        return;
    }

    TCHAR szMsg[1024];

    sprintf(szMsg, "./clear_no_player_map.sh %ld %ld %ld",
        ddwUid, ddwSid, ddwPos);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ClearNoPlayerMap: delay msg content:%s ", szMsg));

    CMsgBase::SendDelaySystemMsg(szMsg);
    return;
}