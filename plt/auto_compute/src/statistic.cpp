#include  <sys/time.h>
#include "statistic.h"
#include "conf_base.h"
#include "global_serv.h"

CStatistic *CStatistic::m_stat_tool = NULL;

CStatistic::CStatistic()
{
}

CStatistic::~CStatistic()
{
}

CStatistic * CStatistic::Instance()
{
	if (NULL == m_stat_tool)
	{
		m_stat_tool = new CStatistic();
	}

	return m_stat_tool;
}

int CStatistic::Init(CTseLogger *stat_log, CConf *conf_obj)
{
	if (NULL == stat_log || NULL == conf_obj)
	    return -1;

	// 日志对象
	m_stat_log = stat_log;
	m_conf = conf_obj;
	// 统计间隔
    m_stat_interval_time = CConfBase::GetInt("interval_time");
	// 统计信息初始化
	m_stat_info.Reset();

	return 0;
}

void * CStatistic::Start(void *param)
{
	CStatistic *stat_tool = (CStatistic *)param;
	while (1)
	{
		stat_tool->OutputResult();
		stat_tool->CheckSendMessage();
		stat_tool->Clear();
		sleep(stat_tool->m_stat_interval_time);
	}

	return NULL;
}

int CStatistic::Clear()
{
	m_stat_info.Reset();

	return 0;
}

int CStatistic::OutputResult()
{
	TSE_LOG_INFO(m_stat_log, ("In [%u] seconds: [Req=%u](%u/s) [Succ=%u](%u/s) "
	        "[Fail=%u](%u/s) [Shield=%u](%u/s)",
	        m_stat_interval_time,
	        m_stat_info.search_total_count,
	        m_stat_info.search_total_count / m_stat_interval_time,
	        m_stat_info.search_succ_count,
	        m_stat_info.search_succ_count / m_stat_interval_time,
	        m_stat_info.search_fail_count,
	        m_stat_info.search_fail_count / m_stat_interval_time,
	        m_stat_info.search_shield_count,
	        m_stat_info.search_shield_count / m_stat_interval_time));

	TUINT32 total = m_stat_info.search_total_count;
	TSE_LOG_INFO(m_stat_log, ("---------------: [AvgTime=%.2f ms] [MaxTime=%.2f ms] "
	        "[0-50ms=%u%%] [50-100ms=%u%%] [100-300ms=%u%%] [300-500ms=%u%%] "
	        "[500-1000ms=%u%%] [>1000ms=%u%%]",
	        total > 0 ? m_stat_info.search_total_time_us / total / 1000.0 : 0,
	        total > 0 ? m_stat_info.search_max_time_us / 1000.0 : 0 ,
	        total > 0 ? m_stat_info.search_time_0_50_count * 100 / total : 0,
	        total > 0 ? m_stat_info.search_time_50_100_count * 100 / total : 0,
	        total > 0 ? m_stat_info.search_time_100_300_count * 100 / total : 0,
	        total > 0 ? m_stat_info.search_time_300_500_count * 100 / total : 0,
	        total > 0 ? m_stat_info.search_time_500_1000_count * 100 / total : 0,
	        total > 0 ? m_stat_info.search_time_1000_up_count * 100 / total : 0
	        ));

    return 0;
}

int CStatistic::AddSearchSucc(TUINT64 search_time)
{
	AddTimeStat(search_time);
	m_stat_info.search_succ_count++;
	m_stat_info.search_total_count++;

    	return 0;
}

int CStatistic::AddSearchFail(TUINT64 search_time)
{
	AddTimeStat(search_time);
	m_stat_info.search_fail_count++;
	m_stat_info.search_total_count++;

    	return 0;
}

int CStatistic::AddSearchShield(TUINT64 search_time)
{
	AddTimeStat(search_time);
	m_stat_info.search_shield_count++;
	m_stat_info.search_total_count++;

	return 0;
}

int CStatistic::AddTimeStat(TUINT64 search_time)
{
	if (search_time > m_stat_info.search_max_time_us)
	    m_stat_info.search_max_time_us = search_time;

	m_stat_info.search_total_time_us += search_time;

	if (search_time < 50 * 1000)
	    m_stat_info.search_time_0_50_count++;
	else if (search_time < 100 * 1000)
	    m_stat_info.search_time_50_100_count++;
	else if (search_time < 300 * 1000)
	    m_stat_info.search_time_100_300_count++;
	else if (search_time < 500 * 1000)
	    m_stat_info.search_time_300_500_count++;
	else if (search_time < 1000 * 1000)
	    m_stat_info.search_time_500_1000_count++;
	else // >1s
	    m_stat_info.search_time_1000_up_count++;

	return 0;
}

int CStatistic::CheckSendMessage()
{
    if (!CConfBase::GetInt("need_send_message"))
	    return 0;

	// 当错误数大于阈值并且错误比例大于阈值的时候才报警
    if ((m_stat_info.search_fail_count >= (TUINT32)CConfBase::GetInt("err_num_threshold")) &&
	    ((float)m_stat_info.search_fail_count / m_stat_info.search_total_count >=
        (float)CConfBase::GetInt("err_rate_threshold") / 100))
	{
		// 报警内容
		snprintf(m_message, kMaxSendMessageSize,
		    "./send_warning.sh \"%s_%s-"
		    "Error:%u/Total:%u in %u sec![%s:%u]\" stat",
			CConfBase::GetString("project").c_str(),
            CGlobalServ::m_poConf->m_szModuleName,
		    m_stat_info.search_fail_count,
		    m_stat_info.search_total_count,
            CConfBase::GetInt("interval_time"),
            CGlobalServ::m_poConf->m_szModuleIp,
            CConfBase::GetInt("serv_port"));
		system(m_message);
		TSE_LOG_INFO(m_stat_log, ("Send error message: ErrorNum[%u]"
		        "/TotalNum[%u] in %u seconds!",
                m_stat_info.search_fail_count, m_stat_info.search_total_count, CConfBase::GetInt("interval_time")));
	}

	return 0;
}

int CStatistic::GetStatInfo(StatisticInfo *stat_info)
{
	*stat_info = m_stat_info;
	return 0;
}

int CStatistic::SendMessage( const TCHAR *pszMsg )
{
	// 报警内容
	snprintf(m_message, kMaxSendMessageSize,
		"./send_warning.sh \"%s\"", pszMsg);
	system(m_message);

    return 0;
}



