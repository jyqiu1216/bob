#ifndef _DB_RESPONSE_H_
#define _DB_RESPONSE_H_

#include <assert.h>
#include "base/log/wtselogger.h"
#include "db_rsp_info.h"
#include <string>
using namespace std;

class CDbResponse
{
public:
    static int OnCountResponse(const DbRspInfo& rspInfo);
};

#endif


