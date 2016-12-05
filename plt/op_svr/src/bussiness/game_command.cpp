#include "game_command.h"
#include "process_event.h"

CClientCmd *CClientCmd::m_poClientCmdInstance = NULL;

// todo: 命令字输出有待优化
static struct SCmdInfo stszClientReqCommand[EN_CLIENT_REQ_COMMAND__END + 1] =
{
    { EN_CLIENT_REQ_COMMAND__UNKNOW,            {"unknow", EN_UNKNOW_PROCEDURE, EN_UNKNOW_CMD, NULL, NULL}},
    { EN_CLIENT_REQ_COMMAND__ALL_EVENT_GET,     {"all_event_get", EN_NORMAL, EN_EVENT, CProcessEvent::ProcessCmd_AllEventGet, NULL}},
    { EN_CLIENT_REQ_COMMAND__END,               {"cmdend", EN_UNKNOW_PROCEDURE, EN_UNKNOW_CMD, NULL, NULL}},
};

CClientCmd::CClientCmd()
{
}

CClientCmd *CClientCmd::GetInstance()
{
    if (NULL == m_poClientCmdInstance)  //判断是否第一次调用 
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
    for (TINT32 dwIdx = 0; dwIdx < EN_CLIENT_REQ_COMMAND__END; ++dwIdx)
    {
        m_oCmdMap.insert(make_pair(m_stszClientReqCommand[dwIdx].udwCmdEnum, m_stszClientReqCommand[dwIdx].stFunctionSet));
    }
    return 0;
}

TINT32 CClientCmd::Init_CmdEnumMap()
{
    for (TINT32 dwIdx = 0; dwIdx < EN_CLIENT_REQ_COMMAND__END; ++dwIdx)
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

TUINT32 CClientCmd::GetCommandID(const char* pszCommand)
{
    EClientReqCommand udwCommandID = EN_CLIENT_REQ_COMMAND__UNKNOW;

    map<string, EClientReqCommand>::iterator itCmdEnum;
    itCmdEnum = m_oCmdEnumMap.find(pszCommand);
    if (itCmdEnum != m_oCmdEnumMap.end())
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
