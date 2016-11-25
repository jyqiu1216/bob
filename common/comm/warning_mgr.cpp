#include "warning_mgr.h"
#include "curl/curl.h"
#include "jsoncpp/json/json.h"

using namespace std;

CWarningMgr *CWarningMgr::m_poWarningMgr = NULL;                             

CWarningMgr *CWarningMgr::GetInstance()
{
    if(NULL == m_poWarningMgr)
    {
        try
        { 
            m_poWarningMgr = new CWarningMgr;
        }
        catch(const std::bad_alloc &memExp)
        {       
            /*
            TSE_LOG_ERROR(CGameLog::m_poServLog, ("COpensslConnMgr::GetInstance [err_msg=%s]", \
                                                  memExp.what()));
            return NULL;      
            */      
            assert(m_poWarningMgr);
        }
    }
    return m_poWarningMgr;
}


CWarningMgr::~CWarningMgr()
{
    curl_global_cleanup();
}



TINT32 CWarningMgr::Init(CTseLogger *poLog)
{
    m_poServLog = poLog;

    // 初始化curl
    curl_global_init(CURL_GLOBAL_ALL);
 
    return 0;
}


// 发送告警信息到agent
TINT32 CWarningMgr::Send_Warning(string strWarningInfo)
{ 
     char szResultJson[MAX_JSON_LEN];
     szResultJson[0] = '\0';
     
     CURL *curl;
     struct curl_slist *chunk = NULL;
     CURLcode res;
     
     // head
     string strContentType = "Content-Type: application/json";

     curl = curl_easy_init();
     TSE_LOG_INFO(m_poServLog, ("CWarningMgr::Send_Warning: [curladdressbegin=%p]", curl));

     if(curl)
     {
         // 设置请求         
         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
         
         curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
         curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
         curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L); 
         curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
         curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
         curl_easy_setopt(curl, CURLOPT_POST, 1L); //设置为非0表示本次操作为post
         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
         curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strWarningInfo.c_str()); //postdata参数
         curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:1988/v1/push");
     
         // 设置回调函数
         curl_easy_setopt(curl, CURLOPT_WRITEDATA, szResultJson);
         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CWarningMgr::WriteData);
     
     
         // 发送请求
         res = curl_easy_perform(curl);
     
         if((CURLcode)0U != res)
         {        
             /* always cleanup */
             curl_easy_cleanup(curl);
             curl_slist_free_all(chunk);
             chunk = NULL;
             TSE_LOG_ERROR(m_poServLog, ("CWarningMgr::Send_Warning: curl_easy_perform failed [curlcodemsg=%s]", curl_easy_strerror(res)));
             return -3;
         }

         TSE_LOG_INFO(m_poServLog, ("CWarningMgr::Send_Warning: [curlcodemsg=%s]", curl_easy_strerror(res)));
         /* always cleanup */
         curl_easy_cleanup(curl);
         curl_slist_free_all(chunk);
         chunk = NULL;
     }
     else
     {
         TSE_LOG_ERROR(m_poServLog, ("CWarningMgr::Send_Warning: curl_easy_init failed"));
         return -4;
     }
     
     
     TSE_LOG_INFO(m_poServLog, ("CWarningMgr::Send_Warning: [ResultJson=%s]", szResultJson));
     return 0;
    
}





