#ifndef _TIME_UTILS_H_
#define _TIME_UTILS_H_

#include "base/common/wtse_std_header.h"

class CTimeUtils
{
public:
	//获取秒数，精确到微妙
	static TUINT64 GetCurTimeUs();

	//获取秒数，精确到秒
	static TUINT32 GetUnixTime();

public:
	static TVOID GetFormatTimeFromSec(TUINT32 udwSecs, TCHAR *pszOut);

public:
    static TUINT16 GetYearMonth(TUINT32 udwCurTime = 0);

    static TUINT16 GetMonth(TUINT32 udwCurTime = 0);
    
    static TUINT16 GetWDay(TUINT32 udwCurTime = 0);
};

#endif