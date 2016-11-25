#ifndef _GAME_COMMAND_DEFINE_H_
#define _GAME_COMMAND_DEFINE_H_

//#pragma pack(1)
#include "session.h"
#include <map>

using namespace std;

// todo: ��ö�ٲ���ɾ��,Ŀǰ����˳���ܵߵ�,���ڿͻ��˵�json���,������Ż���֮��˳����Եߵ�
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

// �����ֳ�ʼ������Ľṹ��
struct SFunctionSet
{
    // cmd������
    TCHAR szCmdName[DEFAULT_NAME_STR_LEN];
    // cmd�����̷���ı�־
    ECmdProcedureType dwProcedureType;
    // cmd����ı�־
    ECmdType dwCmdType;
    // cmd����Ӧ�Ĵ�����
    Process_Proc ProcessProc;
    // cmd����Ӧ��json�������
    Process_Output ProcessOutput;

};

struct SCmdInfo
{
    // cmd����Ӧ��ö��
    EClientReqCommand udwCmdEnum;
    // cmd����Ӧ�Ĺ��ܼ�
    struct SFunctionSet stFunctionSet;
};

class CClientCmd
{
public:
    // ��ȡʵ��
    static CClientCmd *GetInstance();
    // function ===> cmd��Ϣ���ܳ�ʼ������
    TINT32 Init();
    // function ===> ͨ��cmd�����ֻ�ȡcmd��ö��
    TUINT32 GetCommandID(const TCHAR *pszCommand);
    map<EClientReqCommand, struct SFunctionSet> *Get_CmdMap();
    map<string, EClientReqCommand> *Get_CmdEnumMap();

private:
    CClientCmd();
    // function ===> cmd��ʼ����ĳ�ʼ��
    TINT32 Init_CmdInfo();
    // function ===> CmdMap�ĳ�ʼ��
    TINT32 Init_CmdMap();
    // function ===> CmdEnumMap�ĳ�ʼ��
    TINT32 Init_CmdEnumMap();
private:
    static CClientCmd *m_poClientCmdInstance;
    // cmd��ʼ����
    struct SCmdInfo m_stszClientReqCommand[EN_CLIENT_REQ_COMMAND__END + 1];
    // CmdMap(cmdenum --> cmdfunction)
    map<EClientReqCommand, struct SFunctionSet> m_oCmdMap;
    // CmdEnumMap(cmdname --> cmdenum)
    map<string, EClientReqCommand> m_oCmdEnumMap;
};

#endif
