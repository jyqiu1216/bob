#include "op_task_condition_json.h"
#include "user_json.h"
#include "time_utils.h"
#include "action_base.h"
#include "player_info.h"
#include "common_json.h"
#include "wild_info.h"
#include "game_define.h"
#include "common_func.h"
#include "game_command.h"

COpTaskConditionJson::COpTaskConditionJson()
{

}

COpTaskConditionJson::~COpTaskConditionJson()
{

}

TVOID COpTaskConditionJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
/*
	"svr_task_condition":
    {
        type:num
    }
*/
    TbTask *ptbTask = &pstSession->m_stUserInfo.m_tbTask;
    if(pstSession->m_stReqParam.m_udwCommandID == EN_OPSELF_GET_TASK_CONDITION_INFO)
    {
        for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum; ++udwIdx)
        {
            if(ptbTask->m_bTask_normal[udwIdx].m_ddwId != static_cast<TUINT32>(atoi(pstSession->m_stReqParam.m_szKey[0])))
            {
                continue;
            }
            Json::Value& jsonBufferInfo = rJson["svr_task_condition"];
            jsonBufferInfo = Json::Value(Json::objectValue);

            for(TUINT32 udwConditionIdx = 0; udwConditionIdx < MAX_TASK_CONDITION_LIMIT; ++udwConditionIdx)
            {                
                if(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_ddwTaskType == 0)
                {
                    continue;
                }
                Json::Value &jsonConInfo = jsonBufferInfo[CCommonFunc::NumToString(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_ddwTaskType)];
                jsonConInfo = Json::Value(Json::arrayValue);
                jsonConInfo.append(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_ddwId);
                jsonConInfo.append(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_uddwNum);
                jsonConInfo.append(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_ddwValue);
                jsonConInfo.append(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_bIsStand);
                jsonConInfo.append(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_uddwCurrValue);
                jsonConInfo.append(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwConditionIdx].m_uddwBeginValue);
            }
        }
    }
}

