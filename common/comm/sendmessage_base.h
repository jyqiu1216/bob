/************************************************************
文件名称: sendmessage_base.h
作者: daemon
日期: 2014.11.07
模块描述: city模块基础接口。
*************************************************************/

#ifndef _SEND_MESSAGE_BASE_H_
#define _SEND_MESSAGE_BASE_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include "aws_table_city.h"
#include "aws_table_tips.h"
#include "aws_table_user_stat.h"
#include "player_info.h"

class CSendMessageBase
{
//tips
public:
    static TINT32 AddTips(SUserInfo *pstUser, TUINT8 ucType, TUINT32 udwUid, TBOOL bIsNeedPut,
        TUINT32 udwKey0 = 0, TUINT32 udwKey1 = 0, TUINT32 udwKey2 = 0,
        const TCHAR *pszKey0 = "", const TCHAR *pszKey1 = "", const TCHAR *pszKey2 = "");

    static TINT32 AddTips(SUserInfo* pstUser, TUINT32 udwUid, TBOOL bIsNeedPut, TUINT8 ucType, const string& strContent);

    static TINT32 AddAlTips(SUserInfo *pstUser, TUINT8 ucType, TINT64 ddwAid, TBOOL bIsNeedPut,
        TUINT32 udwKey0 = 0, TUINT32 udwKey1 = 0, TUINT32 udwKey2 = 0,
        const TCHAR *pszKey0 = "", const TCHAR *pszKey1 = "", const TCHAR *pszKey2 = "");

    static TINT32 AddCommonTips(SUserInfo* pstUser, TUINT32 udwUid, TBOOL bIsNeedPut,
        SBuffDetail* pstItemEffect, SSpGlobalRes *pstEffectInfo,
        TINT32 dwItemId, TINT64 ddwTargetId);

    static TINT32 AddAlEventTips(SUserInfo *pstUser, TINT64 udwAid,TUINT32 udwEventType, TUINT32 udwRewardType, 
        TUINT32 udwGoalorRank, TUINT32 udwScoreGet, string sScoreList, SGlobalRes *pstReward, string sEventInfo);

    static TINT32 AddEventTips(SUserInfo *pstUser, TUINT32 udwEventType, TUINT32 udwRewardType, TUINT32 udwGoalorRank,
        TUINT32 udwScoreGet, string sScoreList, SOneGlobalRes *astRewardList, TUINT32 udwRewardNum, string sEventInfo,
        TINT64 ddwMailId = 0, TINT64 ddwSuid = 0, TINT32 dwWindowOpt = 0);

public:
    static TVOID SendSysMsgToChat(TINT32 dwSid, TINT64 ddwAlid, TINT32 dwMsgType, string strCustomizeParam);

public:
    static TINT32 SendBroadcast(SUserInfo *pstUser, TUINT32 udwSid, TUINT32 udwUid, TINT32 dwContentId, const string& sReplaceData = "", const string& sParam = "");

    static TINT64 GetBroadcastKeyBySid(TINT32 dwSid);

public:
    static TINT32 GetTips(TbTips *ptbTips, TUINT8 ucType, TUINT32 udwUid, 
        TUINT32 udwKey0 = 0, TUINT32 udwKey1 = 0, TUINT32 udwKey2 = 0,
        const TCHAR *pszKey0 = "", const TCHAR *pszKey1 = "", const TCHAR *pszKey2 = "");
    static TINT32 GetTips(TbTips *ptbTips, TUINT32 udwUid, TUINT8 ucType, const string& strContent);
};

#endif
