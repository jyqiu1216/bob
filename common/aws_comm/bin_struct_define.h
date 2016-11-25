#ifndef _BIN_STRUCT_DEFINE_H_
#define _BIN_STRUCT_DEFINE_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include <bitset>

//#define CHAR_BIT  (8)
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)


#pragma pack(1)

template<class T, unsigned int N>
class CircleQueue
{
    unsigned int udwHead;
    unsigned int udwTail;
    T   astNode[N];
public:
    CircleQueue()
    {
        Reset();
    }
    void Reset()
    {
        udwHead = udwTail = 0; //��ն���
    }
    T* AddNode()
    {
        udwHead = udwHead%N;
        if((udwTail + 1) % N == udwHead) //������
        {
            //udwHead++; //ɾ����ͷ���
            udwHead = (udwHead + 1) % N;
        }
        T* pNode = &astNode[udwTail];
        udwTail = (udwTail + 1) % N;
        return pNode;
    }
    T* FindNode(TUINT64 uddwId)
    {
        udwHead = udwHead%N;
        for(unsigned int idx = udwHead; idx != udwTail; idx = (idx + 1) % N)
        {
            if(astNode[idx].Match(uddwId))
            {
                return &astNode[idx];
            }
        }
        return  NULL;
    }
};

//the T must be copyable, the N must be greater than 0
template<class T, unsigned int N>
class LoopStack
{
public:
    TINT32 dwTop;
    TUINT32 udwSize;
    T astNode[N];

    LoopStack() : dwTop(-1), udwSize(0)
    {
    }
    void Reset()
    {
        dwTop = -1;
        udwSize = 0;
    }
    T* Push(const T& node)//in C++98 and C++03 using reference is better?
    {
        astNode[(dwTop + 1) % N] = node;//copy, may throw?
        dwTop = (dwTop + 1) % N;
        udwSize = udwSize + 1 > N ? N : udwSize + 1;
        return &astNode[dwTop];
    }
    T* Top()
    {
        if(dwTop >= 0)
        {
            return &astNode[dwTop];
        }
        return NULL;
    }
    T* Pop()
    {
        if(udwSize > 0)
        {
            udwSize--;
            dwTop = (dwTop + N - 1) % N;
            return &astNode[(dwTop + 1) % N];
        }
        return NULL;
    }
    T* Find(TINT64 ddwId)
    {
        for(TUINT32 udwIdx = dwTop, udwCount = 1; udwCount <= udwSize; ++udwCount)
        {
            if(astNode[udwIdx].Match(ddwId))
            {
                return &astNode[udwIdx];
            }
            udwIdx = (udwIdx + N - 1) % N;
        }
        return NULL;
    }
    TBOOL Empty()
    {
        return (udwSize == 0);
    }
};

// account
struct SBitFlag
{
    char m_bitFlag[BITNSLOTS(MAX_GUIDE_FLAG_NUM)];

    SBitFlag()
    {
        memset(m_bitFlag, 0, BITNSLOTS(MAX_GUIDE_FLAG_NUM));
    }

    TVOID Reset()
    {
        memset(m_bitFlag, 0, BITNSLOTS(MAX_GUIDE_FLAG_NUM));
    }
};

struct SMailFlag
{
    char m_bitFlag[BITNSLOTS(MAX_MAIL_FLAG_NUM)];

    SMailFlag()
    {
        memset(m_bitFlag, 0, BITNSLOTS(MAX_MAIL_FLAG_NUM));
    }

    TVOID Reset()
    {
        memset(m_bitFlag, 0, BITNSLOTS(MAX_MAIL_FLAG_NUM));
    }
};

// ������Դ
struct SCommonResource
{
    TINT64 m_addwNum[EN_RESOURCE_TYPE__END];

    SCommonResource()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TINT64& operator[](unsigned int udwIdx)
    {
        assert(udwIdx < EN_RESOURCE_TYPE__END);
        return m_addwNum[udwIdx];
    }
};

struct SCommonTroop
{
    TINT64 m_addwNum[EN_TROOP_TYPE__END];

    SCommonTroop()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TINT64& operator[](unsigned int udwIdx)
    {
        assert(udwIdx < EN_TROOP_TYPE__END);
        return m_addwNum[udwIdx];
    }
};

// fort
struct SCommonFort
{
    TINT64 m_addwNum[EN_FORT_TYPE__END];

    SCommonFort()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TINT64& operator[](unsigned int udwIdx)
    {
        assert(udwIdx < EN_FORT_TYPE__END);
        return m_addwNum[udwIdx];
    }
};

struct SCommonResearch
{
    TINT64 m_addwLevel[EN_RESEARCH_TYPE__LIMIT];

    SCommonResearch()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TINT64& operator[](unsigned int udwIdx)
    {
        assert(udwIdx < EN_RESEARCH_TYPE__LIMIT);
        return m_addwLevel[udwIdx];
    }
};

struct SSkill
{
    TINT64 m_addwLevel[EN_SKILL_TYPE__LIMIT];

    SSkill()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TINT64& operator[](unsigned int udwIdx)
    {
        assert(udwIdx < EN_SKILL_TYPE__LIMIT);
        return m_addwLevel[udwIdx];
    }
};

struct SMarchDragonInfo
{
    TINT64 m_ddwLevel;
    TINT64 m_ddwIconId;
    TINT64 m_ddwExpInc;
    TINT64 m_ddwCaptured;
    TCHAR m_szName[MAX_TABLE_NAME_LEN];

    SMarchDragonInfo()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
        m_ddwCaptured = -1;
    }
};

struct SActionBuildingParam
{
    TINT64 m_ddwPos;
    TINT64  m_ddwType; // building id or research id
    TINT64  m_ddwTargetLevel;
    TINT64 m_ddwExp;
    TCHAR m_szUserName[MAX_TABLE_NAME_LEN];// չʾ�� llt add 20131011
    ///////////////////////
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID SetValue(TINT64 ddwPos, TINT64 ddwType, TINT64 ddwLevel, TINT64 ddwExp, TCHAR* pszUserName = "");
};

struct SActionTrainParam
{
    TINT64  m_ddwType; // troop or fort type
    TINT64 m_ddwNum;
    TINT64 m_ddwExp;
    TCHAR m_szUserName[MAX_TABLE_NAME_LEN];// չʾ��
    ///////////////////////
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID SetValue(TINT64 ddwType, TINT64 ddwNum, TINT64 ddwExp, TCHAR* pszUserName = "");
};

struct SMarchKnightInfo
{
    TINT64 ddwId;
    TINT64 ddwLevel;
    TINT64 ddwExpAdd;

    SMarchKnightInfo()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
        ddwId = -1;
    }
};

struct SActionMarchParam
{
    // �����û�
    TINT64 m_ddwSourceUserId;
    TCHAR m_szSourceUserName[MAX_TABLE_NAME_LEN];// չʾ��
    TINT64 m_ddwSourceCityId;
    TCHAR m_szSourceCityName[MAX_TABLE_NAME_LEN];//չʾ��
    TINT64 m_ddwSourceAlliance;
    TCHAR m_szSourceAlliance[MAX_TABLE_NAME_LEN];
    TCHAR m_szSourceAlNick[MAX_TABLE_NAME_LEN];

    // Ŀ���û�
    TINT64 m_ddwTargetUserId;
    TCHAR m_szTargetUserName[MAX_TABLE_NAME_LEN];
    TINT64 m_ddwTargetCityId;
    TCHAR m_szTargetCityName[MAX_TABLE_NAME_LEN];
    TINT64 m_ddwTargetAlliance;
    TCHAR m_szTargetAlliance[MAX_TABLE_NAME_LEN];
    TCHAR m_szTargetAlNick[MAX_TABLE_NAME_LEN];

    // Ŀ������
    TINT64    m_ddwTargetType;
    // Ŀ��ȼ�
    TINT64    m_ddwTargetLevel;

    // ������Ϣ
    SCommonTroop m_stTroop;
    SCommonTroop m_stTroopRaw;

    SMarchDragonInfo m_stDragon;

    // ��Դ
    SCommonResource m_stResource;

    // source scout level
    TINT64 m_ddwScoutLevel;

    // loading rate
    TINT64 m_ddwLoadRate; // ÿСʱ��װ����
    TINT64 m_ddwLoadTime;
    TINT64 m_ddwLoadResTotalNum;
    TINT64 m_ddwBeginLoadTime;
    TINT64 m_ddwTotalLoadTime;

    //�о�ʱ��
    TINT64 m_ddwMarchingTime;

    TINT64 m_ddwLoadGem;

    TINT64 m_ddwPrepareTime;

    TINT64 m_ddwSide;
    TINT64 m_ddwForce;
    TINT64 m_ddwTroopNum;

    SMarchKnightInfo m_stKnight;

    TINT64 m_ddwCaptureDragonFlag;
    TINT64 m_ddwWinFlag;

    SActionMarchParam()
    {
        Reset();
    }
    ///////////////////////
    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
        m_stDragon.Reset();
    }
};

struct SActionItemParam
{
    TINT64 m_ddwBufferId;
    TINT64 m_ddwNum;
    TINT64 m_ddwTime;
    TINT64 m_ddwItemId;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    TVOID SetValue(TINT64 ddwBufferId, TINT64 ddwNum, TINT64 ddwTime, TINT64 ddwItemId = 0);
};

struct SActionEquipParam
{
    TUINT64 m_uddwId;
    TINT64 m_ddwEType;
    TINT64 m_ddwIsMistery; // TODO �����ݺ�ɾ��
    TCHAR m_szUserName[MAX_TABLE_NAME_LEN];
    TCHAR m_szMaterialIdList[64];// չʾ�� 
    TINT64 m_ddwScrollId;
    TCHAR m_szPartsIdList[128];// չʾ�� // TODO �����ݺ�ɾ��
    TINT64 m_ddwLevel;
    TINT64 m_ddwGoldCost;
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    TVOID SetValue(TUINT64 uddwId, TINT64 ddwEid, TINT64 ddwScrollId, TINT64 ddwLevel, TINT64 ddwGoldCost, TCHAR* pszMaterialIdList = "", TCHAR* pszUserName = "");
};

struct SActionAttackMoveParam
{
    TINT64 m_ddwCityId;
    TINT64 m_ddwSpecailMoveFlag;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    TVOID SetValue(TINT64 ddwCityId, TINT64 ddwSpecailMoveFlag = 0)
    {
        m_ddwCityId = ddwCityId;
        m_ddwSpecailMoveFlag = ddwSpecailMoveFlag;
    }
};

struct SPrisonParam
{
    SMarchDragonInfo stDragon;
    TINT64 ddwJoinTimeStamp;
    TINT64 ddwReleaseWait;
    TINT64 ddwExcuteWait;
    TINT64 ddwEscortActionId;
    TCHAR szSourceUserName[MAX_TABLE_NAME_LEN];// չʾ��
    TCHAR szTargetUserName[MAX_TABLE_NAME_LEN];// չʾ��
    TINT64 ddwResult;
    TCHAR szTargetCityName[MAX_TABLE_NAME_LEN];// չʾ��
    TCHAR szSourceAlNick[MAX_TABLE_NAME_LEN];// չʾ��
    TCHAR szTargetAlNick[MAX_TABLE_NAME_LEN];// չʾ��
    SPrisonParam()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
        stDragon.Reset();
    }
};

struct SReinforceResult
{
    TINT64 ddwReinforceDefendWin; //���سɹ�
    TINT64 ddwReinforceDefendFail;//����ʧ��

    TINT64 ddwReinforceAtkWin; //���سɹ�
    TINT64 ddwReinforceAtkFail;//����ʧ��

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

union UActionParam
{
    SActionBuildingParam m_stBuilding;
    SActionTrainParam m_stTrain;
    SActionItemParam m_stItem;
    SActionEquipParam m_stEquip;
    SActionAttackMoveParam m_stAttackMove;
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    UActionParam()
    {
        memset(this, 0, sizeof(*this));
    }
};

// al_help
struct SAlHelpNode
{
    TUINT64 uddwTaskId;
    TINT64 ddwEndTime; //�������ʱ��(>=����ʵ�ʽ�����ʱ��)
    SAlHelpNode()
    {
        memset(this, 0, sizeof(*this));
    }
    bool Match(TUINT64 uddwId)
    {
        return uddwTaskId == uddwId;
    }
};
typedef CircleQueue<SAlHelpNode, MAX_AL_HELP_NODE_NUM>  SAlHelpList;

struct SAlStoreItem
{
    TINT64 ddwItemId;
    TINT64 ddwNum;
    TINT64 ddwStar; //�Ǽ�(���Ѵ���)
    TINT64 ddwClearTime; //����/������������Ǽ�¼��ʱ��
};

struct SMarkItem //���Ǽ�¼
{
    TUINT32 ddwItemId;
    TUINT32 ddwMarkTime; //����/������������Ǽ�¼��ʱ��
};

//mail
struct SMailOperateNode
{
    TINT64 ddwMid;
    TINT64 ddwStatus;
    SMailOperateNode()
    {
        memset(this, 0, sizeof(*this));
    }
    bool Match(TINT64 ddwId)
    {
        return ddwMid == ddwId;
    }
};

typedef LoopStack<SMailOperateNode, MAX_MAIL_OPERATE_NODE_NUM> SMailOperateList;

struct SBuffInfo
{
    TINT64 ddwBuffId;
    TINT64 ddwBuffNum;
    TINT64 ddwBuffExpiredTime; //Ϊ0��ʾ��ʧЧ
    SBuffInfo()
    {
        memset(this, 0, sizeof(*this));
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

// �����ڵ�
struct SCityBuildingNode
{
    TINT64 m_ddwPos;
    TINT64 m_ddwType;
    TINT64 m_ddwLevel;

    TVOID SetKey(TINT64 ddwPos, TINT64 ddwType, TINT64 ddwLevel)
    {
        m_ddwPos = ddwPos;
        m_ddwType = ddwType;
        m_ddwLevel = ddwLevel;
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SOneGlobalRes
{
    TINT64 ddwType; //����
    TINT64 ddwId; // id
    TINT64 ddwNum; //

    SOneGlobalRes()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID SetValue(TINT64 Type, TINT64 Id, TINT64 Num)
    {
        ddwType = Type;
        ddwId = Id;
        ddwNum = Num;
    }
};

struct SGlobalRes
{
    TINT64 ddwTotalNum;                         // ��Ʒ��
    SOneGlobalRes aRewardList[MAX_REWARD_ITEM_NUM];    // 

    SGlobalRes()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    SOneGlobalRes& operator[](TUINT32 udwNum)
    {
        assert(udwNum < MAX_REWARD_ITEM_NUM);

        return aRewardList[udwNum];

    }
};


///////////////
struct SQuestComm
{
    TINT64 m_ddwBTime;
    TINT64 m_ddwCTime; //�����Ҫʱ��

    TINT64 m_ddwStatus;
    TINT64 m_ddwLv;

    SGlobalRes m_stReward;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    SQuestComm()
    {
        Reset();
    }
};

struct SQuestNode
{
    TINT64 m_ddwRTime; //ˢ��ʱ��
    TINT64 m_ddwCollectNum;    //���ֶ�������������

    TINT64 m_ddwQuestNum;
    SQuestComm m_stQuestCom[MAX_TIME_QUEST_NUM];

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    SQuestNode()
    {
        Reset();
    }

};

//quest
struct STopQuest
{
    char m_bitQuest[BITNSLOTS(EN_TOP_QUEST_NUM_LIMIT)];

    STopQuest()
    {
        memset(m_bitQuest, 0, BITNSLOTS(EN_TOP_QUEST_NUM_LIMIT));
    }
    TVOID Reset()
    {
        memset(m_bitQuest, 0, BITNSLOTS(EN_TOP_QUEST_NUM_LIMIT));
    }
};

struct SLevelQuest
{
    char m_bitLevel[BITNSLOTS(MAX_LEVEL_LIMIT)];

    SLevelQuest()
    {
        memset(m_bitLevel, 0, BITNSLOTS(MAX_LEVEL_LIMIT));
    }
    TVOID Reset()
    {
        memset(m_bitLevel, 0, BITNSLOTS(MAX_LEVEL_LIMIT));
    }
};
/////////////task/////////////
struct STaskCondiStatusNode
{
    TINT64 m_ddwTaskType;
    TINT64 m_ddwId;
    TUINT64 m_uddwNum;
    TINT64 m_ddwValue;
    TBOOL m_bIsStand;
    TUINT64 m_uddwCurrValue;
    TUINT64 m_uddwBeginValue;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STaskNodeNow
{
    TINT64 m_ddwId;
    TINT64 m_ddwUpdateTime;
    TBOOL m_bIsNew;
    TBOOL m_bIsProgress;
    TINT64 m_ddwEndTime;
    STaskCondiStatusNode astFinishCondition[MAX_TASK_CONDITION_LIMIT];

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STaskFinsh
{
    char m_bitTask[BITNSLOTS(EN_TASK_NUM_LIMIT)];
    TVOID Reset()
    {
        memset(m_bitTask, 0, BITNSLOTS(EN_TASK_NUM_LIMIT));
    }

};

struct SPersonGuideFinish
{
    char m_bitGuide[BITNSLOTS(EN_PERSON_GUIDE_NUM_LIMIT)];
    TVOID Reset()
    {
        memset(m_bitGuide, 0, BITNSLOTS(EN_PERSON_GUIDE_NUM_LIMIT));
    }
};


/////////////////////////////////////
//report
struct SReportUserInfo
{
    // �û�
    TINT64 m_ddwUserId;
    TCHAR m_szUserName[MAX_TABLE_NAME_LEN];// չʾ��

    // ����
    TINT64 m_ddwPos;
    TINT64 m_ddwPosType;
    TINT64 m_ddwPosLevel;
    TINT64 m_ddwOwnedCityId;
    TCHAR m_szOwnedCityName[MAX_TABLE_NAME_LEN];
    TINT64 m_ddwAlId;
    TCHAR m_szAllianceName[MAX_TABLE_NAME_LEN];
    TINT64 m_ddwSvrWildType;
    TINT64 m_ddwSid;
    TCHAR m_szSvrName[MAX_TABLE_NAME_LEN * 2];
};

struct SPlayerItem
{
    TINT64 m_ddwItemId;
    TINT64 m_ddwItemNum;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SNotictaskFlag
{
    TCHAR m_bitFlag[BITNSLOTS(LIMIT_NOTIC_TASK_FLAG_NUM)];

    SNotictaskFlag()
    {
        memset(m_bitFlag, 0, BITNSLOTS(LIMIT_NOTIC_TASK_FLAG_NUM));
    }

    TVOID Reset()
    {
        memset(m_bitFlag, 0, BITNSLOTS(LIMIT_NOTIC_TASK_FLAG_NUM));
    }
};


struct BuildingPoint
{
    TINT64 x;
    TINT64 y;
};


struct SAttackTimesInfo
{
    TINT64 m_ddwId;
    TINT64 m_ddwTimes; // �ڼ�������
    TINT64 m_ddwAttackTime; // ��������ʱ��

    SAttackTimesInfo()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SRallySlot
{
    TINT64 ddwUid;
    TINT64 ddwMarchId;
    TCHAR szUserName[MAX_TABLE_NAME_LEN];
    TBOOL bPrivate;

    SRallySlot()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SRallyForce
{
    TINT64 ddwReinforceNum;
    TINT64 ddwReinforceForce;
    TINT64 ddwTotalNum;
    TINT64 ddwTotalForce;
    TINT64 ddwReinforceTroopLimit;

    SRallyForce()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};


struct SCrystal
{
    TINT64 m_addwNum[MAX_CRYSTAL_IN_EQUIPMENT];

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
    TINT64& operator[](unsigned int udwIdx)
    {
        assert(udwIdx < MAX_CRYSTAL_IN_EQUIPMENT);
        return m_addwNum[udwIdx];
    }
};

struct SWarStatistics  //ս��ͳ��
{
    TINT64 ddwAtkWin;              //����ʤ���ܴ�������������֡�ս����Ч���������ͬ�� //Attacks Won
    TINT64 ddwAtkFail;             //����ʧ���ܴ���                                   //Attacks Lost
    TINT64 ddwDfnWin;              //����ʤ���ܴ���                                   //Defense Won
    TINT64 ddwDfnFail;             //����ʧ���ܴ���                                   //Defense Lost
    TINT64 ddwScoutNum;            //�����������������ҡ����Σ�                    //Players Scouted
    TINT64 ddwDamageTroopNum;      //�Ե����������������������ҡ����Σ����Ͱ���������//Troops Killed
    TINT64 ddwDamageFortNum;       //����������ҳ���ʱ���Ե������Ƿ����������������� //Forts Destroyed
    TINT64 ddwMyTroopDamagedNum;   //����ı�����                                        //Troops Lost
    TINT64 ddwMyFortDamagedNum;    //����ĳǷ�����                                       //Forts Lost
    TINT64 ddwHosTroopNum;         //ҽԺ���ɹ����˱�����                                //Troops Hospitalized
    TINT64 ddwHurtEnemyTroopNum;   //�Ե�����˱�������                                 //Enemies Hospitalized
    TINT64 ddwDestroyCityNum;      //����������ҳ��еĴ���                               //Cities Destroyed
    TINT64 ddwMyCityDestroyedNum;  //���������                                             //My City Destroyed
    TINT64 ddwKillMonsterNum;      //�ɹ���ɱ����Ĵ���                                     //Monster Killed
    TINT64 ddwForceKilled;         //ɱ�б��ͳǷ�����forceֵ�����������ҡ����Σ�������Ӣ�۴�֣��������� //Force Killed

    SWarStatistics()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SDragonStatistics  //Ӣ��ͳ��
{
    TINT64 ddwCaptureDragonNum;      //ץ���������Ĵ���
    TINT64 ddwExecuteDragonNum;      //�����������Ĵ���
    TINT64 ddwEscapeDragonNum;       //���ܱ������Ĵ���
    TINT64 ddwMyDragonCapturedNum;   //�Լ�����ץ����
    TINT64 ddwMyDragonExecutedNum;   //�Լ�������������
    TINT64 ddwMyDragonEscapedNum;    //�Լ������ܵĴ���

    SDragonStatistics()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STitleInfo
{
    TINT64 ddwOid;
    TINT64 ddwTitleId;
    TINT64 ddwAid;
    TCHAR szAlnick[MAX_TABLE_NAME_LEN];
    TCHAR szUserName[MAX_TABLE_NAME_LEN];

    STitleInfo()
    {
        Reset();
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STaskPriority
{
    TINT64 ddwTaskId; // ��Ʒ��
    TBOOL bCanStart;
    TINT32  dwPriority;
    STaskNodeNow  *pstTask;

    TVOID Reset()
    {
        ddwTaskId = 0;
        bCanStart = FALSE;
        dwPriority = 0;
        pstTask = NULL;
    }
};


struct SEventPriority
{
    TUINT64 uddwEventId;
    TINT64 ddwEventType;
    TINT64 ddwRewardType;
    TINT64 ddwKey;
    TINT64 ddwIdx; //��������idx

    TVOID Reset()
    {
        uddwEventId = 0;
        ddwEventType = 0;
        ddwRewardType = 0;
        ddwKey = 0;
        ddwIdx = 0;
    }
};

struct STradeInfo
{
    TINT64 m_ddwRewardNum;
    TINT64 m_ddwReceptionRewardNum;
    TINT64 m_ddwTargetUid;
    TINT64 m_ddwMarchTime;
    TINT64 m_ddwStayTime;
    TINT64 m_ddwStatus;

    TINT64 m_ddwTradeBeginTime;
    TINT64 m_ddwTradeEndTime;

    TINT64 m_ddwType;

    TINT64 m_ddwTid;

    TCHAR m_TargetCityName[MAX_TABLE_NAME_LEN];

    STradeInfo()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STradeList
{
    TINT64 m_ddwNum;
    STradeInfo m_astInfo[MAX_TRADE_NUM];

    STradeList()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

//TODO
struct SMysteryItem
{
    TINT64 m_ddwRewardId;
    TINT64 m_ddwStatus;

    TINT64 m_ddwType;
    TINT64 m_ddwId;
    TINT64 m_ddwNum;
    TINT64 m_ddwPrice;
    SMysteryItem()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SMysteryItemList
{
    TINT64 m_ddwAppearFlag;
    TINT64 m_ddwNum;
    SMysteryItem m_astItem[MAX_MYSTERY_PACKAGE_ITEM_NUM];
    SMysteryItemList()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SMysteryStorePackage
{
    TINT64 m_ddwTotalPackageNum;
    TINT64 m_ddwCurPackageIdx; //m_udwCurPackageIdx == m_udwTotalPackageNum��ʾ���а����������....
    TINT64 m_ddwHasPassNum;

    SMysteryItemList m_astPackage[MAX_MYSTERY_PACKAGE_NUM];

    SMysteryStorePackage()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SChestLottery
{
    TINT64 ddwChestId;
    TINT64 ddwSeq;
    TINT64 ddwUsed;
    TCHAR szTmp[32];

    SChestLottery()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SDragonInfo
{
    TINT64 ddwId;

    SDragonInfo()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SKnightInfo
{
    TINT64 ddwStatus;
    TINT64 ddwPos;
    TINT64 ddwExp;
    TINT64 ddwTid;

    SKnightInfo()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SMonsterInfo
{
    TINT64 ddwLv;
    TINT64 ddwType;
    TINT64 ddwDeadFlag;
    TINT64 ddwHpLose;
    TINT64 ddwSpecailHitType;
    TINT64 ddwExpGet;
    TINT64 ddwRawHp;
    TINT64 ddwLead;
    TINT64 ddwChallengerId;
    TINT64 ddwRid;

    SMonsterInfo()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SMonsterHit
{
    TINT64 addwNum[MAX_MONSTER_LV + 1];

    SMonsterHit()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SLeaderMonsterGen
{
    TINT64 addwNum[MAX_MONSTER_LV + 1];

    SLeaderMonsterGen()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SLeaderMonsterKill
{
    TINT64 addwNum[MAX_MONSTER_LV + 1];

    SLeaderMonsterKill()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STrialMonster
{
    TUINT32 udwAtkTime;
    SOneGlobalRes aRewardList[MAX_TRIAL_ATK_TIME];

    STrialMonster()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STrialLuckyBagNormal
{
    SOneGlobalRes aRewardList[MAX_TRIAL_LUCKY_BAG_NORMAL_NUM];

    STrialLuckyBagNormal()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct STrialLuckyBagRage
{
    TUINT32 udwOpenNum;
    SOneGlobalRes aRewardList[MAX_TRIAL_LUCKY_BAG_RAGE_NUM];

    STrialLuckyBagRage()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SFinishGuideList
{
    TUINT32 udwNum;
    TINT64 addwFinishGuide[MAX_FINISH_GUIDE_LIST_NUM];

    SFinishGuideList()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SReportBuffer
{
    SBuffInfo astDragonBuff[MAX_REPORT_BUFFER_NUM];
    TUINT32 udwDragonBuffNum;
    SBuffInfo astKnightBuff[MAX_REPORT_BUFFER_NUM];
    TUINT32 udwKnightBuffNum;
    SBuffInfo astResearchBuff[MAX_REPORT_BUFFER_NUM];
    TUINT32 udwResearchBuffNum;

    SReportBuffer()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SReinforceLimit
{
    TUINT32 udwMarchNum;
    TUINT32 udwMarchLimit;
    TUINT32 udwTroopNum;
    TUINT32 udwTroopLimit;
    TINT64 ddwTroopForce;

    SReinforceLimit()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

#pragma pack()
#endif
