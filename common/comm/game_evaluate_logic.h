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

    // function ==> ������Ϸ������������
    // param0: �������
    // param1: ��������userinfo����
    // param2: ���ɵ���Ϸ������������
    static TBOOL GenGameEvaluateData(SReqInfo *pstReqInfo, SUserInfo *pstUserInfo, SGameEvaluateData *pstGameEvaluateData);
  
    // function ==> ������Ϸ����ex����
    // param0: ��������userinfo����
    // param1: ex_data
    // param2: �û�����(source/target)
    static TBOOL GenGameEvaluateExData(SUserInfo *pstUserInfo, SGameEvaluateExData *pstGameEvaluateExDataResult, TINT64 ddwExDataUserType);

    // function ==> ������Ϸ�����������־
    // param0: ���ɵ�source��������
    // param1: ���ɵ�source�������ݳɹ����ı�־
    // param2: ���ɵ�target��������
    // param3: ���ɵ�target�������ݳɹ����ı�־
    // param4: ����source�û���ex_data
    // param5: ����source�û���ex_data�ɹ����ı�־
    // param6: ����target�û���ex_data    
    // param7: ����target�û���ex_data�ɹ����ı�־
    // param8: ����ex_data(source/target/both)
    static string GenGameEvaluateLog(SGameEvaluateData *pstGameEvaluateDataSource, TBOOL bGetGameEvaluateDataSourceFlag, 
                                     SGameEvaluateData *pstGameEvaluateDataTarget, TBOOL bGetGameEvaluateDataTargetFlag, 
                                     SGameEvaluateExData *pstGameEvaluateExDataSourceResult, TBOOL bGetGameEvaluateExDataSourceResultFlag, 
                                     SGameEvaluateExData *pstGameEvaluateExDataTargetResult, TBOOL bGetGameEvaluateExDataTargetResultFlag,
                                     SGameEvaluateAddData *pstGameEvaluateAddData, TBOOL bGameEvaluateType);


public:
    // function ==> ����ex_data
    // param0: ������ǰ����userinfo����
    // param1: ex_data���û�����(source/target)
    // param2: ex_data������(raw/new)
    static TBOOL SaveGameEvaluateExData(SReqInfo *pstReqInfo, SUserInfo *pstUserInfo, TINT64 ddwExDataUserType, TINT64 ddwExDataType);



private:

    // function ==> ������key    
    template<class T>
    static TVOID SetGameEvaluateLogKey(ostringstream &oss, const T &tVariable);

    // function ==> ������key(��ʽA:B)
    template<class T1, class T2>
    static TVOID SetGameEvaluateSubLogKey(ostringstream &oss, const T1 &tVariable0, const T2 &tVariable1);
    
    // function ==> ������key(��ʽA:B:C)
    template<class T1, class T2, class T3>
    static TVOID SetGameEvaluateSubLogKey(ostringstream &oss, const T1 &tVariable0, const T2 &tVariable1, const T3 &tVariable2);

    // function ==> ��ȡ���ʱ����ߵ���ʱ��
    static TINT64 GetTotalTimeItemValue(TbBackpack *ptbBackpack);

    // function ==> ��ȡ���ָ�����͵��ߵ���Ϣ(ʱ�����/��Դ����)
    static TVOID GetItemInfo(map<TINT64, TINT64> &ItemInfoMap, TbBackpack *ptbBackpack, TINT64 ddwItemType);

    // function ==> ��ȡ���ѵ��troop������
    static TVOID GetTroopAbility(vector<TINT64> &stTroopAbilityVector, SUserInfo *pstUserInfo);

    // function ==> ��ȡ���ѵ��fort������
    static TVOID GetFortAbility(vector<TINT64> &stFortAbilityVector, SUserInfo *pstUserInfo);

    // function ==> �Լ��ķ�march��action
    static TVOID GetOwnActionIdSecClassNoMarch(vector<SActionInfo> &m_stOwnActionInfoNoMatchVector, SUserInfo *pstUserInfo);

};




#endif
