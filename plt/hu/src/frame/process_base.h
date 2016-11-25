#ifndef _BASE_CMD_PROCESS_H_
#define _BASE_CMD_PROCESS_H_

#include "session.h"

class CBaseCmdProcess
{
public:
    // function  ===> 命令字处理类的cmd分发
    // in_value  ===> pstSession: 业务session
    // out_value ===> 返回命令字的处理结果(0:成功;非0:失败)
    static TINT32 ProcedureCmd(SSession *pstSession, TBOOL &bNeedResponse);
};

#endif