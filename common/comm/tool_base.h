#ifndef _TOOL_BASE_H_
#define _TOOL_BASE_H_

#include "base/common/wtse_std_header.h"
#include "aws_table_include.h"
#include "game_define.h"
#include "time_utils.h"
#include "game_info.h"
#include <curl/curl.h>
#include <math.h>

class CToolBase
{
public:
    // function  ===> ��������((x1,y1)��(x2,y2))�ľ��� y = sqrt((x1-x2)^2 + (y1-y2)^2)
    // in_value  ===> dwPosA: ����posֵ
    //                dwPosB: �յ��posֵ
    // out_value ===> ������posֵ֮��ľ���
    static double GetDistance(TINT32 dwPosA, TINT32 dwPosB);

    // function  ===> ��ȡ��ֵ֮��������
    // in_value  ===> udwBegin: ���ֵ
    //                udwEnd: �յ�ֵ
    // out_value ===> ������ֵ֮��������
    static TUINT32 GetRandNumber(TUINT32 udwBegin, TUINT32 udwEnd);

    // function  ===> ��ȡ�û���action_id
    // in_value  ===> ddwPlayId: �û���uid
    //           ===> udwSeq: ����actionʱ��seq��
    // out_value ===> �����û���action_id
    static TUINT64 GetPlayerNewTaskId(TINT64 ddwPlayId, TUINT32 udwSeq);

    // function  ===> ��ȡalliance��action_id
    // in_value  ===> udwAlid: 
    // out_value ===> ����alliance��action_id
    static TUINT64 GetAllianceNewTaskId(TUINT32 udwAlid);

    // function  ===> ���ַ������ȫСд���ַ���
    // in_value  ===> str: ��ת�����ַ���
    // out_value ===> �����Ѿ�ת�����ַ���
    static string ToLower(const string& str);

    // function  ===> ͨ��curl�ⷢurl����
    // in_value  ===> szUrl: ������url����
    //           ===> szRes: url����֮�󷵻ص�����
    //           ===> udwTimeOut: url����ĳ�ʱʱ��
    // out_value ===> ����url����Ľ��
    static CURLcode ResFromUrl(TCHAR *szUrl, TCHAR *szRes, TUINT32 udwTimeOut = 5);

    // function  ===> ͨ��troop_id��ȡtroop�ı���
    // in_value  ===> udwTroopType: troop��id
    // out_value ===> ����troop�ı���
    static TUINT32 GetTroopCategoryByTroopType(TUINT32 udwTroopType);

    // function  ===> ͨ��fort_id��ȡfort�ı���
    // in_value  ===> udwFortType: fort��id
    // out_value ===> ����fort�ı���
    static TUINT32 GetTroopCategoryByFortType(TUINT32 udwFortType);

    // function  ===> ͨ��troop_id��ȡtroop��mightֵ
    // in_value  ===> udwTroopId: troop��id
    // out_value ===> ����troop_id����Ӧ��mightֵ
    static TUINT32 GetTroopSingleMight(TUINT32 udwTroopId);

    // function  ===> ͨ��fort_id��ȡfort��mightֵ
    // in_value  ===> udwFortId: fort��id
    // out_value ===> ����fort_id����Ӧ��mightֵ
    static TUINT32 GetFortSingleMight(TUINT32 udwFortId);

    static TINT32 GetTroopSpeed(TUINT32 udwTroopId);

    // function  ===> ��ȡtroop��������
    // in_value  ===> pudwTroop: ����troop������
    // out_value ===> ����troop��������
    static TINT64 GetTroopSumNum(TINT64 *addwTroop);
    static TINT64 GetTroopSumNum(const SCommonTroop& stTroop);

    static TINT64 GetFortSumNum(const SCommonFort& stFort);

    // function  ===> ��ȡtroop����mightֵ
    // in_value  ===> pudwTroop: ����troop������
    // out_value ===> ����troop����mightֵ
    static TUINT64 GetTroopSumForce(TINT64 *addwTroop);
    static TUINT64 GetTroopSumForce(const SCommonTroop& stTroop);

    static TINT32 GetTroopMarchSpeed(const SCommonTroop& stTroop);

    static TINT64 GetMarchTime(TbMarch_action* ptbMarch, TINT64 ddwTpos);

    static TUINT32 GetArmyClsByTroopId(TUINT32 udwTroopId);
    static TUINT32 GetArmyClsByFortId(TUINT32 udwFortId);
    static TUINT32 GetTroopIdByArmyCls(TUINT32 udwArmyCls);
    static TUINT32 GetFortIdByArmyCls(TUINT32 udwArmyCls);
    static TBOOL IsArmyClsTroop(TUINT32 udwArmyCls);
    static TBOOL IsArmyClsFort(TUINT32 udwArmyCls);

    static TUINT32 GetFortLvByFortId(TUINT32 udwFortType);

    static TUINT32 GetTroopLvByTroopId(TUINT32 udwTroopType);

    static TVOID AddTroop(const SCommonTroop& stAdd, SCommonTroop& stTotal);

    // function  ===> ��ȡfort����mightֵ
    // in_value  ===> pudwFort: ����fort������
    // out_value ===> ����fort����mightֵ
    static TUINT64 GetFortSumMight(TINT64 *addwFort);

    // function  ===> ��ȡ���˰����ܰ������ٵ�ʱ��
    // in_value  ===> udwCTime: action��costtime
    //           ===> udwCanHelpNum�� action�ܹ��ܰ������ٴ�
    // out_value ===> �������˰����ܰ������ٵ�ʱ��
    static TINT32  Get_AlHelpTime(TUINT32 udwCTime, TUINT32 udwCanHelpNum);

    ////////////////////////////////////////////////////////////////////////////
    //����ҵ�uid���뵽report�Ľ����û����б��У���������report_user
    ////////////////////////////////////////////////////////////////////////////
    static TINT32 AddUserToMailReceiverList(TINT32 *adwRecieverList, TUINT32 &udwRecieverNum, TINT64 ddwUserId);

    //��ȡ����report_user��key(uid)ֵ
    static TINT32 GetAllianceUserReportKey(TINT32 dwAllianceId, TBOOL IsAllMember = FALSE);

    static TBOOL IsOpCommand(TCHAR* pszCommand);

    static TINT32 GetBuildingSize(TINT32 dwType);

    static TBOOL IsObstacle(TINT32 dwType);

    static TBOOL LoadNewJson(Json::Value& rawJson, Json::Value& resultJson);

    static TBOOL IsScoutShowBuff(TINT32 dwBufferId);

    static TBOOL IsWatchTowerShowBuff(TINT32 dwBufferId);

    static TBOOL IsValidName(const string& strName, const TINT32 dwType);

    // function  ===> ��ȡ�û���client_action_id
    // in_value  ===> ddwUid: �û���uid
    //           ===> udwSeq: ����actionʱ��seq��
    // out_value ===> �����û���client_action_id
    static TUINT64 GetClientBuildTaskId(TINT64 ddwUid, TUINT32 udwClientSeq);

private:
    // function  ===> curl���������������Ļص�����
    // writedata
    static size_t writedata(void *buffer, size_t size, size_t nmemb, void *userp);

};





#endif
