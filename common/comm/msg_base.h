#ifndef _MSG_BASE_
#define _MSG_BASE_

#include "base/common/wtse_std_header.h"
#include <sstream>
#include <string>
#include "aws_table_include.h"
#include "jsoncpp/json/json.h"

using std::ostringstream;
using std::string;

enum ENewNoticType
{
    EN_NOTIC_TYPE__UNKNOWN = 0,
    EN_NOTIC_TYPE__TIMER,
    EN_NOTIC_TYPE__WAR,
    EN_NOTIC_TYPE__ALLIANCE,
    EN_NOTIC_TYPE__SOCIAL,
    EN_NOTIC_TYPE__GIFT,
    EN_NOTIC_TYPE__BUFF,
    EN_NOTIC_TYPE__SPECIAL,

    EN_NOTIC_TYPE__END,
};

enum ENewNoticSwitchType
{
    EN_NOTIC_SWITCH_TYPE__CLOSE = 0,
    EN_NOTIC_SWITCH_TYPE__TEXT = 1,
    EN_NOTIC_SWITCH_TYPE__TEXT_SOUND = 2,
};

enum ENotificationId
{
    EN_NOTI_ID__TRAIN_SUCC = 0,
    EN_NOTI_ID__HEAL_SUCC,
    EN_NOTI_ID__LOAD_SUCC,
    EN_NOTI_ID__QUEST_COMPLETE,
    EN_NOTI_ID__FORGE_EQUIP_SUCC,
    EN_NOTI_ID__BUILDING_UPGRADE_OK,
    EN_NOTI_ID__RESEARCH_UPGRADE_OK,
    EN_NOTI_ID__BUILDING_REMOVE_OK,
    EN_NOTI_ID__ATTACK_SUCC,
    EN_NOTI_ID__ATTACK_FAIL,
    EN_NOTI_ID__DEFENSE_FAIL,
    EN_NOTI_ID__DEFENSE_SUCC,
    EN_NOTI_ID__COMING_ATTACK_7,//CHANGE, ԭEN_NOTI_ID__COMING_ATTACK
    EN_NOTI_ID__SCOUT_SUCC,
    EN_NOTI_ID__BE_SCOUT,
    EN_NOTI_ID__PEACETIME_END,          //CHANGE,ok
    EN_NOTI_ID__NEW_PLAYER_END,         //CHANGE,ok
    EN_NOTI_ID__HERO_ATTACK_SUCC,
    EN_NOTI_ID__HERO_BE_RELEASED,
    EN_NOTI_ID__HERO_BE_KILLED,
    EN_NOTI_ID__HERO_BE_CAPTURED,
    EN_NOTI_ID__HERO_UNLOCKED,          //NEW, //TODO
    EN_NOTI_ID__HERO_ENERGY_RECOVERED,  //NEW, ok
    EN_NOTI_ID__COMING_ATTACK_1,        //CHANGE
    EN_NOTI_ID__COMING_ATTACK_5,        //CHANGE
    EN_NOTI_ID__OCCUPY_COMING_ATTACK_1, //CHANGE
    EN_NOTI_ID__OCCUPY_COMING_ATTACK_5, //CHANGE
    EN_NOTI_ID__OCCUPY_COMING_ATTACK_7, //CHANGE
    EN_NOTI_ID__BE_TRANSPORT_SUCC,
    EN_NOTI_ID__BE_REINFORCE_SUCC,
    
    EN_NOTI_ID__RALLYWAR_START,
    EN_NOTI_ID__RALLYWAR_DEFENSE,
    EN_NOTI_ID__RALLY_ATTACK_SUCC,
    EN_NOTI_ID__RALLY_ATTACK_FAIL,
    EN_NOTI_ID__RALLY_DEFENSE_SUCC,
    EN_NOTI_ID__RALLY_DEFENSE_FAIL,

    EN_NOTI_ID__THRONE_BE_CAPTURED,
    EN_NOTI_ID__PROVINCE_BE_CAPTURED,
    EN_NOTI_ID__OCCUPY_THRONE_SUCC,
    EN_NOTI_ID__OCCUPY_PROVINCE_SUCC,

    EN_NOTI_ID__AL_INVITE,
    EN_NOTI_ID__AL_REQUEST,
    EN_NOTI_ID__MAIL,
    EN_NOTI_ID__MYSTERY_GIFT,
    EN_NOTI_ID__AL_GIFT,
    EN_NOTI_ID__KNIGHT_BE_DISMISSED,    //NEW, ok
    EN_NOTI_ID__VIP_EXPIRED,            //NEW,ok
    EN_NOTI_ID__BUFF_EXPIRED,           //NEW,ok

    EN_NOTI_ID__FORT_TRAIN_SUCC = 49,
    EN_NOTI_ID__FORT_REPAIR_SUCC = 50,
    EN_NOTI_ID__CAMP_PROTECT_EXPIRE = 51,

    EN_NOTI_ID__IDOL_CAN_BE_ATTACKED = 52,
    EN_NOTI_ID__OCCUPY_IDOL = 53,
    EN_NOTI_ID__OCCUPY_THRONE = 54,
    EN_NOTI_ID__THRONE_BE_ROBED = 55,
    EN_NOTI_ID__THRONE_PEACE_TIME_END_IN_ONE_HOUR = 56,

    EN_NOTI_ID__SCOUT_BE_PREVENTED = 64,
    EN_NOTI_ID__PREVENT_SCOUT = 65,
    EN_NOTI_ID__END,

    //-------BOB DEL---------------------
    EN_NOTI_ID__TRANSPORT_SUCC,    
    EN_NOTI_ID__REINFORCE_SUCC,        
    EN_NOTI_ID__REVIVE_SUCC,//useless    
    EN_NOTI_ID__BUILDING_CAN_FREE,
    EN_NOTI_ID__RESEARCH_CAN_FREE,
    EN_NOTI_ID__REMOVE_CAN_FREE,    
    EN_NOTI_ID__HERO_LEVEL_UP, // ������Ҫ��������..һֱ��û��    
    EN_NOTI_ID__NEW_MONSTER,  // û��..��Ҫˢ�س���ȥ����...����...�ֲ���˲��ˢ��...����..��ô�������˷�����...
    EN_NOTI_ID__HERO_ATTACK_SUCC_AND_AL_GIFT,
    EN_NOTI_ID__NEW_AGE,    
    EN_NOTI_ID__COMBINE_EQUIP_SUCC,
    EN_NOTI_ID__NEW_EVENT,  // û��...��ô�������˷�����...
    EN_NOTI_ID__EVENT_REWARD,    
    EN_NOTI_ID__FORGE_CAN_FREE,
    EN_NOTI_ID__BONUTY_FINISH,    
    EN_NOTI_ID__HAVE_TRADE_COMING,
    EN_NOTI_ID__REINFORCE_THRONE_SUCC,
    EN_NOTI_ID__COLLECT_TAX,
    EN_NOTI_ID__PAY_TAX,
    EN_NOTI_ID__HERO_RETURN,
    EN_NOTI_ID__TROOP_RETURN,


};

struct SNoticInfo
{
    TINT32 m_dwNoticId;
    string m_strSName;
    string m_strTName;
    string m_strSPosX;
    string m_strSPosY;
    string m_strTPosX;
    string m_strTPosY;
    string m_strSCidx; 
    string m_strTCidx;
    string m_strCostTime;
    string m_strKeyName;
    string m_strValueName;
    string m_strLang;

    TVOID Reset()
    {
        m_dwNoticId = 0;
        m_strSName = "";
        m_strTName = "";
        m_strSPosX = "";
        m_strSPosY = "";
        m_strTPosX = "";
        m_strTPosY = "";
        m_strSCidx = "";
        m_strTCidx = "";
        m_strCostTime = "";
        m_strKeyName = "";
        m_strValueName = "";
        m_strLang = "";
    }

    TVOID SetValue(TINT32 dwNoticId, string strSName, string strTName,
                   TINT64 ddwSPos, TINT64 ddwTPos, 
                   TINT32 dwSCidx, TINT32 dwTCidx, 
                   TINT64 ddwCostTime, string strKeyName, TINT64 ddwValueName,
                   string strLang = "english")
    {
        ostringstream oss;

        m_dwNoticId = dwNoticId;
        
        m_strSName = strSName;
        m_strTName = strTName;

        oss.str("");
        oss << ddwSPos / MAP_X_Y_POS_COMPUTE_OFFSET;
        m_strSPosX = oss.str();
        
        oss.str("");
        oss << ddwSPos % MAP_X_Y_POS_COMPUTE_OFFSET;
        m_strSPosY = oss.str();
        
        oss.str("");
        oss << ddwTPos / MAP_X_Y_POS_COMPUTE_OFFSET;
        m_strTPosX = oss.str();
        
        oss.str("");
        oss << ddwTPos % MAP_X_Y_POS_COMPUTE_OFFSET;
        m_strTPosY = oss.str();
        
        oss.str("");
        oss << dwSCidx + 1;
        m_strSCidx = oss.str();
        
        oss.str("");
        oss << dwTCidx + 1;
        m_strTCidx = oss.str();
        
        m_strKeyName = strKeyName;
        
        oss.str("");
        oss << ddwValueName;
        m_strValueName = oss.str();
        

        m_strCostTime = SecondFormat(ddwCostTime);

        m_strLang = strLang;
    }

    string SecondFormat(TINT64 ddwCostTime)
    {
        ostringstream oss;
        oss.str("");
        TINT64 ddwMinOrigin = ddwCostTime / 60;
        TINT64 ddwSecond = ddwCostTime % 60;
        TINT64 ddwHour = ddwMinOrigin / 60;
        TINT64 ddwMin = ddwMinOrigin % 60;

        if(0 != ddwHour)
        {
            oss << ddwHour << "h";
        }
        if(0 != ddwMin)
        {
            oss << ddwMin << "m";
        }
        if(0 != ddwSecond)
        {
            oss << ddwSecond << "s";
        }
        // �쳣���
        if(0 == ddwHour 
           && 0 == ddwMin
           && 0 == ddwSecond)
        {
            oss << ddwSecond << "s";
        }

        return oss.str();
    }
};

class CMsgBase
{
public:
    // �ʼ����
    // ��������ͷ��ʼ������˵Ľӿ�,�������߼��ж�
    static TVOID SendHelpMail(TINT64 ddwTargetUid, TINT32 dwDocId, TUINT32 udwSid, const TCHAR *szContent);

    // �ʼ����
    // ����������Ӫ�ʼ������˵Ľӿ�,�������߼��ж�
    static TVOID SendOperateMail(TINT64 ddwTargetUid, TINT32 dwDocId, TUINT32 udwSid, const TCHAR *szContent);
    static TVOID SendOperateMail(TINT64 ddwTargetUid, TINT32 dwDocId, TUINT32 udwSid, TINT32 dwDisplayClass,
        const TCHAR *szContent, const TCHAR* szExtraContent, const TCHAR* szReward);

    // �ʼ����
    // ����������Ӫ�ʼ������˵Ľӿ�,�������߼��ж�
    static TVOID SendOperateAlMail(TINT64 ddwTargetAid, TINT32 dwDocId, TUINT32 udwSid, const TCHAR *szContent);
    static TVOID SendOperateAlMail(TINT64 ddwTargetAid, TINT32 dwDocId, TUINT32 udwSid, TINT32 dwDisplayClass,
        const TCHAR *szContent, const TCHAR* szExtraContent, const TCHAR* szReward);

    //���͹����ʼ�
    // ���巢�͹����ʼ��ĺ����ӿ�, ���ж��Ƿ��Ѿ����͹�ͬ���Ĺ����ʼ�
    static TINT32 SendEncourageMail(TbUser_stat *ptbStat, TINT64 ddwSid, TUINT32 udwMailDocId, TUINT32 udwSPos = 0, string strContent = "");

    // �������
    // function ===> �������ͽӿ�(���˵�)
    static TVOID SendNotificationPerson(string strProject, TUINT32 udwSid, TINT64 ddwTargetUid, SNoticInfo &rstNoticInfo);

    // function ===> �������ͽӿ�(���˵�)
    static TVOID SendNotificationAlliance(string strProject, TUINT32 udwSid, TINT64 ddwSuid, TINT64 ddwAid, SNoticInfo &rstNoticInfo);

    static TVOID CheckMap(TINT64 ddwUid, TUINT32 udwSid);

    // function ===> �����Χ���õ������ļ�(�ʼ�,���͵�)
    static TINT32 SendDelaySystemMsg(const TCHAR *pszMsg);

    static string GetEventMailContentByEventId(TINT32 dwMailId, const Json::Value &rstDocumentJson);
    
    static string GetEventMailTitleByEventId(TINT32 dwMailId, const Json::Value &rstDocumentJson);

    static string GetItemNameByItemId(TUINT32 udwItenId, const Json::Value &rstDocumentJson);

    static string GetSvrNameBySvrId(string szSvrId, const Json::Value &rstDocumentJson);

    // function ===> �滻�ַ����е�ĳ���ַ���Ϊָ�����ַ���ֵ
    // in_value ===> strSrc: ԭ�ַ���
    //          ===> strFind: ��Ҫ���滻���ַ���
    //          ===> strReplace: ָ���滻����ַ���
    static TVOID StringReplace(string &strSrc, const string &strFind, const string &strReplace);


    static TVOID AddAlRank(TbAlliance* ptbAlliance);
    static TVOID DelAlRank(TbAlliance* ptbAlliance);
    static TVOID ChangeAlLang(TbAlliance* ptbAlliance);
    static TVOID ChangeAlPolicy(TbAlliance* ptbAlliance);
    static TVOID ChangeAlName(TbAlliance* ptbAlliance);
    static TVOID ChangeAlNick(TbAlliance* ptbAlliance);
private:
    // function ===> ͨ������id��ȡ���͵��İ�
    static string GetNoticDocumentByNoticId(TINT32 dwNoticId, const Json::Value &rstDocumentJson);

    static TINT32 GetNotiType(TINT32 dwNoticId, const Json::Value &rstDocumentJson);
    static string GetNotiSound(TINT32 dwType, const Json::Value &rstDocumentJson);

public:
    static string GetBuffInfoName(TINT32 dwBuffId);

public:
    static TVOID RefreshUserInfo(TINT64 ddwUid);
    static TVOID ClearNoPlayerMap(TINT64 ddwUid, TINT64 ddwSid, TINT64 ddwPos);

};

#endif 
