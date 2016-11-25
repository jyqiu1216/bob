#ifndef _GAME_EVALUATE_LOGIC_H_
#define _GAME_EVALUATE_LOGIC_H_

#include <string>
using namespace std;
#include "player_info.h"
#include "game_info.h"
#include "game_evaluate_base.h"


class CGameEvaluateLogic
{
public:

    // function ==> 生成游戏评估基本数据
    // param0: 请求参数
    // param1: 请求处理后的userinfo数据
    // param2: 生成的游戏评估基本数据
    static TBOOL GenGameEvaluateData(SReqInfo *pstReqInfo, SUserInfo *pstUserInfo, SGameEvaluateData *pstGameEvaluateData);
  
    // function ==> 生成游戏评估ex数据
    // param0: 请求处理后的userinfo数据
    // param1: ex_data
    // param2: 用户类型(source/target)
    static TBOOL GenGameEvaluateExData(SUserInfo *pstUserInfo, SGameEvaluateExData *pstGameEvaluateExDataResult, TINT64 ddwExDataUserType);

    // function ==> 生成游戏评估所需的日志
    // param0: 生成的source基本数据
    // param1: 生成的source基本数据成功与否的标志
    // param2: 生成的target基本数据
    // param3: 生成的target基本数据成功与否的标志
    // param4: 生成source用户的ex_data
    // param5: 生成source用户的ex_data成功与否的标志
    // param6: 生成target用户的ex_data    
    // param7: 生成target用户的ex_data成功与否的标志
    // param8: 生成ex_data(source/target/both)
    static string GenGameEvaluateLog(SGameEvaluateData *pstGameEvaluateDataSource, TBOOL bGetGameEvaluateDataSourceFlag, 
                                     SGameEvaluateData *pstGameEvaluateDataTarget, TBOOL bGetGameEvaluateDataTargetFlag, 
                                     SGameEvaluateExData *pstGameEvaluateExDataSourceResult, TBOOL bGetGameEvaluateExDataSourceResultFlag, 
                                     SGameEvaluateExData *pstGameEvaluateExDataTargetResult, TBOOL bGetGameEvaluateExDataTargetResultFlag,
                                     SGameEvaluateAddData *pstGameEvaluateAddData, TBOOL bGameEvaluateType);


public:
    // function ==> 保存ex_data
    // param0: 请求处理前或后的userinfo数据
    // param1: ex_data的用户类型(source/target)
    // param2: ex_data的类型(raw/new)
    static TBOOL SaveGameEvaluateExData(SReqInfo *pstReqInfo, SUserInfo *pstUserInfo, TINT64 ddwExDataUserType, TINT64 ddwExDataType);



private:

    // function ==> 生成主key    
    template<class T>
    static TVOID SetGameEvaluateLogKey(ostringstream &oss, const T &tVariable);

    // function ==> 生成子key(格式A:B)
    template<class T1, class T2>
    static TVOID SetGameEvaluateSubLogKey(ostringstream &oss, const T1 &tVariable0, const T2 &tVariable1);
    
    // function ==> 生成子key(格式A:B:C)
    template<class T1, class T2, class T3>
    static TVOID SetGameEvaluateSubLogKey(ostringstream &oss, const T1 &tVariable0, const T2 &tVariable1, const T3 &tVariable2);

    // function ==> 获取玩家时间道具的总时间
    static TINT64 GetTotalTimeItemValue(TbBackpack *ptbBackpack);

    // function ==> 获取玩家指定类型道具的信息(时间道具/资源道具)
    static TVOID GetItemInfo(map<TINT64, TINT64> &ItemInfoMap, TbBackpack *ptbBackpack, TINT64 ddwItemType);

    // function ==> 获取玩家训练troop的能力
    static TVOID GetTroopAbility(vector<TINT64> &stTroopAbilityVector, SUserInfo *pstUserInfo);

    // function ==> 获取玩家训练fort的能力
    static TVOID GetFortAbility(vector<TINT64> &stFortAbilityVector, SUserInfo *pstUserInfo);

    // function ==> 自己的非march的action
    static TVOID GetOwnActionIdSecClassNoMarch(vector<SActionInfo> &m_stOwnActionInfoNoMatchVector, SUserInfo *pstUserInfo);

};




#endif
