#ifndef _BUSSINESS_STRUCT_H_
#define _BUSSINESS_STRUCT_H_

#include <bitset>
#include "aws_table_include.h"

//#define CHAR_BIT  (8)
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)


struct SCityActionStat
{
    TUINT8  m_ucDoingBuildingNum;
    TUINT8  m_ucDoingEncampNum;    // 外部驻兵数量,不算本城的驻兵
    TUINT8  m_ucDoingMarchNum;
    TUINT8  m_ucDoingScoutNum;
    TUINT8  m_ucDoingCampNum;
    TUINT8  m_ucDoingResearchNum;
    TUINT8  m_ucDoingEquipUpgradeNum;
    TUINT8  m_ucDoingFortNum;
    TUINT8  m_aucDoingTroopNum[EN_TROOP_CATEGORY__END];
    TUINT8 m_ucDoingTransportNum;
    TBOOL   m_bCanPeaceTime;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SResourceProduction
{
    // 生产率相关
    TINT64 m_ddwBaseProduction;
    TINT64 m_addwBonus[EN_BONUS_TYPE__END]; // 单位1/1000
    TINT64 m_ddwTotalBonus;
    TINT64 m_ddwTotalBonusAmount;

    // 限制相关
    TINT64 m_ddwCurProduction;
    TUINT64 m_uddwUpkeep;
    TINT64 m_ddwCapacity;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};


struct SBuffDetail
{
    TUINT32 m_udwId;
    TINT64 m_ddwNum;
    TUINT32 m_dwTime;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }

    TVOID SetVal(TUINT32 udwType, TINT64 ddwNum, TUINT32 dwTime)
    {
        m_udwId = udwType;
        m_ddwNum = ddwNum;
        m_dwTime = dwTime;
    }

};

struct SMaterialChance
{
    TUINT32 m_udwItemId;
    TUINT32 m_udwRate;
    SMaterialChance()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SEquipBuffer
{
    TUINT32 m_udwId;
    TINT32 m_dwNum;
    TUINT32 m_udwType;
    

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }

    TVOID SetVal(TUINT32 udwId, TINT32 dwNum, TUINT32 udwType)
    {
        m_udwId = udwId;
        m_dwNum = dwNum;
        m_udwType = udwType;
    }

};


struct SRawEquipBuffer
{
    TUINT32 m_udwId;
    TINT32 m_dwMinNum;
    TUINT32 m_udwType;
    TINT32 m_dwMaxNum;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }

    TVOID SetVal(TUINT32 udwId, TUINT32 dwMinNum,TINT32 dwMaxNum, TUINT32 udwType)
    {
        m_udwId = udwId;
        m_dwMinNum = dwMinNum;
        m_dwMaxNum = dwMaxNum;
        m_udwType = udwType;
    }

};

struct SEquipStatusInfo
{
    //状态信息
    TUINT32			udwStatus;
    TUINT32         audwSlot[5];
    TUINT32			udwEquipmentPutOnTime;
    //buffer 
    SEquipBuffer	astBuffInfo[MAX_BASE_BUFFER_NUM_IN_EQUIP];
    TUINT32			udwBufferNum;
    TUINT32         udwPotOnPos;

    //mistroy buffer 
    SEquipBuffer	astMisteryBuffInfo[MAX_BASE_BUFFER_NUM_IN_EQUIP];
    TUINT32			udwMistoryBufferNum;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }

};


struct SEquipBaseInfo
{
    //基本信息
    TUINT32 udwEType; //game json id
    TUINT32 udwPos;
    TUINT32 udwLv;

    TUINT32 udwOnLv;
    TUINT32 udwEffectTime;
    TUINT32 udwCategory;

    TINT64 ddwScrollId;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SEquipMentInfo
{
    TUINT64 uddwId; //唯一id
    SEquipStatusInfo stStatusInfo;
    SEquipBaseInfo stBaseInfo;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SCrystalInfo
{
    TUINT32 udwId;
    TUINT32 udwType;
    TUINT32 udwLv;

    SEquipBuffer	astBuffInfo[MAX_CRYSTAL_BUFFER];
    TUINT32			udwBufferNum;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};


struct SSoulInfo
{
    TUINT32 udwId;
    TUINT32 udwType;
    TUINT32 udwLv;

    SRawEquipBuffer	astBuffInfo[MAX_CRYSTAL_BUFFER];
    TUINT32			udwBufferNum;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SMaterialInfo
{
    TUINT32 udwId;
    TUINT32 udwType;
    TUINT32 udwLv;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SComposeEquipParts
{
    TUINT32 udwParts[MAX_COMPOSE_PARTS_USE];
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};
struct SAttribute
{
    TINT32 dwAttrId;	//0 无意义
    TINT32 dwAttrNum;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SAttrInfo
{
    SAttribute m_stAttribute[MAX_ATTRIBUTE_NUM];
    TUINT32 udwAttributeNum;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }

    TVOID SetAttrInfo(TINT32 dwAttrId, TINT32 dwAttrNum)
    {
        TBOOL bExit = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < udwAttributeNum; ++udwIdx)
        {
            if (m_stAttribute[udwIdx].dwAttrId == dwAttrId)
            {
                bExit = TRUE;
                m_stAttribute[udwIdx].dwAttrNum = dwAttrNum;
            }
        }
        if (!bExit)
        {
            if (udwAttributeNum >= MAX_ATTRIBUTE_NUM)
            {
                return;
            }
            m_stAttribute[udwAttributeNum].dwAttrId = dwAttrId;
            m_stAttribute[udwAttributeNum].dwAttrNum = dwAttrNum;
            ++udwAttributeNum;

        }
    }

    TINT32 GetAttrInfo(TINT32 dwAttrId)
    {
        for (TUINT32 udwIdx = 0; udwIdx < udwAttributeNum; ++udwIdx)
        {
            if (dwAttrId == m_stAttribute[udwIdx].dwAttrId)
            {
                return m_stAttribute[udwIdx].dwAttrNum;
            }
        }
        return 0;
    }
};

/////////////
struct SAttrGlobalRes
{
    TUINT32 udwType; //类型
    TUINT64 udwId; // id
    TUINT32 udwNum; //

    SAttrInfo stAttrInfo;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID SetValue(TUINT32 udwTmpType, TUINT32 udwTmpId, TUINT32 udwTmpNum)
    {
        udwType = udwTmpType;
        udwId = udwTmpId;
        udwNum = udwTmpNum;
    }
};

struct SSpGlobalRes
{
    TUINT32 udwTotalNum;                         // 物品数
    SAttrGlobalRes aRewardList[MAX_SP_REWARD_ITEM_NUM];    // 

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    SAttrGlobalRes& operator[](TUINT32 udwNum)
    {
        assert(udwNum < MAX_REWARD_ITEM_NUM);
        
        return aRewardList[udwNum];
        
    }
};

struct SBuffTotal
{
    TUINT32 m_udwBuffId;
    TINT64 m_ddwBuffTotal;       //buffer的总和 后台计算直接拿该值即可
    SBuffDetail m_astBuffDetail[EN_BUFF_TYPE_END];    //buffer详情

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }

};

struct SPlayerBuffInfo
{
    SBuffTotal m_astPlayerBuffInfo[EN_BUFFER_INFO_END];

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }

    SBuffTotal& operator[](unsigned int udwIdx)
    {
        assert(udwIdx < EN_BUFFER_INFO_END);
        return m_astPlayerBuffInfo[udwIdx];
    }
};

struct SScoreList
{
    TUINT32 udwScoreType;
    TUINT32 udwScoreId;
    TUINT32 udwScore;
    TINT64 ddwUid;
    TUINT32 udwSid;
    TUINT64 uddwAlid;
    string strUname;
    SScoreList()
    {
        Reset();
    }
    TVOID Reset()
    {
        udwScoreType = 0;
        udwScoreId = 0;
        udwScore = 0;
        ddwUid = 0;
        udwSid = 0;
        uddwAlid = 0;
        strUname = "";
    }
};

struct SActiveScore
{
    TUINT32 audwScoreList[EN_SCORE_TYPE__END][MAX_SCORE_ID_NUM];
    struct SScoreList sScoreList[MAX_SCORE_LIST_LENGTH];

    SActiveScore()
    {
        Reset();
    }
    TVOID Reset()
    {
//         for (TUINT32 udwIdx = 0; udwIdx < EN_SCORE_TYPE__END; udwIdx++)
//         {
//             for (TUINT32 udwIdy = 0; udwIdy < MAX_SCORE_ID_NUM; udwIdy++)
//             {
//                 audwScoreList[udwIdx][udwIdy] = 0;
//             }
//         }
        memset(audwScoreList, 0, sizeof(audwScoreList));
        for (TUINT32 udwIdx = 0; udwIdx < MAX_SCORE_LIST_LENGTH; udwIdx++)
        {
            sScoreList[udwIdx].Reset();
        }
    }
};

struct SMailEntry
{
    TINT64 ddwMid;
    TINT32 dwUnreadCount;
    TINT32 dwDisplayType;
    TINT64 ddwDisplayClass;
    TINT32 dwTime;
    TINT32 dwStatus;
    TINT32 dwNum;
    TINT32 dwReceiverAvatar;
    TINT64 ddwTuid;
    TCHAR strReceiver[32];
    TINT64 ddwSmallestMid;
    TINT32 dwSmallestMailUnread;
    TCHAR strReceiverAlnick[16];
    TINT32 dwHasReward;
    TBOOL bAvailable;
    TINT64 ddwIndexUid;
    TINT64 ddwSuid;

    SMailEntry()
    {
        Reset();
    }
    TVOID Reset()
    {
        ddwMid = 0;
        dwUnreadCount = 0;
        dwDisplayType = 0;
        ddwDisplayClass = 0;
        dwTime = 0;
        dwStatus = 0;
        dwNum = 0;
        dwReceiverAvatar = 0;
        memset(strReceiver, 0, sizeof(strReceiver));
        ddwSmallestMid = 0;
        dwSmallestMailUnread = 0;
        memset(strReceiverAlnick, 0, sizeof(strReceiverAlnick));
        dwHasReward = 0;
        bAvailable = TRUE;
        ddwIndexUid = 0;
        ddwSuid = 0;
    }
};

struct SMailUserRspInfo
{
    TUINT32 m_udwMailTotalNum;
    TUINT32 m_udwMailUnreadNum;
    TINT64 m_ddwNewestMid;
    TUINT32 m_udwMailEntryNum;
    SMailEntry m_aMailToReturn[MAX_MAIL_PERPAGE_NUM];
    TVOID Reset()
    {
        m_udwMailTotalNum = 0;
        m_udwMailUnreadNum = 0;
        m_ddwNewestMid = 0;
        m_udwMailEntryNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < MAX_MAIL_PERPAGE_NUM; udwIdx++)
        {
            m_aMailToReturn[udwIdx].Reset();
        }
    }

    SMailUserRspInfo()
    {
        Reset();
    }
};

struct SReportEntry
{
    TINT64 ddwRid;
    TINT32 dwUnread;
    TINT32 dwReportType;
    TINT64 ddwDisplayClass;
    TINT32 dwTime;
    TINT32 dwStatus;
    TINT32 dwNum;

    SReportEntry()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SReportUserRspInfo
{
    TUINT32 m_udwReportTotalNum;
    TUINT32 m_udwReportUnreadNum;
    TINT64 m_ddwNewestRid;
    TUINT32 m_udwReportEntryNum;
    SReportEntry m_aReportToReturn[MAX_PERPAGE_NUM];
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    SReportUserRspInfo()
    {
        Reset();
    }
};

struct STitleInfoList
{
    TUINT32 udwNum;
    TbTitle atbTitle[MAX_TITLEINFO_NUM_IN_ONE_SERVER];
    TUINT8 aucFlag[MAX_TITLEINFO_NUM_IN_ONE_SERVER]; //同步个人信息用change  将title任命给某人用new   不关心是否已存在...

    STitleInfoList()
    {
        for(TUINT32 udwIdx = 0; udwIdx < MAX_TITLEINFO_NUM_IN_ONE_SERVER; ++udwIdx)
        {
            atbTitle[udwIdx].Reset();
            aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
        }
        udwNum = 0;
    }
    TVOID Reset()
    {
        for(TUINT32 udwIdx = 0; udwIdx < MAX_TITLEINFO_NUM_IN_ONE_SERVER; ++udwIdx)
        {
            atbTitle[udwIdx].Reset();
            aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
        }
        udwNum = 0;
    }
};

struct SEventGoal
{
    TINT32 dwFirstGroup; // rank分组
    TINT32 dwSecGroup; //goal分组
    TINT32 dwRand;   //难度
    TINT32 dwGoalId; //日志记录

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SUserEventGoals
{
    TINT64 ddwF1; // 染色信息1
    TINT64 ddwF2; // 染色信息2
    TINT64 ddwF3; // 染色信息3
    TINT64 ddwF4; // 染色信息4
    SEventGoal asGoalList[EN_EVENT_TYPE__END]; // 0联盟 1个人 2地狱个人

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SIdolRank
{
    TUINT32 udwIdx;
    TUINT32 udwPoint;
    TINT64 ddwTime;
    SIdolRank()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    static TBOOL Comp(const SIdolRank &stA, const SIdolRank &stB)
    {
        if (stA.udwPoint > stB.udwPoint || (stA.udwPoint == stB.udwPoint && stA.ddwTime < stB.ddwTime))
        {
            return TRUE;
        }
        return FALSE;
    }
};

#endif
