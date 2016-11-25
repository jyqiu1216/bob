#ifndef _QUEST_NOTIC_LOGIC_H_
#define _QUEST_NOTIC_LOGIC_H_

#include "aws_table_include.h"
#include "bin_struct_define.h"
#include "player_info.h"

// daily/alliance/mistery/top quest
class CQuestNoticLogic
{   
public:

    // function ===> ��ȡ��������action
    static TbMarch_action *GetTaskNoticAction(TbMarch_action *pstActionList, TUINT32 udwActionNum);
    // function ===> hu��ⴥ�����������Ƿ���Ҫ��������(1 --> 0)(���û����ɵ�)
    // out_value ===> ���ش������͵�����ʱ���
    static TVOID CheckTaskNotic(SUserInfo *pstUserInfo, TbMarch_action *ptbAction);

    // function ===> ��ʾ����action��flag״̬
    static TVOID ShowNoticFlag(SUserInfo *pstUserInfo, TbMarch_action *ptbAction);

};

#endif
