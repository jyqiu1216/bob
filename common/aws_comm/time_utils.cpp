#include "time_utils.h"
#include <time.h>

TUINT64 CTimeUtils::GetCurTimeUs()
{
	struct timeval  stV;
	struct timezone stZ;
	TUINT64 cur_time = 0;

	stZ.tz_minuteswest = 0;
	stZ.tz_dsttime     = 0;
	gettimeofday(&stV, &stZ);
	cur_time = stV.tv_sec * (TUINT64)1000000 + stV.tv_usec;
	return cur_time;
}

TUINT32 CTimeUtils::GetUnixTime()
{
	return time((time_t*)NULL);
}

TVOID CTimeUtils::GetFormatTimeFromSec( TUINT32 udwSecs, TCHAR *pszOut )
{
	TUINT32 udwDayNum = udwSecs/86400;
	TUINT32 udwHourNum = (udwSecs%86400)/3600;
	TUINT32 udwMinuteNum = (udwSecs%3600)/60;
	TUINT32 udwSecNum = udwSecs%60;

	TUINT32 udwCurLen = 0;
	TCHAR *pCur = pszOut;
	pCur[0] = 0;

	if(udwDayNum)
	{
		udwCurLen = sprintf(pCur, "%ud", udwDayNum);
		pCur += udwCurLen;
	}
	if(udwHourNum)
	{
		udwCurLen = sprintf(pCur, "%uh", udwHourNum);
		pCur += udwCurLen;
	}
	if(udwMinuteNum)
	{
		udwCurLen = sprintf(pCur, "%um", udwMinuteNum);
		pCur += udwCurLen;
	}
	if(udwSecNum)
	{
		udwCurLen = sprintf(pCur, "%us", udwSecNum);
		pCur += udwCurLen;
	}
}

TUINT16 CTimeUtils::GetYearMonth( TUINT32 udwCurTime /*= 0*/ )
{
    time_t conv_time = udwCurTime;
    if(conv_time == 0)
    {
        conv_time = GetUnixTime();
    }

    struct tm *cur_tm = gmtime(&conv_time);

    TUINT16 uwRetTime =  (cur_tm->tm_year%100)*100 + cur_tm->tm_mon;
    return uwRetTime;
}

TUINT16 CTimeUtils::GetMonth(TUINT32 udwCurTime /*= 0*/)
{
    time_t conv_time = udwCurTime;
    if(conv_time == 0)
    {
        conv_time = GetUnixTime();
    }

    struct tm *cur_tm = gmtime(&conv_time);

    return cur_tm->tm_mon;
}

TUINT16 CTimeUtils::GetWDay(TUINT32 udwCurTime /*= 0*/)
{
    time_t conv_time = udwCurTime;
    if(conv_time == 0)
    {
        conv_time = GetUnixTime();
    }

    struct tm *cur_tm = gmtime(&conv_time);

    return cur_tm->tm_wday;
}
