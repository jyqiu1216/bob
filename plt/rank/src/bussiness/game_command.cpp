#include "game_command.h"
#include "process_rank_get.h"
#include "process_al_list_get.h"
#include "process_al_recommend_get.h"
#include "process_new_al_recommend_get.h"
#include "process_get_al_rank_by_id.h"
#include "process_change_al_info.h"
#include "process_add_al_info.h"
#include "process_del_al_info.h"
#include "process_player_self_rank_get.h"


CClientCmd *CClientCmd::m_poClientCmdInstance = NULL;

// todo: 命令字输出有待优化
static struct SCmdInfo stszClientReqCommand[EN_CLIENT_REQ_COMMAND__END + 1] = 
{    
    {EN_CLIENT_REQ_COMMAND__UNKNOW,                         {"unknow", EN_UNKNOW_PROCEDURE, EN_UNKNOW_CMD, NULL, NULL}},

    {EN_CLIENT_REQ_COMMAND__ALLIANCE_GET_RANK,              {"rank", EN_NORMAL, EN_PLAYER, CProcessRankGet::requestHandler, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_GET_LIST,              {"al_list_get", EN_NORMAL, EN_PLAYER, CProcessAlListGet::requestHandler, NULL}},
    {EN_CLIENT_REQ_COMMAND__ALLIANCE_GET_RECOMMEND,         {"al_recommend_get", EN_NORMAL, EN_PLAYER, CProcessAlRecommendGet::requestHandler, NULL}},
    {EN_CLIENT_REQ_COMMAND__NEW_PLAYER_AL_GET_RECOMMEND,    {"new_al_recommend_get", EN_NORMAL, EN_PLAYER, CProcessNewAlRecommendGet::requestHandler, NULL}},
    {EN_CLIENT_REQ_COMMAND__GET_AL_RANK_BY_ID,              {"get_al_rank_by_id", EN_NORMAL, EN_PLAYER, CProcessGetAlRankById::requestHandler, NULL}},
    {EN_CLIENT_REQ_COMMAND__CHANGE_AL_INFO,                 {"change_al_info", EN_NORMAL, EN_PLAYER, CProcessChangeAlInfo::requestHandler, NULL}},
    {EN_CLIENT_REQ_COMMAND__ADD_AL_INFO,                    {"add_al_info", EN_NORMAL, EN_PLAYER, CProcessAddAlInfo::requestHandler, NULL}},
    {EN_CLIENT_REQ_COMMAND__DEL_AL_INFO,                    {"del_al_info", EN_NORMAL, EN_PLAYER, CProcessDelAlInfo::requestHandler, NULL}},

    {EN_CLIENT_REQ_COMMAND__GET_PLAYER_SELF_RANK,           {"get_player_self_rank", EN_NORMAL, EN_PLAYER, CProcessPlayerSelfRankGet::requestHandler, NULL}},

    {EN_CLIENT_REQ_COMMAND__END,                            {"cmdend", EN_UNKNOW_PROCEDURE, EN_UNKNOW_CMD, NULL, NULL}},
};

CClientCmd::CClientCmd()
{
}

CClientCmd *CClientCmd::GetInstance()  
{
    if(NULL == m_poClientCmdInstance)  //判断是否第一次调用 
    {
        m_poClientCmdInstance = new CClientCmd;  
    }
    return m_poClientCmdInstance;  
}

TINT32 CClientCmd::Init()
{
    Init_CmdInfo();
    Init_CmdMap();
    Init_CmdEnumMap();

    return 0;
}

TINT32 CClientCmd::Init_CmdMap()
{
    for(TINT32 dwIdx = 0; dwIdx < EN_CLIENT_REQ_COMMAND__END; ++dwIdx)
    {
        m_oCmdMap.insert(make_pair(m_stszClientReqCommand[dwIdx].udwCmdEnum, m_stszClientReqCommand[dwIdx].stFunctionSet));        
    }
    return 0;
}

TINT32 CClientCmd::Init_CmdEnumMap()
{
    for(TINT32 dwIdx = 0; dwIdx < EN_CLIENT_REQ_COMMAND__END; ++dwIdx)
    {
        m_oCmdEnumMap.insert(make_pair(m_stszClientReqCommand[dwIdx].stFunctionSet.szCmdName, m_stszClientReqCommand[dwIdx].udwCmdEnum));
    }
    return 0;
}

TINT32 CClientCmd::Init_CmdInfo()
{
    memmove(m_stszClientReqCommand, stszClientReqCommand, sizeof(m_stszClientReqCommand));
    return 0;
}

TUINT32 CClientCmd::GetCommandID( const char* pszCommand )
{
    EClientReqCommand udwCommandID = EN_CLIENT_REQ_COMMAND__UNKNOW;

    map<string, EClientReqCommand>::iterator itCmdEnum;
    itCmdEnum = m_oCmdEnumMap.find(pszCommand);
    if(itCmdEnum != m_oCmdEnumMap.end())
    {
        udwCommandID = itCmdEnum->second;
    }

    return udwCommandID;
}

map<EClientReqCommand, struct SFunctionSet> *CClientCmd::Get_CmdMap()
{
    return &m_oCmdMap;
}

map<string, EClientReqCommand> *CClientCmd::Get_CmdEnumMap()
{
    return &m_oCmdEnumMap;
}
