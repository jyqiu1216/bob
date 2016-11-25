#include "notic_task_mgr.h"
#include "notic_process.h"
#include "cgi_socket.h"
#include "conf_base.h"
#include "jsoncpp/json/json.h"

CTseLogger *CNoticProcess::m_poServLog = NULL;
CNoticTaskQueue *CNoticProcess::m_pTaskQueue = NULL;

TINT32 CNoticProcess::Init(CNoticTaskQueue *pTaskQueue, CTseLogger *poServLog)
{
    if (NULL == pTaskQueue || NULL == poServLog)
    {
        return -1;
    }

    m_poServLog = poServLog;
    m_pTaskQueue = pTaskQueue;
    // bzero(&m_server_addr, sizeof(m_server_addr)); 
    for (TUINT32 udwIdx = 0; udwIdx < m_vserver_addr_list.size(); udwIdx++)
    {
        delete m_vserver_addr_list[udwIdx];
    }
    m_vserver_addr_list.clear();
    if(0 == m_vserver_addr_list.size())
    {
        struct hostent *he;
        struct in_addr **addr_list;
        he = gethostbyname(CConfBase::GetString("domain", "notic_sever").c_str());
        TSE_LOG_INFO(m_poServLog, ("CNoticProcess::Init, [notic_server=%s]", he->h_name));
        
        addr_list = (struct in_addr **)he->h_addr_list;
        for(TINT32 dwIdx = 0; addr_list[dwIdx] != NULL; ++dwIdx)
        {
            struct sockaddr_in *psockaddr_in = new struct sockaddr_in;
            bzero(psockaddr_in, sizeof(struct sockaddr_in));
            psockaddr_in->sin_family = AF_INET; 
            psockaddr_in->sin_addr.s_addr = inet_addr(inet_ntoa(*addr_list[dwIdx])); 
            psockaddr_in->sin_port = htons(CConfBase::GetInt("port", "notic_sever")); 

            TSE_LOG_INFO(m_poServLog, ("CNoticProcess::Init, [notic_sever=%s] [notic_sever_port=%ld]", \
                                        inet_ntoa(*addr_list[dwIdx]), \
                                        CConfBase::GetInt("port", "notic_sever")));

            m_vserver_addr_list.push_back(psockaddr_in);
        }
    }
    if(0 == m_vserver_addr_list.size())
    {
        TSE_LOG_ERROR(m_poServLog, ("CNoticProcess::Init, m_vserver_addr_list is null"));        
        return -2;
    }

    m_dwSocket = 0;
    m_szBuf[0] = '\0';
    m_udwBufLen = 0;

    return 0;
}

void *CNoticProcess::Start(void *pParam)
{
    if (NULL == pParam)
    {
        return NULL;
    }

    CNoticProcess *poIns = (CNoticProcess *)pParam;
    poIns->WorkRoutine();
    return NULL;
}

TINT32 CNoticProcess::WorkRoutine()
{
    SNoticTask *pstNoticTask = NULL;
    while (1)
    {
    
        TSE_LOG_DEBUG(m_poServLog, ("CNoticProcess::WorkRoutine"));
    
        if (m_pTaskQueue->WaitTillPop(pstNoticTask) != 0)
        {
            TSE_LOG_DEBUG(m_poServLog, ("WaitTillPop: pop null"));
            continue;
        }

        if(0 != Notic_Process(pstNoticTask))
        {
            TSE_LOG_ERROR(m_poServLog, ("CNoticProcess failed"));
        }

        CNoticTaskMgr::GetInstance()->ReleaselNoticTask(pstNoticTask);
    }

    return 0;
}


TINT32 CNoticProcess::Notic_Process(SNoticTask *pstNoticTask)
{

    TINT32 dwRetCode = 0;

    Json::Value jNoticRoot = Json::Value(Json::objectValue);
    jNoticRoot["title"] = "";
    jNoticRoot["content"] = pstNoticTask->m_strContent;
    jNoticRoot["uid"] = pstNoticTask->m_ddwUid;
    jNoticRoot["aid"] = pstNoticTask->m_ddwAid;
    jNoticRoot["sid"] = pstNoticTask->m_ddwSid;
    jNoticRoot["notic_type"] = pstNoticTask->m_ddwNotiType;
    jNoticRoot["project"] = pstNoticTask->m_strProject;
    jNoticRoot["notic_flag"] = pstNoticTask->m_ddwNoticFlag;
    jNoticRoot["sound"] = pstNoticTask->m_strSound;


    // return ossGameEvaluateLogOut.str();
    Json::FastWriter write;
    string strNotic = write.write(jNoticRoot);
    if(0 < strNotic.size())
    {        
        strNotic.replace(strNotic .length() - 1, strNotic .length(), "\0");
    }
    
    dwRetCode = SendNoticServer(strNotic);
    if (dwRetCode < 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("Notic_Process: fail to reconnect notic server"));
        return -1;
    }


    TSE_LOG_INFO(m_poServLog, ("Notic_Process end: [strNotic=%s] [empty_size=%u]", \
                               strNotic.c_str(), \
                               m_pTaskQueue->Size()));

    return 0;
}




TINT32 CNoticProcess::SendNoticServer(string strNotic)
{
    TINT32 dwRetCode = 0;

    if(0 == m_vserver_addr_list.size())
    {
        TSE_LOG_ERROR(m_poServLog, ("CNoticProcess::SendNoticServer, m_vserver_addr_list is null"));        
        return -2;
    }


    if(m_dwSocket > 0)
    {
        // CCgiSocket::Close(m_dwSocket);
    }
    else
    {    
        /* ´´½¨socket */
        m_dwSocket = socket(AF_INET, SOCK_DGRAM, 0); 
        if(m_dwSocket < 0) 
        { 
            TSE_LOG_ERROR(m_poServLog, ("CNoticProcess::SendNoticServer, Create Socket Failed"));
            return -3;
        } 
    }
    srand(time(0)); 
    TINT32 dwIdxIp = rand() % m_vserver_addr_list.size();
    dwRetCode = sendto(m_dwSocket, strNotic.c_str(), strNotic.length() , 0, (struct sockaddr *)m_vserver_addr_list[dwIdxIp], sizeof(struct sockaddr_in));
    if(dwRetCode < 0) 
    {    
        TSE_LOG_ERROR(m_poServLog, ("CNoticProcess::SendNoticServer, sendto request failed [msg=%s] [retcode=%d] [Error=%s]", \
                                    strNotic.c_str(), dwRetCode, strerror(errno)));
        return -4;
    }
    else
    {
        TSE_LOG_DEBUG(m_poServLog, ("CNoticProcess::SendNoticServer, sendto request ok [msg=%s]", \
            strNotic.c_str()));
    }

    return 0;
}



