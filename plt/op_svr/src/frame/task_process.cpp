#include "task_process.h"
#include "global_serv.h"
#include "base/os/wtsesocket.h"
#include "session_mgr.h"
#include "statistic.h"
#include "common_func.h"
#include "conf_base.h"
#include "rc4.h"
#include "service_key.h"
#include "hu_work.h"
#include "process_base.h"

bool CTaskProcess::EncryptUrl(char *pszIn, char *pszOut, int &dwOutLen, float fVersion)
{
    if (pszIn == NULL || pszOut == NULL)
    {
        return false;
    }
    int dwInLen = strlen(pszIn);
    int dwEncryptDataLen = 0;
    int dwOutSize = dwOutLen;
    CEncryptCR4 *poEncrypt = CEncryptCR4Mgr::get_instance()->GetEncrypt(fVersion);
    if (poEncrypt == NULL)
    {
        return false;
    }

    dwOutLen = 0;
    pszOut[0] = 0;
    m_szEncryptBuf[0] = 0;
    m_szEncryptUrl[0] = 0;

    // md5: 加固定串再MD5，防止外围破解
    TUINT64 uddwKey = 0;
    if (pszIn == NULL || pszIn[0] == 0)
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
    if (dwOutSize < dwInLen)
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
    if (pszIn == NULL || pszOut == NULL)
    {
        return false;
    }

    bool bRetCode = false;
    int dwInLen = strlen(pszIn);
    int dwEncryptDataLen = 0;
    CEncryptCR4 *poEncrypt = CEncryptCR4Mgr::get_instance()->GetEncrypt(fVersion);
    if (poEncrypt == NULL)
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
    if (pMd5Str)
    {


        uddwRawMd5 = strtoul(pMd5Str + strlen(pMd5Param), NULL, 10);
        *pMd5Str = 0;

        strcat(m_szEncryptBuf, "&crystr=LeyiCrypt123");
        MD5String(m_szEncryptBuf, (TUCHAR*)&uddwNewMd5, sizeof(uddwNewMd5));

        if (uddwRawMd5 != uddwNewMd5)
        {
            bRetCode = false;
        }
    }
    else
    {
        bRetCode = false;
    }

    return bRetCode;
}


TINT32 CTaskProcess::Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn,
    CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDayLog)
{
    if (NULL == poConf || NULL == poSearchLongConn || NULL == pTaskQueue || NULL == poServLog)
    {
        return -1;
    }

    TUINT32 idx = 0;
    TINT32 dwRetCode = 0;

    m_poConf = poConf;
    m_poSearchLongConn = poSearchLongConn;
    m_poQueryLongConn = poQueryLongConn;
    m_pTaskQueue = pTaskQueue;
    m_poServLog = poServLog;
    m_poClientReqLog = poDayLog;

    for (idx = 0; idx < MAX_LOCAL_HS_TASK_NUM; idx++)
    {
        m_pPackTool[idx] = new CBaseProtocolPack();
        m_pPackTool[idx]->Init();
    }
    m_pUnPackTool = new CBaseProtocolUnpack();
    m_pUnPackTool->Init();

    m_stHttpParam.Reset();
    m_pTmpBuf = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];

    return 0;
}

void * CTaskProcess::Start(void *pParam)
{
    if (NULL == pParam)
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
    while (1)
    {
        if (m_pTaskQueue->WaitTillPop(pstSession) != 0)
        {
            continue;
        }

        pstSession->m_uddwProcessBegTime = CTimeUtils::GetCurTimeUs();
        TSE_LOG_INFO(m_poServLog, ("WorkRoutine:WaitTillPop:session[%p],is_using[%u] task_size[%u] [seq=%u]", \
            pstSession, pstSession->m_ucIsUsing, m_pTaskQueue->Size(), pstSession->m_udwSeqNo));

        if (pstSession->m_ucIsUsing == 0)
        {
            CSessionMgr::Instance()->ReleaseSession(pstSession);
            continue;
        }

        // reset tmp param
        ResetSessionTmpParam(pstSession);

        // process by command and procedure
        //---------req process begin-------------
        //TODO:
        switch (pstSession->m_udwRequestType)
        {
        case EN_PROCEDURE__CLIENT_REQUEST:
            ProcessClientRequest(pstSession);
            break;
        default:
            ProcessCommand(pstSession);
            break;
        }
        //---------req process end---------------
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
    char *pDeFlag = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("Main_flow: client_req_len[%u] url[%s] [seq=%u]", \
        strlen((char*)pstSession->m_szClientReqBuf), \
        pstSession->m_szClientReqBuf, \
        pstSession->m_udwSeqNo));

    //1.获取url
    pDeFlag = strstr(pstSession->m_stReqParam.m_szReqUrl, "op_en_flag=");
    if (pstSession->m_dwClientReqEnType && pDeFlag == NULL)
    {
        dwDecryptDataLen = MAX_HTTP_REQ_LEN - 2;

        pRealUrl = strstr(pstSession->m_stReqParam.m_szReqUrl, "request=");
        if (pRealUrl)
        {
            bool bDecryptFlag = DecryptUrl(pRealUrl + 8, pRealUrl + 8, dwDecryptDataLen, pstSession->m_stReqParam.m_udwVersion);
            //1.2 版本后加入了固定串MD5，防止外界破解
            if (bDecryptFlag == false)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(m_poServLog, ("Main_flow: DecryptUrl failed req_url[%s] [seq=%u]",
                    pstSession->m_stReqParam.m_szReqUrl,
                    pstSession->m_udwSeqNo));
                goto CLIENT_REQUEST_ERR_RET;
            }
        }
        else
        {
            pRealUrl = strstr(pstSession->m_stReqParam.m_szReqUrl, "?");
            if (pRealUrl)
            {
                bool bDecryptFlag = DecryptUrl(pRealUrl + 1, pRealUrl + 1, dwDecryptDataLen, pstSession->m_stReqParam.m_udwVersion);
                if (bDecryptFlag == false)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                    TSE_LOG_ERROR(m_poServLog, ("Main_flow: DecryptUrl failed req_url[%s] [seq=%u]",
                        pstSession->m_stReqParam.m_szReqUrl,
                        pstSession->m_udwSeqNo));
                    goto CLIENT_REQUEST_ERR_RET;
                }
            }
        }
    }
    TSE_LOG_INFO(m_poServLog, ("Main_flow: recv req_url[%s] [seq=%u]",
        pstSession->m_stReqParam.m_szReqUrl,
        pstSession->m_udwSeqNo));

    //2.获取请求参数
    dwRetVal = CHuWork::GetRequestParam(pstSession, m_poServLog, &m_stHttpParam);
    if (0 > dwRetVal)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow: GetRequestParam fail [%d] [seq=%u]", \
            dwRetVal, \
            pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        goto CLIENT_REQUEST_ERR_RET;
    }

    //3.根据command进行各流程处理
    dwRetVal = ProcessCommand(pstSession);
    if (dwRetVal < 0)
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
    if (EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow: chose normal or special [seq=%u]",
            pstSession->m_udwSeqNo));
    }

    CClientCmd *poCClientCmd = CClientCmd::GetInstance();
    struct SFunctionSet stProcessFunctionSet;
    map<EClientReqCommand, struct SFunctionSet>::iterator itCmdFunctionSet;
    itCmdFunctionSet = poCClientCmd->Get_CmdMap()->find((EClientReqCommand)pstSession->m_stReqParam.m_udwCommandID);
    if (itCmdFunctionSet != poCClientCmd->Get_CmdMap()->end())
    {
        stProcessFunctionSet = itCmdFunctionSet->second;
    }

    switch (stProcessFunctionSet.dwProcedureType)
    {
    case EN_NORMAL:
        if (EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
        {
            TSE_LOG_INFO(m_poServLog, ("Main_flow: chose normal [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        NormalMainProdure(pstSession, stProcessFunctionSet.dwCmdType);
        break;
    case EN_SPECIAL:
        TSE_LOG_ERROR(m_poServLog, ("Main_flow: the procedure type is specail [proceduretype=%d] [seq=%u]",
            stProcessFunctionSet.dwProcedureType,
            pstSession->m_udwSeqNo));
        return -1;
        break;
    case EN_UNKNOW_PROCEDURE:
    default:
        TSE_LOG_ERROR(m_poServLog, ("Main_flow: the procedure type is invalid [proceduretype=%d] [seq=%u]",
            stProcessFunctionSet.dwProcedureType,
            pstSession->m_udwSeqNo));
        return -2;
        break;
    }

    return 0;
}

TVOID CTaskProcess::NormalMainProdure(SSession *pstSession, const TINT32 dwCmdType)
{
    if (EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: main produre [seq=%u]",
            pstSession->m_udwSeqNo));
    }

    TBOOL bNeedResponse = FALSE;
    TINT32 dwRetCode = 0;

    /******************************************主流程函数的参数校验********************************************/
    // 1. 参数校验
    if (EN_UNKNOW_CMD == dwCmdType)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: [cmdtype=%u] [ret=%d] [cmd=%u] [seq=%u]",
            dwCmdType, pstSession->m_stCommonResInfo.m_dwRetCode,
            pstSession->m_stReqParam.m_udwCommandID, pstSession->m_udwSeqNo));

        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        goto PROCESS_COMMAND_END;
    }

    /******************************************主流程的异常处理***********************************************/
    // 2. 业务流程如果已经出错，直接返回
    if (pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: async process error [ret=%d] [cmd=%u] [nextprocedure=%d] [seq=%u]",
            pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_stReqParam.m_udwCommandID,
            pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        goto PROCESS_COMMAND_END;
    }

    /******************************************根据命令字做准备工作***********************************************/
    // 3. 根据命令字做准备工作
    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__INIT)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: prepare process [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
    }

    /*************************************************命令字处理*******************************************************/
    // 4. 根据命令字进行处理
    if (EN_PROCEDURE__COMMAND_PROCESS == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: cmd process [seq=%u]",
            pstSession->m_udwSeqNo));

        dwRetCode = CBaseCmdProcess::ProcedureCmd(pstSession, bNeedResponse);
        if (dwRetCode != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: cmd process error [seq=%u]",
                pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }

        //TODO 必须制定bNeedResponse
        if (EN_COMMAND_STEP__INIT != pstSession->m_udwCommandStep
            && EN_COMMAND_STEP__END != pstSession->m_udwCommandStep)
        {
            // session 已经有网络线程发送了task 有可能回包触发task 导致log打印时session同时被两个线程占用 所以不能打log
            //TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: async cmd process [command_step=%u] [seq=%u]", 
            //    pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
            return;
        }

        if (TRUE == bNeedResponse)
        {
            // session 已经有网络线程发送了task 有可能回包触发task 导致log打印时session同时被两个线程占用 所以不能打log
            //TSE_LOG_INFO(m_poServLog, ("Main_flow:special: async cmd process [seq=%u]",
            //    pstSession->m_udwSeqNo));
            return;
        }

        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
    }

PROCESS_COMMAND_END:
    /****************************************返回客户端结果*********************************************************/
    // 返回客户端结果
    if (EN_PROCEDURE__END == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: send result to client [seq=%u]",
            pstSession->m_udwSeqNo));

        dwRetCode = SendBackResult(pstSession);

        if (dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send result to client error [reqcmd=%s] [ret=%d] [seq=%u]",
                pstSession->m_stReqParam.m_szCommand, dwRetCode, pstSession->m_udwSeqNo));
        }

        CGlobalServ::m_poSessionMgr->ReleaseSession(pstSession);
    }
}

TVOID CTaskProcess::SpecialMainProdure(SSession *pstSession, const TINT32 dwCmdType)
{
    return;
}

TINT32 CTaskProcess::SendBackResult(SSession *pstSession)
{
    m_oJsonResultGenerator.GenResult_Pb(pstSession);
    // 返回结果
    if (pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__HTTP)
    {
        //SendBackResult_Http(pstSession);
    }
    else
    {
        SendBackResult_Binary(pstSession);
    }

    pstSession->m_uddwTimeEnd = CTimeUtils::GetCurTimeUs();
    if (pstSession->m_uddwTimeEnd > pstSession->m_uddwTimeBeg)
    {
        pstSession->m_stCommonResInfo.m_uddwCostTime = pstSession->m_uddwTimeEnd - pstSession->m_uddwTimeBeg;
    }

    //4.打印日志
    PrintLog(pstSession);

    // 6.关闭连接
    //m_poQueryLongConn->RemoveLongConnSession(pstSession->m_stClientHandle);

    return 0;

    return 0;
}

TINT32 CTaskProcess::SendBackResult_Binary(SSession *pstSession)
{
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = m_pPackTool[0];
    TUINT8 ucCompressFlag = 1;
    TUINT32 udwCompressDataLen = 0;
    TUINT32 udwCompressTmpLen = 0;

    // 1. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__COMMAND_RSP);
    pobjPack->SetSeq(pstSession->m_udwPackSeq);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_CODE, pstSession->m_stCommonResInfo.m_dwRetCode);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_COST_TIME, pstSession->m_stCommonResInfo.m_uddwCostTime);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_UID, pstSession->m_stReqParam.m_ddwUserId);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF, (TUCHAR*)&pstSession->m_pTmpBuf[0], pstSession->m_dwFinalPackLength);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_INFO(m_poServLog, ("SendBackResult_Binary: send package_len=%u [seq=%u]", udwPackageLen, pstSession->m_udwSeqNo));

    // 2. send back
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_stClientHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if (!m_poQueryLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(m_poServLog, ("SendBackResult_Binary: send response to client failed [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
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
}

TVOID CTaskProcess::PrintLog(SSession *pstSession)
{
    TUINT64 uddwCostTime = pstSession->m_stCommonResInfo.m_uddwCostTime;
    TSE_LOG_HOUR(m_poDayLog, ("req_url[%s],ret[%d],cost[%llu],svr[%u],ip[%s],uid[%ld],[seq=%u]",
        pstSession->m_stReqParam.m_szReqUrl, \
        pstSession->m_stCommonResInfo.m_dwRetCode, uddwCostTime, \
        pstSession->m_stReqParam.m_dwSvrId, pstSession->m_stReqParam.m_szIp, \
        pstSession->m_stReqParam.m_ddwUserId,
        pstSession->m_udwSeqNo));
}
