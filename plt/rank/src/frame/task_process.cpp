#include "task_process.h"
#include "hu_work.h"
#include "base/os/wtsesocket.h"
#include "session_mgr.h"
#include "template_fun.h"
#include "base_def.h"
#include "procedure_base.h"
#include "rc4.h"
#include "statistic.h"
#include "aws_response.h"
#include "aws_request.h"
#include "common_func.h"
#include "global_serv.h"
#include "conf_base.h"
#include "game_command.h"

bool CTaskProcess::EncryptUrl(char *pszIn, char *pszOut, int &dwOutLen, float fVersion)
{
    if(pszIn == NULL || pszOut == NULL)
    {
        return false;
    }
    int dwInLen = strlen(pszIn);
    int dwEncryptDataLen = 0;
    int dwOutSize = dwOutLen;
    CEncryptCR4 *poEncrypt = CEncryptCR4Mgr::get_instance()->GetEncrypt(fVersion);
    if(poEncrypt == NULL)
    {
        return false;
    }

    dwOutLen = 0;
    pszOut[0] = 0;
    m_szEncryptBuf[0] = 0;
    m_szEncryptUrl[0] = 0;

    // md5: 加固定串再MD5，防止外围破解
    TUINT64 uddwKey = 0;
    if(pszIn == NULL || pszIn[0] == 0)
    {
        uddwKey = 0;
    }
    else
    {
        strcat(pszIn, "&crystr=LeyiCrypt123");
        MD5String(pszIn, (TUCHAR*)&uddwKey, sizeof(uddwKey));
        pszIn[dwInLen] = 0;
        sprintf(pszIn + dwInLen, "&md5str=%lu", uddwKey);
        dwInLen = strlen(pszIn);
    }

    // rc4: 输入长度和输出长度相等
    if(dwOutSize < dwInLen)
    {
        return false;
    }
    poEncrypt->rc4((const unsigned char*)pszIn, dwInLen, (unsigned char*)m_szEncryptBuf, dwEncryptDataLen);

    // base64 encode
    CUtilBase64::encode((char*)m_szEncryptBuf, (char*)m_szEncryptBuf + dwEncryptDataLen, pszOut, dwOutLen);

    return true;
}

bool CTaskProcess::DecryptUrl(char *pszIn, char *pszOut, int &dwOutLen, float fVersion)
{
    if(pszIn == NULL || pszOut == NULL)
    {
        return false;
    }

    bool bRetCode = false;
    int dwInLen = strlen(pszIn);
    int dwEncryptDataLen = 0;
    CEncryptCR4 *poEncrypt = CEncryptCR4Mgr::get_instance()->GetEncrypt(fVersion);
    if(poEncrypt == NULL)
    {
        return false;
    }

    // base64解码
    CUtilBase64::decode((char*)pszIn, (char*)pszIn + dwInLen, (char*)m_szEncryptBuf, dwEncryptDataLen);

    // rc4
    poEncrypt->rc4((const unsigned char*)m_szEncryptBuf, dwEncryptDataLen, (unsigned char*)pszOut, dwOutLen);
    bRetCode = true;

    // md5 check
    strcpy(m_szEncryptBuf, pszOut);
    const TCHAR *pMd5Param = "&md5str=";
    TUINT64 uddwRawMd5 = 0, uddwNewMd5 = 0;
    TCHAR *pMd5Str = strstr(m_szEncryptBuf, pMd5Param);
    if(pMd5Str)
    {
        uddwRawMd5 = strtoul(pMd5Str + strlen(pMd5Param), NULL, 10);
        *pMd5Str = 0;

        strcat(m_szEncryptBuf, "&crystr=LeyiCrypt123");
        MD5String(m_szEncryptBuf, (TUCHAR*)&uddwNewMd5, sizeof(uddwNewMd5));

        if(uddwRawMd5 != uddwNewMd5)
        {
            bRetCode = false;
        }
        bRetCode = true;//暂时屏蔽md5校验
    }
    else
    {
        bRetCode = false;
        bRetCode = true;//暂时屏蔽md5校验
    }

    return bRetCode;
}

TINT32 CTaskProcess::Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn,
    CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDayLog)
{
    if(NULL == poConf || NULL == poSearchLongConn || NULL == pTaskQueue || NULL == poServLog)
    {
        return -1;
    }

    TUINT32 idx = 0;
    //TINT32 dwRetCode = 0;

    m_poConf = poConf;
    m_poSearchLongConn = poSearchLongConn;
    m_poQueryLongConn = poQueryLongConn;
    m_pTaskQueue = pTaskQueue;
    m_poServLog = poServLog;
    m_poDayLog = poDayLog;

    for(idx = 0; idx < MAX_LOCAL_HS_TASK_NUM; idx++)
    {
        m_pPackTool[idx] = new CBaseProtocolPack();
        m_pPackTool[idx]->Init();
    }
    m_pUnPackTool = new CBaseProtocolUnpack();
    m_pUnPackTool->Init();

    m_stHttpParam.Reset();
    m_pTmpBuf = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_pHttpResBuf = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_pTmpBufA = new TCHAR[MAX_TMP_STR_LEN];
    m_pTmpBufB = new TCHAR[MAX_TMP_STR_LEN];
    m_pMd5Buf = new TCHAR[MAX_TMP_STR_LEN];

    m_pobjEncrypt = new CDes;

    return 0;
}

void * CTaskProcess::Start(void *pParam)
{
    if(NULL == pParam)
    {
        return NULL;
    }

    CTaskProcess* poIns = (CTaskProcess*)pParam;
    poIns->WorkRoutine();
    return NULL;
}

TINT32 CTaskProcess::WorkRoutine()
{
    SSession *pstSession = NULL;
    while(1)
    {
        if(m_pTaskQueue->WaitTillPop(pstSession) != 0)
        {
            continue;
        }

        TSE_LOG_DEBUG(m_poServLog, ("WorkRoutine:WaitTillPop:session[%p] [seq=%u]", pstSession, pstSession->m_udwSeqNo));

        if(pstSession->m_ucIsUsing == 0)
        {
            CSessionMgr::Instance()->ReleaseSession(pstSession);
            continue;
        }

        // reset tmp param
        ResetSessionTmpParam(pstSession);

        // process by command and procedure
        switch(pstSession->m_udwExpectProcedure)
        {
        case EN_PROCEDURE__CLIENT_REQUEST:
            ProcessClientRequest(pstSession);
            break;
        default:
            ProcessCommand(pstSession);
            break;
        }
    }

    return 0;
}

TINT32 CTaskProcess::ProcessClientRequest(SSession *pstSession)
{
    TSE_LOG_INFO(m_poServLog, ("Main_flow: parse client req [seq=%u]", \
        pstSession->m_udwSeqNo));

    TINT32 dwRetVal = 0;
    TINT32 dwDecryptDataLen = 0;
    char *pRealUrl = NULL;
    char *pOpFlag = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("Main_flow: client_req_len[%u] url[%s] [seq=%u]", \
        strlen((char*)pstSession->m_szClientReqBuf), \
        pstSession->m_szClientReqBuf, \
        pstSession->m_udwSeqNo));

    // 1. 获取url
    if(pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__HTTP)
    {
        dwRetVal = CHuWork::GetRequestUrl((char*)pstSession->m_szClientReqBuf, pstSession->m_stReqParam.m_szReqUrl);
        if(0 > dwRetVal)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow: GetRequestUrl fail[%d] url_len[%u] url[%s] [seq=%u]", \
                dwRetVal, \
                strlen((char*)pstSession->m_szClientReqBuf), \
                pstSession->m_szClientReqBuf, \
                pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            goto CLIENT_REQUEST_ERR_RET;
        }
    }
    else
    {
        strcpy(pstSession->m_stReqParam.m_szReqUrl, (char*)pstSession->m_szClientReqBuf);
    }    

    pOpFlag = strstr(pstSession->m_stReqParam.m_szReqUrl, "op_en_flag=");
    if(pOpFlag)
    {
        pstSession->m_stReqParam.m_udwOpEncryptFlag = atoi(pOpFlag + 11);
    }
    else
    {
        pstSession->m_stReqParam.m_udwOpEncryptFlag = 1;
    }
    TSE_LOG_DEBUG(m_poServLog, ("ProcessRequest: req op_en_flag=%u [seq=%u]", pstSession->m_stReqParam.m_udwOpEncryptFlag, pstSession->m_udwSeqNo));

    if(pstSession->m_stReqParam.m_udwOpEncryptFlag == 1
    && CConfBase::GetInt("NeedEncodeAndCompress") == 1)
    {
        dwDecryptDataLen = MAX_HTTP_REQ_LEN - 2;

        // get version
        pRealUrl = strstr(pstSession->m_stReqParam.m_szReqUrl, "version=");
        if(pRealUrl == NULL)
        {
            pstSession->m_stReqParam.m_udwVersion = 1.0;
        }
        else
        {
            pstSession->m_stReqParam.m_udwVersion = atof(pRealUrl + 8);
        }

        pRealUrl = strstr(pstSession->m_stReqParam.m_szReqUrl, "request=");
        if(pRealUrl)
        {
            bool bDecryptFlag = DecryptUrl(pRealUrl + 8, pRealUrl + 8, dwDecryptDataLen, pstSession->m_stReqParam.m_udwVersion);
            if(bDecryptFlag == false)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(m_poServLog, ("Main_flow: DecryptUrl failed req_url[%s] [seq=%u]", \
                    pstSession->m_stReqParam.m_szReqUrl, \
                    pstSession->m_udwSeqNo));
                goto CLIENT_REQUEST_ERR_RET;
            }
        }
        else
        {
            pRealUrl = strstr(pstSession->m_stReqParam.m_szReqUrl, "?");
            if(pRealUrl)
            {
                bool bDecryptFlag = DecryptUrl(pRealUrl + 1, pRealUrl + 1, dwDecryptDataLen, pstSession->m_stReqParam.m_udwVersion);
                if(bDecryptFlag == false)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                    TSE_LOG_ERROR(m_poServLog, ("Main_flow: DecryptUrl failed req_url[%s] [seq=%u]", \
                        pstSession->m_stReqParam.m_szReqUrl, \
                        pstSession->m_udwSeqNo));
                    goto CLIENT_REQUEST_ERR_RET;
                }
            }
        }
    }
    else
    {
        pstSession->m_stReqParam.m_udwOpEncryptFlag = 0;
    }
    TSE_LOG_DEBUG(m_poServLog, ("ProcessRequest: final op_en_flag=%u [seq=%u]", pstSession->m_stReqParam.m_udwOpEncryptFlag, pstSession->m_udwSeqNo));
    TSE_LOG_INFO(m_poServLog, ("Main_flow: recv req_url[%s] [seq=%u]", \
        pstSession->m_stReqParam.m_szReqUrl, \
        pstSession->m_udwSeqNo));

    // 2. 获取请求参数
    dwRetVal = CHuWork::GetRequestParam(pstSession, m_poServLog, &m_stHttpParam);
    if(0 > dwRetVal)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow: GetRequestParam fail [%d] [seq=%u]", \
            dwRetVal, \
            pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        goto CLIENT_REQUEST_ERR_RET;
    }
    else
    {
        //if(pstSession->m_stReqParam.m_uddwDeviceId == 0)
        //{
        //    TSE_LOG_ERROR(m_poServLog, ("Main_flow: GetRequestParam did is zero [seq=%u]", \
        //        pstSession->m_udwSeqNo));
        //    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        //    goto CLIENT_REQUEST_ERR_RET;
        //}
    }

    pstSession->m_udwCommand = pstSession->m_stReqParam.m_udwCommandID;

    TSE_LOG_DEBUG(m_poServLog, ("Main_flow: uid[%u] did[%Lu] svr[%u] city[%u] command[%d] reqno[%u] [seq=%u]", \
        pstSession->m_stReqParam.m_udwUserId, \
        pstSession->m_stReqParam.m_uddwDeviceId, \
        pstSession->m_stReqParam.m_udwSvrId, \
        pstSession->m_stReqParam.m_udwCityId, \
        pstSession->m_stReqParam.m_udwCommandID, \
        pstSession->m_stReqParam.m_udwSeqNo, \
        pstSession->m_udwSeqNo));

    // 4. 根据command进行各流程处理
    dwRetVal = ProcessCommand(pstSession);
    if(dwRetVal < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        goto CLIENT_REQUEST_ERR_RET;
    }
    return 0;

CLIENT_REQUEST_ERR_RET:
    SendBackResult(pstSession);
    CSessionMgr::Instance()->ReleaseSession(pstSession);
    return -1;
}

TINT32 CTaskProcess::ProcessCommand(SSession *pstSession)
{
    if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow: chose normal or special [seq=%u]", \
            pstSession->m_udwSeqNo));
    }

    NormalMainProdure(pstSession, 0);

    return 0;
}

TVOID CTaskProcess::NormalMainProdure(SSession *pstSession, const TINT32 dwCmdType)
{
    if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: mian produre [seq=%u]", \
            pstSession->m_udwSeqNo));
    }

    TINT32 dwRetCode = 0;

    // 0. 如果已经出错，直接返回
    if(EN_RET_CODE__SUCCESS != pstSession->m_stCommonResInfo.m_dwRetCode)
    {
        TSE_LOG_ERROR(m_poServLog, ("Process err :  ret=%d,cmd=%u [seq=%u]",
            pstSession->m_stCommonResInfo.m_dwRetCode,
            pstSession->m_stReqParam.m_udwCommandID,
            pstSession->m_udwSeqNo));
        if(pstSession->m_udwNextProcedure != EN_PROCEDURE__SEND_RESULT_BACK)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__SEND_RESULT_BACK;
            goto PROCESS_COMMAND_END;
        }
    }

    // 1. 根据命令字做准备工作
    if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        //note: a. 所有命令字都需要获取和比对登陆信息
        pstSession->m_udwNextProcedure = EN_PROCEDURE__PROCESS_CMD;
        pstSession->m_udwExpectProcedure = EN_PROCEDURE__EXPERT_NODE_AWS;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
    }

    /*************************************************命令字处理*******************************************************/
    if(EN_PROCEDURE__PROCESS_CMD == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: cmd process [seq=%u]", \
            pstSession->m_udwSeqNo));

        // TODO
        dwRetCode = CBaseCmdProcess::ProcedureCmd(pstSession);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: cmd process error [seq=%u]", \
                pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__SEND_RESULT_BACK;
            goto PROCESS_COMMAND_END;
        }
        if(EN_COMMAND_STEP__INIT != pstSession->m_udwCommandStep
            && EN_COMMAND_STEP__END != pstSession->m_udwCommandStep)
        {
            TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: async cmd process [seq=%u]", \
                pstSession->m_udwSeqNo));

            return;
        }

        pstSession->m_udwNextProcedure = EN_PROCEDURE__SEND_RESULT_BACK;
    }

PROCESS_COMMAND_END:
    /****************************************返回客户端结果*********************************************************/

    // 返回客户端json
    if(EN_PROCEDURE__SEND_RESULT_BACK == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: send result to client [seq=%u]", \
            pstSession->m_udwSeqNo));

        dwRetCode = SendBackResult(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send result to client error [reqcmd=%s] [ret=%d] [seq=%u]", \
                pstSession->m_stReqParam.m_szCommand, \
                dwRetCode, \
                pstSession->m_udwSeqNo));

        }
        CSessionMgr::Instance()->ReleaseSession(pstSession);
    }
}

TINT32 CTaskProcess::SendBackResult(SSession *pstSession)
{
    // 计算耗时
    pstSession->m_uddwTimeEnd = CTimeUtils::GetCurTimeUs();
    if(pstSession->m_uddwTimeEnd > pstSession->m_uddwTimeBeg)
    {
        pstSession->m_stCommonResInfo.m_uddwCostTime = pstSession->m_uddwTimeEnd - pstSession->m_uddwTimeBeg;
    }

    // 返回结果
    if(pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__HTTP)
    {
        SendBackResult_Http(pstSession);
    }
    else
    {
        if (pstSession->m_dwServiceType == EN_SERVICE_TYPE__CLIENT__RANK_REQ)
        {
            SendBackResult_Binary(pstSession);
        }
        else
        {
            SendBackResultToSvr_Binary(pstSession);
        }
    }

    // 打印日志
    PrintLog(pstSession);

    //做出统计
    GetStatistics(pstSession);

    return 0;
}

TVOID CTaskProcess::ResetSessionTmpParam(SSession *pstSession)
{
    pstSession->m_poConf = m_poConf;
    pstSession->m_poServLog = m_poServLog;
    pstSession->m_poDbLog = m_poDayLog;
    pstSession->m_poLongConn = m_poSearchLongConn;
    pstSession->m_ppPackTool = m_pPackTool;
    pstSession->m_pstHttpParam = &m_stHttpParam;
    pstSession->m_pszReqBuf = &m_szReqBuf[0];
    pstSession->m_udwReqBufLen = 0;

    m_pTmpBuf[0] = '\0';
    pstSession->m_pTmpBuf = &m_pTmpBuf[0];
    pstSession->m_udwTmpBufLen = 0;
}

TVOID CTaskProcess::PrintLog(SSession *pstSession)
{
    TUINT64 uddwCostTime = pstSession->m_stCommonResInfo.m_uddwCostTime;
    TINT32 dwTmp = ceil(pstSession->m_stReqParam.m_dwTotalNum / 20.0);
    TSE_LOG_HOUR(m_poDayLog, ("req_url[%s],ret[%d],cost[%llu],svr[%u],ip[%s],uid[%u],aid[%u],cur page[%u],total page[%d],lang[%u],return num[%d],total num[%d],"
        "aws_read_cost[%lu],aws_write_cost[%lu],aws_read_write_cost[%lu],aws_no_op_cost[%lu],client_seq[%u],seq[%u]",
        pstSession->m_stReqParam.m_szReqUrl, \
        pstSession->m_stCommonResInfo.m_dwRetCode, uddwCostTime, \
        pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_szIp, \
        pstSession->m_stReqParam.m_udwUserId,
        pstSession->m_stReqParam.m_udwAllianceId,
        pstSession->m_stReqParam.m_udwPage,
        dwTmp,
        pstSession->m_stReqParam.m_udwLang,
        pstSession->m_stReqParam.m_dwSelectNum,
        pstSession->m_stReqParam.m_dwTotalNum,
        pstSession->m_uddwAwsReadSumTime,
        pstSession->m_uddwAwsWriteSumTime,
        pstSession->m_uddwAwsReadWriteSumTime,
        pstSession->m_uddwAwsNoOpSumTime,
        pstSession->m_udwClientSeqNo,
        pstSession->m_udwSeqNo));
}

TVOID CTaskProcess::GetStatistics(SSession *pstSession)
{
    CStatistic *pstStatistic = CStatistic::Instance();
    TUINT64 uddwCostTime = pstSession->m_stCommonResInfo.m_uddwCostTime;
    if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)	//成功
    {
        pstStatistic->AddSearchSucc(uddwCostTime);
    }
    else if(EN_RET_CODE__BLACK_ACCOUNT == pstSession->m_stCommonResInfo.m_dwRetCode
        || EN_RET_CODE__HAS_LOG_IN == pstSession->m_stCommonResInfo.m_dwRetCode)
    {
        //do nothing
    }
    else if(pstSession->m_stCommonResInfo.m_dwRetCode > EN_RET_CODE__ERROR_COMMAND) // 后台错误
    {
        pstStatistic->AddSearchFail(uddwCostTime);
    }
    else // 参数错误或者请求不满足
    {
        pstStatistic->AddSearchShield(uddwCostTime);
    }
}

TINT32 CTaskProcess::SendBackResult_Http( SSession *pstSession )
{
    TINT32 dwRetVal = 0;
    TINT32 dwClientSocket = 0;
    TUINT32 udwHttpResLen = 0;

    TUINT64 uddwBegin = CTimeUtils::GetCurTimeUs();

    if(pstSession->m_stReqParam.m_udwOpEncryptFlag == 1 && CConfBase::GetInt("NeedEncodeAndCompress"))
    {
        m_oJsonResult.SetCompress(TRUE);
    }
    else
    {
        m_oJsonResult.SetCompress(FALSE);
    }
    const TCHAR* pBody = NULL;
    pBody = m_oJsonResult.GenResult(pstSession);
    TUINT32 udwBodyLen = m_oJsonResult.GetResultJsonLength();

    CHttpUtils::add_http_result_head(pBody, udwBodyLen,
        m_pHttpResBuf, MAX_NETIO_PACKAGE_BUF_LEN, udwHttpResLen, "utf-8", "text/html"
        );

    TUINT64 uddwEnd = CTimeUtils::GetCurTimeUs();
    TSE_LOG_DEBUG(m_poServLog, ("Main_flow:Json gen cost %lu! us[seq=%u]", uddwEnd - uddwBegin, pstSession->m_udwSeqNo));

    // 2.获取socket
    dwClientSocket = m_poQueryLongConn->GetSockHandle(pstSession->m_stClientHandle);
    if(dwClientSocket == INVALID_SOCKET)
    {
        TSE_LOG_DEBUG(m_poServLog, ("INVALID_SOCKET in send back message! [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    // 3.发送结果
    tse_socket_writeFull(dwClientSocket, m_pHttpResBuf, udwHttpResLen);

    // 6.关闭连接
    m_poQueryLongConn->RemoveLongConnSession(pstSession->m_stClientHandle);

    return 0;
}

TINT32 CTaskProcess::SendBackResult_Binary( SSession *pstSession )
{
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = m_pPackTool[0];
    TBOOL bRet = FALSE;
    TUINT8 ucCompressFlag = 0;

    TUINT64 uddwBegin = CTimeUtils::GetCurTimeUs();

    // 0. 生成结果
    m_oJsonResult.GenResult_Pb(pstSession);

    // 1. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__RANK_RSP);
    pobjPack->SetSeq(pstSession->m_udwPackSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_CODE, pstSession->m_stCommonResInfo.m_dwRetCode);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_COST_TIME, pstSession->m_stCommonResInfo.m_uddwCostTime);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_UID, pstSession->m_stReqParam.m_udwUserId);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF, (TUCHAR*)&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen);
    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_ERROR(m_poServLog, ("SendBackResult_Binary: send package_len=%u [seq=%u]", udwPackageLen, pstSession->m_udwSeqNo));

    // 2. send back
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_stClientHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);

    TUINT64 uddwEnd = CTimeUtils::GetCurTimeUs();
    TSE_LOG_DEBUG(m_poServLog, ("SendBackResult_Binary: BJson gen cost %lu! us[seq=%u]", uddwEnd - uddwBegin, pstSession->m_udwSeqNo));

    if(!m_poSearchLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(m_poServLog, ("SendBackResult_Binary: send response to client failed [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
    return 0;
}

TINT32 CTaskProcess::SendBackResultToSvr_Binary(SSession *pstSession)
{
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = m_pPackTool[0];
    TBOOL bRet = FALSE;
    TUINT8 ucCompressFlag = 0;

    TUINT64 uddwBegin = CTimeUtils::GetCurTimeUs();

    // 0. 生成结果
    m_oJsonResult.GenResult_Pb(pstSession);
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    string szResult = writer.write(pstSession->m_stCommonResInfo.m_jResultWriter);

    // 1. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__SVR__RANK_RSP);
    pobjPack->SetSeq(pstSession->m_udwPackSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_CODE, pstSession->m_stCommonResInfo.m_dwRetCode);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_COST_TIME, pstSession->m_stCommonResInfo.m_uddwCostTime);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF, (TUCHAR*)szResult.c_str(), szResult.length());
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_UID, pstSession->m_stReqParam.m_udwUserId);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_AID, pstSession->m_stReqParam.m_udwAllianceId);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_SID, pstSession->m_stReqParam.m_udwSvrId);
    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_ERROR(m_poServLog, ("SendBackResultToSvr_Binary: send package_len=%u [seq=%u]", udwPackageLen, pstSession->m_udwSeqNo));

    // 2. send back
    LTasksGroup stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_stClientHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);

    TUINT64 uddwEnd = CTimeUtils::GetCurTimeUs();
    TSE_LOG_DEBUG(m_poServLog, ("SendBackResultToSvr_Binary: BJson gen cost %lu! us[seq=%u]", uddwEnd - uddwBegin, pstSession->m_udwSeqNo));

    if (!m_poSearchLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(m_poServLog, ("SendBackResultToSvr_Binary: send response to client failed [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
    return 0;
}