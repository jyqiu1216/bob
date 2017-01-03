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
    // function  ===> 计算两点((x1,y1)和(x2,y2))的距离 y = sqrt((x1-x2)^2 + (y1-y2)^2)
    // in_value  ===> dwPosA: 起点的pos值
    //                dwPosB: 终点的pos值
    // out_value ===> 返回两pos值之间的距离
    static double GetDistance(TINT32 dwPosA, TINT32 dwPosB);

    // function  ===> 获取两值之间的随机数
    // in_value  ===> udwBegin: 起点值
    //                udwEnd: 终点值
    // out_value ===> 返回两值之间的随机数
    static TUINT32 GetRandNumber(TUINT32 udwBegin, TUINT32 udwEnd);

    // function  ===> 获取用户的action_id
    // in_value  ===> ddwPlayId: 用户的uid
    //           ===> udwSeq: 生成action时的seq号
    // out_value ===> 返回用户的action_id
    static TUINT64 GetPlayerNewTaskId(TINT64 ddwPlayId, TUINT32 udwSeq);

    // function  ===> 获取alliance的action_id
    // in_value  ===> udwAlid: 
    // out_value ===> 返回alliance的action_id
    static TUINT64 GetAllianceNewTaskId(TUINT32 udwAlid);

    // function  ===> 把字符串变成全小写的字符串
    // in_value  ===> str: 待转换的字符串
    // out_value ===> 返回已经转换的字符串
    static string ToLower(const string& str);

    // function  ===> 通过curl库发url请求
    // in_value  ===> szUrl: 待发的url请求
    //           ===> szRes: url请求之后返回的内容
    //           ===> udwTimeOut: url请求的超时时间
    // out_value ===> 返回url请求的结果
    static CURLcode ResFromUrl(TCHAR *szUrl, TCHAR *szRes, TUINT32 udwTimeOut = 5);

    // function  ===> 通过troop_id获取troop的兵种
    // in_value  ===> udwTroopType: troop的id
    // out_value ===> 返回troop的兵种
    static TUINT32 GetTroopCategoryByTroopType(TUINT32 udwTroopType);

    // function  ===> 通过fort_id获取fort的兵种
    // in_value  ===> udwFortType: fort的id
    // out_value ===> 返回fort的兵种
    static TUINT32 GetTroopCategoryByFortType(TUINT32 udwFortType);

    // function  ===> 通过troop_id获取troop的might值
    // in_value  ===> udwTroopId: troop的id
    // out_value ===> 返回troop_id所对应的might值
    static TUINT32 GetTroopSingleMight(TUINT32 udwTroopId);

    // function  ===> 通过fort_id获取fort的might值
    // in_value  ===> udwFortId: fort的id
    // out_value ===> 返回fort_id所对应的might值
    static TUINT32 GetFortSingleMight(TUINT32 udwFortId);

    static TINT32 GetTroopSpeed(TUINT32 udwTroopId);

    // function  ===> 获取troop的总数量
    // in_value  ===> pudwTroop: 包含troop的数组
    // out_value ===> 返回troop的总数量
    static TINT64 GetTroopSumNum(TINT64 *addwTroop);
    static TINT64 GetTroopSumNum(const SCommonTroop& stTroop);

    static TINT64 GetFortSumNum(const SCommonFort& stFort);

    // function  ===> 获取troop的总might值
    // in_value  ===> pudwTroop: 包含troop的数组
    // out_value ===> 返回troop的总might值
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

    // function  ===> 获取fort的总might值
    // in_value  ===> pudwFort: 包含fort的数组
    // out_value ===> 返回fort的总might值
    static TUINT64 GetFortSumMight(TINT64 *addwFort);

    // function  ===> 获取联盟帮助能帮助减少的时间
    // in_value  ===> udwCTime: action的costtime
    //           ===> udwCanHelpNum： action总共能帮助多少次
    // out_value ===> 返回联盟帮助能帮助减少的时间
    static TINT32  Get_AlHelpTime(TUINT32 udwCTime, TUINT32 udwCanHelpNum);

    ////////////////////////////////////////////////////////////////////////////
    //将玩家的uid加入到report的接收用户的列表中，用于设置report_user
    ////////////////////////////////////////////////////////////////////////////
    static TINT32 AddUserToMailReceiverList(TINT32 *adwRecieverList, TUINT32 &udwRecieverNum, TINT64 ddwUserId);

    //获取联盟report_user的key(uid)值
    static TINT32 GetAllianceUserReportKey(TINT32 dwAllianceId, TBOOL IsAllMember = FALSE);

    static TBOOL IsOpCommand(TCHAR* pszCommand);

    static TINT32 GetBuildingSize(TINT32 dwType);

    static TBOOL IsObstacle(TINT32 dwType);

    static TBOOL LoadNewJson(Json::Value& rawJson, Json::Value& resultJson);

    static TBOOL IsScoutShowBuff(TINT32 dwBufferId);

    static TBOOL IsWatchTowerShowBuff(TINT32 dwBufferId);

    static TBOOL IsValidName(const string& strName, const TINT32 dwType);

    // function  ===> 获取用户的client_action_id
    // in_value  ===> ddwUid: 用户的uid
    //           ===> udwSeq: 生成action时的seq号
    // out_value ===> 返回用户的client_action_id
    static TUINT64 GetClientBuildTaskId(TINT64 ddwUid, TUINT32 udwClientSeq);

private:
    // function  ===> curl库对于请求结果保存的回调函数
    // writedata
    static size_t writedata(void *buffer, size_t size, size_t nmemb, void *userp);

};





#endif
