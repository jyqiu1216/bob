#ifndef TSHP_HU_STATISTIC_H
#define TSHP_HU_STATISTIC_H

#include "std_header.h"
#include "conf.h"
#include "aws_consume_info.h"
#include <map>
using namespace std;

const TUINT32 kMaxSendMessageSize = 2560;

#pragma pack(1)
struct StatisticInfo
{
	TUINT32     search_total_count;
	TUINT32     search_succ_count;
	TUINT32     search_fail_count;
	TUINT32     search_shield_count;
	TUINT64	   aws_consume_unit;

	TUINT64     search_max_time_us;
	TUINT64     search_total_time_us;
	TUINT32     search_time_0_50_count;
	TUINT32     search_time_50_100_count;
	TUINT32     search_time_100_300_count;
	TUINT32     search_time_300_500_count;
	TUINT32     search_time_500_1000_count;
	TUINT32     search_time_1000_up_count;

	StatisticInfo()
	{
		memset(this, 0, sizeof(StatisticInfo));
	}

	void Reset()
	{
		memset(this, 0, sizeof(StatisticInfo));
	}
};
#pragma pack()

class CStatistic
{
public :
	static CStatistic *Instance();
	~CStatistic();
	int Init(CTseLogger *stat_log, CConf *conf_obj);
	static void *Start(void *param);

	int Clear();
	int OutputResult();

	/*
	 * 依次为检索成功,检索零结果,检索失败,检索词为敏感词,检索被屏蔽(预留)
	 */
	int AddSearchSucc(TUINT64 search_time);
	int AddSearchFail(TUINT64 search_time);
	int AddSearchShield(TUINT64 search_time);

	int AddTimeStat(TUINT64 search_time);

	int CheckSendMessage();

	int GetStatInfo(StatisticInfo *stat_info);

	int SendMessage(const TCHAR *pszMsg);
	int AddAwsConsume(const AwsConsumeInfo* pConsumeInfo)
	{
        if (pConsumeInfo->sTableName.empty() || pConsumeInfo->dConsumeUnit < 0.0)
        {
            return 0;
        }
		m_mAwsConsume[pConsumeInfo->sTableName] += pConsumeInfo->dConsumeUnit;
		return 0;
	}
    int AddAwsConsume(const AwsConsumeInfo& consumeInfo)
    {
        if (consumeInfo.sTableName.empty() || consumeInfo.dConsumeUnit < 0.0)
        {
            return 0;
        }
        m_mAwsConsume[consumeInfo.sTableName] += consumeInfo.dConsumeUnit;
        return 0;
    }

private :
	CStatistic();
	static CStatistic *m_stat_tool;

	// 日志对象
	CTseLogger      *m_stat_log;
	// 统计间隔
	TUINT32         m_stat_interval_time;
	// 统计信息
	StatisticInfo   m_stat_info;
	// 配置相关信息
	CConf           *m_conf;
	char            m_message[kMaxSendMessageSize];

private:
	map<string, double> m_mAwsConsume;
};

#endif

