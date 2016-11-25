#ifndef _TBL_COMM_H_
#define _TBL_COMM_H_

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <string>
#include "base/common/wtse_std_header.h"

class CTableComm
{
public:
    static TINT64 TblParam_GetRealKey(TUINT32 udwSid, TUINT32 udwKey);
};


#endif