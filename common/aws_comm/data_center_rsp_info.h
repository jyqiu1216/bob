#pragma once



// ===================================== trade逻辑 ============================== //

struct STradeNode
{
    /*
    "city_name": "string", //不实时
    "reward_num": int, //出使方得到的奖励
    "reception_reward": int, //接待方得到的奖励
    "trade_target_uid": int, //被出使方uid
    "trade_march_time": int,
    "trade_stay_time": int,
    "type": int //用于客户端取icon 根据game.json规则生成的...
    */
    string m_sCityName;
    TINT32 m_dwRewardNum;
    TINT32 m_dwReceptionReward;
    TINT32 m_dwTradeTargetUid;
    TINT32 m_dwTradeMarchTime;
    TINT32 m_dwTradeStayTime;
    TINT32 m_dwType;

    STradeNode()
    {
        Reset();
    }

    void Reset()
    {
        m_sCityName = "";
        m_dwRewardNum = 0;
        m_dwReceptionReward = 0;
        m_dwTradeTargetUid = 0;
        m_dwTradeMarchTime = 0;
        m_dwTradeStayTime = 0;
        m_dwType = 0;
    }
};



struct STradeMarketRsp
{
    /*
    {
        "refresh_time":long, //下次刷新时间
        "trade_list":
        [
            trade_node
        ]
    }
    */
    TINT32 m_dwRefreshTime;
    vector<STradeNode*> m_vecTradeList;

    STradeMarketRsp()
    {
        Reset();
    }

    ~STradeMarketRsp()
    {
        Reset();
    }

    void Reset()
    {
        m_dwRefreshTime = 0;
        for (TUINT32 udwIdx = 0; udwIdx < m_vecTradeList.size(); ++udwIdx)
        {
            delete m_vecTradeList[udwIdx];
            m_vecTradeList[udwIdx] = NULL;
        }
        m_vecTradeList.clear();
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if (!jsonRsp.isMember("trade_info"))
        {
            return -1;
        }
        if (!jsonRsp["trade_info"].isMember("refresh_time"))
        {
            return -2;
        }
        m_dwRefreshTime = jsonRsp["trade_info"]["refresh_time"].asInt();

        if (!jsonRsp.isMember("trade_list"))
        {
            return -3;
        }
        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["trade_info"]["trade_list"].size(); ++udwIdx)
        {
            STradeNode *pstTradeNode = new STradeNode;
            pstTradeNode->m_sCityName = jsonRsp["trade_info"]["trade_list"][udwIdx]["city_name"].asString();
            pstTradeNode->m_dwRewardNum = 
            pstTradeNode->m_dwReceptionReward = jsonRsp["trade_info"]["trade_list"][udwIdx]["reception_reward"].asInt();
            pstTradeNode->m_dwTradeTargetUid = jsonRsp["trade_info"]["trade_list"][udwIdx]["trade_target_uid"].asInt();
            pstTradeNode->m_dwTradeMarchTime = jsonRsp["trade_info"]["trade_list"][udwIdx]["trade_march_time"].asInt();
            pstTradeNode->m_dwTradeStayTime = jsonRsp["trade_info"]["trade_list"][udwIdx]["trade_stay_time"].asInt();
            pstTradeNode->m_dwType = jsonRsp["trade_info"]["trade_list"][udwIdx]["type"].asInt();
            m_vecTradeList.push_back(pstTradeNode);
        }
    }
};


// ===================================== quest逻辑 ============================== //

struct SQuestNodeRsp
{
    /*
    {
        "cost_time":int, //cost time
        "lv" : 1, //level
        "reward": [
        {
            int, //type
            int, //id
            int  //num
        }]
    }
    */
    TINT32 m_dwCostTime;
    TINT32 m_dwLevel;
    vector<SOneGlobalRes *> m_vecReward;
    
    SQuestNodeRsp()
    {
        Reset();
    }

    ~SQuestNodeRsp()
    {
        Reset();
    }

    void Reset()
    {
        m_dwCostTime = 0;
        m_dwLevel = 0;
        for (TUINT32 udwIdx = 0; udwIdx < m_vecReward.size(); ++udwIdx)
        {
            delete m_vecReward[udwIdx];
        }
        m_vecReward.clear();
    }
};

struct SQuestListInfo
{
    /*
    {
        "refresh_time": long, //本次刷新时间
        "quest_num": int,
        "quest_list":[
            quest_node
        ]
    }
    */
    TINT32 m_dwRefreshTime;
    TINT32 m_dwQuestType;
    TINT32 m_dwQuestNum;
    vector<SQuestNodeRsp*> m_vecQuestList;
    
    SQuestListInfo()
    {
        Reset();
    }

    ~SQuestListInfo()
    {
        Reset();
    }

    void Reset()
    {
        m_dwRefreshTime = 0;
        m_dwQuestNum = 0;
        m_dwQuestType = 0;
        for (TUINT32 udwIdx = 0; udwIdx < m_vecQuestList.size(); ++udwIdx)
        {
            delete m_vecQuestList[udwIdx];
        }
        m_vecQuestList.clear();
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if (!jsonRsp.isMember("quest"))
        {
            return -1;
        }

        m_dwRefreshTime = jsonRsp["quest"]["refresh_time"].asInt();
        m_dwQuestType = jsonRsp["quest"]["quest_type"].asInt();
        m_dwQuestNum = jsonRsp["quest"]["quest_num"].asInt();
        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["quest"]["quest_list"].size(); ++udwIdx)
        {
            SQuestNodeRsp *pstQuestNode = new SQuestNodeRsp;
            pstQuestNode->m_dwCostTime = jsonRsp["quest"]["quest_list"][udwIdx]["cost_time"].asInt();
            pstQuestNode->m_dwLevel = jsonRsp["quest"]["quest_list"][udwIdx]["lv"].asInt();
            for (TUINT32 udwIdy = 0; udwIdy < jsonRsp["quest"]["quest_list"][udwIdx]["reward"].size(); ++udwIdy)
            {
                SOneGlobalRes *pstReward = new SOneGlobalRes;
                pstReward->ddwType = jsonRsp["quest"]["quest_list"][udwIdx]["reward"][udwIdy][0U].asInt();
                pstReward->ddwId = jsonRsp["quest"]["quest_list"][udwIdx]["reward"][udwIdy][1U].asInt();
                pstReward->ddwNum = jsonRsp["quest"]["quest_list"][udwIdx]["reward"][udwIdy][2U].asInt();
                pstQuestNode->m_vecReward.push_back(pstReward);
            }
            m_vecQuestList.push_back(pstQuestNode);
        }
        return 0;
    }
};

// ===================================== 神秘礼物逻辑 ============================== //


struct SMisteryGiftRsp
{
    /*
    {
        "mistery_gift":
        {
            "cost_time": int,
            "reward":
            [
                {
                    int, // type
                    int, // id
                    int  // num
                }
            ]
        }
    }
    */
    TINT32 m_dwCostTime;
    vector<SOneGlobalRes *> m_vecReward;

    SMisteryGiftRsp()
    {
        Reset();
    }

    ~SMisteryGiftRsp()
    {
        Reset();
    }

    void Reset()
    {
        m_dwCostTime = 0;
        for (TUINT32 udwIdx = 0; udwIdx < m_vecReward.size(); ++udwIdx)
        {
            delete m_vecReward[udwIdx];
            m_vecReward[udwIdx] = NULL;
        }
        m_vecReward.clear();
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if (!jsonRsp.isMember("mistery_gift"))
        {
            return -1;
        }
        
        if (!jsonRsp["mistery_gift"].isMember("reward"))
        {
            return -2;
        }
        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["mistery_gift"]["reward"].size(); ++udwIdx)
        {
            SOneGlobalRes *pstItem = new SOneGlobalRes;
            pstItem->ddwType = jsonRsp["mistery_gift"]["reward"][udwIdx][0U].asInt();
            pstItem->ddwId = jsonRsp["mistery_gift"]["reward"][udwIdx][1U].asInt();
            pstItem->ddwNum = jsonRsp["mistery_gift"]["reward"][udwIdx][2U].asInt();
            m_vecReward.push_back(pstItem);
        }
        m_dwCostTime = jsonRsp["mistery_gift"]["cost_time"].asInt();
        return 0;
    }
};


// ===================================== 联盟礼物逻辑 ============================== //


struct SAllianceGift
{
    vector<SOneGlobalRes *> m_vecReward;
    TINT64 ddwGiftPoint;

    SAllianceGift()
    {
        Reset();
    }

    ~SAllianceGift()
    {
        Reset();
    }

    void Reset()
    {
        for (TUINT32 udwIdx = 0; udwIdx < m_vecReward.size(); ++udwIdx)
        {
            delete m_vecReward[udwIdx];
            m_vecReward[udwIdx] = NULL;
        }
        m_vecReward.clear();
        ddwGiftPoint = 0;
    }
};

struct SAllianceGiftRsp
{
    SAllianceGift m_sAllianceGift[AL_GIFT_OPEN_RSP_NUM];
    TUINT32 m_nGiftNum;

    SAllianceGiftRsp()
    {
        Reset();
    }

    ~SAllianceGiftRsp()
    {
        Reset();
    }

    void Reset()
    {
        for (TUINT32 udwIdx = 0; udwIdx < AL_GIFT_OPEN_RSP_NUM; ++udwIdx)
        {
            m_sAllianceGift[udwIdx].Reset();
        }
        m_nGiftNum = 0;
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if (!jsonRsp.isMember("al_gift"))
        {
            return -1;
        }
        if (!jsonRsp["al_gift"].isArray())
        {
            return -2;
        }
        if (jsonRsp["al_gift"].size() == 0)
        {
            return -3;
        }

        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["al_gift"].size() && udwIdx < AL_GIFT_OPEN_RSP_NUM; ++udwIdx)
        {
            if (!jsonRsp["al_gift"][udwIdx].isMember("reward"))
            {
                return -4;
            }

            for (TUINT32 udwIdy = 0; udwIdy < jsonRsp["al_gift"][udwIdx]["reward"].size(); ++udwIdy)
            {
                SOneGlobalRes *pstItem = new SOneGlobalRes;
                pstItem->ddwType = jsonRsp["al_gift"][udwIdx]["reward"][udwIdy][0U].asInt();
                pstItem->ddwId = jsonRsp["al_gift"][udwIdx]["reward"][udwIdy][1U].asInt();
                pstItem->ddwNum = jsonRsp["al_gift"][udwIdx]["reward"][udwIdy][2U].asInt();
                m_sAllianceGift[udwIdx].m_vecReward.push_back(pstItem);
            }
            m_sAllianceGift[udwIdx].ddwGiftPoint = jsonRsp["al_gift"][udwIdx]["point"].asInt();

            ++m_nGiftNum;
        }

        return 0;
    }
};


// ===================================== 开材料箱子/水晶箱子逻辑 ============================== //


struct SChestRsp
{
    TINT64 ddwOpenNum;
    vector<SOneGlobalRes *> m_vecReward;

    SChestRsp()
    {
        Reset();
    }

    ~SChestRsp()
    {
        Reset();
    }

    void Reset()
    {
        for (TUINT32 udwIdx = 0; udwIdx < m_vecReward.size(); ++udwIdx)
        {
            delete m_vecReward[udwIdx];
            m_vecReward[udwIdx] = NULL;
        }
        m_vecReward.clear();
        ddwOpenNum = 0;
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if(!jsonRsp.isMember("chest"))
        {
            return -1;
        }

        if(!jsonRsp["chest"].isMember("open_num"))
        {
            return -2;
        }

        if(!jsonRsp["chest"].isMember("reward"))
        {
            return -3;
        }

        
        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["chest"]["reward"].size(); ++udwIdx)
        {
            for(TUINT32 udwIdy = 0; udwIdy < jsonRsp["chest"]["reward"][udwIdx].size(); ++udwIdy)
            {
                SOneGlobalRes *pstItem = new SOneGlobalRes;
                pstItem->ddwType = jsonRsp["chest"]["reward"][udwIdx][udwIdy][0U].asInt();
                pstItem->ddwId = jsonRsp["chest"]["reward"][udwIdx][udwIdy][1U].asInt();
                pstItem->ddwNum = jsonRsp["chest"]["reward"][udwIdx][udwIdy][2U].asInt();
                m_vecReward.push_back(pstItem);
            }
        }
        ddwOpenNum = jsonRsp["chest"]["open_num"].asInt();
        return 0;
    }
};
    


// =========================================== 打野怪逻辑 ==================================== //

struct SMonsterRsp
{
    vector<SOneGlobalRes *> m_vecAttackReward;
    vector<SOneGlobalRes *> m_vecEliteReward;

    SMonsterRsp()
    {
        Reset();
    }

    ~SMonsterRsp()
    {
        Reset();
    }

    void Reset()
    {
        for (TUINT32 udwIdx = 0; udwIdx < m_vecAttackReward.size(); ++udwIdx)
        {
            delete m_vecAttackReward[udwIdx];
            m_vecAttackReward[udwIdx] = NULL;
        }
        m_vecAttackReward.clear();

        for (TUINT32 udwIdx = 0; udwIdx < m_vecEliteReward.size(); ++udwIdx)
        {
            delete m_vecEliteReward[udwIdx];
            m_vecEliteReward[udwIdx] = NULL;
        }
        m_vecEliteReward.clear();
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if(!jsonRsp.isMember("monster"))
        {
            return -1;
        }

        if(!jsonRsp["monster"].isMember("attack_reward"))
        {
            return -2;
        }

        if(!jsonRsp["monster"].isMember("elite_reward"))
        {
            return -3;
        }

        
        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["monster"]["attack_reward"].size(); ++udwIdx)
        {
            SOneGlobalRes *pstItem = new SOneGlobalRes;
            pstItem->ddwType = jsonRsp["monster"]["attack_reward"][udwIdx][0U].asInt();
            pstItem->ddwId = jsonRsp["monster"]["attack_reward"][udwIdx][1U].asInt();
            pstItem->ddwNum = jsonRsp["monster"]["attack_reward"][udwIdx][2U].asInt();
            m_vecAttackReward.push_back(pstItem);
        }

        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["monster"]["elite_reward"].size(); ++udwIdx)
        {
            SOneGlobalRes *pstItem = new SOneGlobalRes;
            pstItem->ddwType = jsonRsp["monster"]["elite_reward"][udwIdx][0U].asInt();
            pstItem->ddwId = jsonRsp["monster"]["elite_reward"][udwIdx][1U].asInt();
            pstItem->ddwNum = jsonRsp["monster"]["elite_reward"][udwIdx][2U].asInt();
            m_vecEliteReward.push_back(pstItem);
        }


        return 0;
    }
};
    



// ===================================== 拉资源掉落逻辑 ============================== //

struct SOccupyRsp
{
    vector<SOneGlobalRes *> m_vecReward;

    SOccupyRsp()
    {
        Reset();
    }

    ~SOccupyRsp()
    {
        Reset();
    }

    void Reset()
    {
        for (TUINT32 udwIdx = 0; udwIdx < m_vecReward.size(); ++udwIdx)
        {
            delete m_vecReward[udwIdx];
            m_vecReward[udwIdx] = NULL;
        }
        m_vecReward.clear();
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if(!jsonRsp.isMember("wild"))
        {
            return -1;
        }

        if(!jsonRsp["wild"].isMember("reward"))
        {
            return -2;
        }

        
        for (TUINT32 udwIdx = 0; udwIdx < jsonRsp["wild"]["reward"].size(); ++udwIdx)
        {
            SOneGlobalRes *pstItem = new SOneGlobalRes;
            pstItem->ddwType = jsonRsp["wild"]["reward"][udwIdx][0U].asInt();
            pstItem->ddwId = jsonRsp["wild"]["reward"][udwIdx][1U].asInt();
            pstItem->ddwNum = jsonRsp["wild"]["reward"][udwIdx][2U].asInt();
            m_vecReward.push_back(pstItem);
        }

        return 0;
    }
};
    



// ============================================ 其他逻辑 ===================================== //


struct SMisteryStoreRsp
{
    //TODO

    SMisteryStoreRsp()
    {
        Reset();
    }

    void Reset()
    {

    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        return 0;
    }
};

struct SQuestRsp
{
    SQuestListInfo m_stDaliyQuest;
    SQuestListInfo m_stAllianceQuest;
    SQuestListInfo m_stVipQuest;

    SQuestRsp()
    {
        Reset();
    }

    void Reset()
    {
        m_stDaliyQuest.Reset();
        m_stAllianceQuest.Reset();
        m_stVipQuest.Reset();
    }

    TINT32 setVal(const Json::Value &jsonRsp)
    {
        if (!jsonRsp.isMember("time_quest"))
        {
            return -1;
        }

        if (!jsonRsp["time_quest"].isMember("daily"))
        {
            return -2;
        }
        m_stDaliyQuest.setVal(jsonRsp["time_quest"]["daily"]);

        if (!jsonRsp["time_quest"].isMember("alliance"))
        {
            return -3;
        }
        m_stAllianceQuest.setVal(jsonRsp["time_quest"]["alliance"]);

        if (!jsonRsp["time_quest"].isMember("vip"))
        {
            return -4;
        }
        m_stVipQuest.setVal(jsonRsp["time_quest"]["vip"]);

        return 0;
    }
};



// ===================================== 回包数据保存 ============================== //

struct DataCenterRspInfo
{
    TINT32 m_dwRetCode;
    TUINT32 m_udwServiceType;
    TUINT32 m_udwType;
    string m_sRspJson;

    DataCenterRspInfo()
    {
        Reset();
    }

    void Reset()
    {
        m_dwRetCode = 0;
        m_udwServiceType = 0;
        m_udwType = 0;
        m_sRspJson = "";
    }
};



struct SRefreshData
{
    STradeMarketRsp m_stTradeMarketRsp;
    SMisteryGiftRsp m_stMisteryGiftRsp;
    SAllianceGiftRsp m_stAllianceGiftRsp;
    SMisteryStoreRsp m_stMisteryStoreRsp;
    SQuestRsp m_stQuestRsp;
    SChestRsp m_stChestRsp;
    SMonsterRsp m_stMonsterRsp;
    SOccupyRsp m_stOccupyRsp;

    SRefreshData()
    {
        Reset();
    }

    void Reset()
    {
        m_stTradeMarketRsp.Reset();
        m_stMisteryGiftRsp.Reset();
        m_stAllianceGiftRsp.Reset();
        m_stMisteryStoreRsp.Reset();
        m_stQuestRsp.Reset();
        m_stChestRsp.Reset();
        m_stMonsterRsp.Reset();
        m_stOccupyRsp.Reset();
    }
};


