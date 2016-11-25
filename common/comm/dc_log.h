#ifndef _DC_LOG_H_
#define _DC_LOG_H_

#include "base/common/wtse_std_header.h"
#include "base/log/wtselogger.h"
#include <string>
#include <cstring>

using namespace std;
using namespace wtse::log;

class CDcUdpLog
{
public:
    static TINT32 SendFormatLogToDc(string strNotic, CTseLogger *poLog);

};

#endif





