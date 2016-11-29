#include <sys/time.h>
#include "statistic.h"
#include "conf_base.h"
#include "jsoncpp/json/json.h"
#include "warning_mgr.h"
#include "global_serv.h"

CStatistic *CStatistic::m_stat_tool = NULL;

CStatistic::CStatistic()
{
    pthread_mutex_init(&m_mtxLock, NULL);
}

CStatistic::~CStatistic()
{
    pthread_mutex_destroy(&m_mtxLock);
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
    GuardMutex Guard(m_mtxLock);
    m_stat_info.Reset();
    m_mmAwsConsume.clear();
    return 0;
}

int CStatistic::OutputResult()
{

    StatisticInfo stStatInfo;

    // GuardMutex Guard(m_mtxLock);
    pthread_mutex_lock(&m_mtxLock);

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
        total > 0 ? m_stat_info.search_max_time_us / 1000.0 : 0,
        total > 0 ? m_stat_info.search_time_0_50_count * 100 / total : 0,
        total > 0 ? m_stat_info.search_time_50_100_count * 100 / total : 0,
        total > 0 ? m_stat_info.search_time_100_300_count * 100 / total : 0,
        total > 0 ? m_stat_info.search_time_300_500_count * 100 / total : 0,
        total > 0 ? m_stat_info.search_time_500_1000_count * 100 / total : 0,
        total > 0 ? m_stat_info.search_time_1000_up_count * 100 / total : 0
        ));

    TSE_LOG_INFO(m_stat_log, ("awsconsumeinfo begin:"));
    for (map<string, map<string, double> >::iterator iter_table = m_mmAwsConsume.begin(); iter_table != m_mmAwsConsume.end(); ++iter_table)
    {
        map<string, double>& mAwsConsume = iter_table->second;
        for (map<string, double>::iterator iter_oper = mAwsConsume.begin(); iter_oper != mAwsConsume.end(); ++iter_oper)
        {
            TSE_LOG_INFO(m_stat_log, ("%s  %s:%lf", iter_table->first.c_str(), iter_oper->first.c_str(), iter_oper->second));
        }
    }

    /*TSE_LOG_INFO(m_stat_log, ("--------------table:%s--------------", m_mmAwsConsume));
    for (map<string, double>::iterator iter = m_mAwsConsume.begin(); iter != m_mAwsConsume.end(); ++iter)
    {
    TSE_LOG_INFO(m_stat_log, ("%s:%lf", iter->first.c_str(), iter->second));
    }*/
    TSE_LOG_INFO(m_stat_log, ("awsconsumeinfo end:"));

    // 建立在该结构体的定义里不含指针
    stStatInfo = m_stat_info;

    pthread_mutex_unlock(&m_mtxLock);



    // 统计指标上报
    Json::FastWriter jsonWriter;
    Json::Value jPostDataBody = Json::Value(Json::arrayValue);

    SetWarningData(jPostDataBody, "total_count", stStatInfo.search_total_count);
    SetWarningData(jPostDataBody, "succ_count", stStatInfo.search_succ_count);
    SetWarningData(jPostDataBody, "fail_count", stStatInfo.search_fail_count);
    SetWarningData(jPostDataBody, "shield_count", stStatInfo.search_shield_count);
    SetWarningData(jPostDataBody, "req_avg_time", total > 0 ? stStatInfo.search_total_time_us / total / 1000.0 : 0);
    SetWarningData(jPostDataBody, "req_max_time", total > 0 ? stStatInfo.search_max_time_us / 1000.0 : 0);
    SetWarningData(jPostDataBody, "0_50ms_ratio", total > 0 ? stStatInfo.search_time_0_50_count * 100 / total : 0);
    SetWarningData(jPostDataBody, "50_100ms_ratio", total > 0 ? stStatInfo.search_time_50_100_count * 100 / total : 0);
    SetWarningData(jPostDataBody, "100_300ms_ratio", total > 0 ? stStatInfo.search_time_100_300_count * 100 / total : 0);
    SetWarningData(jPostDataBody, "300_500ms_ratio", total > 0 ? stStatInfo.search_time_300_500_count * 100 / total : 0);
    SetWarningData(jPostDataBody, "500_1000ms_ratio", total > 0 ? stStatInfo.search_time_500_1000_count * 100 / total : 0);
    SetWarningData(jPostDataBody, "1000ms_up_ratio", total > 0 ? stStatInfo.search_time_1000_up_count * 100 / total : 0);


    string strPostData = jsonWriter.write(jPostDataBody);
    CWarningMgr::GetInstance()->Send_Warning(strPostData);


    TSE_LOG_INFO(m_stat_log, ("warning msg [%s]", strPostData.c_str()));


    return 0;
}

TVOID CStatistic::SetWarningData(Json::Value &jPostDataBody, string strMetric, TUINT64 uddwValue)
{
    ostringstream ossContent;

    // endpoint
    ossContent.str("");
    ossContent << "ip-" << CGlobalServ::m_poConf->m_szModuleIp << ":" << CConfBase::GetInt("serv_port");
    string strEndpoint = ossContent.str().c_str();
    // tags
    ossContent.str("");
    ossContent << "proj=" << CConfBase::GetString("project") << "," << "mod=" << CGlobalServ::m_poConf->m_szModuleName;
    string strTags = ossContent.str().c_str();


    Json::Value jSignalPostDataBody = Json::Value(Json::objectValue);
    jSignalPostDataBody["metric"] = strMetric;
    jSignalPostDataBody["endpoint"] = strEndpoint;
    jSignalPostDataBody["tags"] = strTags;
    jSignalPostDataBody["value"] = uddwValue;
    jSignalPostDataBody["step"] = m_stat_interval_time;
    jSignalPostDataBody["counterType"] = "GAUGE";
    jSignalPostDataBody["timestamp"] = CTimeUtils::GetUnixTime();
    jPostDataBody.append(jSignalPostDataBody);
}


int CStatistic::AddSearchSucc(TUINT64 search_time)
{
    GuardMutex Guard(m_mtxLock);
    AddTimeStat(search_time);
    m_stat_info.search_succ_count++;
    m_stat_info.search_total_count++;
    return 0;
}

int CStatistic::AddSearchFail(TUINT64 search_time)
{
    GuardMutex Guard(m_mtxLock);
    AddTimeStat(search_time);
    m_stat_info.search_fail_count++;
    m_stat_info.search_total_count++;
    return 0;
}

int CStatistic::AddSearchShield(TUINT64 search_time)
{
    GuardMutex Guard(m_mtxLock);
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

    bool bFlag = false;

    { //控制Guard的生命周期
        GuardMutex Guard(m_mtxLock);

        // 当错误数大于阈值并且错误比例大于阈值的时候才报警
        if ((m_stat_info.search_fail_count >= (TUINT32)CConfBase::GetInt("err_num_threshold")) &&
            ((float)m_stat_info.search_fail_count / m_stat_info.search_total_count >=
            (float)CConfBase::GetInt("err_rate_threshold") / 100))
        {
            bFlag = true;
            // 报警内容
            snprintf(m_message, kMaxSendMessageSize,
                "./send_warning.sh \"%s_%s-"
                "Error:%u/Total:%u in %u sec![%s:%d]\" stat",
                CConfBase::GetString("project").c_str(),
                CGlobalServ::m_poConf->m_szModuleName,
                m_stat_info.search_fail_count,
                m_stat_info.search_total_count,
                CConfBase::GetInt("interval_time"),
                CGlobalServ::m_poConf->m_szModuleIp,
                CConfBase::GetInt("serv_port"));
            TSE_LOG_INFO(m_stat_log, ("Send error message: ErrorNum[%u]"
                "/TotalNum[%u] in %u seconds!",
                m_stat_info.search_fail_count, m_stat_info.search_total_count, CConfBase::GetInt("interval_time")));
        }
    }

    if (bFlag)
    {
        system(m_message);
    }

    return 0;
}

int CStatistic::GetStatInfo(StatisticInfo *stat_info)
{
    *stat_info = m_stat_info;
    return 0;
}

int CStatistic::SendMessage(const TCHAR *pszMsg)
{
    // 报警内容
    snprintf(m_message, kMaxSendMessageSize,
        "./send_warning.sh \"%s\"", pszMsg);
    system(m_message);
    return 0;
}



