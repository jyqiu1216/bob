#ifndef _REPORT_LOGIC_H_
#define _REPORT_LOGIC_H_

#include "aws_table_include.h"
#include "player_info.h"

class CReportLogic
{
public:
    CReportLogic();
    ~CReportLogic();

    static TVOID GenFromTo(TbReport* ptbReport, TbMarch_action* ptbMarch, TINT32 dwSvrId);
    static TVOID GenMajorPlayer(TbMarch_action* ptbMarch, Json::Value& rjson);

    static TVOID GenDisMissReport(TbMarch_action* ptbMarch, TbReport* ptbReport);

private:

};

#endif

