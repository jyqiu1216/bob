#ifndef _QUEST_LOGIC_H_
#define _QUEST_LOGIC_H_

#include "game_info.h"
#include "game_define.h"
#include "player_info.h"
#include "data_center_rsp_info.h"

enum ECollectQuestType
{
    EN_COLLECT_QUEST_DAILY = 1,
    EN_COLLECT_QUEST_ALLIANCE = 2,
    EN_COLLECT_QUEST_KINGDOMQUEST = 3,
    EN_COLLECT_QUEST_END
};

class CQuestLogic
{
public:

    static TVOID CheckTimeQuestFinish(SUserInfo *pstUser, SCityInfo *pstCity,TbQuest *pstQuest);
    
    static TVOID CheckTimeQuestRefresh(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TbQuest *pstQuest);
    static TVOID RefreshTimequest(SUserInfo *pstUserInfo, SCityInfo * pstCityInfo, TUINT32 udwQuestType, SQuestNode* pstQuestNode, TBOOL bIsInitDaily = FALSE);

    static TVOID NewCheckTimeQuestRefresh(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TbQuest *pstQuest);
    static TVOID NewRefreshTimequest(SUserInfo *pstUserInfo, SCityInfo * pstCityInfo, TUINT32 udwQuestType, SQuestNode* pstQuestNode, TINT32 dwRefreshType);
    static TVOID NewGenQuestNum(SQuestNode* pstQuestNode, SQuestListInfo *pstQuestListInfo, TINT32 dwCurQuestNum, TUINT32 udwCurTime, TUINT32 udwUserCreateTime);

    static TVOID CheckQuestNodeFinish(SUserInfo *pstUser, SQuestNode* pstQuestNode, SCityInfo *pstCity, TUINT32 udwType,TINT32 udwAuto, TBOOL &bFinish);
    static TVOID GenQuestNode(TUINT32 udwQuestType, TUINT32 udwCastleLv, TUINT32 udwTimeGiftCollectNum, SQuestComm *pstQuestComm);

    static TVOID HasFinishQuestNode(SQuestNode* pstQuestNode, TBOOL &bFinish);

    static TBOOL IsTopQuestFinish(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwQuestId);

    static TUINT32 GetDoneQuestNum(SUserInfo *pstUser, TUINT32 udwType);

    static TVOID TestFinishBeginQuest(SUserInfo *pstUser, TUINT32 udwQuestType, TUINT32 udwTargetId, TUINT32 udwLv);
    static TVOID GenInitDailyQuestNode(TUINT32 udwQuestType, TUINT32 udwCastleLv, TUINT32 udwTimeGiftCollectNum, SQuestComm *pstQuestComm, TUINT32 udwQuestIdx);

    static TVOID CheckIsClaim(SUserInfo *pstUserInfo, SCityInfo *pstCity);

    static TBOOL IsToClaim(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwQuestId);
    static TBOOL HasRuningTimeQuest(TbQuest* ptbQuest, TUINT8 ucType);


    static TVOID RemoveAgeTask(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwAge);
    static TVOID RemoveAgeTopQuest(SUserInfo *pstUser, TUINT32 udwAge);

    static TINT32 CheckTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity);

    static TBOOL IsTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity, TINT32 dwTaskIdx, TBOOL bIsRepeatType);    

    static TBOOL IsSubTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity, TINT32 dwTaskIdx, TINT32 dwSubIdx, TBOOL bIsRepeatType);
    static TBOOL IsSubTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity, STaskNodeNow *pstTask, TINT32 dwSubIdx);

    static TINT64 GetOwnNum(SCityInfo *pstCity, SUserInfo *pstUser, TUINT32 udwTaskType, TUINT32 udwId, TUINT32 udwValue);

    static TINT32 GenTaskTimeQuest(SUserInfo *pstUser, SCityInfo *pstCity);

    static TINT32 GenTaskNormalQuest(SUserInfo *pstUser, SCityInfo *pstCity);

    static TBOOL IsTaskShowNow(SUserInfo *pstUser, TUINT32 udwTaskId);
    
    //wave@20160414
    static TINT32 CheckSpecialTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity);
    static TBOOL IsSpecialTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskId);
    static TBOOL IsSpecialTask(TUINT32 udwTaskId);
    static TBOOL IsTaskNeedFinishShow(TUINT32 udwTaskId);
    static TBOOL IsNormalTaskCheckNow(SUserInfo *pstUser, TUINT32 udwTaskId);
    static TINT32 GetIdxInNowTaskList(SUserInfo *pstUser, TUINT32 udwTaskId, STaskNodeNow* pstTaskList, TUINT32 udwListNum);

    static TVOID InsertQuest(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskId, TUINT32 udwIdx);

    static TINT32 GetTaskRewardById(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskId);

    static TINT32 CheckTaskRefresh(SUserInfo *pstUser, SCityInfo *pstCity);

    static TVOID CheckOwnNumUpdate(SUserInfo *pstUser, SCityInfo *pstCity);

    static TVOID SetTaskCurValue(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwCondiType, TUINT32 udwNumAdd = 1, TUINT32 udwId = 0, TUINT32 Value = 0);

    static TVOID ProcessSpTask(SUserInfo *pstUser, SCityInfo *pstCity);

    static TVOID CheckTask(SUserInfo *pstUser, SCityInfo *pstCity, TBOOL IsHanderBefore = TRUE);

    static TVOID CheckTaskCondition(SUserInfo *pstUser, SCityInfo *pstCity);

    static TUINT32 GetTaskTypeEndTime(SUserInfo* pstUser, TUINT32 udwTaskType, TUINT32 udwId, TUINT32 udwValue);

    static TBOOL IsTaskCanStart(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskType, TUINT32 udwId, TUINT32 udwValue, TBOOL bStrictCheck = FALSE);

    static TBOOL CheckTaskRealCanStart(SUserInfo *pstUser, SCityInfo *pstCity, STaskNodeNow *pstTask);

    static TVOID CheckFirstLoginStatus(SUserInfo *pstUser, SCityInfo *pstCity, TBOOL bIsFirstLogin);

    static TUINT32 GetPriorityTask(vector<TUINT32> vTaskId);

    static TBOOL CompareTaskPriority(STaskPriority sTaskA, STaskPriority sTaskB);
    static TBOOL CompareTaskPriorityNoState(STaskPriority sTaskA, STaskPriority sTaskB);

    static vector<TUINT32> GetSortTaskList(SUserInfo *pstUser, SCityInfo *pstCity, vector<TUINT32> &vTaskCanStart, TBOOL &bTopCanStart);

    static TVOID SetBuildingProgress(SUserInfo *pstUser, SCityInfo *pstCity);
    // function ===> 检测任务类型节点是否有任务在执行
    // out_value ===> 返回任务的结束时间
    static TUINT32 CheckQuestNodeRunning(SQuestNode* pstQuestNode, TBOOL &bRunning);

    // function ===> 检测topquest是否有可以领取的
    static TVOID CheckTopQuestCanClaim(SUserInfo *pstUser, TBOOL &bClaim);

    // function ===> 检测玩家的peacetime打破是否需要提醒
    // out_value ===> false: 不需要; true: 需要
    static TBOOL CheckPeacetimeRemind(SUserInfo *pstUser);


    // function ===> 检测玩家的new protect打破是否需要提醒
    // out_value ===> false: 不需要; true: 需要
    static TBOOL CheckNewProtectRemind(SUserInfo *pstUser);

    static TVOID CheckTaskResetFlag(SUserInfo *pstUser, SCityInfo *pstCity);
};


#endif
