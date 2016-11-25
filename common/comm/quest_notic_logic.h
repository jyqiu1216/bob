#ifndef _QUEST_NOTIC_LOGIC_H_
#define _QUEST_NOTIC_LOGIC_H_

#include "aws_table_include.h"
#include "bin_struct_define.h"
#include "player_info.h"

// daily/alliance/mistery/top quest
class CQuestNoticLogic
{   
public:

    // function ===> 获取任务推送action
    static TbMarch_action *GetTaskNoticAction(TbMarch_action *pstActionList, TUINT32 udwActionNum);
    // function ===> hu检测触发推送任务是否需要发送推送(1 --> 0)(检测没有完成的)
    // out_value ===> 返回触发推送的最早时间点
    static TVOID CheckTaskNotic(SUserInfo *pstUserInfo, TbMarch_action *ptbAction);

    // function ===> 显示触发action的flag状态
    static TVOID ShowNoticFlag(SUserInfo *pstUserInfo, TbMarch_action *ptbAction);

};

#endif
