#include "task_process.h"
#include "hu_work.h"
#include "base/os/wtsesocket.h"
#include "session_mgr_t.h"
#include "template_fun.h"
#include "base_def.h"
#include "procedure_base.h"
#include "rc4.h"
#include "statistic.h"
#include "aws_response.h"
#include "aws_request.h"
#include "common_func.h"
#include "global_serv.h"
#include "jasonlog.h"
#include "game_command.h"
#include "common_handle_before.h"
#include "common_handle_after.h"
#include "hu_cblog.h"
#include "conf_base.h"
#include "common_handle.h"
#include "game_evaluate_logic.h"
#include "game_evaluate_base.h"
#include "user_dyeing_info.h"
#include "common_base.h"
#include "city_base.h"
#include "pushdata_action.h"


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
    if(NULL == poConf || NULL == poSearchLongConn || NULL == pTaskQueue || NULL == poServLog)
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
    m_pMd5Buf = new TCHAR[MAX_TMP_STR_LEN];

    m_pobjEncrypt = new CDes;

    m_pobjNpcUpgrade = new CNpcUpgrade;
    dwRetCode = m_pobjNpcUpgrade->Initalize("../data/game.json");
    assert(dwRetCode == 0);

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
        
        pstSession->m_uddwProcessBegTime = CTimeUtils::GetCurTimeUs();
        TSE_LOG_INFO(m_poServLog, ("WorkRoutine:WaitTillPop:session[%p],is_using[%u] task_size[%u] [seq=%u]", \
                                   pstSession, pstSession->m_ucIsUsing, m_pTaskQueue->Size(), pstSession->m_udwSeqNo));

        if(pstSession->m_ucIsUsing == 0)
        {
            CGlobalServ::m_poSessionMgr->ReleaseSession(pstSession);
            continue;
        }

        // reset tmp param
        ResetSessionTmpParam(pstSession);

        // process by command and procedure
        switch(pstSession->m_udwRequestType)
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
    TSE_LOG_INFO(m_poServLog, ("Main_flow: parse client req [seq=%u]",
        pstSession->m_udwSeqNo));
    TINT32 dwRetVal = 0;
    TINT32 dwDecryptDataLen = 0;
    char *pRealUrl = NULL;
	TCHAR *pDeFlag = NULL;

    pstSession->m_udwRequestType = EN_PROCEDURE__INIT;

    TSE_LOG_INFO(m_poServLog, ("Main_flow: client_req_len[%u] url[%s] [seq=%u]",
        strlen((char*)pstSession->m_szClientReqBuf),
        pstSession->m_szClientReqBuf,
        pstSession->m_udwSeqNo));

    // 1. 获取url
    if(pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__HTTP)
    {
        dwRetVal = CHuWork::GetRequestUrl((char*)pstSession->m_szClientReqBuf, pstSession->m_stReqParam.m_szReqUrl);
        if(0 > dwRetVal)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow: GetRequestUrl fail[%d] url_len[%u] url[%s] [seq=%u]",
                dwRetVal, strlen((char*)pstSession->m_szClientReqBuf),
                pstSession->m_szClientReqBuf, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            goto CLIENT_REQUEST_ERR_RET;
        }
    }
    else
    {
        strcpy(pstSession->m_stReqParam.m_szReqUrl, (char*)pstSession->m_szClientReqBuf);
    }
    

    TSE_LOG_INFO(m_poServLog, ("Main_flow: client_req_len[%u] true_url[%s] [seq=%u]",
        strlen((char*)pstSession->m_stReqParam.m_szReqUrl),
        pstSession->m_stReqParam.m_szReqUrl,
        pstSession->m_udwSeqNo));

    //--------------URL PreProcess------------------------------------------------------------------
    // get version
    pRealUrl = strstr(pstSession->m_stReqParam.m_szReqUrl, "vs=");
    if(pRealUrl == NULL)
    {
        pstSession->m_stReqParam.m_udwVersion = 1.0;
    }
    else
    {
        pstSession->m_stReqParam.m_udwVersion = atof(pRealUrl + 3);
    }

    // 1.1 获取url，并进行解密
    pDeFlag = strstr(pstSession->m_stReqParam.m_szReqUrl, "op_en_flag=");
	if(pstSession->m_dwClientReqEnType && pDeFlag == NULL)
    {
        dwDecryptDataLen = MAX_HTTP_REQ_LEN - 2;

        pRealUrl = strstr(pstSession->m_stReqParam.m_szReqUrl, "request=");
        if(pRealUrl)
        {
            bool bDecryptFlag = DecryptUrl(pRealUrl + 8, pRealUrl + 8, dwDecryptDataLen, pstSession->m_stReqParam.m_udwVersion);
            //1.2 版本后加入了固定串MD5，防止外界破解
            if(bDecryptFlag == false)
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
            if(pRealUrl)
            {
                bool bDecryptFlag = DecryptUrl(pRealUrl + 1, pRealUrl + 1, dwDecryptDataLen, pstSession->m_stReqParam.m_udwVersion);
                if(bDecryptFlag == false)
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

    // 2. 获取请求参数
    // "CHuWork::GetRequestParam"保证了命令字属于定义的命令字
    dwRetVal = CHuWork::GetRequestParam(pstSession, m_poServLog, &m_stHttpParam);
    if(0 > dwRetVal)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow: GetRequestParam fail [%d] [seq=%u]",
            dwRetVal, pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        goto CLIENT_REQUEST_ERR_RET;
    }
    else
    {
        if(pstSession->m_stReqParam.m_uddwDeviceId == 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow: GetRequestParam did is zero [seq=%u]",
                pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            goto CLIENT_REQUEST_ERR_RET;
        }

        // 获取客户端时间
        if(pstSession->m_dwClientReqEnType)
        {
            TBOOL bHacker = FALSE;
            TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
            const TCHAR *pszTime = strstr(pstSession->m_stReqParam.m_szReqUrl, "time=");
            if(pszTime)
            {
                pstSession->m_stReqParam.m_udwInReqTime = strtoul(pszTime + 5, NULL, 10);
                if(pstSession->m_stReqParam.m_udwInReqTime < udwCurTime - 10 * 60 || pstSession->m_stReqParam.m_udwInReqTime > udwCurTime + 10 * 60)
                {
                    switch(pstSession->m_stReqParam.m_udwCommandID)
                    {
                        //case EN_CLIENT_REQ_COMMAND__MARCH_CAMP:
                        bHacker = TRUE;
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                if(strstr(pstSession->m_stReqParam.m_szReqUrl, "did=monitor") != NULL)
                {
                    bHacker = FALSE;
                }
                else
                {
                    bHacker = TRUE;
                }
            }

            if(bHacker == TRUE)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HACKER;
                TSE_LOG_ERROR(m_poServLog, ("Main_flow: CheckClientReqTime error req_time[%u] cur_time[%u]! [seq=%u]",
                    pstSession->m_stReqParam.m_udwInReqTime,
                    udwCurTime, pstSession->m_udwSeqNo));
                goto CLIENT_REQUEST_ERR_RET;
            }
        }
        else
        {
            const TCHAR *pszTime = strstr(pstSession->m_stReqParam.m_szReqUrl, "time=");
            if(pszTime)
            {
                pstSession->m_stReqParam.m_udwInReqTime = strtoul(pszTime + 5, NULL, 10);
            }
        }

        //黑名单判定
        if(pstSession->m_dwClientReqEnType)
        {
            TBOOL bBlackAccount = FALSE;
            CGameSvrInfo *pobjGameSvr = CGameSvrInfo::GetInstance();

            if(pstSession->m_stReqParam.m_udwUserId > 0)
            {
                ostringstream oss;
                oss << pstSession->m_stReqParam.m_udwUserId;
                if(pobjGameSvr->m_pobjMasterBlackAccount->count(oss.str()) > 0)
                {
                    bBlackAccount = TRUE;
                }
            }

            if(pobjGameSvr->m_pobjMasterBlackAccount->count(pstSession->m_stReqParam.m_szDevice) > 0)
            {
                bBlackAccount = TRUE;
            }

            if(bBlackAccount == TRUE)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Main_flow: CheckReq black account!! uid[%lld], did[%s]",
                    pstSession->m_stReqParam.m_udwUserId,
                    pstSession->m_stReqParam.m_szDevice));
                goto CLIENT_REQUEST_ERR_RET;
            }
        }
    }

    pstSession->m_udwCommand = pstSession->m_stReqParam.m_udwCommandID;

    TSE_LOG_DEBUG(m_poServLog, ("Main_flow: uid[%u] did[%Lu] platform[%s] svr[%u] city[%u] command[%d] reqno[%u] [seq=%u]",
        pstSession->m_stReqParam.m_udwUserId,
        pstSession->m_stReqParam.m_uddwDeviceId,
        pstSession->m_stReqParam.m_szPlatForm,
        pstSession->m_stReqParam.m_udwSvrId,
        pstSession->m_stReqParam.m_udwCityId,
        pstSession->m_stReqParam.m_udwCommandID,
        pstSession->m_stReqParam.m_udwSeqNo,
        pstSession->m_udwSeqNo));

    /*
    // 兼容wot2.3 客户端版本bug
    // 重新登录 并且 传入的sid
    if (0 == pstSession->m_stReqParam.m_udwSvrId
    && 0 == pstSession->m_stReqParam.m_udwUserId
    && EN_CLIENT_REQ_COMMAND__LOGIN_GET == pstSession->m_stReqParam.m_udwCommandID
    && EN_LOGIN_STATUS__LOGIN == pstSession->m_stReqParam.m_ucLoginStatus )
    {
    TSE_LOG_INFO(m_poServLog, ("Main_flow: [Modify sid=-1] uid[%u] did[%Lu] svr[%u] city[%u] command[%d] reqno[%u] [seq=%u]",
    pstSession->m_stReqParam.m_udwUserId,
    pstSession->m_stReqParam.m_uddwDeviceId,
    pstSession->m_stReqParam.m_udwSvrId,
    pstSession->m_stReqParam.m_udwCityId,
    pstSession->m_stReqParam.m_udwCommandID,
    pstSession->m_stReqParam.m_udwSeqNo,
    pstSession->m_udwSeqNo));
    pstSession->m_stReqParam.m_udwSvrId = (TUINT32)-1;
    }
    */

    // 3. 对请求进行特殊处理，获取要加锁的taskid
    PreprocessReq(pstSession);

    // 4. 根据command进行各流程处理
    if(pstSession->m_udwCommand != EN_CLIENT_OPERATE_CMD__LOG)
    {
        dwRetVal = ProcessCommand(pstSession);
        if(dwRetVal < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            goto CLIENT_REQUEST_ERR_RET;
        }
        return 0;
    }

CLIENT_REQUEST_ERR_RET:
    if(pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__HTTP)
    {
        if (pstSession->m_udwCommand == EN_CLIENT_OPERATE_CMD__LOG)
        {
            const TCHAR* pBody = NULL;
            pBody = m_oJsonResultGenerator.GenNobodyResult(pstSession);
            TUINT32 udwBodyLen = m_oJsonResultGenerator.GetResultJsonLength();
            CHttpUtils::add_http_result_head(pBody, udwBodyLen,
                pstSession->m_szClientRspBuf, MAX_NETIO_PACKAGE_BUF_LEN, pstSession->m_dwFinalPackLength, "utf-8", "text/html"
                );
            pstSession->m_dwOriPackLength = pstSession->m_dwFinalPackLength;
        }
        else if (pstSession->m_dwFinalPackLength == 0)
        {
            CHttpUtils::add_http_result_head("", 0,
                pstSession->m_szClientRspBuf, MAX_NETIO_PACKAGE_BUF_LEN, pstSession->m_dwFinalPackLength, "utf-8", "text/html");
        }
    }
    else
    {
        m_oJsonResultGenerator.GenResult_Pb(pstSession, FALSE);
    }   

    SendBackResult(pstSession);
    CGlobalServ::m_poSessionMgr->ReleaseSession(pstSession);
    return -1;
}

TINT32 CTaskProcess::ProcessCommand(SSession *pstSession)
{
    if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow: chose normal or special [seq=%u]",
            pstSession->m_udwSeqNo));
    }

    CClientCmd *poCClientCmd = CClientCmd::GetInstance();
    struct SFunctionSet stProcessFunctionSet;
    map<EClientReqCommand, struct SFunctionSet>::iterator itCmdFunctionSet;
    itCmdFunctionSet = poCClientCmd->Get_CmdMap()->find((EClientReqCommand)pstSession->m_stReqParam.m_udwCommandID);
    if(itCmdFunctionSet != poCClientCmd->Get_CmdMap()->end())
    {
        stProcessFunctionSet = itCmdFunctionSet->second;
    }

    switch(stProcessFunctionSet.dwProcedureType)
    {
    case EN_NORMAL:
        if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
        {
            TSE_LOG_INFO(m_poServLog, ("Main_flow: chose normal [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        NormalMainProdure(pstSession, stProcessFunctionSet.dwCmdType);
        break;
    case EN_SPECIAL:
        if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
        {
            TSE_LOG_INFO(m_poServLog, ("Main_flow: chose special [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        SpecialMainProdure(pstSession, stProcessFunctionSet.dwCmdType);
        break;
    case EN_UNKNOW_PROCEDURE:
    default:
        TSE_LOG_ERROR(m_poServLog, ("Main_flow: the procedure type is invalid [proceduretype=%d] [seq=%u]",
            stProcessFunctionSet.dwProcedureType,
            pstSession->m_udwSeqNo));
        return -3;
        break;
    }

    return 0;
}

TVOID CTaskProcess::NormalMainProdure(SSession *pstSession, const TINT32 dwCmdType)
{

    if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: mian produre [seq=%u]",
            pstSession->m_udwSeqNo));
    }

    TBOOL bNeedResponse = FALSE;
    TINT32 dwRetCode = 0;

    /******************************************主流程函数的参数校验********************************************/
    // 1. 参数校验
    if(EN_UNKNOW_CMD == dwCmdType)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: parameter check err [cmdtype=%d] [ret=%d] [cmd=%u] [nextprocedure=%d] [seq=%u]",
            dwCmdType,
            pstSession->m_stCommonResInfo.m_dwRetCode,
            pstSession->m_stReqParam.m_udwCommandID,
            pstSession->m_udwNextProcedure,
            pstSession->m_udwSeqNo));

        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        goto PROCESS_COMMAND_END;
    }

    /******************************************主流程的异常处理***********************************************/
    // 2. 业务流程如果已经出错，直接返回
    if(pstSession->m_udwCommand != EN_CLIENT_REQ_COMMAND__USER_INFO_CREATE) // 用户创建命令的出错信息由命令字自己处理
    {
        if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: async process error [ret=%d] [cmd=%u] [nextprocedure=%d] [seq=%u]",
                pstSession->m_stCommonResInfo.m_dwRetCode,
                pstSession->m_stReqParam.m_udwCommandID,
                pstSession->m_udwNextProcedure,
                pstSession->m_udwSeqNo));
            if(pstSession->m_udwNextProcedure == EN_PROCEDURE__DATA_CENTER_GET_RESPONSE
                || pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORT_SVR_RESPONSE)
            {
                //wave@20160506: 自己处理错误
            }
            else if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PUSH_DATA_REQUEST)
            {
                // do nothing;
            }
            else if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PUSH_DATA_RESPONSE)
            {
                // do nothing;
            }
            else if(pstSession->m_udwNextProcedure != EN_PROCEDURE__SEND_RESULT_BACK)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
                goto PROCESS_COMMAND_END;
            }
            else
            {
                //do nothing
            }
        }
    }

    /******************************************根据命令字做准备工作***********************************************/
    // 3. 根据命令字做准备工作
    // "pstSession->m_udwNextProcedure"的初始值有session的reset决定,接受了客户端请求之后会reset session
    if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: prepare process [seq=%u]",
            pstSession->m_udwSeqNo));

        // 正常情况下,所有命令字都需要获取和比对登陆信息
        pstSession->m_udwNextProcedure = EN_PROCEDURE__ACCOUNT_SEARCH_REQUEST;
    }

    /******************************************用户帐号信息校验***********************************************/
    // todo: 帐号的验证和更新可以做成独立的服务(下游),login_get&&lg=1的情况下走帐号服务

    // 4. 获取帐号信息——需要网络交互
    if(EN_PROCEDURE__ACCOUNT_SEARCH_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: get account info [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__ACCOUNT_SEARCH_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if(EN_OP == dwCmdType
            || FALSE == pstSession->m_stReqParam.m_bNeedLoginCheck)
        {
            dwRetCode = CBaseProcedure::ProcessOperate_LoginGet(pstSession);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: op request get account info error [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_udwSeqNo));

                pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
                goto PROCESS_COMMAND_END;
            }
            return;
        }
        else
        {
            if(EN_CLIENT_REQ_COMMAND__LOGIN_GET == pstSession->m_stReqParam.m_udwCommandID)
            {
                // 预留的帐号和密码功能
                dwRetCode = CBaseProcedure::ProcessProcedure_LoginRequest(pstSession);
            }
            else
            {
                dwRetCode = CBaseProcedure::ProcessProcedure_LoginRequest(pstSession);
            }
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request get account info error [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_udwSeqNo));

                pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
                goto PROCESS_COMMAND_END;
            }
            return;
        }
    }

    // 5. 处理帐号信息响应(里面包含用户创建流程)
    if(EN_PROCEDURE__ACCOUNT_SEARCH_RESPONSE == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: response account info [seq=%u]",
            pstSession->m_udwSeqNo));

        if(EN_OP == dwCmdType
            || FALSE == pstSession->m_stReqParam.m_bNeedLoginCheck)
        {
            dwRetCode = CBaseProcedure::ProcessOperate_LoginRes(pstSession);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: op response account info error [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_udwSeqNo));

                pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
                goto PROCESS_COMMAND_END;
            }

            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_GET_REQUEST;
        }
        else
        {
            dwRetCode = CBaseProcedure::ProcessProcedure_LoginResponse(pstSession);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: response account info error [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_udwSeqNo));

                pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
                goto PROCESS_COMMAND_END;
            }

            if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__LOGIN_CREATE)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
            }
            else if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__LOGIN_FAKE)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
            }
            else
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_GET_REQUEST;
            }

            TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: check account result [reqcmd=%s] [switchcmd=%d] [nextprocedure=%d] [seq=%u]",
                pstSession->m_stReqParam.m_szCommand,
                pstSession->m_udwCommand,
                pstSession->m_udwNextProcedure,
                pstSession->m_udwSeqNo));
        }
    }
    /**************************************对用户uid/actionid/alid加锁**********************************************/


    // 6. 获取锁(不需要response)
    if(EN_PROCEDURE__PLAYER_LOCK_GET_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: get data lock [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_SEARCH_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__LOCK_GET;

        // todo:lock函数待优化,(hu的锁模式可能只锁了自己数据, 但是会改除了自己以外的数据)
        dwRetCode = CBaseProcedure::ProcessProcedure_LockReq(pstSession, EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request get data lock error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_VALID_DOWNSTEAM;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        else
        {
            return;
        }
    }

    /********************************************获取aws数据(uid)**************************************************/

    // 7. 获取用户数据 ——发送请求
    if(EN_PROCEDURE__USER_INFO_SEARCH_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: get user data [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_SEARCH_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = CBaseProcedure::ProcessProcedure_UserDataGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request get user data error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        return;
    }

    // 8. 获取用户数据 ——处理结果
    if(EN_PROCEDURE__USER_INFO_SEARCH_RESPONSE == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: response user data [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__ALLIANCE_SEARCH_REQUEST;

        dwRetCode = CBaseProcedure::ProcessProcedure_UserDataGetResponse(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: response user data error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        if (pstSession->m_stUserInfo.m_tbUserStat.m_nLast_help_bubble_set_time < pstSession->m_tbTmpGlobalParam.m_nVal)
        {
            TSE_LOG_DEBUG(m_poServLog, ("Main_flow: Updt help bubble status. [cur_time=%ld][updt_time=%ld][time_out=%ld] [uid=%u][seq=%u]",
                pstSession->m_stUserInfo.m_tbUserStat.m_nLast_help_bubble_set_time, pstSession->m_tbTmpGlobalParam.m_nVal,
                pstSession->m_stUserInfo.m_tbUserStat.m_nLast_help_bubble_time_out, pstSession->m_stUserInfo.m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
            pstSession->m_stUserInfo.m_tbUserStat.Set_Last_help_bubble_set_time(pstSession->m_tbTmpGlobalParam.m_nVal);
            pstSession->m_stUserInfo.m_tbUserStat.Set_Last_help_bubble_time_out(pstSession->m_tbTmpGlobalParam.m_nVal + 3 * 3600);
        }
        else
        {
            TSE_LOG_DEBUG(m_poServLog, ("Main_flow: help bubble status not updt. [updt_time=%ld][time_out=%ld][last_time=%ld] [uid=%u][seq=%u]",
                pstSession->m_stUserInfo.m_tbUserStat.m_nLast_help_bubble_set_time, pstSession->m_stUserInfo.m_tbUserStat.m_nLast_help_bubble_time_out,
                pstSession->m_tbTmpGlobalParam.m_nVal, pstSession->m_stUserInfo.m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
        }
    }

    /*********************************************获取aws数据(aid)************************************************/

    // 9. 获取alliance数据 ——发送请求
    if(EN_PROCEDURE__ALLIANCE_SEARCH_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: get user's alliance data and alliance data [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__ALLIANCE_SEARCH_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = CBaseProcedure::ProcessProcedure_AllianceDataGetRequest(pstSession);

        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request get user's alliance data and alliance data error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }

        if(0 == dwRetCode)
        {
            return;
        }
    }

    // 10. 获取alliance数据 ——处理结果(异常和兼容暂时在这里处理)
    if(EN_PROCEDURE__ALLIANCE_SEARCH_RESPONSE == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: response user's alliance data and alliance data  [ret=%d] [seq=%u]",
            dwRetCode, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_SVR_REQUEST;
        dwRetCode = CBaseProcedure::ProcessProcedure_AllianceDataGetResponse(pstSession);

        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: response user's alliance data and alliance data  error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
    }

    if (EN_PROCEDURE__REPORT_SVR_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: get user's report user data [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_SVR_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

        dwRetCode = CBaseProcedure::ProcessProcedure_ReportSvrRequest(pstSession);

        if (dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request get user's report user error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMON_HANDLE_BEFORE;
        }
        else if (0 == dwRetCode)
        {
            return;
        }
    }

    if (EN_PROCEDURE__REPORT_SVR_RESPONSE == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: response user's report user data  [ret=%d] [seq=%u]",
            dwRetCode, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMON_HANDLE_BEFORE;
        dwRetCode = CBaseProcedure::ProcessProcedure_ReportSvrResponse(pstSession);

        if (dwRetCode < 0)
        {
            // do nothing except log
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: response user's report user data error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));
        }
    }

    /************************************获取用户数据后的公共命令字***********************************************/

    // 13. 获取用户数据后的公共命令字
    if(EN_PROCEDURE__COMMON_HANDLE_BEFORE == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: common handle before [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMON_HANDLE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
        dwRetCode = CCommonHandleBefore::Process_CommonHandleBefore(pstSession);

        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: common handle before error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }

        //wave&push_data
        CPushDataProcess::InitRawInfo(&pstSession->m_stUserInfo, &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl);
    }

    /************************************************生成错误恢复json*******************************************************/
    if(EN_PROCEDURE__COMMON_HANDLE == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: common handle before [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;

        CCommonHandle::Process_CommonHandle(pstSession);
        //pstSession->m_stCommonResInfo.m_bRefreshCode = TRUE;
        //StoreJson(pstSession, TRUE, FILTER_JSON);
    }
    /*************************************************命令字处理*******************************************************/

    // 14. 命令字处理
    if(EN_PROCEDURE__COMMAND_PROCESS == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: cmd process [seq=%u]",
            pstSession->m_udwSeqNo));

        dwRetCode = CBaseCmdProcess::ProcedureCmd(pstSession, bNeedResponse);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: cmd process error [seq=%u]",
                pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        if(TRUE == bNeedResponse)
        {
            // session 已经有网络线程发送了task 有可能回包触发task 导致log打印时session同时被两个线程占用 所以不能打log
            //TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: async cmd process [command_step=%u] [seq=%u]",
            //    pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
            return;
        }

        //TODO 必须制定bNeedResponse
        if(EN_COMMAND_STEP__INIT != pstSession->m_udwCommandStep
            && EN_COMMAND_STEP__END != pstSession->m_udwCommandStep)
        {
            // session 已经有网络线程发送了task 有可能回包触发task 导致log打印时session同时被两个线程占用 所以不能打log
            //TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: async cmd process [command_step=%u] [seq=%u]", 
            //    pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
            return;
        }

        //特殊处理流程
        if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__LOGIN_FAKE)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        }
        else if(pstSession->m_udwCommand == EN_OPSELF_CHECK_PLAYER_MAP)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMON_HANDLE_AFTER;
        }

    }
    /************************************更新用户数据前的公共命令字***********************************************/

    // 15. 更新用户数据前的公共命令字
    if(EN_PROCEDURE__COMMON_HANDLE_AFTER == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: common handle after [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__DATA_CENTER_GET_REQUEST;


        dwRetCode = CCommonHandleAfter::Process_CommonHandleAfter(pstSession);

        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: common handle after error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
    }


    /*****************************************请求数据中心数据****************************************************/
    if(EN_PROCEDURE__DATA_CENTER_GET_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: data center info req [seq=%u]", \
                                   pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__DATA_CENTER_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;

        dwRetCode = CBaseProcedure::ProcessProcedure_DataCenterRequest(pstSession);
        if(-1 == dwRetCode)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
            TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: data center info not need to update [seq=%u]", \
                                        pstSession->m_udwSeqNo));
        }
        else if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: data center info req error [ret=%d] [seq=%u]", \
                                        dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        else
        {
            return;        
        }  
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__DATA_CENTER_GET_RESPONSE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
        CBaseProcedure::ProcessProcedure_DataCenterResponse(pstSession);
    }
    


    /*****************************************更新用户数据*********************************************************/

    // 15. 更新用户信息(不需要解析response数据)
    // 数据的类型有五种: (1)用户自身数据; (2) 用户的联盟数据; (3) 联盟自身的数据 (4)用户的帐号信息 (5)拍卖场数据 
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: update user data/user's alliance/alliance data [seq=%u]",
                                    pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 7.4 更新用户数据(uid/aid)
        dwRetCode = CBaseProcedure::ProcessProcedure_UserAndAllianceDataUpdtRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request update user data/user's alliance/alliance data error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        else if(dwRetCode == 0)
        {
            TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: data not need to update [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else
        {
            // session 已经有网络线程发送了task 有可能回包触发task 导致log打印时session同时被两个线程占用 所以不能打log
            //TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: data need to update [update_req_num=%d] [seq=%u]",
            //    dwRetCode, pstSession->m_udwSeqNo));
            return;
        }
    }

    /*****************************************更新用户帐号信息********************************************************/

    // 16. 更新用户帐号信息(不需要解析response数据)
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: update account info [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_UPDATE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        //wave@push_data
        CPushDataProcess::SendPushDataRequest_ForHuActionComm(pstSession);

        CAwsRequest::UpdateUserAccountInfo(pstSession, &pstSession->m_stUserInfo.m_tbLogin, &pstSession->m_stReqParam);
        dwRetCode = CBaseProcedure::ProcessProcedure_LoginUpdtRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request update account info error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        else if(dwRetCode == 1)
        {
            //terry@20141014 do not need send AWS request, continue the following step
            TSE_LOG_DEBUG(m_poServLog, ("Main_flow:normal: account info not need to update [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 0)
        {
            // session 已经有网络线程发送了task 有可能回包触发task 导致log打印时session同时被两个线程占用 所以不能打log
            //TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: account info need to update [seq=%u]",
            //    pstSession->m_udwSeqNo));
            return;
        }
    }

    //17 活动信息
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_UPDATE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: event info req [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_RESPONSE;

        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        SActiveScore *pstActiveScore = &pstSession->m_stUserInfo.m_stScore;
        TbPlayer *pstPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
        TbAlliance *pstAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
        SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

        if(pstAlliance->m_nAl_star == 0)
        {
            pstAlliance->m_nAvatar = 2;
        }

        TINT64 ddwAlid = 0;
        TINT64 ddwAlMight = 0;
        string sAlName = "";
        string sAlNickName = "";
        TUINT32 udwAlPos = 0;
        TUINT32 udwAlGiftLv = 0;

        if(pstPlayer->m_nAlid && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
        {
            ddwAlid = pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            ddwAlMight = pstAlliance->m_nMight;
            sAlName = pstSession->m_stUserInfo.m_tbPlayer.m_sAlname;
            sAlNickName = pstAlliance->m_sAl_nick_name;
            udwAlPos = pstPlayer->m_nAlpos;
            udwAlGiftLv = CCommonBase::GetAlGiftLevel(pstAlliance);
        }

        if (pstSession->m_stUserInfo.m_bIsSendEventReq == TRUE)
        {
            for (TUINT32 udwIdx = 0; udwIdx < EN_SCORE_TYPE__END; ++udwIdx)
            {
                for (TUINT32 tmpIdx = 0; tmpIdx < MAX_SCORE_ID_NUM; ++tmpIdx)
                {
                    if (pstActiveScore->audwScoreList[udwIdx][tmpIdx] == 0)
                    {
                        continue;
                    }

                    EventReqInfo *pstEventReq = new EventReqInfo;
                    TbLogin *ptbLogin = &pstSession->m_stUserInfo.m_tbLogin;

                    pstSession->m_vecEventReq.push_back(pstEventReq);
                    pstEventReq->SetVal(pstPlayer->m_nSid, pstPlayer->m_nUid, pstPlayer->m_sUin, pstPlayer->m_nMight,
                        ddwAlid, sAlName, ddwAlMight, sAlNickName, udwAlPos,
                        ptbLogin->m_nGem_buy, ptbLogin->m_nMax_buy, ptbLogin->m_nLast_buy, ptbLogin->m_nLast_buy_time,
                        ptbLogin->m_nTotal_pay, ptbLogin->m_nMax_pay, ptbLogin->m_nLast_pay,
                        udwAlGiftLv, ptbLogin->m_nCtime, CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 3),
                        CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__TRIAL),
                        EN_REQUEST_TYPE__UPDATE, pstSession->m_stUserInfo.m_uddwCurEventId, udwIdx,
                        pstActiveScore->audwScoreList[udwIdx][tmpIdx], tmpIdx, 0, 0);

                    //log
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: [score_type=%u][score_id=%u][score=%u][uid=%u][command=%s][req_string=%s][seq=%u]",
                        udwIdx, tmpIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx],
                        pstPlayer->m_nUid, pstSession->m_stReqParam.m_szCommand,
                        pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
                }
            }
        }
        else
        {
            TSE_LOG_ERROR(m_poServLog, ("EventReqInfo: bIsSendEventReqFlag = FALSE, do not send event req [seq=%u]",
                pstSession->m_udwSeqNo));
        }

        dwRetCode = CBaseProcedure::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if (dwRetCode == 0)
        {
            return;
        }
        else if (dwRetCode == -1)
        {
            TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: have no event request to send [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__DB_DATA_UPDATE_REQUEST;
        }
        else
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send event request fail [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal:event info rsp [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__DB_DATA_UPDATE_REQUEST;
    }

    // db 数据更新
    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__DB_DATA_UPDATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: update db data [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__RANK_SVR_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DB;

        dwRetCode = CBaseProcedure::ProcessProcedure_DbDataUpdateRequest(pstSession);
        if (dwRetCode == 0)
        {
            return;
        }
        else if (dwRetCode == -1)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request update db data error [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }
        else if (dwRetCode == -2)
        {
            TSE_LOG_DEBUG(m_poServLog, ("Main_flow:normal: db data not need to update [seq=%u]",
                pstSession->m_udwSeqNo));
        }
    }

    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__RANK_SVR_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: rank info req [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__RANK_SVR_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__RANK_SVR;

        dwRetCode = CBaseProcedure::ProcessProcedure_RankSvrRequest(pstSession);
        if (dwRetCode == 0)
        {
            return;
        }
        else
        {
            TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: have no event request to send [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        }
    }

    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__RANK_SVR_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal:rank info rsp [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;

        CBaseProcedure::ProcessProcedure_RankSvrResponse(pstSession);
    }

PROCESS_COMMAND_END:
    /*****************************************释放锁*********************************************************/

    // 释放锁(不需要response)
    if(EN_PROCEDURE__END == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: release data lock [seq=%u]",
            pstSession->m_udwSeqNo));

        // next procedure
        pstSession->m_udwNextProcedure = EN_PROCEDURE__SEND_RESULT_BACK;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__LOCK_RELEASE;
        if(pstSession->m_bLockedData == TRUE)
        {
            dwRetCode = CBaseProcedure::ProcessProcedure_LockReq(pstSession, EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request release data lock error [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
    }
    /****************************************返回客户端结果*********************************************************/

    // 返回客户端json
    if(EN_PROCEDURE__SEND_RESULT_BACK == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: send result to client [seq=%u]",
            pstSession->m_udwSeqNo));
        StoreJson(pstSession, FALSE,FILTER_JSON);
        dwRetCode = SendBackResult(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send result to client error [reqcmd=%s] [ret=%d] [seq=%u]",
                pstSession->m_stReqParam.m_szCommand, dwRetCode, pstSession->m_udwSeqNo));
        }

        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            //to update data_output 
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PUSH_DATA_REQUEST;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            dwRetCode = CBaseProcedure::ProcessProcedure_DataOutputUpdateRequest(pstSession);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: request update output info error, do nothing [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_udwSeqNo));
            }
            else if(dwRetCode > 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("Main_flow:normal: output info not need to update [seq=%u]",
                    pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PUSH_DATA_RESPONSE;
        }
    }

    if(EN_PROCEDURE__PUSH_DATA_REQUEST == pstSession->m_udwNextProcedure)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__PUSH_DATA_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__USER_LINKER;

        dwRetCode = CBaseProcedure::ProcessProcedure_PushDataRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send pushdata info error, do nothing [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode > 0)
        {
            TSE_LOG_DEBUG(m_poServLog, ("Main_flow:normal: pushdata info not need to update [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else
        {
            return;
        }
    }

    if(EN_PROCEDURE__PUSH_DATA_RESPONSE == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_DEBUG(m_poServLog, ("Main_flow:normal: release session [seq=%u]", pstSession->m_udwSeqNo));
        CGlobalServ::m_poSessionMgr->ReleaseSession(pstSession);
        return;
    }
}

TVOID CTaskProcess::SpecialMainProdure(SSession *pstSession, const TINT32 dwCmdType)
{
    if(EN_PROCEDURE__INIT == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:special: main produre [seq=%u]",
            pstSession->m_udwSeqNo));
    }

    TBOOL bNeedResponse = FALSE;
    TINT32 dwRetCode = 0;

    /******************************************主流程函数的参数校验********************************************/
    // 1. 参数校验
    if(EN_UNKNOW_CMD == dwCmdType)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow:special: [cmdtype=%u] [ret=%d] [cmd=%u] [seq=%u]",
            dwCmdType, pstSession->m_stCommonResInfo.m_dwRetCode,
            pstSession->m_stReqParam.m_udwCommandID, pstSession->m_udwSeqNo));

        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        goto PROCESS_COMMAND_END;
    }

    /******************************************主流程的异常处理***********************************************/
    // 2. 业务流程如果已经出错，直接返回
    if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        TSE_LOG_ERROR(m_poServLog, ("Main_flow:special: async process error [ret=%d] [cmd=%u] [nextprocedure=%d] [seq=%u]",
            pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_stReqParam.m_udwCommandID,
            pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        goto PROCESS_COMMAND_END;
    }

    /******************************************根据命令字做准备工作***********************************************/
    // 3. 根据命令字做准备工作
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__INIT)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:special: prepare process [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
    }

    /*************************************************命令字处理*******************************************************/
    // 4. 根据命令字进行处理
    if(EN_PROCEDURE__COMMAND_PROCESS == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:special: cmd process [seq=%u]",
            pstSession->m_udwSeqNo));

        dwRetCode = CBaseCmdProcess::ProcedureCmd(pstSession, bNeedResponse);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:special: cmd process error [seq=%u]",
                pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            goto PROCESS_COMMAND_END;
        }

        //TODO 必须制定bNeedResponse
        if(EN_COMMAND_STEP__INIT != pstSession->m_udwCommandStep
            && EN_COMMAND_STEP__END != pstSession->m_udwCommandStep)
        {
            // session 已经有网络线程发送了task 有可能回包触发task 导致log打印时session同时被两个线程占用 所以不能打log
            //TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: async cmd process [command_step=%u] [seq=%u]", 
            //    pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
            return;
        }

        if(TRUE == bNeedResponse)
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
    if(EN_PROCEDURE__END == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:special: send result to client [seq=%u]",
            pstSession->m_udwSeqNo));

        StoreJson(pstSession,FALSE, FALSE);
        dwRetCode = SendBackResult(pstSession);

        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:special: send result to client error [reqcmd=%s] [ret=%d] [seq=%u]",
                pstSession->m_stReqParam.m_szCommand, dwRetCode, pstSession->m_udwSeqNo));
        }

        CGlobalServ::m_poSessionMgr->ReleaseSession(pstSession);
    }
}

TINT32 CTaskProcess::SendBackResult(SSession *pstSession)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;

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
        SendBackResult_Binary(pstSession);
    }
    
    TINT64 ddwGemAdd = 0;
    if(pstSession->m_stUserInfo.m_tbLogin.m_nUid > 0)
    {
        ddwGemAdd = pstSession->m_stUserInfo.m_tbLogin.m_nGem - pstSession->m_ddwGemBegin;
    }

    // 记录一些请求参数数据
    SReqInfo stReqInfo;
    stReqInfo.Reset();
    stReqInfo.SetValue(pstSession->m_stReqParam.m_udwSvrId, pstSession->m_dwFinalPackLength, pstSession->m_stReqParam.m_udwCityId, pstSession->m_stReqParam.m_szCommand, pstSession->m_stReqParam.m_szIdfa, pstSession->m_stReqParam.m_szKey, pstSession->m_stReqParam.m_ddwReqCost);
    // 游戏评估系统需要保存的new ex_data
    // source
    CGameEvaluateLogic::SaveGameEvaluateExData(&stReqInfo, &pstSession->m_stUserInfo, EN_EX_DATA_USER_TYPE_SOURCE, EN_EX_DATA_TYPE_NEW);
    
    // 生成游戏评估基本数据
    TBOOL bGetGameEvaluateDataSourceFlag = false;
    SGameEvaluateData stGameEvaluateDataSource;
    stGameEvaluateDataSource.Reset();
    TBOOL bGetGameEvaluateDataTargetFlag = false;
    SGameEvaluateData stGameEvaluateDataTarget;
    stGameEvaluateDataTarget.Reset();
    // source
    SDyeingInfo stDyeingInfo;
    stDyeingInfo.Reset();
        CUserDyeingInfo::GetInstance()->GetUserDyeingInfo(pstSession->m_stUserInfo.m_tbUserStat.m_nUid, &stDyeingInfo);
    stGameEvaluateDataSource.m_addwF[0] = stDyeingInfo.m_ddwF1;
    bGetGameEvaluateDataSourceFlag = CGameEvaluateLogic::GenGameEvaluateData(&stReqInfo, &pstSession->m_stUserInfo, &stGameEvaluateDataSource);

    // 生成游戏评估ex数据 
    TBOOL bGetGameEvaluateExDataSourceResultFlag = false;  
    SGameEvaluateExData stGameEvaluateExDataSourceResult;
    stGameEvaluateExDataSourceResult.Reset();
    TBOOL bGetGameEvaluateExDataTargetResultFlag = false;  
    SGameEvaluateExData stGameEvaluateExDataTargetResult;
    stGameEvaluateExDataTargetResult.Reset();
    // source
    bGetGameEvaluateExDataSourceResultFlag = CGameEvaluateLogic::GenGameEvaluateExData(&pstSession->m_stUserInfo, &stGameEvaluateExDataSourceResult, EN_EX_DATA_USER_TYPE_SOURCE);

    // 游戏评估额外数据
    SGameEvaluateAddData stGameEvaluateAddData;
    stGameEvaluateAddData.Reset();


    TSE_LOG_DEBUG(m_poServLog, ("game_flag::bGetGameEvaluateDataSourceFlag=%u, bGetGameEvaluateDataTargetFlag=%u, bGetGameEvaluateExDataSourceResultFlag=%u, bGetGameEvaluateExDataTargetResultFlag=%u, seq=%u", \
                                bGetGameEvaluateDataSourceFlag, bGetGameEvaluateDataTargetFlag, \
                                bGetGameEvaluateExDataSourceResultFlag, bGetGameEvaluateExDataTargetResultFlag, \
                                pstSession->m_udwSeqNo));

    string strGameEvaluateLog = "";
    strGameEvaluateLog = CGameEvaluateLogic::GenGameEvaluateLog(&stGameEvaluateDataSource, bGetGameEvaluateDataSourceFlag, \
                                                                &stGameEvaluateDataTarget, bGetGameEvaluateDataTargetFlag, \
                                                                &stGameEvaluateExDataSourceResult, bGetGameEvaluateExDataSourceResultFlag, \
                                                                &stGameEvaluateExDataTargetResult, bGetGameEvaluateExDataTargetResultFlag, \
                                                                &stGameEvaluateAddData, pstSession->m_stUserInfo.m_bGameEvaluateType);

    //4. 记录日志
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    TUINT64 uddwLastMight = pstSession->m_stUserInfo.m_uddwLastMight;
    string sLastJson = pstSession->m_stUserInfo.m_sLastTroopAndFort;

    TUINT64 uddwCurMight = pstSession->m_stUserInfo.m_uddwCurMight;
    string sCurJson = pstSession->m_stUserInfo.m_sCurTroopAndFort;
    
    TUINT64 uddwCostTime = pstSession->m_stCommonResInfo.m_uddwCostTime;
    TUINT64 udddwTotalMight = CCommonBase::ComputeTotalMight(&pstSession->m_stUserInfo);
    TSE_LOG_HOUR(m_poDayLog, ("req_url[%s],ret[%d],cost[%lu],svr[%u],new_player[%u],new_svr[%u],ip[%s],tid[%lu],might[%ld],"
        "lv[%ld],uname[%s],city_pos[%u],gem_cost[%u],"
        "gem_cur[%lld],resource[%u:%u:%u:%u:%u],g_uid[%u],g_sid[%u],fgem[%u],gem_add[%ld],alid[%d],alpos[%ld],"
        "ex_troop_end[%lu],ex_might[%lu],troop_flag[%u],troop_begin[%u],troop_change[%ld],get_troop[%u],altar_building[%u],action_num[%u],p_action_num[%u],"
        "npc[%lu],err_stat[%u,%u,%u,%u,%u,%u,%u,%u],AddPersonAlGiftStatus[%u],mail_id[%ld],"
        "t_gem_buy[%ld],gem_buy[%u],age[%ld],hero_lv[%ld],castle_lv[%u],ctime[%ld],lang[%ld],force[%ld],avatar[%ld],alstar[%ld],"
        "game_evaluate[%s],"
        "aws_read_cost[%lu],aws_write_cost[%lu],aws_read_write_cost[%lu],aws_no_op_cost[%lu],db_read_cost[%lu],"
        "db_write_cost[%lu],db_read_write_cost[%lu],db_no_op_cost[%lu],auction_cost[%lu],lock_cost[%lu],res_len[%u],[event_seq=%lu],"
        "now_might[%lu],change[%ld],last_json[%s],now_json[%s] [client_seq=%u] [seq=%u]",
        pstSession->m_stReqParam.m_szReqUrl,
        pstSession->m_stCommonResInfo.m_dwRetCode, uddwCostTime,
        pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_ucIsNewPlayer,
        pstSession->m_stReqParam.m_ucIsNewSvrPlayer, pstSession->m_szClientIp,
        pstSession->m_stUserInfo.m_uddwNewActionId, pstSession->m_stUserInfo.m_tbPlayer.m_nMight,
        pstSession->m_stUserInfo.m_tbPlayer.m_nLevel, pstSession->m_stUserInfo.m_tbPlayer.m_sUin.c_str(),
        pstSession->m_stUserInfo.m_tbPlayer.m_nCid, pstSession->m_udwGemCost, pstSession->m_stUserInfo.m_tbLogin.m_nGem,
        //stResource[0], stResource[1], stResource[2], stResource[3], stResource[4],
        0, 0, 0, 0, 0,
        pstSession->m_stUserInfo.m_tbLogin.m_nUid, pstSession->m_stUserInfo.m_tbLogin.m_nSid,
        pstSession->m_ucFakeRecharge, ddwGemAdd, pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET,
        pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos,
        0, udddwTotalMight, 0,
        //udddwTotalTroop,
        //udddwTotalMight,
        //bTroopChangeValid,
        pstSession->m_ddwTroopBegin,
        pstSession->m_ddwTroopChange,
        pstSession->m_bGetTroop,
        0, //bHaveAltar,
        pstSession->m_stUserInfo.m_udwActionNum,
        0,
        pstSession->m_stUserInfo.m_tbLogin.m_nNpc,
        pstSession->m_stUserInfo.m_dwErrTroop,
        pstSession->m_stUserInfo.m_dwErrFort,
        pstSession->m_stUserInfo.m_dwErrGem,
        pstSession->m_stUserInfo.m_dwErrProdBonus,
        pstSession->m_stUserInfo.m_dwErrMarchBonus,
        pstSession->m_stUserInfo.m_dwErrProd,
        pstSession->m_stUserInfo.m_dwErrItem,
        pstSession->m_stUserInfo.m_dwErrBuildLv,
        pstSession->m_ucAddPersonAlGiftStatus,
        pstSession->m_ddwNewMailId,
        pstUser->m_tbLogin.m_nGem_buy, pstSession->m_udwGembuy,
        pstUser->m_tbPlayer.m_nAge, pstUser->m_tbPlayer.m_nLevel,
        pstSession->m_udwCastlelv,pstUser->m_tbPlayer.m_nCtime,
        pstUser->m_tbLogin.m_nLang, pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_ddwBuffTotal,
        pstUser->m_tbPlayer.m_nAvatar, pstUser->m_tbAlliance.m_nAl_star,
        strGameEvaluateLog.c_str(),
        pstSession->m_uddwAwsReadSumTime,
        pstSession->m_uddwAwsWriteSumTime,
        pstSession->m_uddwAwsReadWriteSumTime,
        pstSession->m_uddwAwsNoOpSumTime,
        pstSession->m_uddwDbReadSumTime,
        pstSession->m_uddwDbWriteSumTime,
        pstSession->m_uddwDbReadWriteSumTime,
        pstSession->m_uddwDbNoOpSumTime,
        pstSession->m_uddwAuctionSumTime,
        pstSession->m_uddwLockSumTime,
        pstSession->m_dwFinalPackLength,
        pstSession->m_stUserInfo.m_uddwCurEventId,
        uddwCurMight,
        (TINT64)uddwCurMight - (TINT64)uddwLastMight,
        sLastJson.c_str(),
        sCurJson.c_str(),
        pstSession->m_udwClientSeqNo, 
        pstSession->m_udwSeqNo));

    if (pstSession->m_stCommonResInfo.m_ucJsonType == EN_JSON_TYPE_USER_JSON
        && pstSession->m_sRspJsonContent.size() > 0)
    {
        CJasonLog::OutLogForStat(pstSession, pstSession->m_sRspJsonContent.c_str(), pstSession->m_sRspJsonContent.size());
    }

    // 什么命令字不要打cblog
    if(pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_OPERATE_CMD__LOG)
    {   
        CHuCBLog::OutOperateLogCbLog(pstSession);
    }
    else if(pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__MAIL_GET &&
       pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__MAP_BLOCK_GET &&
       pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__MAP_GET &&
       pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__REPORT_GET)
    {
        CHuCBLog::OutLogCbLog(pstSession);
    }

    // 5.统计
    CStatistic *pstStatistic = CStatistic::Instance();
    if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)//成功
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

    pstSession->m_pJsonGenerator = &m_oJsonResultGenerator;
}

TVOID CTaskProcess::PreprocessReq(SSession *pstSession)
{
    // 获取需要加锁的taskid
    GetLockedTasked(pstSession);
}

TVOID CTaskProcess::GetLockedTasked(SSession *pstSession)
{
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__MARCH_RALLY_REINFORCE)
    {
        pstSession->m_ddwLockedTaskId = strtoll(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    }
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__MARCH_RALLY_SLOT_BUY)
    {
        pstSession->m_ddwLockedTaskId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    }
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__MARCH_RALLY_RECALL)
    {
        pstSession->m_ddwLockedTaskId = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    }
}

TVOID CTaskProcess::StoreJson(SSession* pstSession,TBOOL  bRefresh/* = FALSE*/, TBOOL bToFilter/* = TRUE*/)
{
    TUINT64 uddwBegin = CTimeUtils::GetCurTimeUs();
    pstSession->m_uddwTimeEnd = uddwBegin;
    if(pstSession->m_uddwTimeEnd > pstSession->m_uddwTimeBeg)
    {
        pstSession->m_stCommonResInfo.m_uddwCostTime = pstSession->m_uddwTimeEnd - pstSession->m_uddwTimeBeg;
    }

    pstSession->m_dwClientResType = UpdateDataOutputType(pstSession);
    
    // force push data
    if(pstSession->m_dwClientResType == EN_CONTENT_UPDATE_TYPE__ALL && pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__TCP)
    {
        pstSession->m_stUserInfo.m_tbDataOutput.m_jData.clear();
        TSE_LOG_INFO(m_poServLog, ("data_output Reset: req_type=%d, cmd_ref=%d, cmd=%s type=%d [seq=%u]", 
            pstSession->m_stReqParam.m_ucResType, pstSession->m_udwLinkerCmdRef, pstSession->m_stReqParam.m_szCommand, 
            pstSession->m_dwOutputCmpType, pstSession->m_udwSeqNo));
    }
    
    if(pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__HTTP)
    {
        StoreJson_String(pstSession, bRefresh, bToFilter);
    }
    else
    {
        StoreJson_Binary(pstSession, bRefresh, bToFilter);
    }

    TUINT64 uddwEnd = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(m_poServLog, ("Main_flow:Json gen cost[%lu]us [seq=%u]", uddwEnd - uddwBegin, pstSession->m_udwSeqNo));
}

TINT32 CTaskProcess::SendBackResult_Http( SSession *pstSession )
{
    TINT32 dwClientSocket = 0;

    // 1.获取socket
    dwClientSocket = m_poQueryLongConn->GetSockHandle(pstSession->m_stClientHandle);
    if(dwClientSocket == INVALID_SOCKET)
    {
        TSE_LOG_DEBUG(m_poServLog, ("INVALID_SOCKET in send back message! [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }
    
    // 2.发送结果
    tse_socket_writeFull(dwClientSocket, pstSession->m_szClientRspBuf, pstSession->m_dwFinalPackLength);

    // 3. 发送完结果后，关闭连接
    m_poQueryLongConn->RemoveLongConnSession(pstSession->m_stClientHandle);

    return 0;
}

TINT32 CTaskProcess::SendBackResult_Binary( SSession *pstSession )
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
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_UID, pstSession->m_stReqParam.m_udwUserId);

    //小于1k的包，不做压缩
    if(pstSession->m_dwFinalPackLength < 600)
    {
        ucCompressFlag = 0;
    }
    else
    {
        ucCompressFlag = 1;
    }

    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);

    if(ucCompressFlag)
    {
        //udwCompressDataLen = m_oJsonResultGenerator.Compress(pstSession->m_stReqParam.m_udwVersion, pstSession->m_dwFinalPackLength, &pstSession->m_szClientRspBuf[0]);
        udwCompressDataLen = m_oJsonResultGenerator.CompressZip(pstSession->m_dwFinalPackLength, &pstSession->m_szClientRspBuf[0]);
        pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF, (TUCHAR*)m_oJsonResultGenerator.m_szEncodeBuffer, udwCompressDataLen);
    }
    else
    {
        pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF, (TUCHAR*)&pstSession->m_szClientRspBuf[0], pstSession->m_dwFinalPackLength);
    }
    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_INFO(m_poServLog, ("SendBackResult_Binary: send package_len=%u  compress_flag=%u raw_len=%u compress=%u [seq=%u]", 
        udwPackageLen, ucCompressFlag, pstSession->m_dwFinalPackLength, udwCompressTmpLen, pstSession->m_udwSeqNo));

    // 2. send back
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_stClientHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if(!m_poQueryLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(m_poServLog, ("SendBackResult_Binary: send response to client failed [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
    return 0;
}

TVOID CTaskProcess::StoreJson_String( SSession* pstSession,TBOOL bRefresh, TBOOL bToFilter )
{
    const TCHAR* pBody = NULL;

    if(pstSession->m_dwClientReqEnType)
    {
        m_oJsonResultGenerator.SetCompress(TRUE);
    }
    else
    {
        m_oJsonResultGenerator.SetCompress(FALSE);
    }
    pBody = m_oJsonResultGenerator.GenResult(pstSession);
    TUINT32 udwBodyLen = m_oJsonResultGenerator.GetResultJsonLength();

    if(bRefresh)
    {
        memcpy(pstSession->m_szTmpClientRspBuf, pBody, udwBodyLen);
        pstSession->m_szTmpClientRspBuf[udwBodyLen] = '\0';
    }
    else
    {
        CHttpUtils::add_http_result_head(pBody, udwBodyLen,
            pstSession->m_szClientRspBuf, MAX_NETIO_PACKAGE_BUF_LEN, pstSession->m_dwFinalPackLength, "utf-8", "text/html");
    }

    pstSession->m_dwOriPackLength = pstSession->m_dwFinalPackLength;
}

TVOID CTaskProcess::StoreJson_Binary( SSession* pstSession,TBOOL bRefresh, TBOOL bToFilter )
{
    m_oJsonResultGenerator.GenResult_Pb(pstSession);
    pstSession->m_dwOriPackLength = pstSession->m_dwFinalPackLength;
}

TINT32 CTaskProcess::UpdateDataOutputType( SSession* pstSession )
{
    SReqParam *pstReq = &pstSession->m_stReqParam;
    
    if(pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__HTTP)
    {
        return EN_CONTENT_UPDATE_TYPE__ALL;
    }

    //if(pstSession->m_stCommonResInfo.m_ucJsonType != EN_JSON_TYPE_USER_JSON)
    //{
    //    return EN_CONTENT_UPDATE_TYPE__ALL;
    //}

    if(pstReq->m_ucResType == EN_CONTENT_UPDATE_TYPE__ALL)
    {
        return EN_CONTENT_UPDATE_TYPE__ALL;
    }
    
    if(pstReq->m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN)
    {
        return EN_CONTENT_UPDATE_TYPE__ALL;
    }

    if(pstReq->m_udwCommandID == EN_CLIENT_REQ_COMMAND__GUIDE_FINISH
        || pstReq->m_udwCommandID == EN_CLIENT_REQ_COMMAND__GUIDE_FINISH_STAGE)
    {
        return EN_CONTENT_UPDATE_TYPE__ALL;
    }

    if(pstSession->m_udwLinkerCmdRef <= 1) //重新连接到linker时，做一次强制刷新
    {
        return EN_CONTENT_UPDATE_TYPE__ALL;
    }

    return EN_CONTENT_UPDATE_TYPE__TABLE_INC;
}
