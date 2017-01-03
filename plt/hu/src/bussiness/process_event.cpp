#include "process_event.h"
#include "procedure_base.h"
#include "common_func.h"
#include "game_define.h"
#include "common_base.h"
#include "city_base.h"
#include "msg_base.h"

TINT32 CProcessEvent::ProcessCmd_AllEventInfoGet(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    string szEventInfo = pstSession->m_stReqParam.m_szKey[0];

    Json::Value jTmp;
    Json::Reader reader;
    if (reader.parse(szEventInfo, jTmp) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
        return -1;
    }

    TUINT32 udwAlid = jTmp["alid"].asUInt();
    TUINT32 udwSid = jTmp["sid"].asUInt();
    TUINT32 udwUid = jTmp["uid"].asUInt();
    string strUname = jTmp["uname"].asString();
    string strAlNick = jTmp["alnick"].asString();
    TUINT32 udwTrialLv = jTmp["trial_lv"].asUInt();
    TUINT32 udwLang = pstSession->m_stReqParam.m_udwLang + 1;

    // 1.信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: event_info=%s [seq=%u]",
            szEventInfo.c_str(), pstSession->m_udwSeqNo));
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        // send request
        bNeedResponse = TRUE;
        EventReqInfo *pstEventReq = new EventReqInfo;
        EventReqInfo *pstThemeEvenReq = new EventReqInfo;

        pstSession->m_vecEventReq.push_back(pstEventReq);
        pstEventReq->m_udwIdxNo = 0;
        pstEventReq->m_udwRequestType = EN_REQUEST_TYPE__GET_ALL_INFO;
        pstEventReq->m_sReqContent = szEventInfo;
        pstEventReq->m_udwIsTheme = 0;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: EventReqInfo req_string=%s IsTheme=%u [seq=%u]",
            pstEventReq->m_sReqContent.c_str(), pstEventReq->m_udwIsTheme, pstSession->m_udwSeqNo));
        //theme event req
        pstSession->m_vecEventReq.push_back(pstThemeEvenReq);
        pstThemeEvenReq->SetVal(udwSid, udwUid, udwAlid, strUname, strAlNick, udwTrialLv, udwLang, 1, EN_REQUEST_TYPE__GET_ALL_INFO, 0);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: EventReqInfo req_string=%s IsTheme=%u [seq=%u]",
            pstThemeEvenReq->m_sReqContent.c_str(), pstThemeEvenReq->m_udwIsTheme, pstSession->m_udwSeqNo));
        dwRetCode = CBaseProcedure::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_EVENT_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: send event req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecEventRsp.size(); udwIdx++)
        {
            Json::Reader reader;
            Json::Value stEventDesc;

            if (pstSession->m_vecEventRsp[udwIdx]->dwRetCode != 0 || FALSE == reader.parse(pstSession->m_vecEventRsp[udwIdx]->sRspContent, stEventDesc))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: pase event info fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: [json_value=%s] [seq=%u]", pstSession->m_vecEventRsp[udwIdx]->sRspContent.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
        }        
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: pase event info [%s][seq=%u]", pstSession->m_vecEventRsp[0]->sRspContent.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
    }
    // 1. 发送获取数据的请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if (udwAlid)
        {
            // reset req
            pstSession->ResetCacheReq();
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__CACHE;


            // send request
            bNeedResponse = TRUE;
            CacheReqInfo *pstCacheReq = new CacheReqInfo;
            pstCacheReq->SetVal("al_member", "get", udwAlid, "");
            pstSession->m_vecCacheReq.push_back(pstCacheReq);

            //log
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: [CacheName=%s] [CacheOperate=%s] [uddwCacheKey=%ld] [CacheValue=%s] [seq=%u]",
                                                            pstCacheReq->m_strCacheName.c_str(), \
                                                            pstCacheReq->m_strCacheOperate.c_str(), \
                                                            pstCacheReq->m_uddwCacheKey, \
                                                            pstCacheReq->m_strCacheValue.c_str(), \
                                                            pstSession->m_udwSeqNo));

            dwRetCode = CBaseProcedure::SendCacheRequest(pstSession, EN_SERVICE_TYPE_CACHE_REQ);
            if(dwRetCode != 0)
            {        
                bNeedResponse = FALSE;
                pstSession->m_udwTmpAlmemberNum = 0;
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
                // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: send cache req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                // return -1;
            }

            return 0;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        }
    }

    if(EN_COMMAND_STEP__3 == pstSession->m_udwCommandStep)
    {
        if(pstSession->m_vecCacheRsp.size() > 0)
        {
            if(pstSession->m_vecCacheRsp[0]->dwRetCode != 0)
            {
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: parse cache info fail [seq=%u]", \
                                                        pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: [ServiceType=%u] [CacheName=%s] [CacheOperate=%s] [CacheKey=%lld] [CacheExist=%d] [seq=%u]", \
                                                       pstSession->m_vecCacheRsp[0]->udwServiceType, \
                                                       pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                       pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                       pstSession->m_vecCacheRsp[0]->m_uddwCacheKey, \
                                                       pstSession->m_vecCacheRsp[0]->m_bExist, \
                                                       pstSession->m_stUserInfo.m_udwBSeqNo));
                
                if("al_member" == pstSession->m_vecCacheRsp[0]->m_strCacheName)
                {
                    if("get" == pstSession->m_vecCacheRsp[0]->m_strCacheOperate)
                    {
                        if(TRUE == pstSession->m_vecCacheRsp[0]->m_bExist)
                        {
                            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: cache info exist [seq=%u]", \
                                                                    pstSession->m_stUserInfo.m_udwBSeqNo));
                            
                            AwsRspInfo stAwsRspInfo;
                            stAwsRspInfo.Reset();
                            stAwsRspInfo.dwRetCode = 200;
                            stAwsRspInfo.sRspContent = pstSession->m_vecCacheRsp[0]->m_strCacheValue;
                            dwRetCode = CAwsResponse::OnQueryRsp(stAwsRspInfo, pstSession->m_atbTmpAlmember, \
                                                                 sizeof(TbAl_member), 1000/*MAX_ALLIANCE_MEMBER_NUM*/);
                            if(dwRetCode > 0)
                            {
                                pstSession->m_udwTmpAlmemberNum = dwRetCode;
                                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                            }
                            else
                            {
                                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;                                
                                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: not alliance member [seq=%u]", \
                                                                        pstSession->m_stUserInfo.m_udwBSeqNo));
                            }
                        }
                        else
                        {
                            // 获取al_member表
                            // reset req
                            pstSession->ResetAwsReq();
                            // next procedure
                            pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
                            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

                            // set player get
                            CAwsRequest::AllAlMemberQuery(pstSession, udwAlid);

                            // send request
                            bNeedResponse = TRUE;
                            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                            if (dwRetCode < 0)
                            {
                                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: send aws req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                                return -1;
                            }
                            return 0;
                        }
                    }
                    else
                    {   
                        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: error cache operate [CacheOperate=%s] [seq=%u]", \
                                                                pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                                pstSession->m_stUserInfo.m_udwBSeqNo));
                    }
                }
                else
                {
                    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: error cache name [CacheName=%s] [seq=%u]", \
                                                            pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                            pstSession->m_stUserInfo.m_udwBSeqNo));
                }
            }
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: not cache response [seq=%u]", \
                                                    pstSession->m_stUserInfo.m_udwBSeqNo));

        }
    }

    

    // 2. 解析获取的数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        // get data info
        TUINT32 udwMemberNum = 0;
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0],
            pstSession->m_atbTmpAlmember, sizeof(TbAl_member), 1000/*MAX_ALLIANCE_MEMBER_NUM*/);
        if(dwRetCode > 0)
        {
            udwMemberNum = dwRetCode;
            pstSession->m_udwTmpAlmemberNum = dwRetCode;

            // reset req
            pstSession->ResetCacheReq();
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__CACHE;

            // send request
            bNeedResponse = TRUE;
            CacheReqInfo *pstCacheReq = new CacheReqInfo;
            pstCacheReq->SetVal("al_member", "set", udwAlid, pstSession->m_vecAwsRsp[0]->sRspContent);
            pstSession->m_vecCacheReq.push_back(pstCacheReq);

            //log
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: [CacheName=%s] [CacheOperate=%s] [uddwCacheKey=%lu] [seq=%u]",
                                                               pstCacheReq->m_strCacheName.c_str(), \
                                                               pstCacheReq->m_strCacheOperate.c_str(), \
                                                               pstCacheReq->m_uddwCacheKey, \
                                                               pstSession->m_udwSeqNo));

            dwRetCode = CBaseProcedure::SendCacheRequest(pstSession, EN_SERVICE_TYPE_CACHE_REQ);
            if(dwRetCode != 0)
            {
                bNeedResponse = FALSE;
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
                // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: send cache req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                // return -1;
            }

            return 0;
            
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: not aws response [seq=%u]", \
                                                    pstSession->m_stUserInfo.m_udwBSeqNo));

        }
    }


    if(EN_COMMAND_STEP__5 == pstSession->m_udwCommandStep)
    {
        if(pstSession->m_vecCacheRsp.size() > 0)
        {
            if(pstSession->m_vecCacheRsp[0]->dwRetCode != 0)
            {
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: parse cache info fail [seq=%u]", \
                                                        pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: [ServiceType=%u] [CacheName=%s] [CacheOperate=%s] [CacheKey=%lld] [CacheExist=%d] [seq=%u]", \
                                                       pstSession->m_vecCacheRsp[0]->udwServiceType, \
                                                       pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                       pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                       pstSession->m_vecCacheRsp[0]->m_uddwCacheKey, \
                                                       pstSession->m_vecCacheRsp[0]->m_bExist, \
                                                       pstSession->m_stUserInfo.m_udwBSeqNo));
                if("al_member" == pstSession->m_vecCacheRsp[0]->m_strCacheName)
                {
                    if("set" == pstSession->m_vecCacheRsp[0]->m_strCacheOperate)
                    {
                        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: set cache info success [seq=%u]", \
                                                                pstSession->m_stUserInfo.m_udwBSeqNo));
                    }
                    else
                    {
                        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: error cache operate [CacheOperate=%s] [seq=%u]", \
                                                                pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                                pstSession->m_stUserInfo.m_udwBSeqNo));
                    }
                }
                else
                {
                    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: error cache name [CacheName=%s] [seq=%u]", \
                                                            pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                            pstSession->m_stUserInfo.m_udwBSeqNo));
                }

                
            }
        }
        else
        {    
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: not cache response [seq=%u]", \
                                                    pstSession->m_stUserInfo.m_udwBSeqNo));

        }
    }
    

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessEvent::ProcessCmd_EventInfoGet(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwEventId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    string szEventInfo = pstSession->m_stReqParam.m_szKey[1];

    Json::Value jTmp;
    Json::Reader reader;
    if (reader.parse(szEventInfo, jTmp) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
        return -1;
    }

    TUINT32 udwAlid = jTmp["alid"].asUInt();

    // 1.信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: event_info=%s [seq=%u]",
            szEventInfo.c_str(), pstSession->m_udwSeqNo));

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;


        // send request
        bNeedResponse = TRUE;
        EventReqInfo *pstEventReq = new EventReqInfo;

        pstSession->m_vecEventReq.push_back(pstEventReq);
        pstEventReq->m_udwIdxNo = 0;
        pstEventReq->m_udwRequestType = EN_REQUEST_TYPE__GET_INFO;
        jTmp["request_type"] = EN_REQUEST_TYPE__GET_INFO;
        jTmp["event_type"] = udwEventId;
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        pstEventReq->m_sReqContent = writer.write(jTmp);

        //log
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: EventReqInfo req_string=%s [seq=%u]",
            pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));

        dwRetCode = CBaseProcedure::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_EVENT_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet send event req fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;

    }

    //
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if(pstSession->m_vecEventRsp.size() > 0)
        {
            Json::Reader reader;
            Json::Value stEventDesc;

            if(pstSession->m_vecEventRsp[0]->dwRetCode != 0 || FALSE == reader.parse(pstSession->m_vecEventRsp[0]->sRspContent, stEventDesc))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: pase event info fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
        }
    }
    // 1. 发送获取数据的请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
       if(udwAlid)
        {
            // reset req
            pstSession->ResetCacheReq();
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__CACHE;


            // send request
            bNeedResponse = TRUE;
            CacheReqInfo *pstCacheReq = new CacheReqInfo;
            pstCacheReq->SetVal("al_member", "get", udwAlid, "");
            pstSession->m_vecCacheReq.push_back(pstCacheReq);

            //log
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: [CacheName=%s] [CacheOperate=%s] [uddwCacheKey=%ld] [CacheValue=%s] [seq=%u]",
                                                            pstCacheReq->m_strCacheName.c_str(), \
                                                            pstCacheReq->m_strCacheOperate.c_str(), \
                                                            pstCacheReq->m_uddwCacheKey, \
                                                            pstCacheReq->m_strCacheValue.c_str(), \
                                                            pstSession->m_udwSeqNo));

            dwRetCode = CBaseProcedure::SendCacheRequest(pstSession, EN_SERVICE_TYPE_CACHE_REQ);
            if(dwRetCode != 0)
            {        
                bNeedResponse = FALSE;
                pstSession->m_udwTmpAlmemberNum = 0;
                pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: send cache req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                // return -1;
            }

            return 0;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        }
    }

    
    if(EN_COMMAND_STEP__3 == pstSession->m_udwCommandStep)
   {
       if(pstSession->m_vecCacheRsp.size() > 0)
       {
           if(pstSession->m_vecCacheRsp[0]->dwRetCode != 0)
           {
               pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
               TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: parse cache info fail [seq=%u]", \
                                                       pstSession->m_stUserInfo.m_udwBSeqNo));
           }
           else
           {
               TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: [ServiceType=%u] [CacheName=%s] [CacheOperate=%s] [CacheKey=%lld] [CacheValue=%s] [CacheExist=%d] [seq=%u]", \
                                                      pstSession->m_vecCacheRsp[0]->udwServiceType, \
                                                      pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                      pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                      pstSession->m_vecCacheRsp[0]->m_uddwCacheKey, \
                                                      pstSession->m_vecCacheRsp[0]->m_strCacheValue.c_str(), \
                                                      pstSession->m_vecCacheRsp[0]->m_bExist, \
                                                      pstSession->m_stUserInfo.m_udwBSeqNo));
               
               if("al_member" == pstSession->m_vecCacheRsp[0]->m_strCacheName)
               {
                   if("get" == pstSession->m_vecCacheRsp[0]->m_strCacheOperate)
                   {
                       if(TRUE == pstSession->m_vecCacheRsp[0]->m_bExist)
                       {
                           TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: cache info exist [seq=%u]", \
                                                                   pstSession->m_stUserInfo.m_udwBSeqNo));
                           
                           AwsRspInfo stAwsRspInfo;
                           stAwsRspInfo.Reset();
                           stAwsRspInfo.dwRetCode = 200;
                           stAwsRspInfo.sRspContent = pstSession->m_vecCacheRsp[0]->m_strCacheValue;
                           dwRetCode = CAwsResponse::OnQueryRsp(stAwsRspInfo, pstSession->m_atbTmpAlmember, \
                                                                sizeof(TbAl_member), 1000/*MAX_ALLIANCE_MEMBER_NUM*/);
                           if(dwRetCode > 0)
                           {
                               pstSession->m_udwTmpAlmemberNum = dwRetCode;
                               pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                           }
                           else
                           {
                               pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;                                
                               TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: not alliance member [seq=%u]", \
                                                                       pstSession->m_stUserInfo.m_udwBSeqNo));
                           }
                       }
                       else
                       {
                           // 获取al_member表
                           // reset req
                           pstSession->ResetAwsReq();
                           // next procedure
                           pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
                           pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

                           // set player get
                           CAwsRequest::AllAlMemberQuery(pstSession, udwAlid);

                           // send request
                           bNeedResponse = TRUE;
                           dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                           if (dwRetCode < 0)
                           {
                               pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                               TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: send aws req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                               return -1;
                           }
                           return 0;
                       }
                   }
                   else
                   {   
                       pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                       TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: error cache operate [CacheOperate=%s] [seq=%u]", \
                                                               pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                               pstSession->m_stUserInfo.m_udwBSeqNo));
                   }
               }
               else
               {
                   pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                   TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: error cache name [CacheName=%s] [seq=%u]", \
                                                           pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                           pstSession->m_stUserInfo.m_udwBSeqNo));
               }
           }
       }
       else
       {
           pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
           TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: not cache response [seq=%u]", \
                                                   pstSession->m_stUserInfo.m_udwBSeqNo));

       }
   }
    
      
    // 2. 解析获取的数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        // get data info
        TUINT32 udwMemberNum = 0;
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0],
            pstSession->m_atbTmpAlmember, sizeof(TbAl_member), 1000/*MAX_ALLIANCE_MEMBER_NUM*/);
        if(dwRetCode > 0)
        {
            udwMemberNum = dwRetCode;
            pstSession->m_udwTmpAlmemberNum = dwRetCode;

            // reset req
            pstSession->ResetCacheReq();
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__CACHE;

            // send request
            bNeedResponse = TRUE;
            CacheReqInfo *pstCacheReq = new CacheReqInfo;
            pstCacheReq->SetVal("al_member", "set", udwAlid, pstSession->m_vecAwsRsp[0]->sRspContent);
            pstSession->m_vecCacheReq.push_back(pstCacheReq);

            //log
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: [CacheName=%s] [CacheOperate=%s] [uddwCacheKey=%lu] [CacheValue=%s] [seq=%u]",
                                                              pstCacheReq->m_strCacheName.c_str(), \
                                                              pstCacheReq->m_strCacheOperate.c_str(), \
                                                              pstCacheReq->m_uddwCacheKey, \
                                                              pstCacheReq->m_strCacheValue.c_str(), \
                                                              pstSession->m_udwSeqNo));

            dwRetCode = CBaseProcedure::SendCacheRequest(pstSession, EN_SERVICE_TYPE_CACHE_REQ);
            if(dwRetCode != 0)
            {
                bNeedResponse = FALSE;                
                pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: send cache req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                // return -1;
            }

            return 0;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: not aws response [seq=%u]", \
                                                    pstSession->m_stUserInfo.m_udwBSeqNo));

        }
  }


  if(EN_COMMAND_STEP__5 == pstSession->m_udwCommandStep)
  {
      if(pstSession->m_vecCacheRsp.size() > 0)
      {
          if(pstSession->m_vecCacheRsp[0]->dwRetCode != 0)
          {
              pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
              TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: parse cache info fail [seq=%u]", \
                                                      pstSession->m_stUserInfo.m_udwBSeqNo));
          }
          else
          {
              TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: [ServiceType=%u] [CacheName=%s] [CacheOperate=%s] [CacheKey=%lld] [CacheValue=%s] [CacheExist=%d] [seq=%u]", \
                                                     pstSession->m_vecCacheRsp[0]->udwServiceType, \
                                                     pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                     pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                     pstSession->m_vecCacheRsp[0]->m_uddwCacheKey, \
                                                     pstSession->m_vecCacheRsp[0]->m_strCacheValue.c_str(), \
                                                     pstSession->m_vecCacheRsp[0]->m_bExist, \
                                                     pstSession->m_stUserInfo.m_udwBSeqNo));
              if("al_member" == pstSession->m_vecCacheRsp[0]->m_strCacheName)
              {
                  if("set" == pstSession->m_vecCacheRsp[0]->m_strCacheOperate)
                  {
                      TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: set cache info success [seq=%u]", \
                                                              pstSession->m_stUserInfo.m_udwBSeqNo));
                  }
                  else
                  {
                      pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                      TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: error cache operate [CacheOperate=%s] [seq=%u]", \
                                                              pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                              pstSession->m_stUserInfo.m_udwBSeqNo));
                  }
              }
              else
              {
                  pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                  TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: error cache name [CacheName=%s] [seq=%u]", \
                                                          pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                          pstSession->m_stUserInfo.m_udwBSeqNo));
              }

              
          }
      }
      else
      {    
          pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
          TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_EventInfoGet: not cache response [seq=%u]", \
                                                  pstSession->m_stUserInfo.m_udwBSeqNo));

      }
  }
  

  pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
  pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
  return 0;
}

TINT32 CProcessEvent::ProcessCmd_HistoryEventInfoGet(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwEventId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    string szEventInfo = pstSession->m_stReqParam.m_szKey[1];

    Json::Value jTmp;
    Json::Reader reader;
    if (reader.parse(szEventInfo, jTmp) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
        return -1;
    }

    // 1.信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_HistoryEventInfoGet: event_info=%s [seq=%u]",
            szEventInfo.c_str(), pstSession->m_udwSeqNo));

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;


        // send request
        bNeedResponse = TRUE;
        EventReqInfo *pstEventReq = new EventReqInfo;
        pstSession->m_vecEventReq.push_back(pstEventReq);
        pstEventReq->m_udwIdxNo = 0;
        pstEventReq->m_udwRequestType = EN_REQUEST_TYPE__GEN_HISTORY_INFO;
        jTmp["request_type"] = EN_REQUEST_TYPE__GEN_HISTORY_INFO;
        jTmp["event_type"] = udwEventId;
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        pstEventReq->m_sReqContent = writer.write(jTmp);
        pstEventReq->m_udwIsTheme = 0;

        //log
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("EventReqInfo req_string=%s  [IsTheme=%u] [seq=%u]",
            pstEventReq->m_sReqContent.c_str(), pstEventReq->m_udwIsTheme, pstSession->m_udwSeqNo));

        dwRetCode = CBaseProcedure::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_EVENT_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessEvent::ProcessCmd_AllEventInfoGet send event req fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;

    }

    //
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        if(pstSession->m_vecEventRsp.size() > 0)
        {
            Json::Reader reader;
            Json::Value stEventDesc;

            if(pstSession->m_vecEventRsp[0]->dwRetCode != 0 || FALSE == reader.parse(pstSession->m_vecEventRsp[0]->sRspContent, stEventDesc))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessEvent::ProcessCmd_AllEventInfoGet pase event info fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
        }
    }
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

    return 0;
}

TINT32 CProcessEvent::ProcessCmd_ThemeHistoryEventInfoGet(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    string szEventInfo = pstSession->m_stReqParam.m_szKey[0];

    Json::Value jTmp;
    Json::Reader reader;
    if (reader.parse(szEventInfo, jTmp) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
        return -1;
    }

    TUINT32 udwSid = jTmp["sid"].asUInt();
    TUINT32 udwUid = jTmp["uid"].asUInt();
    TUINT32 udwLang = pstSession->m_stReqParam.m_udwLang + 1;

    // 1.信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ThemeHistoryEventInfoGet: event_info=%s [seq=%u]",
            szEventInfo.c_str(), pstSession->m_udwSeqNo));

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        // send request
        bNeedResponse = TRUE;
        EventReqInfo *pstEventReq = new EventReqInfo;
        pstSession->m_vecEventReq.push_back(pstEventReq);
        pstEventReq->SetVal(udwSid, udwUid, udwLang, 1, EN_REQUEST_TYPE__GEN_HISTORY_INFO, 0);
        //log
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("EventReqInfo req_string=%s [seq=%u]",
            pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));

        dwRetCode = CBaseProcedure::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_EVENT_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessEvent::ProcessCmd_AllEventInfoGet send event req fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        if (pstSession->m_vecEventRsp.size() > 0)
        {
            Json::Reader reader;
            Json::Value stEventDesc;

            if (pstSession->m_vecEventRsp[0]->dwRetCode != 0 || FALSE == reader.parse(pstSession->m_vecEventRsp[0]->sRspContent, stEventDesc))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessEvent::ProcessCmd_AllEventInfoGet pase event info fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ThemeHistoryEventInfoGet: [response=%s] [seq=%u]",
                pstSession->m_vecEventRsp[0]->sRspContent.c_str(), pstSession->m_udwSeqNo));

        }
    }
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

    return 0;
}

TINT32 CProcessEvent::ProcessCmd_SendEventScore(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwTuid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT64 ddwScore = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    TINT64 ddwAlid = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
    //发送赠送分请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        // send request
        bNeedResponse = TRUE;
        EventReqInfo *pstEventReq = new EventReqInfo;

        pstSession->m_vecEventReq.push_back(pstEventReq);
        pstEventReq->SetVal(ptbPlayer->m_nSid, ptbPlayer->m_nUid, ptbPlayer->m_sUin, ddwAlid, udwTuid, ddwScore,
            EN_REQUEST_TYPE__SEND_EVENT_SCORE, 1, 0);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_SendEventScore: EventReqInfo req_string=%s [seq=%u]",
            pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
        dwRetCode = CBaseProcedure::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_EVENT_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: send event req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }
    //解析响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        TINT32 dwStatus = -1;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecEventRsp.size(); udwIdx++)
        {
            Json::Reader reader;
            Json::Value stEventDesc;

            if (pstSession->m_vecEventRsp[udwIdx]->dwRetCode != 0 || FALSE == reader.parse(pstSession->m_vecEventRsp[udwIdx]->sRspContent, stEventDesc))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SendEventScore: pase event info fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_SendEventScore: [json_value=%s] [seq=%u]", pstSession->m_vecEventRsp[udwIdx]->sRspContent.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
            dwStatus = stEventDesc["status"].asInt();
            if (dwStatus != 0)
            {
                switch (dwStatus)
                {
                case 1:
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_GIVE_SCORE;
                    break;
                case 2:
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__AL_MEMBER_NOT_PARTICIPATE;
                    break;
                case 3:
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_THE_SAME_ALLIANCE;
                    break;
                case 4:
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__POINT_OVERNUMBERED;
                    break;
                case 5:
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EVENT_ENDED;
                    break;
                default:
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
                    break;
                }
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SendEventScore: send score fail [status=%d] [seq=%u]", dwStatus, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
        }
        //赠送分数成功，发送邮件
        if (dwStatus == 0)
        {
            Json::Value jsonEscs = Json::Value(Json::arrayValue);
            Json::FastWriter tmpWriter;
            tmpWriter.omitEndingLineFeed();
            jsonEscs.append(ptbPlayer->m_sUin);
            jsonEscs.append(ddwScore);
            CMsgBase::SendOperateMail(udwTuid, EN_MAIL_ID__EVENT_SEND_SCORE, ptbPlayer->m_nSid, SYSTEM_EVENT, tmpWriter.write(jsonEscs).c_str(), tmpWriter.write(jsonEscs).c_str(), "");
        }
    }
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}