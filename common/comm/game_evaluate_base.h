#ifndef _GAME_EVALUATE_BASE_H_
#define _GAME_EVALUATE_BASE_H_

#include <string>
#include <map>
#include <vector>
using namespace std;

#define F_SIZE (4)


enum EUserInfoRecordType
{
    EN_USER_INFO_RECORD_TYPE__GEM = 0,
    EN_USER_INFO_RECORD_TYPE__ITEM = 1,
    EN_USER_INFO_RECORD_TYPE__RESOURCE = 2,
    EN_USER_INFO_RECORD_TYPE__BUILDING = 3,
    EN_USER_INFO_RECORD_TYPE__RESEARCH = 4,
    EN_USER_INFO_RECORD_TYPE__TROOP = 5,        // 活的troop，包含家里的和外出的
    EN_USER_INFO_RECORD_TYPE__FORT = 6,    
    EN_USER_INFO_RECORD_TYPE__MATERIAL = 7,
    EN_USER_INFO_RECORD_TYPE__CRYSTAL = 8,
    EN_USER_INFO_RECORD_TYPE__SP_CRYSTAL = 9,
    EN_USER_INFO_RECORD_TYPE__SOUL = 10,
    EN_USER_INFO_RECORD_TYPE__PATRS = 11,
    EN_USER_INFO_RECORD_TYPE__EQUIP = 12,
    EN_USER_INFO_RECORD_TYPE__MIGHT = 13,
    EN_USER_INFO_RECORD_TYPE__HOSPITAL_TROOP = 14,
    EN_USER_INFO_RECORD_TYPE__ALTER_TROOP = 15,
    EN_USER_INFO_RECORD_TYPE__VIP_POINT = 16,
    EN_USER_INFO_RECORD_TYPE__LORD_EXP = 17,
    EN_USER_INFO_RECORD_TYPE__TIME_ITEM_VALUE = 18,
    EN_USER_INFO_RECORD_TYPE__BUILD_FORCE = 19,
    EN_USER_INFO_RECORD_TYPE__RESEARCH_FORCE = 20,
    EN_USER_INFO_RECORD_TYPE__TRAIN_FORCE = 21,
    EN_USER_INFO_RECORD_TYPE__HERO_FORCE = 22,
    EN_USER_INFO_RECORD_TYPE__END,
};


enum EUserAddInfoRecordType
{
    EN_USER_ADD_INFO_RECORD_TYPE__ATTACKED_TYPE = 0,                        // [0, "city_type"]
    EN_USER_ADD_INFO_RECORD__TYPE__END,
};



enum EExDataUserType
{
    EN_EX_DATA_USER_TYPE_SOURCE = 0,
    EN_EX_DATA_USER_TYPE_TARGET = 1,       
};

enum EExDataType
{
    EN_EX_DATA_TYPE_RAW = 0,
    EN_EX_DATA_TYPE_NEW = 1,       
};

struct SActionInfo
{
    TINT64 m_ddwActionId;
    TINT64 m_ddwSecClass;
    TINT64 m_ddwOrginCostTime;
    TINT64 m_ddwRealCostTime;

    TVOID Reset()
    {
        m_ddwActionId = 0;
        m_ddwSecClass = 0;
        m_ddwOrginCostTime = 0;
        m_ddwRealCostTime = 0;
    } 
    
    TVOID SetValue(TINT64 ddwActionId, TINT64 ddwSecClass, TINT64 ddwOrginCostTime, TINT64 ddwRealCostTime)
    {
        m_ddwActionId = ddwActionId;
        m_ddwSecClass = ddwSecClass;
        m_ddwOrginCostTime = ddwOrginCostTime;
        m_ddwRealCostTime = ddwRealCostTime;
    }
};


struct SGameEvaluateData
{
    TINT64 m_ddwSid;
    TINT64 m_ddwUid;
    string m_strIdfa;
    string m_strCmd;
    TINT32 m_dwCurTimeStamp;
    TINT64 m_addwCastleLv[2];
    TINT64 m_ddwLordLv;
    TINT64 m_ddwForce;
    TINT64 m_ddwBuildForce;
    TINT64 m_ddwResearchForce;
    TINT64 m_ddwTrainForce;
    TINT64 m_ddwHeroForce;
    TINT64 m_ddwEquipForce;
    map<TINT64, TINT64> m_stResItemValueMap;
    map<TINT64, TINT64> m_stTimeItemValueMap;
    TINT64 m_addwRes[EN_RESOURCE_TYPE__END];
    TINT64 m_ddwGemCostTotal;
    TINT64 m_ddwGemBuyTotal;
    TINT64 m_ddwConnTime;
    TINT64 m_ddwPacketSize;
    TINT64 m_ddwNetworkDelay;
    TINT64 m_addwF[F_SIZE];
    TINT64 m_ddwEventSeq;
    TINT64 m_ddwBuildSpeedBuff;
    TINT64 m_ddwSearchSpeedBuff;
    TINT64 m_ddwTrainTroopSpeedBuff;
    TINT64 m_ddwTrainFortSpeedBuff;
    TINT64 m_ddwComposEquipSepeedBuff;
    TINT64 m_ddwCompEquipRedResBuff;
    TINT64 m_ddwTrainTroopRedResBuff;
    TINT64 m_ddwCreateTime;
    string m_strPlatform;
    vector<TINT64> m_stTroopAbilityVector;
    vector<TINT64> m_stFortAbilityVector;
    vector<SActionInfo> m_stOwnActionInfoNoMatchVector;
    TINT64 m_ddwAid;
    TINT64 m_ddwAlGiftLv;
    TINT64 m_ddwAForce;
    TINT64 m_ddwGemCur;    

    TVOID Reset()
    {
        m_ddwSid = 0;
        m_ddwUid = 0;
        m_strIdfa = "";
        m_strCmd = "";
        m_dwCurTimeStamp = 0;
        memset((TCHAR *)m_addwCastleLv, 0, sizeof(TINT64) * 2);
        m_ddwLordLv = 0;
        m_ddwForce = 0;
        m_ddwBuildForce = 0;
        m_ddwResearchForce = 0;
        m_ddwTrainForce = 0;
        m_ddwHeroForce = 0;
        m_ddwEquipForce = 0;
        m_stResItemValueMap.clear();
        m_stTimeItemValueMap.clear();
        memset((TCHAR *)m_addwRes, 0, sizeof(TINT64) * EN_RESOURCE_TYPE__END);
        m_ddwGemCostTotal = 0;
        m_ddwGemBuyTotal = 0;
        m_ddwConnTime = 0;
        m_ddwPacketSize = 0;
        m_ddwNetworkDelay = 0;
        memset((TCHAR *)m_addwF, 0, sizeof(TINT64) * F_SIZE);
        m_ddwEventSeq = 0;
        m_ddwBuildSpeedBuff = 0;
        m_ddwSearchSpeedBuff = 0;
        m_ddwTrainTroopSpeedBuff = 0;
        m_ddwTrainFortSpeedBuff = 0;
        m_ddwComposEquipSepeedBuff = 0;
        m_ddwCompEquipRedResBuff = 0;
        m_ddwTrainTroopRedResBuff = 0;
        m_ddwCreateTime = 0;
        m_strPlatform = "";
        m_stTroopAbilityVector.clear();
        m_stFortAbilityVector.clear();
        m_stOwnActionInfoNoMatchVector.clear();
        m_ddwAid = 0;
        m_ddwAlGiftLv = 0;
        m_ddwAForce = 0;
        m_ddwGemCur = 0;
    }
};

struct SBuildInfo
{
    TINT64 m_ddwBuildId;
    TINT64 m_ddwBuildLevel;

    TVOID Reset()
    {
        m_ddwBuildId = 0;
        m_ddwBuildLevel = 0;
    } 
    
    TVOID SetValue(TINT64 ddwBuildId, TINT64 ddwBuildLevel)
    {
        m_ddwBuildId = ddwBuildId;
        m_ddwBuildLevel = ddwBuildLevel;
    }
};

struct SGameEvaluateExData
{
    map<TINT64, TINT64> m_astExDataMap[EN_USER_INFO_RECORD_TYPE__END];
    map<TINT64, SBuildInfo> m_astExBuildDataMap;
    TVOID Reset()
    {        
        for(TINT32 dwIdx = 0; dwIdx < EN_USER_INFO_RECORD_TYPE__END; ++dwIdx)
        {
            m_astExDataMap[dwIdx].clear();
        }
        m_astExBuildDataMap.clear();
    }
};

struct SGameEvaluateAddData
{
    map<TINT64, string> m_astAddDataMap;
    TVOID Reset()
    {        
        m_astAddDataMap.clear();
    }
};



struct SReqInfo
{
    TINT64 m_ddwSid;
    TINT32 m_ddwFinalPackLength;
    TUINT32 m_udwCid;
    string m_strCommand;
    string m_strIdfa;
    string m_astrKey[MAX_REQ_PARAM_KEY_NUM];
    TINT64 m_ddwReqCost;

    TVOID Reset()
    {
        m_ddwSid = 0;
        m_ddwFinalPackLength = 0;
        m_udwCid = 0;
        m_strCommand = "";
        m_strIdfa = "";
		for(TUINT32 udwIdx = 0; udwIdx < MAX_REQ_PARAM_KEY_NUM; udwIdx++)
		{
			m_astrKey[udwIdx] = "";
		}
        m_ddwReqCost = 0;
    }  

    TVOID SetValue(TINT64 ddwSid, TINT32 dwFinalPackLength, TUINT32 udwCityId, string strCmd, string strIdfa, TCHAR szKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN], TINT64 ddwReqCost)
    {        
        m_ddwSid = ddwSid;
        m_ddwFinalPackLength = dwFinalPackLength;
        m_udwCid = udwCityId;
        m_strCommand = strCmd;
        m_strIdfa = strIdfa;
		for(TUINT32 udwIdx = 0; udwIdx < MAX_REQ_PARAM_KEY_NUM; udwIdx++)
		{
		    m_astrKey[udwIdx] = szKey[udwIdx];
		}
        m_ddwReqCost = ddwReqCost;
    }
};



#endif


