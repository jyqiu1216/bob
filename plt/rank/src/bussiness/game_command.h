#ifndef _GAME_COMMAND_DEFINE_H_
#define _GAME_COMMAND_DEFINE_H_

//#pragma pack(1)
#include "session.h"
#include <map>

using namespace std;

// todo: 该枚举不能删除,目前并且顺序不能颠倒,用于客户端的json输出,若输出优化了之后顺序可以颠倒
enum EClientReqCommand
{
    EN_CLIENT_REQ_COMMAND__UNKNOW = 0,
    
    EN_CLIENT_REQ_COMMAND__ALLIANCE_GET_RANK,
    EN_CLIENT_REQ_COMMAND__ALLIANCE_GET_LIST,
    EN_CLIENT_REQ_COMMAND__ALLIANCE_GET_RECOMMEND,
    EN_CLIENT_REQ_COMMAND__NEW_PLAYER_AL_GET_RECOMMEND,
    EN_CLIENT_REQ_COMMAND__GET_AL_RANK_BY_ID,
    EN_CLIENT_REQ_COMMAND__CHANGE_AL_INFO,
    EN_CLIENT_REQ_COMMAND__ADD_AL_INFO,
    EN_CLIENT_REQ_COMMAND__DEL_AL_INFO,
    
    EN_CLIENT_REQ_COMMAND__GET_PLAYER_SELF_RANK,

    EN_CLIENT_REQ_COMMAND__END,
};

enum ECmdProcedureType
{
    EN_UNKNOW_PROCEDURE = 0,
    EN_NORMAL,
    EN_SPECIAL,
};

enum ECmdType
{
    EN_UNKNOW_CMD = 0,
    EN_PLAYER,
    EN_ACTION,
    EN_ITEM,
    EN_MARCH,
    EN_ALLIANCE,
    EN_ACCOUNT,
    EN_RANK,
    EN_CARD,
    EN_OP,
    EN_BOOKMARK,
    EN_MAIL_REPORT,
    EN_MAP,
};

typedef TINT32(*Process_Proc)(SSession *pstSession);
typedef TVOID(*Process_Output)(SSession *pstSession, TCHAR *pszOut, TUINT32 &udwOutLen);

// 命令字初始化所需的结构体
struct SFunctionSet
{
    // cmd的名字
    TCHAR szCmdName[DEFAULT_NAME_STR_LEN];
    // cmd主流程分类的标志
    ECmdProcedureType dwProcedureType;
    // cmd分类的标志
    ECmdType dwCmdType;
    // cmd所对应的处理函数
    Process_Proc ProcessProc;
    // cmd所对应的json输出函数
    Process_Output ProcessOutput;

};

struct SCmdInfo
{
    // cmd所对应的枚举
    EClientReqCommand udwCmdEnum;
    // cmd所对应的功能集
    struct SFunctionSet stFunctionSet;
};

class CClientCmd
{
public:
    // 获取实例
    static CClientCmd *GetInstance();
    // function ===> cmd信息的总初始化函数
    TINT32 Init();
    // function ===> 通过cmd的名字获取cmd的枚举
    TUINT32 GetCommandID(const TCHAR *pszCommand);
    map<EClientReqCommand, struct SFunctionSet> *Get_CmdMap();
    map<string, EClientReqCommand> *Get_CmdEnumMap();

private:
    CClientCmd();
    // function ===> cmd初始化表的初始化
    TINT32 Init_CmdInfo();
    // function ===> CmdMap的初始化
    TINT32 Init_CmdMap();
    // function ===> CmdEnumMap的初始化
    TINT32 Init_CmdEnumMap();
private:
    static CClientCmd *m_poClientCmdInstance;
    // cmd初始化表
    struct SCmdInfo m_stszClientReqCommand[EN_CLIENT_REQ_COMMAND__END + 1];
    // CmdMap(cmdenum --> cmdfunction)
    map<EClientReqCommand, struct SFunctionSet> m_oCmdMap;
    // CmdEnumMap(cmdname --> cmdenum)
    map<string, EClientReqCommand> m_oCmdEnumMap;
};

#endif
