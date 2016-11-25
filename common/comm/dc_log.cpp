#include "dc_log.h"
#include "time_utils.h"
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "conf_base.h"

TINT32 CDcUdpLog::SendFormatLogToDc( string strNotic, CTseLogger *poLog )
{
    TUINT64 uddwTimeBeg = CTimeUtils::GetCurTimeUs();
    TUINT64 uddwTimeEnd = 0;

    TUINT32 udwDcSvrNum = CConfBase::GetInt("num", "dc_log_svr");
    TINT32 udwSvrIdx = rand()%udwDcSvrNum;
    TCHAR szKeyIp[64];
    TCHAR szKeyPort[32];
    sprintf(szKeyIp, "ip_%d", udwSvrIdx);
    sprintf(szKeyPort, "port_%d", udwSvrIdx);
    string strDcIp =  CConfBase::GetString(szKeyIp, "dc_log_svr");
    TINT32 dwDcPort = CConfBase::GetInt(szKeyPort, "dc_log_svr");

    TINT32 dwSocket;
    struct sockaddr_in server_addr; 
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = inet_addr(strDcIp.c_str()); 
    server_addr.sin_port = htons(dwDcPort);

    /* ´´½¨socket */
    dwSocket = socket(AF_INET, SOCK_DGRAM, 0); 
    if(dwSocket < 0) 
    { 
        TSE_LOG_ERROR(poLog, ("SendFormatLogToDc, Create Socket Failed"));
        return -1;
    } 
    srand(time(0)); 
    TINT32 dwRetCode = sendto(dwSocket, strNotic.c_str(), strNotic.length()+1 , 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    uddwTimeEnd = CTimeUtils::GetCurTimeUs();
    if(dwRetCode < 0) 
    {
        TSE_LOG_ERROR(poLog, ("SendFormatLogToDc, send data Failed, ret=%d, error=%s, cost_us=%ld, ip=%s:%d", dwRetCode, strerror(errno), uddwTimeEnd-uddwTimeBeg, strDcIp.c_str(), dwDcPort));
        return -2;
    }
    else
    {
        //TODO:delete
        TSE_LOG_DEBUG(poLog, ("SendFormatLogToDc, send data suc, len=%d, socket=%d, cost_us=%ld, ip=%s:%d", dwRetCode, dwSocket, uddwTimeEnd-uddwTimeBeg, strDcIp.c_str(), dwDcPort));
    }

    close(dwSocket);

    return 0;
}

