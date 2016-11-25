#include "task_process.h"
#include "base/os/wtsesocket.h"
#include "session_mgr.h"
#include "statistic.h"
#include "global_serv.h"
#include "aws_request.h"
#include "aws_response.h"
#include "game_info.h"
#include "common_func.h"
#include "global_serv.h"
#include "db_request.h"
#include "db_response.h"
#include "war_process.h"
#include "process_action.h"
#include "quest_notic_logic.h"
#include "production_system.h"
#include "buffer_base.h"
#include "conf_base.h"
#include "event_req_info.h"
#include "activities_logic.h"
#include "common_handle_after.h"
#include "quest_logic.h"
#include "game_evaluate_logic.h"
#include "msg_base.h"
#include "common_handle_before.h"
#include "common_base.h"
#include "map_base.h"
#include "action_base.h"
#include "dc_log.h"
#include "wild_info.h"
#include "map_logic.h"
#include "report_svr_request.h"
#include "jsonlog.h"
#include "player_base.h"

TINT32 CTaskProcess::Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn,
    CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDbLog)
{
    if(NULL == poConf || NULL == poSearchLongConn || NULL == pTaskQueue || NULL == poServLog)
    {
        return -1;
    }
    TSE_LOG_DEBUG(poServLog, ("CTaskProcess:sizeof(LTasksGroup)=%u", sizeof(LTasksGroup)));

    TUINT32 idx = 0;

    m_poConf = poConf;
    m_poSearchLongConn = poSearchLongConn;
    m_poQueryLongConn = poQueryLongConn;
    m_pTaskQueue = pTaskQueue;
    m_poServLog = poServLog;
    m_poDbLog = poDbLog;
    m_poReqLog = CGlobalServ::m_poReqLog;
    for(idx = 0; idx < MAX_LOCAL_HS_TASK_NUM; idx++)
    {
        m_pPackTool[idx] = new CBaseProtocolPack();
        m_pPackTool[idx]->Init();
    }
    //m_pPackTool = new CBaseProtocolPack();
    //m_pPackTool->Init();
    m_pUnPackTool = new CBaseProtocolUnpack();
    m_pUnPackTool->Init();

    m_jWriter.omitEndingLineFeed();

    m_pTmpBuf = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];

    m_pJsonGenerator = new CJsonResultGenerator;

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
            TSE_LOG_DEBUG(m_poServLog, ("WaitTillPop: pop null"));
            continue;
        }
        
        pstSession->m_ppPackTool = m_pPackTool;
        pstSession->m_poLongConn = m_poSearchLongConn;
        m_pTmpBuf[0] = '\0';
        pstSession->m_pTmpBuf = &m_pTmpBuf[0];
        pstSession->m_udwTmpBufLen = 0;
        pstSession->m_pJsonGenerator = m_pJsonGenerator;

        switch(pstSession->m_stActionTable.dwTableType)
        {
        case EN_UID_ACTION:
            UidActionProdure(pstSession);
            break;
        case EN_AID_ACTION:
            AidActionProdure(pstSession);
            break;
        case EN_MARCH_ACTION:
            MarchActionProdure(pstSession);
            break;
        default:
            break;
        }
    }

    return 0;
}

TVOID CTaskProcess::MarchActionProdure(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    TBOOL bNeedResponse = FALSE;

    TSE_LOG_DEBUG(m_poServLog, ("check nextprocedure: step[%u],[code=%u] [nextprocedure=%d], [seq=%u]",
        pstSession->m_udwProcessSeq, pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

    // 0. 如果已经出错，直接返回
    if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__DATA_CENTER
            && pstSession->m_udwNextProcedure == EN_PROCEDURE__COMMAND_PROCESS)
        {
            for (unsigned int i = 0; i < pstSession->m_vecDataCenterRsp.size(); ++i)
            {
                delete pstSession->m_vecDataCenterRsp[i];
            }
            pstSession->m_vecDataCenterRsp.clear();
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SUCCESS;
            
            TSE_LOG_DEBUG(m_poServLog, ("data center rsp error: [commandstep=%d] [seq=%u]", \
                                        pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
        }
        else
        {
            if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_REQUEST)   ///////
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
            }
            else if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
            }
            else if(EN_PROCEDURE__EVENT_PROCESS_UPDATE == pstSession->m_udwNextProcedure)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
            }
            else if (pstSession->m_udwNextProcedure != EN_PROCEDURE__END 
                && pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__TIMEOUT)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
            }
            else if (pstSession->m_udwNextProcedure != EN_PROCEDURE__END)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            }

            TSE_LOG_INFO(m_poServLog, ("SidActionProdure: check_retcode [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

            goto PROCESS_REQ_COMMAND_END;
        }
    }

    // 1. 根据命令字做准备工作
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__INIT)
    {
        TINT64 ddwTroopTotalNumInit = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, init=%ld [seq=%u]",
            pstSession->m_stReqMarch.m_nId,
            pstSession->m_stReqMarch.m_nMclass,
            pstSession->m_stReqMarch.m_nSclass,
            pstSession->m_stReqMarch.m_nStatus,
            ddwTroopTotalNumInit,
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__LOCK_FOR_FISRT_DATA_GET;

        pstSession->m_poServLog = m_poServLog;
        pstSession->m_stSourceUser.m_udwBSeqNo = pstSession->m_udwSeqNo;
        pstSession->m_stTargetUser.m_udwBSeqNo = pstSession->m_udwSeqNo;
        pstSession->m_stRawReqMarch = pstSession->m_stReqMarch;

        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: recv_req: svr[%u],action[%ld],main=%ld,sec=%ld,status=%ld,"
            "suid=%ld,scid=%ld,tuid=%ld,tpos=%ld,btime=%ld,etime=%ld [seq=%u]",
            pstSession->m_udwReqSvrId, pstSession->m_stReqMarch.m_nId,
            pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
            pstSession->m_stReqMarch.m_nStatus, pstSession->m_stReqMarch.m_nSuid,
            pstSession->m_stReqMarch.m_nScid, pstSession->m_stReqMarch.m_nTuid,
            pstSession->m_stReqMarch.m_nTpos, pstSession->m_stReqMarch.m_nBtime,
            pstSession->m_stReqMarch.m_nEtime, pstSession->m_udwSeqNo));
    }

    //第一步锁数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__LOCK_FOR_FISRT_DATA_GET)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: lock_for_first_data_get: [nextprocedure=%d] [seq=%u]",
            pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_DATA_GET_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDUER__LOCK_GET;

        dwRetCode = FirstLockGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("FirstLockGetRequest: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("FirstLockGetRequest: no data to lock, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__LOCK_FOR_SECOND_DATA_GET;
        }
        else
        {
            return;
        }
    }

    //第一步数据拉取  map source_user
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("ProcessReqCommand: first_data_get_request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = FirstDataGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            TSE_LOG_ERROR(m_poServLog, ("FirstDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            goto PROCESS_REQ_COMMAND_END;
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("FirstDataGetRequest: no data to get, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__LOCK_FOR_SECOND_DATA_GET;
        }
        else
        {
            return;
        }
    }

    //解析第一步数据拉取拉回的数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_DATA_GET_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: first data get response: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_REQUEST;

        dwRetCode = UserInfoGetResponse(pstSession, &pstSession->m_stSourceUser, TRUE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: source_user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    //联盟信息拉取
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: first alliance data get request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos)
        {
            dwRetCode = AllianceInfoRequest(pstSession, &pstSession->m_stSourceUser, pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                TSE_LOG_ERROR(m_poServLog, ("FirstDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
                goto PROCESS_REQ_COMMAND_END;
            }
            else
            {
                return;
            }
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_HANDLE_BEFORE;
        }
    }

    //联盟信息解析
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_RESPONSE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_HANDLE_BEFORE;

        dwRetCode = AllianceInfoGetResponse(pstSession, &pstSession->m_stSourceUser, TRUE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: source alliance user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    //handle before
    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_HANDLE_BEFORE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__LOCK_FOR_SECOND_DATA_GET;
        dwRetCode = CAuCommonBefore::AuCommonHandleBefore(pstSession, &pstSession->m_stSourceUser);
        if (dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("AuCommonHandleBefore: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    //第二步锁数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__LOCK_FOR_SECOND_DATA_GET)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: lock for second data get: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDUER__SECOND_DATA_GET_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDUER__LOCK_GET;

        dwRetCode = SecondLockGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("SecondLockGetRequest: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("SecondLockGetRequest: no data to lock, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        }
        else
        {
            return;
        }
    }

    //第二步数据拉取 target_user
    if(pstSession->m_udwNextProcedure == EN_PROCEDUER__SECOND_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: second data get request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDUER__SECOND_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = SecondDataGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            TSE_LOG_ERROR(m_poServLog, ("SecondDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            goto PROCESS_REQ_COMMAND_END;
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("SecondLockGetRequest: no data to get, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        }
        else
        {
            return;
        }
    }

    //解析第二步数据拉取拉回的数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDUER__SECOND_DATA_GET_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: second data get response: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__SECOND_ALLIANCE_DATA_GET_REQUEST;

        dwRetCode = UserInfoGetResponse(pstSession, &pstSession->m_stTargetUser, FALSE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: target_user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    //联盟信息拉取
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__SECOND_ALLIANCE_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: second alliance data get request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__SECOND_ALLIANCE_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid && pstSession->m_stTargetUser.m_tbPlayer.m_nAlpos)
        {
            dwRetCode = AllianceInfoRequest(pstSession, &pstSession->m_stTargetUser, pstSession->m_stTargetUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                TSE_LOG_ERROR(m_poServLog, ("FirstDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
                goto PROCESS_REQ_COMMAND_END;
            }
            else
            {
                return;
            }
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__SECOND_HANDLE_BEFORE;
        }
    }

    //联盟信息解析
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__SECOND_ALLIANCE_DATA_GET_RESPONSE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__SECOND_HANDLE_BEFORE;

        dwRetCode = AllianceInfoGetResponse(pstSession, &pstSession->m_stTargetUser, FALSE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: target alliance user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    //handle before
    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__SECOND_HANDLE_BEFORE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        dwRetCode = CAuCommonBefore::AuCommonHandleBefore(pstSession, &pstSession->m_stTargetUser);
        if (dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("AuCommonHandleBefore: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    // 10. 根据命令字进行处理
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__COMMAND_PROCESS)
    {
        TINT64 ddwTroopTotalNumBegin = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: command_process: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        //wave@push_data
        CAuPushData::InitRawAlInfo(&pstSession->m_stSourceUser, &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl);
        if(pstSession->m_stTargetUser.m_tbPlayer.m_nUid && pstSession->m_stTargetUser.m_tbPlayer.m_nUid != pstSession->m_stSourceUser.m_tbPlayer.m_nUid)
        {
            CAuPushData::InitRawAlInfo(&pstSession->m_stTargetUser, &pstSession->m_objAuPushDataNode.m_objPushDataTargetAl);
        }        
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("[WAVETEST-0]ProcessProcedure_UserUpdtRequest al_source[uid=%u,aid=%u] al_target[uid=%u,aid=%u] [seq=%u]", 
            pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwUid,
            pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwAllianceId,
            pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwUid,
            pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwAllianceId,
            pstSession->m_udwSeqNo));

        // update action info
        dwRetCode = UpdateAndCheckActionInfo(pstSession, &pstSession->m_stMapItem, &pstSession->m_stSourceUser, &pstSession->m_stTargetUser);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UpdateAndCheckActionInfo: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        // command process
        pstSession->m_udwProcessSeq++;
        bNeedResponse = FALSE;

        // process action
        dwRetCode = CProcessAction::ProcessMarchAction(pstSession, bNeedResponse);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("process_by_cmd: main=%ld,sec=%ld,status=%ld,ret=%d [seq=%u]",
                pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                pstSession->m_stReqMarch.m_nStatus, dwRetCode, pstSession->m_udwSeqNo));
            if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
            }

            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        // 处理Action过程中是否请求下游数据
        if (bNeedResponse == TRUE 
            || (EN_COMMAND_STEP__INIT != pstSession->m_udwCommandStep && EN_COMMAND_STEP__END != pstSession->m_udwCommandStep))
        {
            if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__AWS)
            {
                dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("process_by_cmd: send aws req failed,main=%ld,sec=%ld,status=%ld,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                        pstSession->m_stReqMarch.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__DB)
            {
                dwRetCode = SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("process_by_cmd: send db req failed,main=%ld,sec=%ld,status=%ld,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                        pstSession->m_stReqMarch.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__DATA_CENTER)
            {
                // do nothing
            }
            /*
            else if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__DATA_CENTER)
            {
                dwRetCode = SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("process_by_cmd: send data_center req failed,main=%ld,sec=%ld,status=%ld,ret=%d,command_step=%u [seq=%u]",
                                                             pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                                                             pstSession->m_stReqMarch.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            */
            else if (pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__MAP_SVR)
            {
                dwRetCode = SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
                if (dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send map svr req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                        pstSession->m_stReqMarch.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else if (pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__REPORT_SVR)
            {
                dwRetCode = SendReportSvrRequest(pstSession);
                if (dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send report svr req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                        pstSession->m_stReqMarch.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("process_by_cmd: expect_procedure_error:%u,main=%ld,sec=%ld,status=%ld,ret=%d,command_step=%u [seq=%u]",
                    pstSession->m_udwExpectProcedure, pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                    pstSession->m_stReqMarch.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            return;
        }

        TINT64 ddwTroopTotalNumEnd = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, begin=%ld, end=%ld [seq=%u]",
            pstSession->m_stReqMarch.m_nId,
            pstSession->m_stReqMarch.m_nMclass,
            pstSession->m_stReqMarch.m_nSclass,
            pstSession->m_stReqMarch.m_nStatus,
            ddwTroopTotalNumBegin,
            ddwTroopTotalNumEnd,
            pstSession->m_stSourceUser.m_udwBSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST;
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: gen_report_id_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));
        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_INSERT_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        if(pstSession->m_dwReportFlag)
        {
            dwRetCode = GenReportIdRequest(pstSession);
            if(dwRetCode < 0)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            return;
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
        }
    }

    // 11. 优先插入report
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORT_INSERT_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: report_insert_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_USER_INSERT_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbReportId);
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: report_insert_req: ret[%d] report_id[%ld] [seq=%u]", dwRetCode, pstSession->m_tbReportId.m_nVal, pstSession->m_udwSeqNo));
        if(dwRetCode <= 0 || pstSession->m_tbReportId.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REPORT_ID_GET_FAIL;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        dwRetCode = InsertReport(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
        else if(dwRetCode == 0)
        {
            return;
        }
    }

    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORT_USER_INSERT_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: report_user_insert_req: size=%u [nextprocedure=%d] [seq=%u]", 
            pstSession->m_vecReportReq.size(), pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));
        pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;

        if (pstSession->m_vecReportReq.size() > 0)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
            dwRetCode = SendReportSvrRequest(pstSession);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            else if (dwRetCode == 0)
            {
                return;
            }
        }
    }

PROCESS_REQ_COMMAND_END:
    if(EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: abnormal_action_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        //异常处理逻辑
        dwRetCode = AbNormalActionHandle(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("send abnormal action handle req failed. ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 0)
        {
            return;
        }
    }

    // 12. 更新用户信息
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: user_info_update_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 如果处理出错，则按照命令字处理
        if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
        {
            TSE_LOG_ERROR(m_poServLog, ("process error: action[%ld],main=%ld,sec=%ld,status=%ld,suid=%ld,scid=%ld,tuid=%ld,tpos=%ld,flag=%u,ret=%d [seq=%u]",
                pstSession->m_stReqMarch.m_nId, pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
                pstSession->m_stReqMarch.m_nStatus, pstSession->m_stReqMarch.m_nSuid, pstSession->m_stReqMarch.m_nScid,
                pstSession->m_stReqMarch.m_nTuid, pstSession->m_stReqMarch.m_nTpos, pstSession->m_ucReqMarchFlag,
                pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else
        {
            // 进行数据更新
            dwRetCode = ProcessProcedure_UserUpdtRequest(pstSession);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("ProcessProcedure_UserUpdtRequest: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
                pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
            }
            else if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("ProcessProcedure_UserUpdtRequest: all info do not need to update [seq=%u]", pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
    }
    // 13. wavewang@20130511: 对已经加锁的目标进行解锁操作
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: unlock_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_UPDATE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDUER__LOCK_RELEASE;

        if(pstSession->m_bLockedData)
        {
            dwRetCode = ProcessProcedure_LockReq(pstSession, EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(m_poServLog, ("lock_release ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
    }


    //17 活动信息
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_UPDATE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: event  info  req [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        SActiveScore *pstActiveScore = &pstSession->m_stSourceUser.m_stScore;
        TbPlayer *pstPlayer = &pstSession->m_stSourceUser.m_tbPlayer;

        TbAlliance *pstAlliance = &pstSession->m_stSourceUser.m_tbAlliance;

        SUserInfo *pstSUser = &pstSession->m_stSourceUser;
        //SCityInfo *pstSCity = CCityBase::GetCityInfo(pstSUser->m_astCityInfo, pstSUser->m_udwCityNum, pstSession->m_stReqMarch.m_nScid);
		SCityInfo *pstSCity = &pstSUser->m_stCityInfo;
        TINT64 ddwAlid = 0;
        TINT64 ddwAlMight = 0;
        string sAlName = "";
        string sAlNickName = "";
        TUINT32 udwAlPos = 0;
        TUINT32 udwAlGiftLv = 0;
        TUINT32 udwScoreType = 0;
        TUINT32 udwScoreId = 0;

        if(pstPlayer->m_nAlid && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos)
        {
            ddwAlid = pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            ddwAlMight = pstAlliance->m_nMight;
            sAlName = pstSession->m_stSourceUser.m_tbPlayer.m_sAlname;
            sAlNickName = pstAlliance->m_sAl_nick_name;
            udwAlPos = pstPlayer->m_nAlpos;
            udwAlGiftLv = CCommonBase::GetAlGiftLevel(pstAlliance);
        }
        if (pstSUser->m_bIsSendEventReq == TRUE)
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
                    TbLogin *ptbLogin = &pstSession->m_stSourceUser.m_tbLogin;

                    pstSession->m_vecEventReq.push_back(pstEventReq);
                    pstEventReq->SetVal(pstPlayer->m_nSid, pstPlayer->m_nUid, pstPlayer->m_sUin, pstPlayer->m_nMight,
                        ddwAlid, sAlName, ddwAlMight, sAlNickName, udwAlPos,
                        ptbLogin->m_nGem_buy, ptbLogin->m_nMax_buy, ptbLogin->m_nLast_buy, ptbLogin->m_nLast_buy_time,
                        ptbLogin->m_nTotal_pay, ptbLogin->m_nMax_pay, ptbLogin->m_nLast_pay,
                        udwAlGiftLv, ptbLogin->m_nCtime, CCityBase::GetBuildingLevelById(&pstSCity->m_stTblData, 3),
                        CCityBase::GetBuildingLevelByFuncType(pstSCity, EN_BUILDING_TYPE__TRIAL),
                        EN_REQUEST_TYPE__UPDATE, pstSession->m_stSourceUser.m_uddwCurEventId,
                        udwIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx], tmpIdx, 0, 0);

                    //log
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: [score_type=%u][score_id=%u][score=%u][uid=%u][command=%s][req_string=%s][seq=%u]",
                        udwIdx, tmpIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx],
                        pstPlayer->m_nUid, pstSession->m_stReqParam.m_szCommand,
                        pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
                }
            }
            for (TUINT32 udwIdx = 0; udwIdx < MAX_SCORE_LIST_LENGTH; ++udwIdx)
            {
                if (pstActiveScore->sScoreList[udwIdx].ddwUid != 0)
                {
                    EventReqInfo *pstEventReq = new EventReqInfo;
                    SScoreList *pstScoreList = &pstActiveScore->sScoreList[udwIdx];
                    pstSession->m_vecEventReq.push_back(pstEventReq);
                    pstEventReq->SetVal(pstScoreList->udwSid, pstScoreList->ddwUid, pstScoreList->strUname, pstScoreList->uddwAlid, 
                        pstScoreList->udwScoreType, pstScoreList->udwScore, pstScoreList->udwScoreId, EN_REQUEST_TYPE__UPDATE);

                    //log
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: [score_type=%u][score_id=%u][score=%u][uid=%u][command=%s][req_string=%s][seq=%u]",
                        pstScoreList->udwScoreType, pstScoreList->udwScoreId, pstScoreList->udwScore,
                        pstScoreList->ddwUid, pstSession->m_stReqParam.m_szCommand,
                        pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
                }
                else
                {
                    break;
                }
            }

        }
        
        if(pstSession->m_stTargetUser.m_tbPlayer.m_nUid != 0 && pstSession->m_stTargetUser.m_bIsSendEventReq == TRUE)
        {
            SActiveScore *pstActiveScore = &pstSession->m_stTargetUser.m_stScore;
            TbPlayer *pstPlayer = &pstSession->m_stTargetUser.m_tbPlayer;

            TUINT32 udwAllianceId = pstSession->m_stTargetUser.m_tbAlliance.m_nAid;
            TbAlliance *pstAlliance = &pstSession->m_stTargetUser.m_tbAlliance;

            TINT64 ddwAlid = 0;
            TINT64 ddwAlMight = 0;
            string sAlName = "";
            string sAlNickName = "";
            TUINT32 udwAlPos = 0;
            TUINT32 udwAlGiftLv = 0;

            TUINT32 udwTCastleLv = 0;
            if(pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetType == EN_WILD_TYPE__CITY)
            {
                if(pstSession->m_stTargetUser.m_tbPlayer.m_nAge == 3)
                {
                    udwTCastleLv = pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetLevel;
                }
                else
                {
                    udwTCastleLv = 0;
                }
                
            }
            SUserInfo *pstTUser = &pstSession->m_stTargetUser;
            
            if(udwAllianceId && pstSession->m_stTargetUser.m_tbPlayer.m_nAlpos)
            {
                ddwAlid = udwAllianceId;
                ddwAlMight = pstAlliance->m_nMight;
                sAlName = pstAlliance->m_sName;
                sAlNickName = pstAlliance->m_sAl_nick_name;
                udwAlPos = pstPlayer->m_nAlpos;
                udwAlGiftLv = CCommonBase::GetAlGiftLevel(pstAlliance);
            }
            for(TUINT32 udwIdx = 0; udwIdx < EN_SCORE_TYPE__END; ++udwIdx)
            {
                for(TUINT32 tmpIdx = 0; tmpIdx < MAX_SCORE_ID_NUM; ++tmpIdx)
                {
                    if(pstActiveScore->audwScoreList[udwIdx][tmpIdx] == 0)
                    {
                        continue;
                    }

                    EventReqInfo *pstEventReq = new EventReqInfo;
                    TbLogin *ptbLogin = &pstSession->m_stTargetUser.m_tbLogin;

                    pstSession->m_vecEventReq.push_back(pstEventReq);
                    pstEventReq->SetVal(pstPlayer->m_nSid, pstPlayer->m_nUid, pstPlayer->m_sUin, pstPlayer->m_nMight,
                        ddwAlid, sAlName, ddwAlMight, sAlNickName, udwAlPos,
                        ptbLogin->m_nGem_buy, ptbLogin->m_nMax_buy, ptbLogin->m_nLast_buy, ptbLogin->m_nLast_buy_time,
                        ptbLogin->m_nTotal_pay, ptbLogin->m_nMax_pay, ptbLogin->m_nLast_pay,
                        udwAlGiftLv, ptbLogin->m_nCtime, udwTCastleLv, 
                        CCityBase::GetBuildingLevelByFuncType(&pstTUser->m_stCityInfo, EN_BUILDING_TYPE__TRIAL),
                        EN_REQUEST_TYPE__UPDATE, pstSession->m_stTargetUser.m_uddwCurEventId,
                        udwIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx], tmpIdx, 0, 0);
                    //log
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: [score_type=%u][score_id=%u][score=%u][uid=%u][command=%s][req_string=%s][seq=%u]",
                        udwIdx, tmpIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx],
                        pstPlayer->m_nUid, pstSession->m_stReqParam.m_szCommand,
                        pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
                }
            }
            for (TUINT32 udwIdx = 0; udwIdx < MAX_SCORE_LIST_LENGTH; ++udwIdx)
            {
                if (pstActiveScore->sScoreList[udwIdx].ddwUid != 0)
                {
                    EventReqInfo *pstEventReq = new EventReqInfo;
                    SScoreList *pstScoreList = &pstActiveScore->sScoreList[udwIdx];
                    pstSession->m_vecEventReq.push_back(pstEventReq);
                    pstEventReq->SetVal(pstScoreList->udwSid, pstScoreList->ddwUid, pstScoreList->strUname, pstScoreList->uddwAlid,
                        pstScoreList->udwScoreType, pstScoreList->udwScore, pstScoreList->udwScoreId, EN_REQUEST_TYPE__UPDATE);

                    //log
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: [score_type=%u][score_id=%u][score=%u][uid=%u][command=%s][req_string=%s][seq=%u]",
                        pstScoreList->udwScoreType, pstScoreList->udwScoreId, pstScoreList->udwScore,
                        pstScoreList->ddwUid, pstSession->m_stReqParam.m_szCommand,
                        pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
                }
                else
                {
                    break;
                }
            }
        }
        dwRetCode = CTaskProcess::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send event request fail [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        }
        else
        {
            return;
        }
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal:event info rsp [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
    }
    // 14. 处理结束
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__END)
    {
        TSE_LOG_INFO(m_poServLog, ("SidActionProdure: process_end: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        CComputeMap::GetInstance()->FinishAction(0,
            &pstSession->m_stReqMarch, pstSession->m_ucActionRawStatus);

        //wave@push_data
        CAuPushData::AuPushData(pstSession);

        // 记录日志
        pstSession->m_uddwTimeEnd = CTimeUtils::GetCurTimeUs();
        TUINT64 uddwCostTime = 0;
        if(pstSession->m_uddwTimeEnd > pstSession->m_uddwTimeBeg)
        {
            uddwCostTime = pstSession->m_uddwTimeEnd - pstSession->m_uddwTimeBeg;
        }

        TUINT32 udwKey0 = 0, udwKey1 = 0;

        switch(pstSession->m_stReqMarch.m_nMclass)
        {
        case EN_ACTION_MAIN_CLASS__MARCH:
            udwKey0 = pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetType;
            udwKey1 = pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetCityId;
            break;
        default:
            udwKey0 = 0;
            udwKey1 = 0;
            break;
        }

        if (pstSession->m_stReqMarch.m_nSclass != EN_ACTION_SEC_CLASS__NOTI_TIMER)
        {
            if (pstSession->m_stSourceUser.m_tbPlayer.m_nUid)
            {
                m_jUserJson.clear();
                m_jUserJson = Json::Value(objectValue);
                m_oUserJson.GenDataJson(pstSession, &pstSession->m_stSourceUser, m_jUserJson);
                m_sRspJsonContent.clear();
                m_sRspJsonContent = m_jWriter.write(m_jUserJson);
                CJsonLog::OutLogForStat(pstSession, &pstSession->m_stSourceUser, m_sRspJsonContent.c_str());
            }
            if (pstSession->m_stTargetUser.m_tbPlayer.m_nUid)
            {
                m_jUserJson.clear();
                m_jUserJson = Json::Value(objectValue);
                m_oUserJson.GenDataJson(pstSession, &pstSession->m_stTargetUser, m_jUserJson);
                m_sRspJsonContent.clear();
                m_sRspJsonContent = m_jWriter.write(m_jUserJson);
                CJsonLog::OutLogForStat(pstSession, &pstSession->m_stTargetUser, m_sRspJsonContent.c_str());
            }
        }
        
        GenSidAcitonHourLog(pstSession, uddwCostTime, udwKey0, udwKey1, pstSession->m_strGameEvaluateLog);
        GenFormatLog(pstSession, uddwCostTime);

        // 统计相关
        CStatistic *poStatistic = CStatistic::Instance();
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            poStatistic->AddSearchSucc(uddwCostTime);
        }
        else
        {
            poStatistic->AddSearchFail(uddwCostTime);
        }

        TINT64 ddwTroopTotalNumFinal = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, final=%ld [seq=%u]",
            pstSession->m_stReqMarch.m_nId,
            pstSession->m_stReqMarch.m_nMclass,
            pstSession->m_stReqMarch.m_nSclass,
            pstSession->m_stReqMarch.m_nStatus,
            ddwTroopTotalNumFinal,
            pstSession->m_stSourceUser.m_udwBSeqNo));

        if(pstSession->m_bCheckValidFlag == FALSE && pstSession->m_ddwCheckUid > 0)
        {
            CMsgBase::CheckMap(pstSession->m_ddwCheckUid, pstSession->m_udwReqSvrId);
        }

        // release
        CSessionMgr::Instance()->ReleaseSession(pstSession);
    }
}

TVOID CTaskProcess::UidActionProdure(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    TBOOL bNeedResponse = FALSE;

    TSE_LOG_DEBUG(m_poServLog, ("check nextprocedure: step[%u],[code=%u] [nextprocedure=%d], [seq=%u]",
        pstSession->m_udwProcessSeq, pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

    // 0. 如果已经出错，直接返回
    if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_REQUEST)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else if(EN_PROCEDURE__EVENT_PROCESS_UPDATE == pstSession->m_udwNextProcedure)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        }
        else if (pstSession->m_udwNextProcedure != EN_PROCEDURE__END
            && pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__TIMEOUT)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else if(pstSession->m_udwNextProcedure != EN_PROCEDURE__END)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
        }

        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: check_retcode [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        goto PROCESS_REQ_COMMAND_END;
    }

    // 1. 根据命令字做准备工作
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__INIT)
    {
        TINT64 ddwTroopTotalNumInit = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, init=%ld [seq=%u]",
            pstSession->m_stReqAction.m_nId,
            pstSession->m_stReqAction.m_nMclass,
            pstSession->m_stReqAction.m_nSclass,
            pstSession->m_stReqAction.m_nStatus,
            ddwTroopTotalNumInit,
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__LOCK_FOR_FISRT_DATA_GET;

        pstSession->m_poServLog = m_poServLog;
        pstSession->m_stSourceUser.m_udwBSeqNo = pstSession->m_udwSeqNo;
        pstSession->m_stTargetUser.m_udwBSeqNo = pstSession->m_udwSeqNo;
        pstSession->m_uParam = pstSession->m_stReqAction.m_bParam[0];
        pstSession->m_stRawReqAction = pstSession->m_stReqAction;

        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: recv_req: action[%ld],main=%ld,sec=%ld,status=%ld,"
            "suid=%ld,scid=%ld,btime=%ld,etime=%ld [seq=%u]",
            pstSession->m_stReqAction.m_nId,
            pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
            pstSession->m_stReqAction.m_nStatus, pstSession->m_stReqAction.m_nSuid,
            pstSession->m_stReqAction.m_nScid,
            pstSession->m_stReqAction.m_nBtime,
            pstSession->m_stReqAction.m_nEtime, pstSession->m_udwSeqNo));
    }

    //第一步锁数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__LOCK_FOR_FISRT_DATA_GET)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: lock_for_first_data_get: [nextprocedure=%d] [seq=%u]",
            pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_DATA_GET_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDUER__LOCK_GET;

        dwRetCode = FirstLockGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("FirstLockGetRequest: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("FirstLockGetRequest: no data to lock, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        }
        else
        {
            return;
        }
    }

    //第一步数据拉取  map source_user
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: first_data_get_request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = FirstDataGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            TSE_LOG_ERROR(m_poServLog, ("FirstDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            goto PROCESS_REQ_COMMAND_END;
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("FirstDataGetRequest: no data to get, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        }
        else
        {
            return;
        }
    }

    //解析第一步数据拉取拉回的数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_DATA_GET_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: first data get response: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_REQUEST;

        dwRetCode = UserInfoGetResponse(pstSession, &pstSession->m_stSourceUser, TRUE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: source_user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    //联盟信息拉取
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: first alliance data get request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos)
        {
            dwRetCode = AllianceInfoRequest(pstSession, &pstSession->m_stSourceUser, pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                TSE_LOG_ERROR(m_poServLog, ("FirstDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
                goto PROCESS_REQ_COMMAND_END;
            }
            else
            {
                return;
            }
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_HANDLE_BEFORE;
        }
    }

    //联盟信息解析
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_RESPONSE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_HANDLE_BEFORE;

        dwRetCode = AllianceInfoGetResponse(pstSession, &pstSession->m_stSourceUser, TRUE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: source alliance user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

    }

    //handle before
    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_HANDLE_BEFORE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        dwRetCode = CAuCommonBefore::AuCommonHandleBefore(pstSession, &pstSession->m_stSourceUser);
        if (dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("AuCommonHandleBefore: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        //wave@push_data
        CAuPushData::InitRawAlInfo(&pstSession->m_stSourceUser, &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl);
    }

    // 10. 根据命令字进行处理
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__COMMAND_PROCESS)
    {
        TINT64 ddwTroopTotalNumBegin = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_INFO(m_poServLog, ("ProcessUidActionProdure: command_process: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        // update action info
        dwRetCode = UpdateAndCheckActionInfo(pstSession, &pstSession->m_stMapItem, &pstSession->m_stSourceUser, &pstSession->m_stTargetUser);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UpdateAndCheckActionInfo: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        // command process
        pstSession->m_udwProcessSeq++;
        bNeedResponse = FALSE;

        dwRetCode = CProcessAction::ProcessUidAction(pstSession, bNeedResponse);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UidActionProdure: main=%u,sec=%u,status=%u,ret=%d [seq=%u]",
                pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
                pstSession->m_stReqAction.m_nStatus, dwRetCode, pstSession->m_udwSeqNo));
            if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
            }

            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        // 处理Action过程中是否请求下游数据
        if(bNeedResponse == TRUE || (EN_COMMAND_STEP__INIT != pstSession->m_udwCommandStep && EN_COMMAND_STEP__END != pstSession->m_udwCommandStep))
        {
            if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__AWS)
            {
                dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send aws req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
                        pstSession->m_stReqAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__DB)
            {
                dwRetCode = SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send db req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
                        pstSession->m_stReqAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else if (pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__MAP_SVR)
            {
                dwRetCode = SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
                if (dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send map svr req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
                        pstSession->m_stReqAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: expect_procedure_error:%u,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                    pstSession->m_udwExpectProcedure, pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
                    pstSession->m_stReqAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            return;
        }

        TINT64 ddwTroopTotalNumEnd = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, begin=%ld, end=%ld [seq=%u]",
            pstSession->m_stReqAction.m_nId, \
            pstSession->m_stReqAction.m_nMclass, \
            pstSession->m_stReqAction.m_nSclass, \
            pstSession->m_stReqAction.m_nStatus, \
            ddwTroopTotalNumBegin, \
            ddwTroopTotalNumEnd, \
            pstSession->m_stSourceUser.m_udwBSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST;
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: gen_report_id_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));
        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_INSERT_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        if(pstSession->m_dwReportFlag)
        {
            dwRetCode = GenReportIdRequest(pstSession);
            if(dwRetCode < 0)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            return;
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
        }
    }

    // 11. 优先插入report
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORT_INSERT_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: report_insert_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_USER_INSERT_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbReportId);
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: report_insert_req: ret[%d] report_id[%ld] [seq=%u]", dwRetCode, pstSession->m_tbReportId.m_nVal, pstSession->m_udwSeqNo));
        if(dwRetCode <= 0 || pstSession->m_tbReportId.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REPORT_ID_GET_FAIL;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        dwRetCode = InsertReport(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
        else if(dwRetCode == 0)
        {
            return;
        }
    }

    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORT_USER_INSERT_REQUEST)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;

        if (pstSession->m_vecReportReq.size() > 0)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
            dwRetCode = SendReportSvrRequest(pstSession);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            else if (dwRetCode == 0)
            {
                return;
            }
        }
    }

PROCESS_REQ_COMMAND_END:
    if(EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: abnormal_action_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        //异常处理逻辑
        dwRetCode = AbNormalActionHandle(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("send abnormal action handle req failed. ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 0)
        {
            return;
        }
    }

    // 12. 更新用户信息
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: user_info_update_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 如果处理出错，则按照命令字处理
        if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
        {
            TSE_LOG_ERROR(m_poServLog, ("process error: action[%ld],main=%ld;,sec=%ld,status=%ld,suid=%ld,scid=%ld,flag=%u,ret=%d [seq=%u]",
                pstSession->m_stReqAction.m_nId, pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
                pstSession->m_stReqAction.m_nStatus, pstSession->m_stReqAction.m_nSuid, pstSession->m_stReqAction.m_nScid,
                pstSession->m_ucReqActionFlag,
                pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else
        {
            // 进行数据更新
            dwRetCode = ProcessProcedure_UserUpdtRequest(pstSession);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("ProcessProcedure_UserUpdtRequest: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
                pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
            }
            else if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("ProcessProcedure_UserUpdtRequest: all info do not need to update [seq=%u]", pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
    }

    // 13. wavewang@20130511: 对已经加锁的目标进行解锁操作
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("ProcessReqCommand: unlock_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_UPDATE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDUER__LOCK_RELEASE;

        if(pstSession->m_bLockedData)
        {
            dwRetCode = ProcessProcedure_LockReq(pstSession, EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(m_poServLog, ("lock_release ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
    }


    //17 活动信息
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_UPDATE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: event  info  req [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        SActiveScore *pstActiveScore = &pstSession->m_stSourceUser.m_stScore;
        TbPlayer *pstPlayer = &pstSession->m_stSourceUser.m_tbPlayer;

        TbAlliance *pstAlliance = &pstSession->m_stSourceUser.m_tbAlliance;

        TINT64 ddwAlid = 0;
        TINT64 ddwAlMight = 0;
        string sAlName = "";
        string sAlNickName = "";
        TUINT32 udwAlPos = 0;
        TUINT32 udwAlGiftLv = 0;

        SUserInfo *pstSUser = &pstSession->m_stSourceUser;
        //SCityInfo *pstSCity = CCityBase::GetCityInfo(pstSUser->m_astCityInfo, pstSUser->m_udwCityNum, pstSession->m_stReqAction.m_nScid);
		SCityInfo *pstSCity = &pstSUser->m_stCityInfo;

        if(pstPlayer->m_nAlid && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos)
        {
            ddwAlid = pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            ddwAlMight = pstAlliance->m_nMight;
            sAlName = pstSession->m_stSourceUser.m_tbPlayer.m_sAlname;
            sAlNickName = pstAlliance->m_sAl_nick_name;
            udwAlPos = pstPlayer->m_nAlpos;
            udwAlGiftLv = CCommonBase::GetAlGiftLevel(pstAlliance);
        }
        if (pstSUser->m_bIsSendEventReq == TRUE)
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
                    TbLogin *ptbLogin = &pstSession->m_stSourceUser.m_tbLogin;

                    pstSession->m_vecEventReq.push_back(pstEventReq);
                    pstEventReq->SetVal(pstPlayer->m_nSid, pstPlayer->m_nUid, pstPlayer->m_sUin, pstPlayer->m_nMight,
                        ddwAlid, sAlName, ddwAlMight, sAlNickName, udwAlPos,
                        ptbLogin->m_nGem_buy, ptbLogin->m_nMax_buy, ptbLogin->m_nLast_buy, ptbLogin->m_nLast_buy_time,
                        ptbLogin->m_nTotal_pay, ptbLogin->m_nMax_pay, ptbLogin->m_nLast_pay,
                        udwAlGiftLv, ptbLogin->m_nCtime, CCityBase::GetBuildingLevelById(&pstSCity->m_stTblData, 3),
                        CCityBase::GetBuildingLevelByFuncType(pstSCity, EN_BUILDING_TYPE__TRIAL),
                        EN_REQUEST_TYPE__UPDATE, 
                        pstSession->m_stSourceUser.m_uddwCurEventId, udwIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx], tmpIdx, 0, 0);

                    //log
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: [score_type=%u][score_id=%u][score=%u][uid=%u][command=%s][req_string=%s][seq=%u]",
                        udwIdx, tmpIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx],
                        pstPlayer->m_nUid, pstSession->m_stReqParam.m_szCommand,
                        pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
                }
            }
        }
        dwRetCode = CTaskProcess::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send event request fail [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        }
        else
        {
            return;
        }
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal:event info rsp [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
    }
    // 14. 处理结束
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__END)
    {
        TSE_LOG_INFO(m_poServLog, ("ProcessReqCommand: process_end: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        CComputeMap::GetInstance()->FinishAction(0,
            &pstSession->m_stReqAction, pstSession->m_ucActionRawStatus);

        //wave@push_data
        CAuPushData::AuPushData(pstSession);

        // 记录日志
        pstSession->m_uddwTimeEnd = CTimeUtils::GetCurTimeUs();
        TUINT64 uddwCostTime = 0;
        if(pstSession->m_uddwTimeEnd > pstSession->m_uddwTimeBeg)
        {
            uddwCostTime = pstSession->m_uddwTimeEnd - pstSession->m_uddwTimeBeg;
        }

        TUINT32 udwKey0 = 0, udwKey1 = 0;

        if(pstSession->m_stActionTable.dwTableType == EN_UID_ACTION)
        {
            udwKey0 = pstSession->m_udwTrainRawType;
            udwKey1 = pstSession->m_udwTrainRawNum;
        }
        else if(pstSession->m_stActionTable.dwTableType == EN_AID_ACTION)
        {
            if(pstSession->m_stReqAlAction.m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING)
            {
                udwKey0 = pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwType;
                udwKey1 = pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwTargetLevel;
            }
        }
        else if(pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
        {
            if(pstSession->m_stReqMarch.m_nMclass == EN_ACTION_MAIN_CLASS__MARCH)
            {
                udwKey0 = pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetType;
                udwKey1 = pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetCityId;
            }
        }

        if (pstSession->m_stSourceUser.m_tbPlayer.m_nUid)
        {
            m_jUserJson.clear();
            m_jUserJson = Json::Value(objectValue);
            m_oUserJson.GenDataJson(pstSession, &pstSession->m_stSourceUser, m_jUserJson);
            m_sRspJsonContent.clear();
            m_sRspJsonContent = m_jWriter.write(m_jUserJson);
            CJsonLog::OutLogForStat(pstSession, &pstSession->m_stSourceUser, m_sRspJsonContent.c_str());
        }

        GenUidAcitonHourLog(pstSession, uddwCostTime, udwKey0, udwKey1, pstSession->m_strGameEvaluateLog);
        GenFormatLog(pstSession, uddwCostTime);

        // 统计相关
        CStatistic *poStatistic = CStatistic::Instance();
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            poStatistic->AddSearchSucc(uddwCostTime);
        }
        else
        {
            poStatistic->AddSearchFail(uddwCostTime);
        }

        TINT64 ddwTroopTotalNumFinal = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, final=%ld [seq=%u]",
            pstSession->m_stReqAction.m_nId, \
            pstSession->m_stReqAction.m_nMclass, \
            pstSession->m_stReqAction.m_nSclass, \
            pstSession->m_stReqAction.m_nStatus, \
            ddwTroopTotalNumFinal, \
            pstSession->m_stSourceUser.m_udwBSeqNo));

        if(pstSession->m_bCheckValidFlag == FALSE && pstSession->m_ddwCheckUid > 0)
        {
            CMsgBase::CheckMap(pstSession->m_ddwCheckUid, pstSession->m_udwReqSvrId);
        }

        // release
        CSessionMgr::Instance()->ReleaseSession(pstSession);
    }
}

TVOID CTaskProcess::AidActionProdure(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    TBOOL bNeedResponse = FALSE;

    TSE_LOG_DEBUG(m_poServLog, ("check nextprocedure: step[%u],[code=%u] [nextprocedure=%d], [seq=%u]",
        pstSession->m_udwProcessSeq, pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

    // 0. 如果已经出错，直接返回
    if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_REQUEST)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else if(EN_PROCEDURE__EVENT_PROCESS_UPDATE == pstSession->m_udwNextProcedure)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        }
        else if (pstSession->m_udwNextProcedure != EN_PROCEDURE__END
            && pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__TIMEOUT)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else if(pstSession->m_udwNextProcedure != EN_PROCEDURE__END)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
        }

        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: check_retcode [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        goto PROCESS_REQ_COMMAND_END;
    }

    // 1. 根据命令字做准备工作
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__INIT)
    {
        TINT64 ddwTroopTotalNumInit = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, init=%ld [seq=%u]",
            pstSession->m_stReqAlAction.m_nId,
            pstSession->m_stReqAlAction.m_nMclass,
            pstSession->m_stReqAlAction.m_nSclass,
            pstSession->m_stReqAlAction.m_nStatus,
            ddwTroopTotalNumInit,
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__LOCK_FOR_FISRT_DATA_GET;

        pstSession->m_poServLog = m_poServLog;
        pstSession->m_stSourceUser.m_udwBSeqNo = pstSession->m_udwSeqNo;
        pstSession->m_stTargetUser.m_udwBSeqNo = pstSession->m_udwSeqNo;
        pstSession->m_uParam = pstSession->m_stReqAlAction.m_bParam[0];
        pstSession->m_stRawReqAlAction = pstSession->m_stReqAlAction;

        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: recv_req: action[%ld],main=%ld,sec=%ld,status=%ld,"
            "suid=%ld,scid=%ld,btime=%ld,etime=%ld [seq=%u]",
            pstSession->m_stReqAlAction.m_nId,
            pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
            pstSession->m_stReqAlAction.m_nStatus, pstSession->m_stReqAlAction.m_nSuid,
            pstSession->m_stReqAlAction.m_nScid,
            pstSession->m_stReqAlAction.m_nBtime,
            pstSession->m_stReqAlAction.m_nEtime, pstSession->m_udwSeqNo));
    }

    //第一步锁数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__LOCK_FOR_FISRT_DATA_GET)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: lock_for_first_data_get: [nextprocedure=%d] [seq=%u]",
            pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_DATA_GET_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDUER__LOCK_GET;

        dwRetCode = FirstLockGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("FirstLockGetRequest: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("FirstLockGetRequest: no data to lock, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        }
        else
        {
            return;
        }
    }

    //第一步数据拉取  map source_user
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: first_data_get_request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = FirstDataGetRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            TSE_LOG_ERROR(m_poServLog, ("FirstDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            goto PROCESS_REQ_COMMAND_END;
        }
        else if(dwRetCode == 1)
        {
            TSE_LOG_INFO(m_poServLog, ("FirstDataGetRequest: no data to get, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        }
        else
        {
            return;
        }
    }

    //解析第一步数据拉取拉回的数据
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_DATA_GET_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: first data get response: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_REQUEST;

        dwRetCode = UserInfoGetResponse(pstSession, &pstSession->m_stSourceUser, TRUE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: source_user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
    }

    //联盟信息拉取
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: first alliance data get request: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos)
        {
            dwRetCode = AllianceInfoRequest(pstSession, &pstSession->m_stSourceUser, pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                TSE_LOG_ERROR(m_poServLog, ("FirstDataGetRequest: send req failed, ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
                goto PROCESS_REQ_COMMAND_END;
            }
            else
            {
                return;
            }
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_HANDLE_BEFORE;
        }
    }

    //联盟信息解析
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_RESPONSE)
    {

        pstSession->m_udwNextProcedure = EN_PROCEDURE__FIRST_HANDLE_BEFORE;

        dwRetCode = AllianceInfoGetResponse(pstSession, &pstSession->m_stSourceUser, TRUE);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UserInfoGetResponse: source alliance user ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

    }

    //handle before
    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__FIRST_HANDLE_BEFORE)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMAND_PROCESS;
        dwRetCode = CAuCommonBefore::AuCommonHandleBefore(pstSession, &pstSession->m_stSourceUser);
        if (dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("AuCommonHandleBefore: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        //wave@push_data
        CAuPushData::InitRawAlInfo(&pstSession->m_stSourceUser, &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl);
    }

    // 10. 根据命令字进行处理
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__COMMAND_PROCESS)
    {
        TINT64 ddwTroopTotalNumBegin = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: command_process: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        // update action info
        dwRetCode = UpdateAndCheckActionInfo(pstSession, &pstSession->m_stMapItem, &pstSession->m_stSourceUser, &pstSession->m_stTargetUser);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("UpdateAndCheckActionInfo: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        // command process
        pstSession->m_udwProcessSeq++;
        bNeedResponse = FALSE;


        dwRetCode = CProcessAction::ProcessAidAction(pstSession, bNeedResponse);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("AidActionProdure: main=%u,sec=%u,status=%u,ret=%d [seq=%u]",
                pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
                pstSession->m_stReqAlAction.m_nStatus, dwRetCode, pstSession->m_udwSeqNo));
            if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
            }

            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        // 处理Action过程中是否请求下游数据
        if(bNeedResponse == TRUE)
        {
            if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__AWS)
            {
                dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send aws req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
                        pstSession->m_stReqAlAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else if(pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__DB)
            {
                dwRetCode = SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send db req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
                        pstSession->m_stReqAlAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else if (pstSession->m_udwExpectProcedure == EN_EXPECT_PROCEDURE__MAP_SVR)
            {
                dwRetCode = SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
                if (dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: send map svr req failed,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                        pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
                        pstSession->m_stReqAlAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                    goto PROCESS_REQ_COMMAND_END;
                }
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("UidActionProdure: expect_procedure_error:%u,main=%u,sec=%u,status=%u,ret=%d,command_step=%u [seq=%u]",
                    pstSession->m_udwExpectProcedure, pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
                    pstSession->m_stReqAlAction.m_nStatus, dwRetCode, pstSession->m_udwCommandStep, pstSession->m_udwSeqNo));
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            return;
        }

        TINT64 ddwTroopTotalNumEnd = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, begin=%ld, end=%ld [seq=%u]",
            pstSession->m_stReqAlAction.m_nId, \
            pstSession->m_stReqAlAction.m_nMclass, \
            pstSession->m_stReqAlAction.m_nSclass, \
            pstSession->m_stReqAlAction.m_nStatus, \
            ddwTroopTotalNumBegin, \
            ddwTroopTotalNumEnd, \
            pstSession->m_stSourceUser.m_udwBSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST;
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: gen_report_id_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));
        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_INSERT_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        if(pstSession->m_dwReportFlag)
        {
            dwRetCode = GenReportIdRequest(pstSession);
            if(dwRetCode < 0)
            {
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            return;
        }
        else
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
        }
    }

    // 11. 优先插入report
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORT_INSERT_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: report_insert_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__REPORT_USER_INSERT_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbReportId);
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: report_insert_req: ret[%d] report_id[%ld] [seq=%u]", dwRetCode, pstSession->m_tbReportId.m_nVal, pstSession->m_udwSeqNo));
        if(dwRetCode <= 0 || pstSession->m_tbReportId.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REPORT_ID_GET_FAIL;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }

        dwRetCode = InsertReport(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
            goto PROCESS_REQ_COMMAND_END;
        }
        else if(dwRetCode == 0)
        {
            return;
        }
    }

    if (pstSession->m_udwNextProcedure == EN_PROCEDURE__REPORT_USER_INSERT_REQUEST)
    {
        pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;

        if (pstSession->m_vecReportReq.size() > 0)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
            dwRetCode = SendReportSvrRequest(pstSession);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                pstSession->m_udwNextProcedure = EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST;
                goto PROCESS_REQ_COMMAND_END;
            }
            else if (dwRetCode == 0)
            {
                return;
            }
        }
    }

PROCESS_REQ_COMMAND_END:
    if(EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST == pstSession->m_udwNextProcedure)
    {
        TSE_LOG_INFO(m_poServLog, ("UidActionProdure: abnormal_action_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        //异常处理逻辑
        dwRetCode = AbNormalActionHandle(pstSession);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("send abnormal action handle req failed. ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        }
        else if(dwRetCode == 0)
        {
            return;
        }
    }

    // 12. 更新用户信息
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__USER_INFO_UPDATE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: user_info_update_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 如果处理出错，则按照命令字处理
        if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
        {
            TSE_LOG_ERROR(m_poServLog, ("process error: action[%ld],main=%ld;,sec=%ld,status=%ld,suid=%ld,scid=%ld,flag=%u,ret=%d [seq=%u]",
                pstSession->m_stReqAlAction.m_nId, pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
                pstSession->m_stReqAlAction.m_nStatus, pstSession->m_stReqAlAction.m_nSuid, pstSession->m_stReqAlAction.m_nScid,
                pstSession->m_ucReqActionFlag,
                pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwSeqNo));
            pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        }
        else
        {
            // 进行数据更新
            dwRetCode = ProcessProcedure_UserUpdtRequest(pstSession);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(m_poServLog, ("ProcessProcedure_UserUpdtRequest: ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
                pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
            }
            else if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("ProcessProcedure_UserUpdtRequest: all info do not need to update [seq=%u]", pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
    }

    // 13. wavewang@20130511: 对已经加锁的目标进行解锁操作
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: unlock_req: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_UPDATE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDUER__LOCK_RELEASE;

        if(pstSession->m_bLockedData)
        {
            dwRetCode = ProcessProcedure_LockReq(pstSession, EN_SERVICE_TYPE_HU2LOCK__RELEASE_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(m_poServLog, ("lock_release ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            }
            else
            {
                return;
            }
        }
    }

    //17 活动信息
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_UPDATE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal: event  info  req [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__EVENT_PROCESS_RESPONSE;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        SActiveScore *pstActiveScore = &pstSession->m_stSourceUser.m_stScore;
        TbPlayer *pstPlayer = &pstSession->m_stSourceUser.m_tbPlayer;

        TbAlliance *pstAlliance = &pstSession->m_stSourceUser.m_tbAlliance;

        TINT64 ddwAlid = 0;
        TINT64 ddwAlMight = 0;
        string sAlName = "";
        string sAlNickName = "";
        TUINT32 udwAlPos = 0;
        TUINT32 udwAlGiftLv = 0;

        SUserInfo *pstSUser = &pstSession->m_stSourceUser;
		SCityInfo *pstSCity = &pstSUser->m_stCityInfo;

        if(pstPlayer->m_nAlid && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos)
        {
            ddwAlid = pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            ddwAlMight = pstAlliance->m_nMight;
            sAlName = pstSession->m_stSourceUser.m_tbPlayer.m_sAlname;
            sAlNickName = pstAlliance->m_sAl_nick_name;
            udwAlPos = pstPlayer->m_nAlpos;
            udwAlGiftLv = CCommonBase::GetAlGiftLevel(pstAlliance);
        }
        if (pstSUser->m_bIsSendEventReq == TRUE)
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
                    TbLogin *ptbLogin = &pstSession->m_stSourceUser.m_tbLogin;

                    pstSession->m_vecEventReq.push_back(pstEventReq);
                    pstEventReq->SetVal(pstPlayer->m_nSid, pstPlayer->m_nUid, pstPlayer->m_sUin, pstPlayer->m_nMight,
                        ddwAlid, sAlName, ddwAlMight, sAlNickName, udwAlPos,
                        ptbLogin->m_nGem_buy, ptbLogin->m_nMax_buy, ptbLogin->m_nLast_buy, ptbLogin->m_nLast_buy_time,
                        ptbLogin->m_nTotal_pay, ptbLogin->m_nMax_pay, ptbLogin->m_nLast_pay,
                        udwAlGiftLv, ptbLogin->m_nCtime, CCityBase::GetBuildingLevelById(&pstSCity->m_stTblData, 3),
                        CCityBase::GetBuildingLevelByFuncType(pstSCity, EN_BUILDING_TYPE__TRIAL),
                        EN_REQUEST_TYPE__UPDATE, 
                        pstSession->m_stSourceUser.m_uddwCurEventId, udwIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx], tmpIdx, 0, 0);

                    //log
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: [score_type=%u][score_id=%u][score=%u][uid=%u][command=%s][req_string=%s][seq=%u]",
                        udwIdx, tmpIdx, pstActiveScore->audwScoreList[udwIdx][tmpIdx],
                        pstPlayer->m_nUid, pstSession->m_stReqParam.m_szCommand,
                        pstEventReq->m_sReqContent.c_str(), pstSession->m_udwSeqNo));
                }
            }
        }
        
        dwRetCode = CTaskProcess::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(m_poServLog, ("Main_flow:normal: send event request fail [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));

            pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
        }
        else
        {
            return;
        }
    }

    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__EVENT_PROCESS_RESPONSE)
    {
        TSE_LOG_INFO(m_poServLog, ("Main_flow:normal:event info rsp [seq=%u]",
            pstSession->m_udwSeqNo));

        pstSession->m_udwNextProcedure = EN_PROCEDURE__END;
    }

    // 14. 处理结束
    if(pstSession->m_udwNextProcedure == EN_PROCEDURE__END)
    {
        TSE_LOG_INFO(m_poServLog, ("AidActionProdure: process_end: [nextprocedure=%d] [seq=%u]", pstSession->m_udwNextProcedure, pstSession->m_udwSeqNo));

        CComputeMap::GetInstance()->FinishAction(0,
            &pstSession->m_stReqAlAction, pstSession->m_ucActionRawStatus);

        //wave@push_data
        CAuPushData::AuPushData(pstSession);

        // 记录日志
        pstSession->m_uddwTimeEnd = CTimeUtils::GetCurTimeUs();
        TUINT64 uddwCostTime = 0;
        if(pstSession->m_uddwTimeEnd > pstSession->m_uddwTimeBeg)
        {
            uddwCostTime = pstSession->m_uddwTimeEnd - pstSession->m_uddwTimeBeg;
        }

        TUINT32 udwKey0 = 0, udwKey1 = 0;

        if(pstSession->m_stActionTable.dwTableType == EN_UID_ACTION)
        {
            udwKey0 = pstSession->m_udwTrainRawType;
            udwKey1 = pstSession->m_udwTrainRawNum;
        }
        else if(pstSession->m_stActionTable.dwTableType == EN_AID_ACTION)
        {
            if(pstSession->m_stReqAlAction.m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING)
            {
                udwKey0 = pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwType;
                udwKey1 = pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwTargetLevel;
            }
        }
        else if(pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
        {
            if(pstSession->m_stReqMarch.m_nMclass == EN_ACTION_MAIN_CLASS__MARCH)
            {
                udwKey0 = pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetType;
                udwKey1 = pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetCityId;
            }
        }


        if (pstSession->m_stSourceUser.m_tbPlayer.m_nUid)
        {
            m_jUserJson.clear();
            m_jUserJson = Json::Value(objectValue);
            m_oUserJson.GenDataJson(pstSession, &pstSession->m_stSourceUser, m_jUserJson);
            m_sRspJsonContent.clear();
            m_sRspJsonContent = m_jWriter.write(m_jUserJson);
            CJsonLog::OutLogForStat(pstSession, &pstSession->m_stSourceUser, m_sRspJsonContent.c_str());
        }

        GenAidAcitonHourLog(pstSession, uddwCostTime, udwKey0, udwKey1, pstSession->m_strGameEvaluateLog);
        GenFormatLog(pstSession, uddwCostTime);

        // 统计相关
        CStatistic *poStatistic = CStatistic::Instance();
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            poStatistic->AddSearchSucc(uddwCostTime);
        }
        else
        {
            poStatistic->AddSearchFail(uddwCostTime);
        }

        TINT64 ddwTroopTotalNumFinal = CGameInfo::GetInstance()->GetTroopTypeNum();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("game_json_troop_num: actionid=%ld, mainclass=%ld, secclass=%ld, status=%ld, final=%ld [seq=%u]",
            pstSession->m_stReqAlAction.m_nId, \
            pstSession->m_stReqAlAction.m_nMclass, \
            pstSession->m_stReqAlAction.m_nSclass, \
            pstSession->m_stReqAlAction.m_nStatus, \
            ddwTroopTotalNumFinal, \
            pstSession->m_stSourceUser.m_udwBSeqNo));

        if(pstSession->m_bCheckValidFlag == FALSE && pstSession->m_ddwCheckUid > 0)
        {
            CMsgBase::CheckMap(pstSession->m_ddwCheckUid, pstSession->m_udwReqSvrId);
        }

        // release
        CSessionMgr::Instance()->ReleaseSession(pstSession);
    }
}

TINT32 CTaskProcess::CheckReqAction(SSession* pstSession, SUserInfo* pstUserInfo)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TBOOL bReqActionValid = FALSE;
    TUINT32 udwIndex = 0;
    switch(pstSession->m_stActionTable.dwTableType)
    {
    case EN_UID_ACTION:
        for(udwIndex = 0; udwIndex < pstUserInfo->m_udwActionNum; ++udwIndex)
        {
            if(pstUserInfo->m_atbAction[udwIndex].m_nId == pstSession->m_stReqAction.m_nId)
            {
                bReqActionValid = TRUE;
                if(pstUserInfo->m_atbAction[udwIndex].m_nEtime > udwCurTime)
                {
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIME_NOT_RIGHT;
                    return -1;
                }
                pstSession->m_stReqAction = pstUserInfo->m_atbAction[udwIndex];
                pstSession->m_uParam = pstSession->m_stReqAction.m_bParam[0];
                pstSession->m_stRawReqAction = pstSession->m_stReqAction;
                break;
            }
        }
        break;
    case EN_AID_ACTION:
        for(udwIndex = 0; udwIndex < pstUserInfo->m_udwSelfAlActionNum; ++udwIndex)
        {
            if(pstUserInfo->m_atbSelfAlAction[udwIndex].m_nId == pstSession->m_stReqAlAction.m_nId)
            {
                bReqActionValid = TRUE;
                if(pstUserInfo->m_atbSelfAlAction[udwIndex].m_nEtime > udwCurTime)
                {
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIME_NOT_RIGHT;
                    return -2;
                }
                pstSession->m_stReqAlAction = pstUserInfo->m_atbSelfAlAction[udwIndex];
                //pstSession->m_uParam = pstSession->m_stReqAction.m_bParam[0];
                pstSession->m_stRawReqAlAction = pstSession->m_stReqAlAction;
                break;
            }
        }
        break;
    case EN_MARCH_ACTION:
        for(udwIndex = 0; udwIndex < pstUserInfo->m_udwMarchNum; ++udwIndex)
        {
            if(pstUserInfo->m_atbMarch[udwIndex].m_nId == pstSession->m_stReqMarch.m_nId)
            {
                bReqActionValid = TRUE;
                if(pstUserInfo->m_atbMarch[udwIndex].m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER)
                {
                    //do nothing
                }
                else if(pstUserInfo->m_atbMarch[udwIndex].m_nEtime > udwCurTime)
                {
                    pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIME_NOT_RIGHT;
                    return -3;
                }
                pstSession->m_stReqMarch = pstUserInfo->m_atbMarch[udwIndex];
                //pstSession->m_uParam = pstSession->m_stReqAction.m_bParam[0];
                pstSession->m_stRawReqMarch = pstSession->m_stReqMarch;
                break;
            }
        }
        break;
    default:
        break;
    }

    if(bReqActionValid == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        pstSession->m_udwNextProcedure = EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST;
        TSE_LOG_ERROR(m_poServLog, ("CheckReqAction: req action[%ld] not exist in user data [seq=%u]",
            pstSession->m_stReqAction.m_nId, pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CTaskProcess::SendDataCenterRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<DataCenterReqInfo*>& vecReq = pstSession->m_vecDataCenterReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: not data_center req [seq=%u]", \
                                                pstSession->m_udwSeqNo));
        return -1;
    }


    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bDataCenterProxyExist == FALSE)
    {
        pstSession->m_pstDataCenterProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__DATA_CENTER_PROXY, &pstSession->m_pstDataCenterProxyNode))
        {
            pstSession->m_bDataCenterProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: Get DataCenterProxy node succ [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: Get DataCenterProxy node fail [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for (TUINT32 udwIdx = 0; udwIdx < vecReq.size(); ++udwIdx)
    {
        DataCenterReqInfo *pstDataCenterReq = vecReq[udwIdx];
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        TUINT32 udwHandleSeq = CGlobalServ::GenerateHsReqSeq();
        pPack->SetSeq(udwHandleSeq);
        pPack->SetKey(EN_DATA_CENTER__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_DATA_CENTER__REQ_TYPE, pstDataCenterReq->m_udwType);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: send data: [handleseq=%u][seq=%u][type=%u] [seq=%u]", \
                                                udwHandleSeq, \
                                                pstSession->m_udwSeqNo, \
                                                pstDataCenterReq->m_udwType, \
                                                pstSession->m_udwSeqNo));
        pPack->SetKey(EN_DATA_CENTER__REQ_JSON, (unsigned char*)pstDataCenterReq->m_sReqContent.c_str(), pstDataCenterReq->m_sReqContent.size());
        ///打包请求到下游
        pPack->GetPackage(&pszPack, &udwPackLen);

        if (NULL == pstSession->m_pstDataCenterProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                                                    udwIdx, \
                                                    pstSession->m_pstDataCenterProxyNode->m_stDownHandle.handle, \
                                                    pstSession->m_udwSeqNo));
            return -3;
        }

        stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstDataCenterProxyNode->m_stDownHandle);
        stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetDataCenterReq();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: send data failed! [seq=%u]", \
                                                pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;

}

TINT32 CTaskProcess::SendAwsRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;

    if(vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: m_vecAwsReq.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bAwsProxyNodeExist == FALSE)
    {
        pstSession->m_pstAwsProxyNode = NULL;
        if(S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__AWS_PROXY, &pstSession->m_pstAwsProxyNode))
        {
            pstSession->m_bAwsProxyNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get AwsProx node succ [handle=%p] [seq=%u]", pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get AwsProxy node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
        if (NULL == pstSession->m_pstAwsProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest:handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -4;
        }
    }

    if (pstSession->m_bMapSvrExist == FALSE)//wave@20160530
    {
        pstSession->m_pstMapSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__MAP_SVR_PROXY, &pstSession->m_pstMapSvrNode))
        {
            pstSession->m_bMapSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get MapSvr node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get MapSvr node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstMapSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest:handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstMapSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    if (pstSession->m_bBuffActionSvrNodeExist == FALSE)
    {
        pstSession->m_pstBuffActionSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__BUFF_ACTION_SVR, &pstSession->m_pstBuffActionSvrNode))
        {
            pstSession->m_bBuffActionSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get buff action node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get buff action node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstBuffActionSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest:buff action handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstBuffActionSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    if (pstSession->m_bAlActionSvrNodeExist == FALSE)
    {
        pstSession->m_pstAlActionSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__AL_ACTION_SVR, &pstSession->m_pstAlActionSvrNode))
        {
            pstSession->m_bAlActionSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get al action node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get al action node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstAlActionSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: al action handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstAlActionSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    if (pstSession->m_bMarchActionSvrNodeExist == FALSE)
    {
        pstSession->m_pstMarchActionSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__MARCH_ACTION_SVR, &pstSession->m_pstMarchActionSvrNode))
        {
            pstSession->m_bMarchActionSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get march action node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get march action node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstMarchActionSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: march action handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstMarchActionSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

//     if (pstSession->m_bReportUserSvrNodeExist == FALSE)
//     {
//         pstSession->m_pstReportUserSvrNode = NULL;
//         if (S_OK == pobjDownMgr->zk_GetGroupNode(pobjDownMgr->m_pReportUserService, &pstSession->m_pstReportUserSvrNode, 0))
//         {
//             pstSession->m_bReportUserSvrNodeExist = TRUE;
//             TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get report user node succ [seq=%u]", pstSession->m_udwSeqNo));
//         }
//         else
//         {
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("Get report user node fail [seq=%u]", pstSession->m_udwSeqNo));
//             return -21;
//         }
//         if (NULL == pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle)
//         {
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: report user handle [handlevalue=0x%p] [seq=%u]",
//                 pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
//             return -3;
//         }
//     }

    // 2. 封装请求、打包、设置task
    for(unsigned int i = 0; i < vecReq.size(); ++i)
    {
        assert(i < MAX_LOCAL_HS_TASK_NUM);

        if(vecReq[i]->sOperatorName == "UpdateItem"
            || vecReq[i]->sOperatorName == "DeleteItem"
            || vecReq[i]->sOperatorName == "PutItem")
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 2) ? 2 : 3;
        }
        else
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 1) ? 1 : 3;
        }

        CBaseProtocolPack* pPack = m_pPackTool[i];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_GLOBAL_KEY__REQ_TYPE, pstSession->m_udwContentType);
        pPack->SetKey(EN_GLOBAL_KEY__INDEX_NO, vecReq[i]->udwIdxNo);
        pPack->SetKey(EN_GLOBAL_KEY__TABLE_NAME, (unsigned char*)vecReq[i]->sTableName.c_str(), vecReq[i]->sTableName.size());
        pPack->SetKey(EN_GLOBAL_KEY__OPERATOR_NAME, (unsigned char*)vecReq[i]->sOperatorName.c_str(), vecReq[i]->sOperatorName.size());
        pPack->SetKey(EN_KEY_HU2HS__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_CACHE_MODE, vecReq[i]->m_udwCacheMode);
        pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_ROUTE_FLD, (unsigned char*)vecReq[i]->m_strRouteFld.c_str(), vecReq[i]->m_strRouteFld.size());
        pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_ROUTE_THRD, vecReq[i]->m_uddwRouteThrd);
        pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_HASH_KEY, (unsigned char*)vecReq[i]->m_strHashKey.c_str(),vecReq[i]->m_strHashKey.size());	
		if( !vecReq[i]->m_strRangeKey.empty() )
			pPack->SetKey(EN_GOLBAL_KEY__AWSTBL_RANGE_KEY, (unsigned char*)vecReq[i]->m_strRangeKey.c_str(),vecReq[i]->m_strRangeKey.size());
        if (!vecReq[i]->bIsDefaultLock)
        {
            pPack->SetKey(EN_MAP_SVR_KEY__NEED_LOCK, vecReq[i]->dwLock);
            pPack->SetKey(EN_MAP_SVR_KEY__NEED_UNLOCK, vecReq[i]->dwUnlock);
        }
        
        if (vecReq[i]->sTableName.find("_split_map_") != string::npos)
        {
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstMapSvrNode->m_stDownHandle);
        }
        else if (vecReq[i]->sTableName.find("_global_action") != string::npos)
        {
            pPack->SetKey(EN_ACTION_SVR_KEY__SVR_ID, pstSession->m_udwReqSvrId);
            pPack->SetKey(EN_ACTION_SVR_KEY__ACTION_TYPE, EN_UID_ACTION);
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstBuffActionSvrNode->m_stDownHandle);
        }
        else if (vecReq[i]->sTableName.find("_global_alliance_action") != string::npos)
        {
            pPack->SetKey(EN_ACTION_SVR_KEY__SVR_ID, pstSession->m_udwReqSvrId);
            pPack->SetKey(EN_ACTION_SVR_KEY__ACTION_TYPE, EN_AID_ACTION);
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstAlActionSvrNode->m_stDownHandle);
        }
        else if (vecReq[i]->sTableName.find("_global_march_action") != string::npos)
        {
            pPack->SetKey(EN_ACTION_SVR_KEY__SVR_ID, pstSession->m_udwReqSvrId);
            pPack->SetKey(EN_ACTION_SVR_KEY__ACTION_TYPE, EN_MARCH_ACTION);
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstMarchActionSvrNode->m_stDownHandle);
        }
        else
        {
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstAwsProxyNode->m_stDownHandle);
        }

        pPack->GetPackage(&pszPack, &udwPackLen);

        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetAwsInfo();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    if(!m_poSearchLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendGlobalHsRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }

    return 0;
}

TINT32 CTaskProcess::SendDbRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<DbReqInfo*>& vecReq = pstSession->m_vecDbReq;

    if(vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDbRequest: m_vecDbReq.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bDbProxyNodeExist == FALSE)
    {
        pstSession->m_pstDbProxyNode = NULL;
        if(S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__DB_PROXY, &pstSession->m_pstDbProxyNode))
        {
            pstSession->m_bDbProxyNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get DbProx node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get DbProx node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for(unsigned int i = 0; i < vecReq.size(); ++i)
    {
        if(vecReq[i]->sOperatorName == "replace"
            || vecReq[i]->sOperatorName == "delete"
            || vecReq[i]->sOperatorName == "set"
            || vecReq[i]->sOperatorName == "insert"
            || vecReq[i]->sOperatorName == "update")
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 2) ? 2 : 3;
        }
        else
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 1) ? 1 : 3;
        }

        CBaseProtocolPack* pPack = m_pPackTool[i];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_GLOBAL_KEY__DB_IDX, vecReq[i]->udwDbIdx);
        pPack->SetKey(EN_GLOBAL_KEY__INDEX_NO, vecReq[i]->udwIdxNo);
        pPack->SetKey(EN_GLOBAL_KEY__TABLE_NAME, (unsigned char*)vecReq[i]->sTableName.c_str(), vecReq[i]->sTableName.size());
        pPack->SetKey(EN_GLOBAL_KEY__OPERATOR_NAME, (unsigned char*)vecReq[i]->sOperatorName.c_str(), vecReq[i]->sOperatorName.size());
        pPack->SetKey(EN_KEY_HU2HS__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        // c. 设置task
        stTasksGroup.m_UserData1.ptr = pstSession;

        if(NULL == pstSession->m_pstDbProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDbRequest:handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                i, pstSession->m_pstDbProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -4;
        }

        stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstDbProxyNode->m_stDownHandle);
        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }
    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetDbInfo();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    if(!m_poSearchLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendGlobalHsRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }

    return 0;
}

TINT32 CTaskProcess::SendEventRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<EventReqInfo*>& vecReq = pstSession->m_vecEventReq;
    TUINT32 udwLength = vecReq.size();

    if (udwLength == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: not event req [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bEventProxyExist == FALSE)
    {
        pstSession->m_pstEventProxyNode = NULL;
        if(S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__EVENT_PROXY, &pstSession->m_pstEventProxyNode))
        {
            pstSession->m_bEventProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get EventProx node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get EventProxy node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    if (pstSession->m_bThemeEventProxyExist == FALSE)
    {
        pstSession->m_pstThemeEventProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__THEME_EVENT_PROXY, &pstSession->m_pstThemeEventProxyNode))
        {
            pstSession->m_bThemeEventProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CBaseProcedure::SendEventRequest Get ThemeEventProx node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CBaseProcedure::SendEventRequest  Get ThemeEventProxy node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for (TUINT32 udwIdx = 0; udwIdx < udwLength; ++udwIdx)
    {
        EventReqInfo *pstEventReq = vecReq[udwIdx];

        pstSession->m_udwEventRqstType = pstEventReq->m_udwRequestType;

        CBaseProtocolPack* pPack = m_pPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_KEY_EVENT_PROXY__REQ_TYPE, pstSession->m_udwEventRqstType);
        pPack->SetKey(EN_KEY_EVENT_PROXY__REQ_JSON, (unsigned char*)pstEventReq->m_sReqContent.c_str(), pstEventReq->m_sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        if (NULL == pstSession->m_pstEventProxyNode->m_stDownHandle.handle
            || NULL == pstSession->m_pstThemeEventProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendEventRequest:handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                udwIdx, pstSession->m_pstEventProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }

        if (vecReq[udwIdx]->m_udwRequestType != EN_REQUEST_TYPE__UPDATE)
        {

            if (pstEventReq->m_udwIsTheme == 0)
            {
                stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstEventProxyNode->m_stDownHandle);
            }
            else
            {
                stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstThemeEventProxyNode->m_stDownHandle);
            }
            stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
            stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
        }
        else
        {
            stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstEventProxyNode->m_stDownHandle);
            stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
            stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
            stTasksGroup.m_Tasks[udwLength + udwIdx].SetConnSession(pstSession->m_pstThemeEventProxyNode->m_stDownHandle);
            stTasksGroup.m_Tasks[udwLength + udwIdx].SetSendData(pszPack, udwPackLen);
            stTasksGroup.m_Tasks[udwLength + udwIdx].SetNeedResponse(1);
        }

    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    if (vecReq[0]->m_udwRequestType != EN_REQUEST_TYPE__UPDATE)
    {
        stTasksGroup.SetValidTasks(vecReq.size());
    }
    else
    {
        stTasksGroup.SetValidTasks(vecReq.size() * 2);
    }
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetEventInfo();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    if(!m_poSearchLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendGlobalHsRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }

    return 0;
}

TINT32 CTaskProcess::SendMapSvrRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<MapSvrReqInfo*>& vecReq = pstSession->m_vecMapSvrReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest: m_vecMapSvrReq.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bMapSvrExist == FALSE)
    {
        pstSession->m_pstMapSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__MAP_SVR_PROXY, &pstSession->m_pstMapSvrNode))
        {
            pstSession->m_bMapSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendMapSvrRequest: Get MapSvr node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest: Get MapSvr node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstMapSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest:handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstMapSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    // 2. 封装请求、打包、设置task
    for (unsigned int i = 0; i < vecReq.size() && i < MAX_AWS_REQ_TASK_NUM; ++i)
    {
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[i];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_GLOBAL_KEY__REQ_TYPE, pstSession->m_udwContentType);
        pPack->SetKey(EN_MAP_SVR_KEY__SVR_ID, vecReq[i]->dwSid);
        pPack->SetKey(EN_MAP_SVR_KEY__OPERATE, (unsigned char*)vecReq[i]->sOperator.c_str(), vecReq[i]->sOperator.size());
        pPack->SetKey(EN_MAP_SVR_KEY__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstMapSvrNode->m_stDownHandle);

        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendMapSvrRequest: [vaildtasksnum=%u] [seq=%u]",
        vecReq.size(), pstSession->m_udwSeqNo));

    pstSession->ResetAwsInfo();

    // 3. 发送数据    
    pstSession->m_bProcessing = FALSE;
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CTaskProcess::SendReportSvrRequest(SSession *pstSession)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<ReportReqInfo*>& vecReq = pstSession->m_vecReportReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest: req.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bReportUserSvrNodeExist == FALSE)
    {
        pstSession->m_pstReportUserSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__REPORT_USER_SVR, &pstSession->m_pstReportUserSvrNode))
        {
            pstSession->m_bReportUserSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendReportSvrRequest: Get report node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest: Get report node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest:handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }


    // 2. 封装请求、打包、设置task
    for (unsigned int i = 0; i < vecReq.size() && i < MAX_AWS_REQ_TASK_NUM; ++i)
    {
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[i];
        pPack->ResetContent();
        if (vecReq[i]->udwReqType == 0)
        {
            pPack->SetServiceType(EN_SERVICE_TYPE_REPORT_CENTER_REQ);
        }
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_KEY_HU2HS__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        if (vecReq[i]->udwReqType == 0)
        {
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstReportUserSvrNode->m_stDownHandle);
        }

        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendReportSvrRequest: [vaildtasksnum=%u] [seq=%u]",
        vecReq.size(), pstSession->m_udwSeqNo));

    pstSession->ResetReportInfo();

    // 3. 发送数据    
    pstSession->m_bProcessing = FALSE;
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CTaskProcess::ProcessProcedure_LockReq(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUINT32 idx = 0;
    LTasksGroup	stTasksGroup;

    TUINT32 udwKeyNum = 0;
    TUINT64 auddwKeyList[MAX_LOCK_KEY_NUM_IN_ONE_REQ];
    memset(auddwKeyList, 0, sizeof(auddwKeyList));

    // 0. 选取svrid和uid
    TUINT32 udwSvrId = pstSession->m_udwReqSvrId;
    if(pstSession->m_udwLockUid == pstSession->m_udwLockUidB)
    {
        pstSession->m_udwLockUidB = 0;
        pstSession->m_bIsUidBNeedLock = FALSE;
    }
    if(pstSession->m_udwLockAid == pstSession->m_udwLockAidB)
    {
        pstSession->m_udwLockAidB = 0;
        pstSession->m_bIsAidBNeedLock = FALSE;
    }

    if(uwReqServiceType == EN_SERVICE_TYPE_HU2LOCK__GET_REQ)
    {
        if(pstSession->m_bIsWildNeedLock && pstSession->m_udwLockWild)
        {
            auddwKeyList[udwKeyNum++] = udwSvrId * WILD_ID_LOCK_OFFSET + pstSession->m_udwLockWild;
            pstSession->m_bIsWildNeedLock = FALSE;
        }
        if (pstSession->m_bIsWildBNeedLock && pstSession->m_udwLockWildB)
        {
            auddwKeyList[udwKeyNum++] = udwSvrId * WILD_ID_LOCK_OFFSET + pstSession->m_udwLockWildB;
            pstSession->m_bIsWildBNeedLock = FALSE;
        }
        if(pstSession->m_bIsUidNeedLock && pstSession->m_udwLockUid)
        {
            auddwKeyList[udwKeyNum++] = USER_ID_LOCK_OFFSET + pstSession->m_udwLockUid;
            pstSession->m_bIsUidNeedLock = FALSE;
        }
        if(pstSession->m_bIsUidBNeedLock && pstSession->m_udwLockUidB)
        {
            auddwKeyList[udwKeyNum++] = USER_ID_LOCK_OFFSET + pstSession->m_udwLockUidB;
            pstSession->m_bIsUidBNeedLock = FALSE;
        }
        if(pstSession->m_bIsAidNeedLock && pstSession->m_udwLockAid)
        {
            auddwKeyList[udwKeyNum++] = ALLIANCE_ID_LOCK_OFFSET + pstSession->m_udwLockAid;
            pstSession->m_bIsAidNeedLock = FALSE;
        }
        if(pstSession->m_bIsAidBNeedLock && pstSession->m_udwLockAidB)
        {
            auddwKeyList[udwKeyNum++] = ALLIANCE_ID_LOCK_OFFSET + pstSession->m_udwLockAidB;
            pstSession->m_bIsAidBNeedLock = FALSE;
        }
    }
    else
    {
        if(pstSession->m_udwLockWild)
        {
            auddwKeyList[udwKeyNum++] = udwSvrId * WILD_ID_LOCK_OFFSET + pstSession->m_udwLockWild;
        }
        if (pstSession->m_udwLockWildB)
        {
            auddwKeyList[udwKeyNum++] = udwSvrId * WILD_ID_LOCK_OFFSET + pstSession->m_udwLockWildB;
        }
        if(pstSession->m_udwLockUid)
        {
            auddwKeyList[udwKeyNum++] = USER_ID_LOCK_OFFSET + pstSession->m_udwLockUid;
        }
        if(pstSession->m_udwLockUidB)
        {
            auddwKeyList[udwKeyNum++] = USER_ID_LOCK_OFFSET + pstSession->m_udwLockUidB;
        }
        if(pstSession->m_udwLockAid)
        {
            auddwKeyList[udwKeyNum++] = ALLIANCE_ID_LOCK_OFFSET + pstSession->m_udwLockAid;
        }
        if(pstSession->m_udwLockAidB)
        {
            auddwKeyList[udwKeyNum++] = ALLIANCE_ID_LOCK_OFFSET + pstSession->m_udwLockAidB;
        }
    }

    if(udwKeyNum == 0)
    {
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_LockReq:[%ld,%ld,%ld,%ld,%ld] [seq=%u]",
        auddwKeyList[0], auddwKeyList[1], auddwKeyList[2], auddwKeyList[3], auddwKeyList[4], pstSession->m_udwSeqNo));

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bLockSvrExist == FALSE)
    {
        pstSession->m_pstLockSvr = NULL;

        // 获取下游服务器
        if(S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__LOCK_SVR, &pstSession->m_pstLockSvr))
        {
            pstSession->m_bLockSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_LockReq: Get lock_svr node succ [handle=%p] [seq=%u]", pstSession->m_pstLockSvr->m_stDownHandle.handle, pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LockReq: Get lock_svr node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    TUINT32 udwTimeOutUs = (DOWN_LOCK_NODE_TIMEOUT_MS)* 1000; // 转为微妙，locksvr超时未微妙级
    idx = 0;
    m_pPackTool[idx]->ResetContent();
    m_pPackTool[idx]->SetServiceType(uwReqServiceType);
    m_pPackTool[idx]->SetSeq(CGlobalServ::GenerateHsReqSeq());
    m_pPackTool[idx]->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    m_pPackTool[idx]->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, udwKeyNum);
    m_pPackTool[idx]->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, (TUCHAR*)&auddwKeyList[0], udwKeyNum*sizeof(TUINT64));
    m_pPackTool[idx]->SetKey(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, udwTimeOutUs);
    m_pPackTool[idx]->GetPackage(&pszPack, &udwPackLen);

    stTasksGroup.m_UserData1.ptr = pstSession;

    if(NULL == pstSession->m_pstLockSvr->m_stDownHandle.handle)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LockReq:handle [handlevalue=0x%p] [seq=%u]", \
            pstSession->m_pstLockSvr->m_stDownHandle.handle, pstSession->m_udwSeqNo));
        return -4;
    }

    stTasksGroup.m_Tasks[idx].SetConnSession(pstSession->m_pstLockSvr->m_stDownHandle);
    stTasksGroup.m_Tasks[idx].SetSendData(pszPack, udwPackLen);
    stTasksGroup.m_Tasks[idx].SetNeedResponse(1);
    stTasksGroup.SetValidTasks(1);
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    // 设置加锁标记
    if(uwReqServiceType == EN_SERVICE_TYPE_HU2LOCK__GET_REQ)
    {
        pstSession->m_bLockedData = TRUE;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_LockReq:lock_svr node succ [handle=%p] [seq=%u]", stTasksGroup.m_Tasks[idx].hSession.handle, pstSession->m_udwSeqNo));
    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    if(!m_poSearchLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LockReq send data failed!, service[%u] [seq=%u]", uwReqServiceType, pstSession->m_udwSeqNo));
        return -3;
    }

    return 0;
}

TINT32 CTaskProcess::FirstLockGetRequest(SSession *pstSession)
{
    TUINT32 udwWildPos = 0;
    TUINT32 udwSCid = 0;
    TUINT32 udwSUid = 0;
    TUINT32 udwSAlid = 0;
    switch(pstSession->m_stActionTable.dwTableType)
    {
    case EN_UID_ACTION:
        udwSUid = pstSession->m_stReqAction.m_nSuid;
        if (pstSession->m_stReqAction.m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE)
        {
            udwWildPos = pstSession->m_stReqAction.m_bParam[0].m_stAttackMove.m_ddwCityId;
        }
        break;
    case EN_AID_ACTION:
        udwSUid = pstSession->m_stReqAlAction.m_nSuid;
        udwSAlid = pstSession->m_stReqAlAction.m_nSal;
        break;
    case EN_MARCH_ACTION:
        udwSUid = pstSession->m_stReqMarch.m_nSuid;
        udwSAlid = pstSession->m_stReqMarch.m_nSal;
        udwWildPos = pstSession->m_stReqMarch.m_nTpos;
        udwSCid = pstSession->m_stReqMarch.m_nScid;
        break;
    default:
        break;
    }

    if (udwWildPos == 0 && udwSUid == 0 && udwSAlid == 0 && udwSCid == 0)
    {
        return 1;
    }

    if(udwWildPos)
    {
        pstSession->m_udwLockWild = udwWildPos;
        pstSession->m_bIsWildNeedLock = TRUE;
    }

    if (udwSCid)
    {
        pstSession->m_udwLockWildB = udwSCid;
        pstSession->m_bIsWildBNeedLock = TRUE;
    }

    if(udwSUid)
    {
        pstSession->m_udwLockUid = udwSUid;
        pstSession->m_bIsUidNeedLock = TRUE;
    }

    if(udwSAlid)
    {
        pstSession->m_udwLockAid = udwSAlid;
        pstSession->m_bIsAidNeedLock = TRUE;
    }

    return ProcessProcedure_LockReq(pstSession, EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
}

TINT32 CTaskProcess::FirstDataGetRequest(SSession *pstSession)
{
    TUINT32 udwSvrId = 0;
    TINT32 dwSUid = 0;
    TINT32 dwSAlid = 0;
    TINT32 dwWildPos = 0;

    switch(pstSession->m_stActionTable.dwTableType)
    {
    case EN_UID_ACTION:
        dwSUid = pstSession->m_stReqAction.m_nSuid;
        break;
    case EN_AID_ACTION:
        dwSUid = pstSession->m_stReqAlAction.m_nSuid;
        dwSAlid = pstSession->m_stReqAlAction.m_nSal;
        break;
    case EN_MARCH_ACTION:
        dwSUid = pstSession->m_stReqMarch.m_nSuid;
        dwSAlid = pstSession->m_stReqMarch.m_nSal;
        udwSvrId = pstSession->m_stReqMarch.m_nSid;
        dwWildPos = pstSession->m_stReqMarch.m_nTpos;
        break;
    default:
        break;
    }

//     if(dwWildPos == 0 && dwSUid == 0 && dwSAlid == 0)
//     {
//         return 1;
//     }

    pstSession->ResetAwsInfo();

    if(dwWildPos)
    {
        CAwsRequest::MapGetByIdWithLock(pstSession, udwSvrId, dwWildPos);
    }

    UserInfoGetRequest(pstSession, dwSUid);

    CAwsRequest::IdolQuery(pstSession, udwSvrId);
    CAwsRequest::ThroneGet(pstSession, udwSvrId);
    CAwsRequest::TitleQuery(pstSession, udwSvrId);

    //发送请求
    if (pstSession->m_vecAwsReq.size() > 0)
    {
        return SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    }
    else
    {
        return 1;
    }
}

TINT32 CTaskProcess::SecondLockGetRequest(SSession *pstSession)
{
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TUINT32 udwTUid = 0;
    TUINT32 udwTAlid = 0;

    if (ptbWild->m_nId > 0)
    {
        udwTUid = ptbWild->m_nUid;
        udwTAlid = ptbWild->m_nAlid;
        if (ptbWild->m_nType == EN_WILD_TYPE__THRONE_NEW
            && ptbWild->m_nId == pstSession->m_tbThrone.m_nPos)
        {
            udwTAlid = pstSession->m_tbThrone.m_nAlid;
            udwTUid = pstSession->m_tbThrone.m_nOwner_id;
        }
    }
    else if (pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
    {
        udwTUid = pstSession->m_stReqMarch.m_nTuid;
    }

    if(udwTUid == 0 && udwTAlid == 0)
    {
        return 1;
    }

    if(udwTUid)
    {
        pstSession->m_udwLockUidB = udwTUid;
        pstSession->m_bIsUidBNeedLock = TRUE;
    }

    if(udwTAlid)
    {
        pstSession->m_udwLockAidB = udwTAlid;
        pstSession->m_bIsAidBNeedLock = TRUE;
    }

    return ProcessProcedure_LockReq(pstSession, EN_SERVICE_TYPE_HU2LOCK__GET_REQ);
}

TINT32 CTaskProcess::SecondDataGetRequest(SSession *pstSession)
{
    TINT32 dwTUid = 0;
    //TINT32 dwTAlid = 0;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    if(ptbWild->m_nId > 0)
    {
        dwTUid = ptbWild->m_nUid;
        if (ptbWild->m_nType == EN_WILD_TYPE__THRONE_NEW
            && ptbWild->m_nId == pstSession->m_tbThrone.m_nPos)
        {
            dwTUid = pstSession->m_tbThrone.m_nOwner_id;
        }
    }
    else if(pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
    {
        dwTUid = pstSession->m_stReqMarch.m_nTuid;
    }

//     if(dwTUid == 0 && dwTAlid == 0)
//     {
//         return 1;
//     }

    pstSession->ResetAwsInfo();

    UserInfoGetRequest(pstSession, dwTUid);

    //发送请求
    if (pstSession->m_vecAwsReq.size() > 0)
    {
        return SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    }
    else
    {
        return 1;
    }
}

TINT32 CTaskProcess::UserInfoGetRequest(SSession *pstSession, TINT32 dwUid)
{
    TUINT32 udwSvrId = pstSession->m_udwReqSvrId;

    if(dwUid > 0)
    {
        CAwsRequest::LoginGetByUid(pstSession, dwUid);
        CAwsRequest::PlayerGetByUid(pstSession, dwUid);
        CAwsRequest::BufferActionQueryBySuid(pstSession, dwUid);
        CAwsRequest::AlActionQueryBySuid(pstSession, dwUid);
        CAwsRequest::MarchActionQueryBySuid(pstSession, udwSvrId, dwUid);
        CAwsRequest::MarchActionQueryByTuid(pstSession, udwSvrId, dwUid);
        CAwsRequest::UserStatGet(pstSession, dwUid);
        CAwsRequest::CityGetByUid(pstSession, dwUid);
        CAwsRequest::BackPackGet(pstSession, dwUid);
        CAwsRequest::QuestGetByUid(pstSession, dwUid);
        CAwsRequest::MapQueryByUid(pstSession, udwSvrId, dwUid);
        CAwsRequest::EquipQueryByUid(pstSession, dwUid);
        CAwsRequest::BountyGetByUid(pstSession, dwUid);
    }

    return 0;
}

TINT32 CTaskProcess::AllianceInfoRequest(SSession *pstSession, SUserInfo *pstUser, TINT32 dwAlid, TBOOL IsNeedReport)
{
    TUINT32 udwSvrId = pstSession->m_udwReqSvrId;

    pstSession->ResetAwsInfo();

    if(dwAlid > 0)
    {
        CAwsRequest::AllianceGetByAid(pstSession, dwAlid);
        // 获取联盟全部人assistance的记录(包括所有的type:申请资源/分享资源/申请援兵(目前只用到前两种type))
        CAwsRequest::AlAssistQuery(pstSession, dwAlid);
        // 获取联盟的外交政策
        CAwsRequest::DiplomacyQuery(pstSession, dwAlid);
        // 获取自己的联盟成员信息
        CAwsRequest::AlMemberGet(pstSession, dwAlid, pstUser->m_tbPlayer.m_nUid);

        CAwsRequest::MarchActionQueryBySal(pstSession, udwSvrId, dwAlid);

        CAwsRequest::MarchActionQueryByTal(pstSession, udwSvrId, dwAlid);

        // 联盟礼物
        TINT64 ddwTimePoint = CTimeUtils::GetUnixTime() - AL_IAP_GIFT_EXPIRE_TIME * 2;
        ddwTimePoint = (ddwTimePoint > pstUser->m_tbPlayer.m_nAl_time) ? ddwTimePoint : pstUser->m_tbPlayer.m_nAl_time;
        TbAl_gift tbAl_gift;
        tbAl_gift.Set_Aid(dwAlid);
        tbAl_gift.Set_Id(ddwTimePoint << 32);
        CompareDesc comp_desc;
        comp_desc.dwCompareType = COMPARE_TYPE_GE;
        CAwsRequest::Query(pstSession, &tbAl_gift, ETbALGIFT_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, MAX_AL_IAP_GIFT_NUM);

    }

    //发送请求
    return SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
}

TINT32 CTaskProcess::AllianceInfoGetResponse(SSession *pstSession, SUserInfo *pstUserInfo, TBOOL IsSourceUser)
{
    AwsRspInfo *pstAwsRspInfo = NULL;
    TUINT32 udwIdx = 0;
    TINT32 dwRetCode = 0;

    // 1. 解析数据
    for(udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
    {
        pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
        string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);

        if(strTableRawName == EN_AWS_TABLE_ALLIANCE)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbAlliance);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_DIPLOMACY)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbDiplomacy, sizeof(TbDiplomacy), MAX_DIPLOMACY_NUM);
            if(dwRetCode > 0)
            {
                pstUserInfo->m_udwDiplomacyNum = dwRetCode;
            }
            else
            {
                pstUserInfo->m_udwDiplomacyNum = 0;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_ASSIST)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbAlAssistAll, sizeof(TbAl_assist), MAX_AL_ASSIST_NUM - 1);
            if(dwRetCode > 0)
            {
                pstUserInfo->m_udwAlAssistAllNum = dwRetCode;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_MEMBER)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbSelfAlmember);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
        {
            TbMarch_action atbTmpMarch[MAX_USER_MARCH_NUM];
            switch(pstAwsRspInfo->udwIdxNo)
            {
            case ETbMARCH_OPEN_TYPE_GLB_SAL:
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, atbTmpMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRetCode >= 0)
                {
                    std::set<TINT64> setMarchId;
                    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwMarchNum; ++udwIdx)
                    {
                        setMarchId.insert(pstUserInfo->m_atbMarch[udwIdx].m_nId);
                    }
                    for(TINT32 dwIdx = 0; dwIdx < dwRetCode; ++dwIdx)
                    {
                        if(pstUserInfo->m_udwMarchNum >= MAX_USER_MARCH_NUM)
                        {
                            break;
                        }
                        if(setMarchId.count(atbTmpMarch[dwIdx].m_nId) > 0)
                        {
                            continue;
                        }
                        pstUserInfo->m_atbMarch[pstUserInfo->m_udwMarchNum++] = atbTmpMarch[dwIdx];
                    }
                }
                break;
            case ETbMARCH_OPEN_TYPE_GLB_TAL:
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, atbTmpMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRetCode >= 0)
                {
                    std::set<TINT64> setPassiveMarchId;
                    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwPassiveMarchNum; ++udwIdx)
                    {
                        setPassiveMarchId.insert(pstUserInfo->m_atbPassiveMarch[udwIdx].m_nId);
                    }
                    for(TINT32 dwIdx = 0; dwIdx < dwRetCode; ++dwIdx)
                    {
                        if(pstUserInfo->m_udwPassiveMarchNum >= MAX_USER_MARCH_NUM)
                        {
                            break;
                        }
                        if(setPassiveMarchId.count(atbTmpMarch[dwIdx].m_nId) > 0)
                        {
                            continue;
                        }
                        pstUserInfo->m_atbPassiveMarch[pstUserInfo->m_udwPassiveMarchNum++] = atbTmpMarch[dwIdx];
                    }
                }
                break;
            default:
                break;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_GIFT)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_stAlGifts.m_atbGifts + pstUserInfo->m_stAlGifts.m_dwGiftNum, sizeof(TbAl_gift), MAX_AL_IAP_GIFT_NUM);
            if(dwRetCode > 0)
            {
                pstUserInfo->m_stAlGifts.m_dwGiftNum += dwRetCode;
            }
            continue;
        }
    }

    return 0;
}

TINT32 CTaskProcess::UserInfoGetResponse(SSession *pstSession, SUserInfo *pstUserInfo, TBOOL IsSourceUser)
{
    AwsRspInfo *pstAwsRspInfo = NULL;
    TUINT32 udwIdx = 0;
    TINT32 dwRetCode = 0;

    // 0. 重置各关键值
    pstUserInfo->m_udwActionNum = 0;
    pstUserInfo->m_udwSelfAlActionNum = 0;
    pstUserInfo->m_udwMarchNum = 0;
    pstUserInfo->m_udwPassiveMarchNum = 0;

    // 1. 解析数据
    for(udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
    {
        pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
        string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
        if(strTableRawName == EN_AWS_TABLE_LOGIN)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbLogin);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_PLAYER)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbPlayer);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_ACTION)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbAction, sizeof(TbAction), MAX_USER_ACTION_NUM);
            if(dwRetCode > 0)
            {
                pstUserInfo->m_udwActionNum = dwRetCode;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_ALLIANCE_ACTION)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbSelfAlAction, sizeof(TbAlliance_action), MAX_USER_AL_ACTION_NUM);
            if(dwRetCode > 0)
            {
                pstUserInfo->m_udwSelfAlActionNum = dwRetCode;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
        {
            switch(pstAwsRspInfo->udwIdxNo)
            {
            case ETbMARCH_OPEN_TYPE_PRIMARY:
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRetCode >= 0)
                {
                    pstUserInfo->m_udwMarchNum = dwRetCode;
                }
                break;
            case ETbMARCH_OPEN_TYPE_GLB_TUID:
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbPassiveMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRetCode >= 0)
                {
                    pstUserInfo->m_udwPassiveMarchNum = dwRetCode;
                }
                break;
            default:
                break;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_USER_STAT)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbUserStat);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_BACKPACK)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbBackpack);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_QUEST)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbQuest);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_CITY)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_stCityInfo.m_stTblData);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_BOUNTY)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbBounty);

            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_MAP)
        {
            if(pstAwsRspInfo->udwIdxNo == ETbMAP_OPEN_TYPE_PRIMARY)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_stMapItem);
                CMapBase::ComputeWildInfo(&pstSession->m_stMapItem);
                pstSession->m_ucRawWildType = pstSession->m_stMapItem.m_nType;
                pstSession->m_ucRawWildLevel = pstSession->m_stMapItem.m_nLevel;
                continue;
            }
            if(pstAwsRspInfo->udwIdxNo == ETbMAP_OPEN_TYPE_GLB_UID)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbWild, sizeof(TbMap), MAX_WILD_RETURN_NUM);
                if(dwRetCode > 0)
                {
                    pstUserInfo->m_udwWildNum = dwRetCode;
                }
                continue;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_EQUIP)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbEquip, sizeof(TbEquip), MAX_USER_EQUIP_NUM);
            if(dwRetCode >= 0)
            {
                pstUserInfo->m_udwEquipNum = dwRetCode;
            }
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_IDOL)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_atbIdol, sizeof(TbIdol), MAX_IDOL_NUM);
            if (dwRetCode >= 0)
            {
                pstSession->m_udwIdolNum = dwRetCode;
            }
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_THRONE)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbThrone);
            if (pstSession->m_tbThrone.m_nAlid > 0)
            {
                pstSession->dwRawThroneAlid = pstSession->m_tbThrone.m_nAlid;
            }
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_TITLE)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_stTitleInfoList.atbTitle, sizeof(TbTitle), MAX_TITLEINFO_NUM_IN_ONE_SERVER);
            if (dwRetCode >= 0)
            {
                pstSession->m_stTitleInfoList.udwNum = dwRetCode;
            }
            continue;
        }
    }

    //2. 检查数据完整
    TUINT32 udwUid = 0;
    TBOOL bNeedCheckAction = FALSE;
    if(IsSourceUser)
    {
        switch(pstSession->m_stActionTable.dwTableType)
        {
        case EN_UID_ACTION:
            udwUid = pstSession->m_stReqAction.m_nSuid;
            bNeedCheckAction = TRUE;
            break;
        case EN_AID_ACTION:
            udwUid = pstSession->m_stReqAlAction.m_nSuid;
            bNeedCheckAction = TRUE;
            break;
        case EN_MARCH_ACTION:
            udwUid = pstSession->m_stReqMarch.m_nSuid;
            bNeedCheckAction = TRUE;
            break;
        default:
            break;
        }
    }
    else
    {
        if(pstSession->m_stMapItem.m_nUid > 0)
        {
            udwUid = pstSession->m_stMapItem.m_nUid;
        }
    }
    if(udwUid > 0)
    {
        // player信息必不可少
        if(pstUserInfo->m_tbPlayer.m_nUid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            pstSession->m_udwNextProcedure = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
            TSE_LOG_ERROR(m_poServLog, ("ProcessUserInfoGetResponse: user info incomplete[%ld][seq=%u]",
                pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
            return -1;
        }

        // check action
        if(bNeedCheckAction)
        {
            dwRetCode = CTaskProcess::CheckReqAction(pstSession, pstUserInfo);
            if(dwRetCode < 0)
            {
                return dwRetCode;
            }
        }
    }

    if (IsSourceUser)
    {
        pstSession->m_ddwGemBegin = pstUserInfo->m_tbLogin.m_nGem;
    }

    TUINT64 uddwTmpId = CTimeUtils::GetCurTimeUs();
    uddwTmpId = uddwTmpId / 1000;
    uddwTmpId = uddwTmpId - (uddwTmpId / 10000000) * 10000000;
    pstUserInfo->m_uddwCurEventId = uddwTmpId;

    return 0;
}

TINT32 CTaskProcess::ProcessProcedure_UserUpdtRequest(SSession *pstSession)
{
    TINT32 dwReqNum = 0;
    TINT32 dwRetCode = 0;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("[WAVETEST-1]ProcessProcedure_UserUpdtRequest al_source[uid=%u,aid=%u] al_target[uid=%u,aid=%u] [seq=%u]", \
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwAllianceId,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwAllianceId,
        pstSession->m_udwSeqNo));

    //init
    pstSession->ResetAwsInfo();

    CTaskProcess::SyncReqActionToUserInfo(pstSession);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("[WAVETEST-2]ProcessProcedure_UserUpdtRequest al_source[uid=%u,aid=%u] al_target[uid=%u,aid=%u] [seq=%u]", \
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwAllianceId,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwAllianceId,
        pstSession->m_udwSeqNo));

    //source user 
    GetUserUpdtRequest(pstSession, &pstSession->m_stSourceUser);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("[WAVETEST-3]ProcessProcedure_UserUpdtRequest al_source[uid=%u,aid=%u] al_target[uid=%u,aid=%u] [seq=%u]", \
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwAllianceId,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwAllianceId,
        pstSession->m_udwSeqNo));

    //target user
    GetUserUpdtRequest(pstSession, &pstSession->m_stTargetUser);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("[WAVETEST-4]ProcessProcedure_UserUpdtRequest al_source[uid=%u,aid=%u] al_target[uid=%u,aid=%u] [seq=%u]", \
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwAllianceId,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwAllianceId,
        pstSession->m_udwSeqNo));
    //cur action
    GetCurActionUpdtRequest(pstSession);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("[WAVETEST-5]ProcessProcedure_UserUpdtRequest al_source[uid=%u,aid=%u] al_target[uid=%u,aid=%u] [seq=%u]", \
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataSourceAl.m_udwAllianceId,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwUid,
        pstSession->m_objAuPushDataNode.m_objPushDataTargetAl.m_udwAllianceId,
        pstSession->m_udwSeqNo));
    //session
    GetSessionUpdtRequest(pstSession);

    // 日志评估系统的日志生成必须在更新后,才能保证数据是最新的
    // 记录一些请求参数数据
    SReqInfo stSourceReqInfo;
    SReqInfo stTargetReqInfo;
    TCHAR szKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN] = {0};
    stSourceReqInfo.Reset();
    stTargetReqInfo.Reset();
    TBOOL bOutputTarget = true;
    // 游戏评估额外数据
    SGameEvaluateAddData stGameEvaluateAddData;
    stGameEvaluateAddData.Reset();

    switch(pstSession->m_stActionTable.dwTableType)
    {
        case EN_UID_ACTION:
            // source        
            stSourceReqInfo.Reset();
            if(EN_ACTION_MAIN_TASK_ATTACK_MOVE == pstSession->m_stReqAction.Get_Mclass()
                && EN_ACTION_SEC_CLASS__ATTACK_MOVE == pstSession->m_stReqAction.Get_Sclass())
            { 
                stSourceReqInfo.SetValue(pstSession->m_stReqAction.m_nSid, 0, pstSession->m_stSourceUser.m_udwMoveNewCityId, CCommonFunc::NumToString(pstSession->m_stReqAction.Get_Sclass()), "", szKey, 0);
            }
            else
            {
                stSourceReqInfo.SetValue(pstSession->m_stReqAction.m_nSid, 0, pstSession->m_stReqAction.m_nScid, CCommonFunc::NumToString(pstSession->m_stReqAction.Get_Sclass()), "", szKey, 0);
            }
            
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_UserUpdtRequest [cid=%lld] [seq=%u]", \
                                                    stSourceReqInfo.m_udwCid, pstSession->m_udwSeqNo));
            if(EN_ACTION_MAIN_CLASS__MARCH == pstSession->m_stReqAction.Get_Mclass()
               && EN_MARCH_STATUS__RETURNING == pstSession->m_stReqAction.Get_Status()
               && EN_MARCH_STATUS__RETURNING == pstSession->m_stRawReqAction.Get_Status())
            {
                bOutputTarget = false;
            }
            break;
        case EN_AID_ACTION:
            // source        
            stSourceReqInfo.Reset();
            stSourceReqInfo.SetValue(pstSession->m_stReqAlAction.m_nSid, 0, pstSession->m_stReqAlAction.m_nScid, CCommonFunc::NumToString(pstSession->m_stReqAlAction.Get_Sclass()), "", szKey, 0);
            if(EN_ACTION_MAIN_CLASS__MARCH == pstSession->m_stReqAlAction.Get_Mclass()
               && EN_MARCH_STATUS__RETURNING == pstSession->m_stReqAlAction.Get_Status()
               && EN_MARCH_STATUS__RETURNING == pstSession->m_stRawReqAlAction.Get_Status())
            {
                bOutputTarget = false;
            }
            break;
        case EN_MARCH_ACTION:
            // 游戏评估系统需要保存的raw ex_data    
            // source        
            stSourceReqInfo.Reset();
            stSourceReqInfo.SetValue(pstSession->m_stReqMarch.m_nSid, 0, pstSession->m_stReqMarch.m_nScid, CCommonFunc::NumToString(pstSession->m_stReqMarch.Get_Sclass()), "", szKey, 0);
            // target
            if(true == pstSession->m_stSourceUser.m_bGameEvaluateType)
            {
                stTargetReqInfo.Reset();
                stTargetReqInfo.SetValue(pstSession->m_stReqMarch.m_nSid, 0, pstSession->m_stReqMarch.m_nTpos, CCommonFunc::NumToString(pstSession->m_stReqMarch.Get_Sclass()), "", szKey, 0);
            }
            if(EN_ACTION_MAIN_CLASS__MARCH == pstSession->m_stReqMarch.Get_Mclass()
               && EN_MARCH_STATUS__RETURNING == pstSession->m_stReqMarch.Get_Status()
               && EN_MARCH_STATUS__RETURNING == pstSession->m_stRawReqMarch.Get_Status())
            {
                bOutputTarget = false;
            }
            if(EN_ACTION_MAIN_CLASS__MARCH == pstSession->m_stReqMarch.Get_Mclass()
               && true == bOutputTarget)
            {
                stGameEvaluateAddData.m_astAddDataMap[EN_USER_ADD_INFO_RECORD_TYPE__ATTACKED_TYPE] = CCommonFunc::NumToString(pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetType);
            }
           break;
        default:
           break;
    }


    
    // 游戏评估系统需要保存的new ex_data
    // source
    CGameEvaluateLogic::SaveGameEvaluateExData(&stSourceReqInfo, &pstSession->m_stSourceUser, EN_EX_DATA_USER_TYPE_SOURCE, EN_EX_DATA_TYPE_NEW);
    if(true == pstSession->m_stSourceUser.m_bGameEvaluateType
        && true == bOutputTarget)
    {   
        // target
        CGameEvaluateLogic::SaveGameEvaluateExData(&stTargetReqInfo, &pstSession->m_stTargetUser, EN_EX_DATA_USER_TYPE_TARGET, EN_EX_DATA_TYPE_NEW);
    }

    // 生成游戏评估基本数据
    TBOOL bGetGameEvaluateDataSourceFlag = false;
    SGameEvaluateData stGameEvaluateDataSource;
    stGameEvaluateDataSource.Reset();
    TBOOL bGetGameEvaluateDataTargetFlag = false;
    SGameEvaluateData stGameEvaluateDataTarget;
    stGameEvaluateDataTarget.Reset();

    // source
    bGetGameEvaluateDataSourceFlag = CGameEvaluateLogic::GenGameEvaluateData(&stSourceReqInfo, &pstSession->m_stSourceUser, &stGameEvaluateDataSource);
    if(true == pstSession->m_stSourceUser.m_bGameEvaluateType
        && true == bOutputTarget)
    {
        // target
        bGetGameEvaluateDataTargetFlag = CGameEvaluateLogic::GenGameEvaluateData(&stTargetReqInfo, &pstSession->m_stTargetUser, &stGameEvaluateDataTarget);
    }

    // 生成游戏评估ex数据 
    TBOOL bGetGameEvaluateExDataSourceResultFlag = false;  
    SGameEvaluateExData stGameEvaluateExDataSourceResult;
    stGameEvaluateExDataSourceResult.Reset();
    TBOOL bGetGameEvaluateExDataTargetResultFlag = false;  
    SGameEvaluateExData stGameEvaluateExDataTargetResult;
    stGameEvaluateExDataTargetResult.Reset();
    
    // source
    bGetGameEvaluateExDataSourceResultFlag = CGameEvaluateLogic::GenGameEvaluateExData(&pstSession->m_stSourceUser, &stGameEvaluateExDataSourceResult, EN_EX_DATA_USER_TYPE_SOURCE);
    if(true == pstSession->m_stSourceUser.m_bGameEvaluateType
        && true == bOutputTarget)
    {
        // target
        bGetGameEvaluateExDataTargetResultFlag = CGameEvaluateLogic::GenGameEvaluateExData(&pstSession->m_stTargetUser, &stGameEvaluateExDataTargetResult, EN_EX_DATA_USER_TYPE_TARGET);
    }

    TSE_LOG_DEBUG(m_poServLog, ("game_flag::bGetGameEvaluateDataSourceFlag=%u, bGetGameEvaluateDataTargetFlag=%u, bGetGameEvaluateExDataSourceResultFlag=%u, bGetGameEvaluateExDataTargetResultFlag=%u, seq=%u", \
                                bGetGameEvaluateDataSourceFlag, bGetGameEvaluateDataTargetFlag, \
                                bGetGameEvaluateExDataSourceResultFlag, bGetGameEvaluateExDataTargetResultFlag, \
                                pstSession->m_udwSeqNo));

    string strGameEvaluateLog = "";
    strGameEvaluateLog = CGameEvaluateLogic::GenGameEvaluateLog(&stGameEvaluateDataSource, bGetGameEvaluateDataSourceFlag, \
                                                                &stGameEvaluateDataTarget, bGetGameEvaluateDataTargetFlag, \
                                                                &stGameEvaluateExDataSourceResult, bGetGameEvaluateExDataSourceResultFlag, \
                                                                &stGameEvaluateExDataTargetResult, bGetGameEvaluateExDataTargetResultFlag, \
                                                                &stGameEvaluateAddData, pstSession->m_stSourceUser.m_bGameEvaluateType);
    pstSession->m_strGameEvaluateLog = strGameEvaluateLog;

    //send request
    dwReqNum = pstSession->m_vecAwsReq.size();
    TSE_LOG_DEBUG(m_poServLog, ("ProcessProcedure_UserUpdtRequest: update_req_num=%d [seq=%u]", dwReqNum, pstSession->m_udwSeqNo));
    if(dwReqNum > 0)
    {
        for (TINT32 dwIdx = 0; dwIdx < dwReqNum; ++dwIdx)
        {
            TSE_LOG_DEBUG(m_poServLog, ("ProcessProcedure_UserUpdtRequest: op[%s] req_str[%s] [seq=%u]",
                pstSession->m_vecAwsReq[dwIdx]->sOperatorName.c_str(), pstSession->m_vecAwsReq[dwIdx]->sReqContent.c_str(), pstSession->m_udwSeqNo));
        }
        dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
    }

    return dwReqNum;
}

TINT32 CTaskProcess::GetUserUpdtRequest(SSession *pstSession, SUserInfo *pstUserInfo)
{
    TINT32 dwRetCode = 0;
    TUINT32 dwIdx = 0;
    TUINT32 udwPushDataAlId = CPlayerBase::GetAllianceId(&pstUserInfo->m_tbPlayer);

    //wave@push_data
    CPushMapData *pobjMapPushData = &pstSession->m_objAuPushDataNode.m_objPushDataMap;
    CPushAlData *pobjAlPushData = NULL;
    if(pstUserInfo == &pstSession->m_stSourceUser)
    {
        pobjAlPushData = &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl;
    }
    else
    {
        pobjAlPushData = &pstSession->m_objAuPushDataNode.m_objPushDataTargetAl;
    }

    // 更新broadcast信息(broadcast表数据)  广播不能删除
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [broadcast=%ld] [seq=%u]",
        pstUserInfo->m_udwBroadcastNum, pstSession->m_udwSeqNo));
    for (dwIdx = 0; dwIdx < pstUserInfo->m_udwBroadcastNum; dwIdx++)
    {
        if (pstUserInfo->m_aucBroadcastFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            CAwsRequest::PutItem(pstSession, &pstUserInfo->m_atbBroadcast[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: broadcast need to update [ctime=%ld] [seq=%u]",
                pstUserInfo->m_atbBroadcast[dwIdx].m_nCtime,
                pstSession->m_udwSeqNo));
        }
    }

    if (pstUserInfo->m_tbPlayer.m_nUid == 0)
    {
        return 0;
    }

    if(pstUserInfo->m_tbPlayer.m_nUid != 0)
    {
        CAuCommonAfter::AuCommonHandleAfter(pstSession, pstUserInfo);
    }

    //1. login 拉取gem  发送鼓励邮件
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbLogin);
    if (dwRetCode == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest[%u]: login [%ld] to update [seq=%u]",
            pstSession->m_vecAwsReq.size(), pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
    }

    //2. player
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbPlayer);
    if(dwRetCode == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest[%u]: player [%ld] to update [seq=%u]",
            pstSession->m_vecAwsReq.size(), pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
    }

    //3. user stat
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbUserStat);
    if(dwRetCode == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest[%u]: user_stat [%ld] to update [seq=%u]",
            pstSession->m_vecAwsReq.size(), pstUserInfo->m_tbUserStat.m_nUid, pstSession->m_udwSeqNo));
    }

    //4. action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: [activeactionnum=%ld] [seq=%u]",
        pstUserInfo->m_udwActionNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwActionNum; dwIdx++)
    {
        if(pstSession->m_stReqAction.m_nId == pstUserInfo->m_atbAction[dwIdx].m_nId)
        {
            continue;
        }

        if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucActionFlag[dwIdx])
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbAction[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active action need to delete [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucActionFlag[dwIdx])
        {
            CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbAction[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active action need to put [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbACTION_FIELD_ID, true, pstUserInfo->m_atbAction[dwIdx].m_nId); //加expect防止边界条件(action已经结束又被新建)
            expect_desc.clear();
            expect_desc.push_back(expect_item);

            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbAction[dwIdx], expect_desc);
            if(0 == dwRetCode)
            {
                pstUserInfo->m_aucActionFlag[dwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active action need to update [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
    }

    // 主动联盟action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: [alactionnum=%ld] [seq=%u]",
        pstUserInfo->m_udwSelfAlActionNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwSelfAlActionNum; dwIdx++)
    {
        if(pstSession->m_stReqAlAction.m_nId == pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId)
        {
            continue;
        }

        if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucSelfAlActionFlag[dwIdx])
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbSelfAlAction[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active al action need to delete [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId, pstSession->m_udwSeqNo));

        }
        else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucSelfAlActionFlag[dwIdx])
        {
            CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbSelfAlAction[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active al action need to put [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbALLIANCE_ACTION_FIELD_ID, true, pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId); //加expect防止边界条件(action已经结束又被新建)
            expect_desc.clear();
            expect_desc.push_back(expect_item);

            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbSelfAlAction[dwIdx], expect_desc);
            if(0 == dwRetCode)
            {
                pstUserInfo->m_aucSelfAlActionFlag[dwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active al action need to update [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }

        //wave@push_data
        CAuPushData::AddPushData_Action(pstSession, pobjAlPushData, pobjMapPushData, &pstUserInfo->m_atbSelfAlAction[dwIdx], EN_AID_ACTION, pstUserInfo->m_aucSelfAlActionFlag[dwIdx], TRUE);
    }

    // 主动march action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: [march_num=%ld] [seq=%u]",
        pstUserInfo->m_udwMarchNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwMarchNum; dwIdx++)
    {
        if(pstSession->m_stReqMarch.m_nId == pstUserInfo->m_atbMarch[dwIdx].m_nId)
        {
            if (pstSession->m_stReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER)
            {
                pstSession->m_stReqMarch = pstUserInfo->m_atbMarch[dwIdx];
            }
            continue;
        }

        if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucMarchFlag[dwIdx])
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbMarch[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active march action need to delete [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucMarchFlag[dwIdx])
        {
            CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbMarch[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active march action need to put [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbMARCH_ACTION_FIELD_ID, true, pstUserInfo->m_atbMarch[dwIdx].m_nId); //加expect防止边界条件(action已经结束又被新建)
            expect_desc.clear();
            expect_desc.push_back(expect_item);

            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbMarch[dwIdx], expect_desc);
            if(0 == dwRetCode)
            {
                pstUserInfo->m_aucMarchFlag[dwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: active march action need to update [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }

        //wave@push_data
        CAuPushData::AddPushData_Action(pstSession, pobjAlPushData, pobjMapPushData, &pstUserInfo->m_atbMarch[dwIdx], EN_MARCH_ACTION, pstUserInfo->m_aucMarchFlag[dwIdx], TRUE);
    }

    // 被动march action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: [passive_march_num=%ld] [seq=%u]",
        pstUserInfo->m_udwPassiveMarchNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwPassiveMarchNum; dwIdx++)
    {
        if(pstSession->m_stReqMarch.m_nId == pstUserInfo->m_atbPassiveMarch[dwIdx].m_nId)
        {
            continue;
        }

        ExpectedItem expect_item;
        ExpectedDesc expect_desc;
        expect_item.SetVal(TbMARCH_ACTION_FIELD_ID, true, pstUserInfo->m_atbPassiveMarch[dwIdx].m_nId);
        expect_desc.clear();
        expect_desc.push_back(expect_item);

        dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbPassiveMarch[dwIdx], expect_desc);
        if(0 == dwRetCode)
        {
            pstUserInfo->m_aucPassiveMarchFlag[dwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: passive march action need to update [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbPassiveMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }

        //wave@push_data
        CAuPushData::AddPushData_Action(pstSession, pobjAlPushData, pobjMapPushData, &pstUserInfo->m_atbPassiveMarch[dwIdx], EN_MARCH_ACTION, pstUserInfo->m_aucPassiveMarchFlag[dwIdx], FALSE);
    }

    // 5. map表, 不支持删除
    for(TUINT32 dwIdx = 0; dwIdx < pstUserInfo->m_udwWildNum; ++dwIdx)
    {
        if (pstUserInfo->m_atbWild[dwIdx].m_nId == pstSession->m_stMapItem.m_nId)
        {
            //wave@20161123:相等时不更新
            //dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbWild[dwIdx]);
            //if (dwRetCode == 0)
            //{
            //    //pstSession->m_vecAwsReq[pstSession->m_vecAwsReq.size() - 1]->bIsDefaultLock = false;

            //    //wave@push_data
            //    CAuPushData::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, &pstUserInfo->m_atbWild[dwIdx]);
            //}
        }
        else
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbWild[dwIdx]);
            if(dwRetCode == 0)
            {
                //wave@push_data
                CAuPushData::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, &pstUserInfo->m_atbWild[dwIdx]);
            }
        }

        if (0 == dwRetCode)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest[%u]:  map need to update[pos=%ld][seq=%u]",
                dwIdx, pstUserInfo->m_atbWild[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }        
    }

    // 6. city
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_stCityInfo.m_stTblData);
    if(dwRetCode == 0)
    {
        TSE_LOG_DEBUG(m_poServLog, ("GetUserUpdtRequest[%u]: update city info[%u] change [seq=%u]",
            pstSession->m_vecAwsReq.size(), pstUserInfo->m_stCityInfo.m_stTblData.m_nPos, pstSession->m_udwSeqNo));
    }

    // 7. alliance
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbAlliance);
    if(dwRetCode == 0)
    {
        TSE_LOG_DEBUG(m_poServLog, ("GetUserUpdtRequest[%u]: update alliance info[%ld] change [seq=%u]",
            pstSession->m_vecAwsReq.size(), pstUserInfo->m_tbAlliance.m_nAid, pstSession->m_udwSeqNo));
    }

    // 8. assist
    for(TUINT32 dwIdx = 0; dwIdx < pstUserInfo->m_udwAlAssistAllNum; ++dwIdx)
    {
        if(pstUserInfo->m_aucAlAssistAllFlag[dwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbAlAssistAll[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: al_assist need to delete [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else if(pstUserInfo->m_aucAlAssistAllFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            CAwsRequest::PutItem(pstSession, &pstUserInfo->m_atbAlAssistAll[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: al_assist need to put [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else if(pstUserInfo->m_aucAlAssistAllFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbAlAssistAll[dwIdx]);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: al_assist need to update [seq=%u]",
                    pstSession->m_udwSeqNo));
            }
        }
    }

    // 9. tips
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: [tipsnum=%ld] [seq=%u]",
        pstUserInfo->m_udwTipsNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwTipsNum; dwIdx++)
    {
        if(pstUserInfo->m_aucTipsFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            CAwsRequest::PutItem(pstSession, &pstUserInfo->m_atbTips[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetUserUpdtRequest: tips need to update [uid=%ld] [time=%ld] [seq=%u]",
                pstUserInfo->m_atbTips[dwIdx].m_nUid, pstUserInfo->m_atbTips[dwIdx].m_nTime,
                pstSession->m_udwSeqNo));
        }
    }

    // 10. backpack表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &(pstUserInfo->m_tbBackpack));

    //equip crystal soul material parts
    for(TUINT32 dwIdx = 0; dwIdx < pstUserInfo->m_udwEquipNum; ++dwIdx)
    {
        if(pstUserInfo->m_aucEquipFlag[dwIdx] > EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucEquipFlag[dwIdx])
            {
                CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbEquip[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: equip need to be deleted [uid=%u] [equip_id=%ld] [seq=%u]",
                    pstUserInfo->m_atbEquip[dwIdx].m_nUid,
                    pstUserInfo->m_atbEquip[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucEquipFlag[dwIdx])
            {
                CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbEquip[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: equip need to be added [uid=%u] [equip_id=%ld] [seq=%u]",
                    pstUserInfo->m_atbEquip[dwIdx].m_nUid,
                    pstUserInfo->m_atbEquip[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__CHANGE == pstUserInfo->m_aucEquipFlag[dwIdx])
            {
                ExpectedItem expect_item;
                ExpectedDesc expect_desc;
                expect_item.SetVal(TbEQUIP_FIELD_ID, true, pstUserInfo->m_atbEquip[dwIdx].m_nId); //加expect防止明明没有装备修改它！
                expect_desc.clear();
                expect_desc.push_back(expect_item);

                dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbEquip[dwIdx], expect_desc);
                if(0 == dwRetCode)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: equip need to be changed  [uid=%u] [equip_id=%ld] [seq=%u]",
                        pstUserInfo->m_atbEquip[dwIdx].m_nUid,
                        pstUserInfo->m_atbEquip[dwIdx].m_nId, pstSession->m_udwSeqNo));
                }
            }
        }
    }
    /********************************************************************************************/
    // 更新al gift
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [al_gift_num=%u] [seq=%u]",
        pstUserInfo->m_stAlGifts.m_dwGiftNum, pstSession->m_udwSeqNo));
    for(TINT32 dwIdx = 0; dwIdx < pstUserInfo->m_stAlGifts.m_dwGiftNum; ++dwIdx)
    {
        if(pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx] > EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx])
            {
                CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al gift need to be deleted [aid=%ld] [gift id=%ld] [seq=%u]",
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nAid,
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx])
            {
                CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al gift need to be added [aid=%ld] [gift id=%ld] [seq=%u]",
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nAid,
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__CHANGE == pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx])
            {
                ExpectedItem expect_item;
                ExpectedDesc expect_desc;
                expect_item.SetVal(TbAL_GIFT_FIELD_ID, true, pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId); //加expect防止明明没有这个礼包却去修改它！
                expect_desc.clear();
                expect_desc.push_back(expect_item);

                dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx], expect_desc);
                if(0 == dwRetCode)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al gift need to be changed [aid=%ld] [gift id=%ld] [seq=%u]",
                        pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nAid,
                        pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId, pstSession->m_udwSeqNo));
                }
            }
        }
    }
    /********************************************************************************************/
    // quest表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbQuest);
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: quest need to update [uid=%ld] [seq=%u]", \
            pstUserInfo->m_tbQuest.m_nUid, \
            pstSession->m_udwSeqNo));
    }
    // bounty表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbBounty);
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: bounty need to update [uid=%ld] [seq=%u]", \
            pstUserInfo->m_tbBounty.m_nUid, \
            pstSession->m_udwSeqNo));
    }

    //self al_member
    if (pstUserInfo->m_tbSelfAlmember.m_nUid > 0)
    {
        if (EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_ucSelfAlmemberFlag)
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_tbSelfAlmember);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: self al_member need to delete [aid=%ld][uid=%ld] [seq=%u]",
                pstUserInfo->m_tbSelfAlmember.m_nAid, pstUserInfo->m_tbSelfAlmember.m_nUid, pstSession->m_udwSeqNo));
        }
        else
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbSelfAlmember);
            if (0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: self al_member need to update [aid=%ld][uid=%ld] [seq=%u]",
                    pstUserInfo->m_tbSelfAlmember.m_nAid, pstUserInfo->m_tbSelfAlmember.m_nUid, pstSession->m_udwSeqNo));
            }
        }
    }

    for (TINT32 dwIdx = 0; dwIdx < pstUserInfo->m_dwRewardWindowNum; dwIdx++)
    {
        if (pstUserInfo->m_aucRewardWindowFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbRewardWindow[dwIdx]);
            if (0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: reward window need to update[uid: %ld id: %ld] [seq=%u]", 
                    pstUserInfo->m_atbRewardWindow[dwIdx].m_nUid, pstUserInfo->m_atbRewardWindow[dwIdx].m_nId, 
                    pstSession->m_udwSeqNo));
            }
        }
    }

    return 0;
}

TINT32 CTaskProcess::GetCurActionUpdtRequest(SSession *pstSession)
{
    TINT32 dwRetCode = 0;

    if(pstSession->m_stActionTable.dwTableType == EN_UID_ACTION)
    {
        TbAction *pstAction = &pstSession->m_stReqAction;
        if(pstSession->m_ucReqActionFlag == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, pstAction);
            if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("GetCurActionUpdtRequest: update req action[%ld] [seq=%u]",
                    pstAction->m_nId, pstSession->m_udwSeqNo));
            }
        }
        else if(pstSession->m_ucReqActionFlag == EN_TABLE_UPDT_FLAG__DEL)
        {
            dwRetCode = CAwsRequest::DeleteItem(pstSession, pstAction);
            TSE_LOG_DEBUG(m_poServLog, ("GetCurActionUpdtRequest: delete req action[%ld] [seq=%u]",
                pstAction->m_nId, pstSession->m_udwSeqNo));
        }
    }
    else if(pstSession->m_stActionTable.dwTableType == EN_AID_ACTION)
    {
        TbAlliance_action *pstAlAction = &pstSession->m_stReqAlAction;
        if(pstSession->m_ucReqAlActionFlag == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, pstAlAction);
            if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("GetCurActionUpdtRequest: update req action[%ld] [seq=%u]",
                    pstAlAction->m_nId, pstSession->m_udwSeqNo));
            }
        }
        else if(pstSession->m_ucReqAlActionFlag == EN_TABLE_UPDT_FLAG__DEL)
        {
            dwRetCode = CAwsRequest::DeleteItem(pstSession, pstAlAction);
            TSE_LOG_DEBUG(m_poServLog, ("GetCurActionUpdtRequest: delete req action[%ld] [seq=%u]",
                pstAlAction->m_nId, pstSession->m_udwSeqNo));
        }

        //wave@push_data
        CAuPushData::AddPushData_Action(pstSession, &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl, 
            &pstSession->m_objAuPushDataNode.m_objPushDataMap, 
            pstAlAction, EN_AID_ACTION, pstSession->m_ucReqAlActionFlag, TRUE);
    }
    else if(pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
    {
        TbMarch_action *ptbMarch = &pstSession->m_stReqMarch;
        if(pstSession->m_ucReqMarchFlag == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, ptbMarch);
            if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("GetCurActionUpdtRequest: update req action[%ld] [seq=%u]",
                    ptbMarch->m_nId, pstSession->m_udwSeqNo));
            }
        }
        else if(pstSession->m_ucReqMarchFlag == EN_TABLE_UPDT_FLAG__DEL)
        {
            dwRetCode = CAwsRequest::DeleteItem(pstSession, ptbMarch);
            TSE_LOG_DEBUG(m_poServLog, ("GetCurActionUpdtRequest: delete req action[%ld] [seq=%u]",
                ptbMarch->m_nId, pstSession->m_udwSeqNo));
        }

        //wave@push_data
        /*CAuPushData::AddPushData_Action(pstSession, &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl, 
        &pstSession->m_objAuPushDataNode.m_objPushDataMap, 
        ptbMarch, EN_MARCH_ACTION, pstSession->m_ucReqMarchFlag, TRUE);
        CAuPushData::AddPushData_Action(pstSession, &pstSession->m_objAuPushDataNode.m_objPushDataTargetAl, 
        &pstSession->m_objAuPushDataNode.m_objPushDataMap, 
        ptbMarch, EN_MARCH_ACTION, pstSession->m_ucReqMarchFlag, FALSE);*/
        CAuPushData::AddPushData_ReqMarchAction(pstSession, ptbMarch, pstSession->m_ucReqMarchFlag);
    }

    return 0;
}

TINT32 CTaskProcess::GetSessionUpdtRequest(SSession* pstSession)
{
    if(pstSession->m_bNeedRallyHistory)
    {
        for(TUINT32 udwIdx = 0; udwIdx < 2; ++udwIdx)
        {
            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpRallyHistory[udwIdx]);
            if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("GetSessionUpdtRequest: new rally history[%ld] [seq=%u]",
                    pstSession->m_atbTmpRallyHistory[udwIdx].m_nAid, pstSession->m_udwSeqNo));
            }
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpMarchNum; udwIdx++)
    {
        if (pstSession->m_audwTmpMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpMarch[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("GetSessionUpdtRequest: add new tmp action[%ld] [seq=%u]",
                    pstSession->m_atbTmpMarch[udwIdx].m_nId, pstSession->m_udwSeqNo));

                //wave@push_data: todo
            }
        }
        else if (pstSession->m_audwTmpMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbMARCH_ACTION_FIELD_ID, true, pstSession->m_atbTmpMarch[udwIdx].m_nId); //加expect防止边界条件(action已经结束又被新建)
            expect_desc.clear();
            expect_desc.push_back(expect_item);

            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpMarch[udwIdx], expect_desc);
            if (0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetSessionUpdtRequest: tmp action need to update [actionid=%ld] [seq=%u]",
                    pstSession->m_atbTmpMarch[udwIdx].m_nId, pstSession->m_udwSeqNo));

                //wave@push_data: todo
            }
        }
    }
    
    // 7. target map
    if (pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
    {
        if (pstSession->m_ucMapItemFlag == EN_TABLE_UPDT_FLAG__CHANGE
            || pstSession->m_ucMapItemFlag == EN_TABLE_UPDT_FLAG__NEW)
        {
            if (pstSession->m_stMapItem.m_nId != 0)
            {
                pstSession->m_stMapItem.SetFlag(TbMAP_FIELD_SID);
            }
            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stMapItem, ExpectedDesc(), 0, true);
            if (dwRetCode == 0)
            {
                //wave@push_data
                //if(pstSession->m_stRawReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__DRAGON_ATTACK
                //    && (pstSession->m_stRawReqMarch.m_nStatus == EN_MARCH_STATUS__MARCHING || pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__FIGHTING))
                //{
                //    //此种情况不推送地块信息，由客户端来拉取数据
                //    TSE_LOG_DEBUG(m_poServLog, ("[wavetest]GetSessionUpdtRequest[%u]: do not update tpos[%ld] [seq=%u]",
                //        pstSession->m_vecAwsReq.size(), pstSession->m_stMapItem.m_nId, pstSession->m_udwSeqNo));
                //}
                //else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("[wavetest]GetSessionUpdtRequest[%u]: update tpos[%ld] [seq=%u]",
                        pstSession->m_vecAwsReq.size(), pstSession->m_stMapItem.m_nId, pstSession->m_udwSeqNo));
                    CAuPushData::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, &pstSession->m_stMapItem);
                }                
            }
        }
        else if (pstSession->m_stMapItem.m_nId != 0)
        {
            //通过更新去解锁该map
            TbMap tbMap;
            tbMap.Reset();
            tbMap.Set_Id(pstSession->m_stMapItem.m_nId);
            tbMap.Set_Sid(pstSession->m_stMapItem.m_nSid);
            CAwsRequest::UpdateItem(pstSession, &tbMap, ExpectedDesc(), 0, true);
        }
    }
    else
    {
        if (pstSession->m_ucMapItemFlag == EN_TABLE_UPDT_FLAG__CHANGE
            || pstSession->m_ucMapItemFlag == EN_TABLE_UPDT_FLAG__NEW)
        {
            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stMapItem);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(m_poServLog, ("GetSessionUpdtRequest[%u]: update tpos[%ld] [seq=%u]",
                    pstSession->m_vecAwsReq.size(), pstSession->m_stMapItem.m_nId, pstSession->m_udwSeqNo));

                //wave@push_data
                CAuPushData::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, &pstSession->m_stMapItem);
            }
        }
    }

    // 更新map
    if(pstSession->m_ucTmpMapItemFlag == EN_TABLE_UPDT_FLAG__CHANGE && pstSession->m_tbTmpMap.m_nId > 0)
    {
        pstSession->m_tbTmpMap.Set_Sid(pstSession->m_stSourceUser.m_tbPlayer.m_nSid);
        TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap);
        if(dwRetCode > 0)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: not need to update [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: tmp map need to update [seq=%u]", pstSession->m_udwSeqNo));

            //wave@push_data
            CAuPushData::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, &pstSession->m_tbTmpMap);
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_ucIdolFlag[udwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbIdol[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate:idol need to be changed[id=%ld, pos=%ld] [seq=%u]",
                    pstSession->m_atbIdol[udwIdx].m_nId, pstSession->m_atbIdol[udwIdx].m_nPos,
                    pstSession->m_udwSeqNo));
            }
        }
        else if (pstSession->m_ucIdolFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            TINT32 dwRetCode = CAwsRequest::DeleteItem(pstSession, &pstSession->m_atbIdol[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate:idol need to be del[id=%ld, pos=%ld][seq=%u]",
                    pstSession->m_atbIdol[udwIdx].m_nId, pstSession->m_atbIdol[udwIdx].m_nPos,
                    pstSession->m_udwSeqNo));
            }
        }
    }

    if (CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbThrone) == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: throne need to be changed[id=%ld, pos=%ld] [seq=%u]",
            pstSession->m_tbThrone.m_nId, pstSession->m_tbThrone.m_nPos,
            pstSession->m_udwSeqNo));
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stTitleInfoList.udwNum; udwIdx++)
    {
        if (pstSession->m_stTitleInfoList.aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            TINT32 dwRetCode = CAwsRequest::DeleteItem(pstSession, &pstSession->m_stTitleInfoList.atbTitle[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: title [id:%ld] need to del [seq=%u]",
                    pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
        else if (pstSession->m_stTitleInfoList.aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stTitleInfoList.atbTitle[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: title[id:%ld] need to put [seq=%u]",
                    pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
        else if (pstSession->m_stTitleInfoList.aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbTITLE_FIELD_ID, true, pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId);
            expect_desc.clear();
            expect_desc.push_back(expect_item);
            TINT32 dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stTitleInfoList.atbTitle[udwIdx], expect_desc);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: title[id:%ld] need to update [seq=%u]",
                    pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
    }

    return 0;
}

TINT32 CTaskProcess::SyncReqActionToUserInfo(SSession *pstSession)
{
    if(pstSession->m_stSourceUser.m_tbPlayer.m_nUid <= 0)
    {
        return 1;
    }

    if(pstSession->m_stReqAction.m_nId > 0)
    {
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stSourceUser.m_udwActionNum; ++udwIdx)
        {
            if(pstSession->m_stReqAction.m_nId == pstSession->m_stSourceUser.m_atbAction[udwIdx].m_nId)
            {
                pstSession->m_stSourceUser.m_atbAction[udwIdx] = pstSession->m_stReqAction;
                pstSession->m_stSourceUser.m_aucActionFlag[udwIdx] = pstSession->m_ucReqActionFlag;
            }
        }
    }

    if(pstSession->m_stReqAlAction.m_nId > 0)
    {
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stSourceUser.m_udwSelfAlActionNum; ++udwIdx)
        {
            if(pstSession->m_stReqAlAction.m_nId == pstSession->m_stSourceUser.m_atbSelfAlAction[udwIdx].m_nId)
            {
                pstSession->m_stSourceUser.m_atbSelfAlAction[udwIdx] = pstSession->m_stReqAlAction;
                pstSession->m_stSourceUser.m_aucSelfAlActionFlag[udwIdx] = pstSession->m_ucReqAlActionFlag;
            }
        }
    }

    if(pstSession->m_stReqMarch.m_nId > 0)
    {
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stSourceUser.m_udwMarchNum; ++udwIdx)
        {
            if(pstSession->m_stReqMarch.m_nId == pstSession->m_stSourceUser.m_atbMarch[udwIdx].m_nId)
            {
                pstSession->m_stSourceUser.m_atbMarch[udwIdx] = pstSession->m_stReqMarch;
                pstSession->m_stSourceUser.m_aucMarchFlag[udwIdx] = pstSession->m_ucReqMarchFlag;
            }
        }
    }

    return 0;
}

TINT32 CTaskProcess::GenReportIdRequest(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    TINT32 dwNewReportCount = 0;
    if(pstSession->m_dwReportFlag & EN_REPORT_FLAG_NORMAL)
    {
        dwNewReportCount++;
    }
    if(pstSession->m_dwReportFlag & EN_REPORT_FLAG_PRISON)
    {
        dwNewReportCount++;
    }

    if (pstSession->m_dwReportFlag & EN_REPORT_FLAG_DRAGON_ATTACK)
    {
        dwNewReportCount++;

        if (pstSession->m_dwReportFlag & EN_REPORT_FLAG_CHALLENGER_REWARD)
        {
            dwNewReportCount++;
        }
    }

    if (pstSession->m_vecAlReport.size() > 0)
    {
        dwNewReportCount += pstSession->m_vecAlReport.size();
    }

    CAwsRequest::GlobalParamNewIdUpdtAndGet(pstSession, 0, dwNewReportCount, EN_GLOBAL_PARAM__REPORT_ID);
    // send req
    dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    if(dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
    }
    return dwRetCode;
}

TINT32 CTaskProcess::InsertReport(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    TINT32 ddwReportId = pstSession->m_tbReportId.m_nVal;
    if(pstSession->m_dwReportFlag == 0)
    {
        return 1;
    }
    else
    {
        pstSession->ResetAwsInfo();
        pstSession->ResetReportInfo();
        if(pstSession->m_dwReportFlag & EN_REPORT_FLAG_NORMAL)
        {
            TINT64 ddwNormalReportId = ddwReportId--;

            pstSession->m_tbReport.Set_Id(ddwNormalReportId);
            TSE_LOG_INFO(m_poServLog, ("Report:send report [new_report_id=%ld][main=%ld] [seq=%u]",
                pstSession->m_tbReport.m_nId, pstSession->m_stReqMarch.m_nMclass, pstSession->m_udwSeqNo));

            CAwsRequest::PutItem(pstSession, &pstSession->m_tbReport);

            // 5. insert user and alliance report info
            if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS && pstSession->m_dwReportFlag)
            {
                GenReportRequest(pstSession);

                if(pstSession->m_bNeedRallyHistory)
                {
                    for(TUINT32 udwIdx = 0; udwIdx < 2; ++udwIdx)
                    {
                        pstSession->m_atbTmpRallyHistory[udwIdx].Set_Rid(ddwNormalReportId);
                    }
                }
            }
        }

        if(pstSession->m_dwReportFlag & EN_REPORT_FLAG_PRISON)
        {
            TINT64 ddwPrisonReportId = ddwReportId--;
            pstSession->m_tbPrisonReport.Set_Id(ddwPrisonReportId);
            TSE_LOG_INFO(m_poServLog, ("Report:send prison [new_report_id=%ld][main=%ld] [seq=%u]",
                pstSession->m_tbPrisonReport.m_nId, pstSession->m_stReqMarch.m_nMclass, pstSession->m_udwSeqNo));

            CAwsRequest::PutItem(pstSession, &pstSession->m_tbPrisonReport);

            for(set<TINT64>::iterator it = pstSession->m_vecPrisonReportReceivers.begin(); it != pstSession->m_vecPrisonReportReceivers.end(); ++it)
            {
                CReportSvrRequest::UserReportPut(pstSession, *it, pstSession->m_tbPrisonReport.m_nId, pstSession->m_tbPrisonReport.m_nType, EN_ALLIANCE_REPORT_GET_TYPE__ALL);
            }
        }

        if (pstSession->m_dwReportFlag & EN_REPORT_FLAG_DRAGON_ATTACK)
        {
            if (pstSession->m_dwReportFlag & EN_REPORT_FLAG_CHALLENGER_REWARD)
            {
                TINT64 ddwRewardReportId = ddwReportId--;
                pstSession->m_tbChallengerReport.Set_Id(ddwRewardReportId);
                TSE_LOG_INFO(m_poServLog, ("Report:send report [new_report_id=%ld][main=%ld] [seq=%u]",
                    pstSession->m_tbChallengerReport.m_nId, pstSession->m_stReqMarch.m_nMclass, pstSession->m_udwSeqNo));

                CAwsRequest::PutItem(pstSession, &pstSession->m_tbChallengerReport);
                pstSession->m_stReqMarch.m_bMonster_info[0].ddwRid = ddwRewardReportId;
                pstSession->m_stReqMarch.SetFlag(TbMARCH_ACTION_FIELD_MONSTER_INFO);
            }

            TINT64 ddwNormalReportId = ddwReportId--;
            pstSession->m_tbReport.Set_Id(ddwNormalReportId);
            TSE_LOG_INFO(m_poServLog, ("Report:send report [new_report_id=%ld][main=%ld] [seq=%u]",
                pstSession->m_tbReport.m_nId, pstSession->m_stReqMarch.m_nMclass, pstSession->m_udwSeqNo));

            CAwsRequest::PutItem(pstSession, &pstSession->m_tbReport);
            pstSession->m_stReqMarch.Set_Delay_report_id(ddwNormalReportId);
            pstSession->m_stReqMarch.Set_Delay_report_id(ddwNormalReportId);
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAlReport.size(); udwIdx++)
        {
            CAwsRequest::PutItem(pstSession, &pstSession->m_vecAlReport[udwIdx]);
            CReportSvrRequest::UserReportPut(pstSession, pstSession->m_vecReportAlReceiver[udwIdx] * -1, 
                pstSession->m_tbPrisonReport.m_nId, pstSession->m_tbPrisonReport.m_nType, EN_ALLIANCE_REPORT_GET_TYPE__ALL);
        }

        // send req
        dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        }
    }
    return dwRetCode;
}

TVOID CTaskProcess::GenReportRequest(SSession *pstSession)
{
    ostringstream oss;
    oss.str("");
    TbReport *ptbReport = &pstSession->m_tbReport;

    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stSourceUser.m_udwMailSendNum; ++udwIdx)
    {
        CReportSvrRequest::UserReportPut(pstSession, pstSession->m_stSourceUser.m_adwMailSendUidList[udwIdx],
            ptbReport->m_nId, ptbReport->m_nType, EN_ALLIANCE_REPORT_GET_TYPE__OUT);
        oss << pstSession->m_stSourceUser.m_adwMailSendUidList[udwIdx] << ",";
    }
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stTargetUser.m_udwMailSendNum; ++udwIdx)
    {
        CReportSvrRequest::UserReportPut(pstSession, pstSession->m_stTargetUser.m_adwMailSendUidList[udwIdx],
            ptbReport->m_nId, ptbReport->m_nType, EN_ALLIANCE_REPORT_GET_TYPE__IN);
        oss << pstSession->m_stTargetUser.m_adwMailSendUidList[udwIdx] << ",";
    }
    TSE_LOG_INFO(pstSession->m_poServLog, ("GenReportRequest: [receiver_ids=%s][seq=%u]", oss.str().c_str(), pstSession->m_udwSeqNo));
}

TVOID CTaskProcess::GenUidAcitonHourLog(SSession *pstSession, TUINT64 uddwCostTime, TUINT32 udwKey0, TUINT32 udwKey1, string strGameEvaluateLog)
{
    TCHAR *pCur = m_szLogBuf;
    TUINT32 udwCurLen = 0;

    TINT64 ddwGemAdd = 0;
    if(pstSession->m_stSourceUser.m_tbLogin.m_nUid > 0)
    {
        ddwGemAdd = pstSession->m_stSourceUser.m_tbLogin.m_nGem - pstSession->m_ddwGemBegin;
    }

    udwCurLen = sprintf(pCur, "svr[%u],action[%ld],report[%ld],main=%ld,sec_raw=%u,sec_res=%ld,status_raw=%u,"
        "status_res=%ld,suid=%ld,scid=%ld,tuid=%u,tpos=%d,btime=%ld,etime=%ld,ctime=%ld,twildtype=%u",
        pstSession->m_udwReqSvrId, pstSession->m_stReqAction.m_nId, pstSession->m_tbReport.m_nId,
        pstSession->m_stReqAction.m_nMclass, pstSession->m_ucActionRawSecClass, pstSession->m_stReqAction.m_nSclass,
        pstSession->m_ucActionRawStatus, pstSession->m_stReqAction.m_nStatus,
        pstSession->m_stReqAction.m_nSuid, pstSession->m_stReqAction.m_nScid,
        pstSession->m_udwRawTuid, 0,
        pstSession->m_stReqAction.m_nBtime, pstSession->m_stReqAction.m_nEtime,
        pstSession->m_stReqAction.m_nCtime, 0u);
    pCur += udwCurLen;

    // might, troop and fort
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    TUINT64 uddwLastMight_S = pstSession->m_stSourceUser.m_uddwLastMight;
    TUINT64 uddwLastMight_T = pstSession->m_stTargetUser.m_uddwLastMight;

    string sLastJson_S = pstSession->m_stSourceUser.m_sLastTroopAndFort;
    string sLastJson_T = pstSession->m_stTargetUser.m_sLastTroopAndFort;

    TUINT64 uddwNowMight_S = pstSession->m_stSourceUser.m_uddwCurMight;
    TUINT64 uddwNowMight_T = pstSession->m_stTargetUser.m_uddwCurMight;

    string sNowJson_S = pstSession->m_stSourceUser.m_sCurTroopAndFort;
    string sNowJson_T = pstSession->m_stTargetUser.m_sCurTroopAndFort;

    TSE_LOG_HOUR(CGlobalServ::m_poReqLog, ("finish_process: %s,flag=%u,lv=%ld,uname=%s,cid=%ld,ret=%d,cost[%lu],key0=%u,key1=%u,key3=%s"
        " [seq=%u],newid=%lu,newid_t=%lu,gem_add=%ld,game_evaluate[%s],aws_read_cost[%lu],aws_write_cost[%lu],aws_read_write_cost[%lu],aws_no_op_cost[%lu],db_read_cost[%lu],"
        "db_write_cost[%lu],db_read_write_cost[%lu],db_no_op_cost[%lu],lock_cost[%lu],now_might_s[%lu],change_s[%ld],last_json_s[%s],now_json_s[%s],"
        "now_might_t[%lu],change_t[%ld],last_json_t[%s],now_json_t[%s]",
        m_szLogBuf, pstSession->m_ucReqActionFlag, pstSession->m_stSourceUser.m_tbPlayer.m_nLevel,
        pstSession->m_stSourceUser.m_tbPlayer.m_sUin.c_str(), pstSession->m_stSourceUser.m_tbPlayer.m_nCid,
        pstSession->m_stCommonResInfo.m_dwRetCode, uddwCostTime, udwKey0, udwKey1, "", pstSession->m_udwSeqNo,
        pstSession->m_stSourceUser.m_uddwNewActionId, pstSession->m_stTargetUser.m_uddwNewActionId, ddwGemAdd, strGameEvaluateLog.c_str(),
        pstSession->m_uddwAwsReadSumTime,
        pstSession->m_uddwAwsWriteSumTime,
        pstSession->m_uddwAwsReadWriteSumTime,
        pstSession->m_uddwAwsNoOpSumTime,
        pstSession->m_uddwDbReadSumTime,
        pstSession->m_uddwDbWriteSumTime,
        pstSession->m_uddwDbReadWriteSumTime,
        pstSession->m_uddwDbNoOpSumTime,
        pstSession->m_uddwLockSumTime,
        uddwNowMight_S, (TINT64)uddwNowMight_S - (TINT64)uddwLastMight_S, sLastJson_S.c_str(), sNowJson_S.c_str(),
        uddwNowMight_T, (TINT64)uddwNowMight_T - (TINT64)uddwLastMight_T, sLastJson_T.c_str(), sNowJson_T.c_str()));
}

TVOID CTaskProcess::GenAidAcitonHourLog(SSession *pstSession, TUINT64 uddwCostTime, TUINT32 udwKey0, TUINT32 udwKey1, string strGameEvaluateLog)
{
    TCHAR *pCur = m_szLogBuf;
    TUINT32 udwCurLen = 0;

    TINT64 ddwGemAdd = 0;
    if(pstSession->m_stSourceUser.m_tbLogin.m_nUid > 0)
    {
        ddwGemAdd = pstSession->m_stSourceUser.m_tbLogin.m_nGem - pstSession->m_ddwGemBegin;
    }

    udwCurLen = sprintf(pCur, "svr[%u],action[%ld],report[%ld],main=%ld,sec_raw=%u,sec_res=%ld,status_raw=%u,"
        "status_res=%ld,suid=%ld,scid=%ld,tuid=%u,tpos=%d,btime=%ld,etime=%ld,ctime=%ld,twildtype=%u",
        pstSession->m_udwReqSvrId, pstSession->m_stReqAlAction.m_nId, pstSession->m_tbReport.m_nId,
        pstSession->m_stReqAlAction.m_nMclass, pstSession->m_ucActionRawSecClass, pstSession->m_stReqAlAction.m_nSclass,
        pstSession->m_ucActionRawStatus, pstSession->m_stReqAlAction.m_nStatus,
        pstSession->m_stReqAlAction.m_nSuid, pstSession->m_stReqAlAction.m_nScid,
        pstSession->m_udwRawTuid, 0,
        pstSession->m_stReqAlAction.m_nBtime, pstSession->m_stReqAlAction.m_nEtime,
        pstSession->m_stReqAlAction.m_nCtime, 0);
    pCur += udwCurLen;

    // might, troop and fort
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    TUINT64 uddwLastMight_S = pstSession->m_stSourceUser.m_uddwLastMight;
    TUINT64 uddwLastMight_T = pstSession->m_stTargetUser.m_uddwLastMight;

    string sLastJson_S = pstSession->m_stSourceUser.m_sLastTroopAndFort;
    string sLastJson_T = pstSession->m_stTargetUser.m_sLastTroopAndFort;

    TUINT64 uddwNowMight_S = pstSession->m_stSourceUser.m_uddwCurMight;
    TUINT64 uddwNowMight_T = pstSession->m_stTargetUser.m_uddwCurMight;

    string sNowJson_S = pstSession->m_stSourceUser.m_sCurTroopAndFort;
    string sNowJson_T = pstSession->m_stTargetUser.m_sCurTroopAndFort;

    TSE_LOG_HOUR(CGlobalServ::m_poReqLog, ("finish_process: %s,flag=%u,lv=%ld,uname=%s,cid=%ld,ret=%d,cost[%lu],key0=%u,key1=%u,key3=%s"
        " [seq=%u],newid=%lu,newid_t=%lu,gem_add=%ld,game_evaluate[%s],aws_read_cost[%lu],aws_write_cost[%lu],aws_read_write_cost[%lu],aws_no_op_cost[%lu],db_read_cost[%lu],"
        "db_write_cost[%lu],db_read_write_cost[%lu],db_no_op_cost[%lu],lock_cost[%lu],now_might_s[%lu],change_s[%ld],last_json_s[%s],now_json_s[%s],"
        "now_might_t[%lu],change_t[%ld],last_json_t[%s],now_json_t[%s]",
        m_szLogBuf, pstSession->m_ucReqAlActionFlag, pstSession->m_stSourceUser.m_tbPlayer.m_nLevel,
        pstSession->m_stSourceUser.m_tbPlayer.m_sUin.c_str(), pstSession->m_stSourceUser.m_tbPlayer.m_nCid,
        pstSession->m_stCommonResInfo.m_dwRetCode, uddwCostTime, udwKey0, udwKey1, "", pstSession->m_udwSeqNo,
        pstSession->m_stSourceUser.m_uddwNewActionId, pstSession->m_stTargetUser.m_uddwNewActionId, ddwGemAdd, strGameEvaluateLog.c_str(),
        pstSession->m_uddwAwsReadSumTime,
        pstSession->m_uddwAwsWriteSumTime,
        pstSession->m_uddwAwsReadWriteSumTime,
        pstSession->m_uddwAwsNoOpSumTime,
        pstSession->m_uddwDbReadSumTime,
        pstSession->m_uddwDbWriteSumTime,
        pstSession->m_uddwDbReadWriteSumTime,
        pstSession->m_uddwDbNoOpSumTime,
        pstSession->m_uddwLockSumTime,
        uddwNowMight_S, (TINT64)uddwNowMight_S - (TINT64)uddwLastMight_S, sLastJson_S.c_str(), sNowJson_S.c_str(),
        uddwNowMight_T, (TINT64)uddwNowMight_T - (TINT64)uddwLastMight_T, sLastJson_T.c_str(), sNowJson_T.c_str()));
}

TVOID CTaskProcess::GenSidAcitonHourLog(SSession *pstSession, TUINT64 uddwCostTime, TUINT32 udwKey0, TUINT32 udwKey1, string strGameEvaluateLog)
{
    TCHAR *pCur = m_szLogBuf;
    TUINT32 udwCurLen = 0;

    TINT64 ddwGemAdd = 0;
    if(pstSession->m_stSourceUser.m_tbLogin.m_nUid > 0)
    {
        ddwGemAdd = pstSession->m_stSourceUser.m_tbLogin.m_nGem - pstSession->m_ddwGemBegin;
    }    

    udwCurLen = sprintf(pCur, "svr[%u],action[%ld],report[%ld],main=%ld,sec_raw=%u,sec_res=%ld,status_raw=%u,"
        "status_res=%ld,suid=%ld,scid=%ld,tuid=%u,tpos=%ld,btime=%ld,etime=%ld,ctime=%ld,twildtype=%ld",
        pstSession->m_udwReqSvrId, pstSession->m_stReqMarch.m_nId, pstSession->m_tbReport.m_nId,
        pstSession->m_stReqMarch.m_nMclass, pstSession->m_ucActionRawSecClass, pstSession->m_stReqMarch.m_nSclass,
        pstSession->m_ucActionRawStatus, pstSession->m_stReqMarch.m_nStatus,
        pstSession->m_stReqMarch.m_nSuid, pstSession->m_stReqMarch.m_nScid,
        pstSession->m_udwRawTuid, pstSession->m_stReqMarch.m_nTpos,
        pstSession->m_stReqMarch.m_nBtime, pstSession->m_stReqMarch.m_nEtime,
        pstSession->m_stReqMarch.m_nCtime, pstSession->m_stReqMarch.m_bParam[0].m_ddwTargetType);
    pCur += udwCurLen;

    // might, troop and fort
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    TUINT64 uddwLastMight_S = pstSession->m_stSourceUser.m_uddwLastMight;
    TUINT64 uddwLastMight_T = pstSession->m_stTargetUser.m_uddwLastMight;

    string sLastJson_S = pstSession->m_stSourceUser.m_sLastTroopAndFort;
    string sLastJson_T = pstSession->m_stTargetUser.m_sLastTroopAndFort;

    TUINT64 uddwNowMight_S = pstSession->m_stSourceUser.m_uddwCurMight;
    TUINT64 uddwNowMight_T = pstSession->m_stTargetUser.m_uddwCurMight;

    string sNowJson_S = pstSession->m_stSourceUser.m_sCurTroopAndFort;
    string sNowJson_T = pstSession->m_stTargetUser.m_sCurTroopAndFort;

    TSE_LOG_HOUR(CGlobalServ::m_poReqLog, ("finish_process: %s,flag=%u,lv=%ld,uname=%s,cid=%ld,ret=%d,cost[%lu],key0=%u,key1=%u,key3=%s"
        " [seq=%u],newid=%lu,newid_t=%lu,gem_add=%ld,game_evaluate[%s],aws_read_cost[%lu],aws_write_cost[%lu],aws_read_write_cost[%lu],aws_no_op_cost[%lu],db_read_cost[%lu],"
        "db_write_cost[%lu],db_read_write_cost[%lu],db_no_op_cost[%lu],lock_cost[%lu],now_might_s[%lu],change_s[%ld],last_json_s[%s],now_json_s[%s],"
        "now_might_t[%lu],change_t[%ld],last_json_t[%s],now_json_t[%s]",
        m_szLogBuf, pstSession->m_ucReqMarchFlag, pstSession->m_stSourceUser.m_tbPlayer.m_nLevel,
        pstSession->m_stSourceUser.m_tbPlayer.m_sUin.c_str(), pstSession->m_stSourceUser.m_tbPlayer.m_nCid,
        pstSession->m_stCommonResInfo.m_dwRetCode, uddwCostTime, udwKey0, udwKey1, "", pstSession->m_udwSeqNo,
        pstSession->m_stSourceUser.m_uddwNewActionId, pstSession->m_stTargetUser.m_uddwNewActionId, ddwGemAdd, strGameEvaluateLog.c_str(),
        pstSession->m_uddwAwsReadSumTime,
        pstSession->m_uddwAwsWriteSumTime,
        pstSession->m_uddwAwsReadWriteSumTime,
        pstSession->m_uddwAwsNoOpSumTime,
        pstSession->m_uddwDbReadSumTime,
        pstSession->m_uddwDbWriteSumTime,
        pstSession->m_uddwDbReadWriteSumTime,
        pstSession->m_uddwDbNoOpSumTime,
        pstSession->m_uddwLockSumTime,
        uddwNowMight_S, (TINT64)uddwNowMight_S - (TINT64)uddwLastMight_S, sLastJson_S.c_str(), sNowJson_S.c_str(),
        uddwNowMight_T, (TINT64)uddwNowMight_T - (TINT64)uddwLastMight_T, sLastJson_T.c_str(), sNowJson_T.c_str()));
}

TVOID CTaskProcess::GenFormatLog(SSession *pstSession, TUINT64 uddwCostTime)
{
    TCHAR *pCur = m_szLogBuf;
    TUINT32 udwCurLen = 0;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    //const TCHAR *pszCommand = NULL;
    TbAction *ptbAction = &pstSession->m_stReqAction;
    SReqParam *pstReqParam = &pstSession->m_stReqParam;

    SUserInfo *pstUser = NULL;
    TbPlayer *ptbPlayer = NULL;
    SCityInfo *pstCity = NULL;
    TbCity *ptbCity = NULL;

    // 0. cmd&keys
    GetReqActionCmdAndKeys(pstSession);
    if(!pstReqParam->m_BIsNeedCb)
    {
        return;
    }

    if(pstReqParam->m_ucIsSuid)
    {
        pstUser = &pstSession->m_stSourceUser;
        ptbPlayer = &pstUser->m_tbPlayer;
		pstCity = &pstUser->m_stCityInfo;
    }
    else
    {
        pstUser = &pstSession->m_stTargetUser;
        ptbPlayer = &pstUser->m_tbPlayer;
		pstCity = &pstUser->m_stCityInfo;
    }

    if(NULL == pstCity)
    {
        pstCity = &pstUser->m_stCityInfo;
    }
    ptbCity = &pstCity->m_stTblData;

    // 1. head & keys
    udwCurLen = sprintf(pCur, "\t%f\t%u\t%s\t%s\t%ld\t%u\t%ld\t%s",
        1.0, udwCurTime, "", pstReqParam->m_szCommand,
        ptbPlayer->m_nUid, pstSession->m_udwReqSvrId,
        pstCity->m_stTblData.m_nPos, pstReqParam->m_szKey);
    pCur += udwCurLen;

    // 2. seqno\ret_code\timecost\gid\aid\sflag
    TINT32 dwAid = ptbPlayer->m_nAlpos ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
    udwCurLen = sprintf(pCur, "\t%u\t%d\t%lu\t%s\t%d\t%d",
        pstSession->m_udwSeqNo, pstSession->m_stCommonResInfo.m_dwRetCode,
        uddwCostTime, "", dwAid, 2);
    pCur += udwCurLen;

    // 3. options
    udwCurLen = sprintf(pCur, "\t%ld|%u|%d|%ld|%ld|%s", pstSession->m_tbReport.m_nId, pstSession->m_ucReqActionFlag,
        pstSession->m_dwMarchResult, ptbAction->m_nId, ptbPlayer->m_nNpc, pstUser->m_tbLogin.m_sPlatform.c_str());
    pCur += udwCurLen;

    // level
    udwCurLen = sprintf(pCur, "\t%ld", ptbPlayer->m_nLevel);
    pCur += udwCurLen;

    // force
    udwCurLen = sprintf(pCur, "\t%ld|%ld|%ld|%ld|%ld|%d|%d",
        ptbPlayer->m_nMight,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum,
        0,
        0);
    pCur += udwCurLen;

    // gem
    TUINT32 udwGemLoad = 0;
    if(ptbAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH)
    {
        //TODO del
        //udwGemLoad = ptbAction->m_bParam[0].m_stMarch.m_udwLoadGem;
    }
    udwCurLen = sprintf(pCur, "\t|||||%u", udwGemLoad);
    pCur += udwCurLen;

    // basic
    udwCurLen = sprintf(pCur, "\t%s|%s|%s|%ld|%ld|%u|%u|%u|%d|%d|%ld|%ld|%ld|%u",
        ptbPlayer->m_sUin.c_str(), ptbCity->m_sName.c_str(), ptbPlayer->m_sAlname.c_str(),
        ptbPlayer->m_nCtime, ptbPlayer->m_nUtime,
        0, 100,
        0, 0,
        0, ptbPlayer->m_nStatus,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_POPULATION_CAPICATY].m_ddwBuffTotal,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HOSPITAL_CAPICATY].m_ddwBuffTotal,
        0);
    pCur += udwCurLen;

    // log
    TSE_LOG_HOUR(CGlobalServ::m_poFormatLog, ("\t%s", m_szLogBuf));

    // send to dc
    CDcUdpLog::SendFormatLogToDc(m_szLogBuf, CGlobalServ::m_poServLog);
}

TINT32 CTaskProcess::UpdateAndCheckActionInfo(SSession *pstSession, TbMap *pstWild, SUserInfo *pstSource, SUserInfo *pstTarget)
{
    CActionBase::CheckSeq(pstSource);
    CActionBase::SyncDragonStatus(pstSource);
    CActionBase::SyncDragonStatus(pstTarget);

    if(pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
    {
        TbMarch_action *ptbMarch = &pstSession->m_stReqMarch;
        SActionMarchParam *pstMarchParam = &ptbMarch->m_bParam[0];

        if(pstSession->m_stMapItem.m_nId && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH)
        {
			TBOOL bValidWild = TRUE;

            // 容错处理
            if(pstWild->m_nUid && pstTarget->m_tbPlayer.m_nUid && pstWild->m_nUid == pstTarget->m_tbPlayer.m_nUid)
            {
				if(pstWild->m_nType == EN_WILD_TYPE__CITY)//wave@20160206: 如果目标城市是city，而且在目标城市信息当中找不到，则可判定为是异常数据，打印日志待后续进行恢复即可
				{
                    if (pstTarget->m_stCityInfo.m_stTblData.m_nPos != pstWild->m_nId)
					{
						TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("UpdateAndCheckActionInfo---target: sid=%ld wild:pos=%ld,type=%ld,uid=%ld; not exist in cityinfo, need to abandon wild [seq=%u]",
							pstWild->m_nSid, pstWild->m_nId, pstWild->m_nType, pstWild->m_nUid, pstSession->m_udwSeqNo));
						bValidWild = FALSE;
						pstSession->m_bCheckValidFlag = FALSE;
                        pstSession->m_ddwCheckUid = pstWild->m_nUid;
					}
				}
				if(bValidWild == TRUE)
				{
					if(pstTarget->m_tbPlayer.m_nAlpos)
					{
						if(pstWild->m_nAlid != pstTarget->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
						{
							pstWild->Set_Alid(pstTarget->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
							pstWild->Set_Alname(pstTarget->m_tbPlayer.m_sAlname);
                            pstWild->Set_Al_nick(pstTarget->m_tbPlayer.m_sAl_nick_name);
							pstWild->Set_Name_update_time(CTimeUtils::GetUnixTime());
						}
					}
					else
					{
						if(pstWild->m_nAlid != 0)
						{
							pstWild->Set_Alid(0);
							pstWild->Set_Alname("");
                            pstWild->Set_Al_nick("");
							pstWild->Set_Name_update_time(CTimeUtils::GetUnixTime());
						}
					}
				}
            }

            if(pstWild->m_nUid == ptbMarch->m_nTuid && bValidWild == TRUE)
            {
                // update action info
                strcpy(pstMarchParam->m_szTargetCityName, pstWild->m_sName.c_str());

                if(pstWild->m_nType == EN_WILD_TYPE__CITY)
                {
                    pstMarchParam->m_ddwTargetCityId = pstWild->m_nId;
                }
                else
                {
                    pstMarchParam->m_ddwTargetCityId = pstWild->m_nCid;
                }
                pstMarchParam->m_ddwTargetLevel = pstWild->m_nLevel;
                pstMarchParam->m_ddwTargetAlliance = pstWild->m_nAlid;
                strcpy(pstMarchParam->m_szTargetAlliance, pstWild->m_sAlname.c_str());
                if(pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos)
                {
                    pstMarchParam->m_ddwSourceAlliance = pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                }
                else
                {
                    pstMarchParam->m_ddwSourceAlliance = 0;
                }
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

                pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        if(ptbMarch->m_nSuid > 0)
        {
            TUINT32 udwScid = ptbMarch->m_nScid;
            SCityInfo* pstCity = &pstSource->m_stCityInfo;
            if(pstCity->m_stTblData.m_nPos != udwScid)
            {
                if ((ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                    || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
                    && (ptbMarch->m_nStatus == EN_MARCH_STATUS__PREPARING
                    || ptbMarch->m_nStatus == EN_MARCH_STATUS__WAITING))
                {
                    TSE_LOG_INFO(CGlobalServ::m_poServLog, ("UpdateAndCheckActionInfo: rally war prepare, scid changed [seq=%u]",
                        pstSession->m_udwSeqNo));
                }
				else if(pstCity->m_stTblData.m_nPos)
				{
					ptbMarch->Set_Scid(pstCity->m_stTblData.m_nPos);
					pstMarchParam->m_ddwSourceCityId = pstCity->m_stTblData.m_nPos;
					strncpy(pstMarchParam->m_szSourceCityName, pstCity->m_stTblData.m_sName.c_str(), MAX_TABLE_NAME_LEN - 1);
					pstMarchParam->m_szSourceCityName[MAX_TABLE_NAME_LEN - 1] = 0;
					ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
					pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
				}
				else
				{
					TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("UpdateAndCheckActionInfo---source city: sid=%ld suid=%ld have no cityinfo, need to check [seq=%u]",
						pstSession->m_stReqAction.m_nSid, pstSession->m_stReqAction.m_nSuid, pstSession->m_udwSeqNo));
				}
            }
        }
    }

    return 0;
}

TINT32 CTaskProcess::AbNormalActionHandle(SSession *pstSession)
{
    TINT32 dwRetCode = 1;

    if(pstSession->m_stActionTable.dwTableType == EN_UID_ACTION)
    {
        TbAction* ptbAction = &pstSession->m_stRawReqAction;
        ptbAction->Set_Retry(ptbAction->m_nRetry + 1);

        TINT64 ddwRetryTime = 120 * (ptbAction->m_nRetry + 1)*ptbAction->m_nRetry / 2; //wave@20140107:重试时间间隔算法
        // Mon Oct  5 10:38:47 UTC 2020
        if(1601894327 > ptbAction->m_nEtime)
        {
            ptbAction->Set_Etime(ptbAction->m_nEtime + ddwRetryTime);
        }

        TSE_LOG_ERROR(CGlobalServ::m_pAbnormalActionLog, ("abnormal action hande: svr[%u],action[%ld],main=%ld,sec=%ld,status=%ld,suid=%ld,"
            "scid=%ld,btime=%lld,etime=%ld,retry=%ld [seq=%u]",
            pstSession->m_udwReqSvrId, ptbAction->m_nId, ptbAction->m_nMclass, ptbAction->m_nSclass,
            ptbAction->m_nStatus, ptbAction->m_nSuid, ptbAction->m_nScid,
            ptbAction->m_nBtime, ptbAction->m_nEtime, ptbAction->m_nRetry, pstSession->m_udwSeqNo));

        //告警
        if(ptbAction->m_nRetry > 2)
        {
            TCHAR szCmd[128];
            sprintf(szCmd, "./send_warning.sh \"bob find abnormal action=%ld,uid=%ld,sclass=%ld,ret=%d,retry=%ld!!!\" retry",
                ptbAction->m_nId,
                ptbAction->m_nSuid,
                ptbAction->m_nSclass,
                pstSession->m_stCommonResInfo.m_dwRetCode,
                ptbAction->m_nRetry);
            system(szCmd);
        }

        ExpectedDesc expect_desc;
        ExpectedItem expect_item;
        expect_item.SetVal(TbACTION_FIELD_ID, true, ptbAction->m_nId); //加expect防止边界条件(action已经结束又被新建)
        expect_desc.push_back(expect_item);
        pstSession->m_vecAwsReq.clear();
        CAwsRequest::UpdateItem(pstSession, ptbAction, expect_desc);
        if(pstSession->m_vecAwsReq.size() > 0)
        {
            dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        }
    }
    else if (pstSession->m_stActionTable.dwTableType == EN_AID_ACTION)
    {
        TbAlliance_action* ptbAlAction = &pstSession->m_stRawReqAlAction;
        ptbAlAction->Set_Retry(ptbAlAction->m_nRetry + 1);

        TINT64 ddwRetryTime = 120 * (ptbAlAction->m_nRetry + 1)*ptbAlAction->m_nRetry / 2; //wave@20140107:重试时间间隔算法
        // Mon Oct  5 10:38:47 UTC 2020
        if(1601894327 > ptbAlAction->m_nEtime)
        {
            ptbAlAction->Set_Etime(ptbAlAction->m_nEtime + ddwRetryTime);
        }

        TSE_LOG_ERROR(CGlobalServ::m_pAbnormalActionLog, ("abnormal action hande: svr[%u],action[%ld],main=%ld,sec=%ld,status=%ld,suid=%ld,"
            "scid=%ld,btime=%lld,etime=%ld,retry=%ld [seq=%u]",
            pstSession->m_udwReqSvrId, ptbAlAction->m_nId, ptbAlAction->m_nMclass, ptbAlAction->m_nSclass,
            ptbAlAction->m_nStatus, ptbAlAction->m_nSuid, ptbAlAction->m_nScid,
            ptbAlAction->m_nBtime, ptbAlAction->m_nEtime, ptbAlAction->m_nRetry, pstSession->m_udwSeqNo));

        //告警
        if(ptbAlAction->m_nRetry > 2)
        {
            TCHAR szCmd[128];
            sprintf(szCmd, "./send_warning.sh \"bob find abnormal action=%ld,uid=%ld,sclass=%ld,ret=%d,retry=%ld!!!\" retry",
                ptbAlAction->m_nId,
                ptbAlAction->m_nSuid,
                ptbAlAction->m_nSclass,
                pstSession->m_stCommonResInfo.m_dwRetCode,
                ptbAlAction->m_nRetry);
            system(szCmd);
        }

        ExpectedDesc expect_desc;
        ExpectedItem expect_item;
        expect_item.SetVal(TbALLIANCE_ACTION_FIELD_ID, true, ptbAlAction->m_nId); //加expect防止边界条件(action已经结束又被新建)
        expect_desc.push_back(expect_item);
        pstSession->m_vecAwsReq.clear();
        CAwsRequest::UpdateItem(pstSession, ptbAlAction, expect_desc);
        if(pstSession->m_vecAwsReq.size() > 0)
        {
            dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        }
    }
    else if (pstSession->m_stActionTable.dwTableType == EN_MARCH_ACTION)
    {
        TbMarch_action* ptbMarch = &pstSession->m_stRawReqMarch;
        ptbMarch->Set_Retry(ptbMarch->m_nRetry + 1);

        TINT64 ddwRetryTime = 120 * (ptbMarch->m_nRetry + 1)*ptbMarch->m_nRetry / 2; //wave@20140107:重试时间间隔算法
        // Mon Oct  5 10:38:47 UTC 2020
        if(1601894327 > ptbMarch->m_nEtime)
        {
            ptbMarch->Set_Etime(ptbMarch->m_nEtime + ddwRetryTime);
        }

        TSE_LOG_ERROR(CGlobalServ::m_pAbnormalActionLog, ("abnormal action hande: svr[%u],action[%ld],main=%ld,sec=%ld,status=%ld,suid=%ld,"
            "scid=%ld,btime=%lld,etime=%ld,retry=%ld [seq=%u]",
            pstSession->m_udwReqSvrId, ptbMarch->m_nId, ptbMarch->m_nMclass, ptbMarch->m_nSclass,
            ptbMarch->m_nStatus, ptbMarch->m_nSuid, ptbMarch->m_nScid,
            ptbMarch->m_nBtime, ptbMarch->m_nEtime, ptbMarch->m_nRetry, pstSession->m_udwSeqNo));

        //告警
        if(ptbMarch->m_nRetry > 2)
        {
            TCHAR szCmd[128];
            sprintf(szCmd, "./send_warning.sh \"bob find abnormal action=%ld,uid=%ld,sclass=%ld,ret=%d,retry=%ld!!!\" retry",
                ptbMarch->m_nId,
                ptbMarch->m_nSuid,
                ptbMarch->m_nSclass,
                pstSession->m_stCommonResInfo.m_dwRetCode,
                ptbMarch->m_nRetry);
            system(szCmd);
        }

        ExpectedDesc expect_desc;
        ExpectedItem expect_item;
        expect_item.SetVal(TbMARCH_ACTION_FIELD_ID, true, ptbMarch->m_nId); //加expect防止边界条件(action已经结束又被新建)
        expect_desc.push_back(expect_item);
        pstSession->m_vecAwsReq.clear();
        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER && ptbMarch->m_nRetry > 10)
        {
            CAwsRequest::DeleteItem(pstSession, ptbMarch);
        }
        else
        {
            CAwsRequest::UpdateItem(pstSession, ptbMarch, expect_desc);
        }
        if (pstSession->m_stMapItem.m_nId != 0)
        {
            TbMap tbMap;
            tbMap.Reset();
            tbMap.Set_Id(pstSession->m_stMapItem.m_nId);
            tbMap.Set_Sid(pstSession->m_stMapItem.m_nSid);
            CAwsRequest::UpdateItem(pstSession, &tbMap, ExpectedDesc(), 0, true);
        }

        if(pstSession->m_vecAwsReq.size() > 0)
        {
            dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        }
    }

    return dwRetCode;

}

TVOID CTaskProcess::GetReqActionCmdAndKeys(SSession *pstSession)
{
    SReqParam *pstReqParam = &pstSession->m_stReqParam;
    //SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    //SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    //TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    //TbPlayer *ptbTPlayer = &pstSUser->m_tbPlayer;
    //TbMap *ptbMap = &pstSession->m_stMapItem;
    //TCHAR *pCur = pstReqParam->m_szKey;
    //TUINT32 udwCurLen = 0;
    //TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if(pstSession->m_stActionTable.dwTableType == EN_AID_ACTION)
    {
        TbAlliance_action *ptbReqAction = &pstSession->m_stReqAlAction;
        if(ptbReqAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING)
        {
            TbAlliance_action *ptbReqAction = &pstSession->m_stReqAlAction;

            // keys
            sprintf(pstReqParam->m_szKey, "%ld|%ld|%ld|%ld", ptbReqAction->m_bParam[0].m_stBuilding.m_ddwPos,
                ptbReqAction->m_bParam[0].m_stBuilding.m_ddwType,
                ptbReqAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                ptbReqAction->m_bParam[0].m_stBuilding.m_ddwExp);

            // cmd
            switch(ptbReqAction->m_nSclass)
            {
            case EN_ACTION_SEC_CLASS__BUILDING_UPGRADE:
                pstReqParam->SetCmd("construct_finish");
                break;
            case EN_ACTION_SEC_CLASS__BUILDING_REMOVE:
                pstReqParam->SetCmd("deconstruct_finish");
                break;
            case EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE:
                pstReqParam->SetCmd("research_finish");
                break;
            }

            pstReqParam->m_BIsNeedCb = TRUE;
        }
        else if(ptbReqAction->m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            // keys
            sprintf(pstReqParam->m_szKey, "%ld|%ld|%ld", ptbReqAction->m_bParam[0].m_stTrain.m_ddwType,
                ptbReqAction->m_bParam[0].m_stTrain.m_ddwNum,
                ptbReqAction->m_bParam[0].m_stTrain.m_ddwExp);

            // cmd
            switch(ptbReqAction->m_nSclass)
            {
            case EN_ACTION_SEC_CLASS__TROOP:
                pstReqParam->SetCmd("troop_train_finish");
                break;
            case EN_ACTION_SEC_CLASS__FORT:
                pstReqParam->SetCmd("fort_train_finish");
                break;
            case EN_ACTION_SEC_CLASS__HOS_TREAT:
                pstReqParam->SetCmd("troop_heal_finish");
                break;
            }

            pstReqParam->m_BIsNeedCb = TRUE;
        }
        else
        {
            //todo:
        }
    }
    else if(pstSession->m_stActionTable.dwTableType == EN_UID_ACTION)
    {
        TbAction *ptbReqAction = &pstSession->m_stReqAction;
        if(ptbReqAction->m_nMclass == EN_ACTION_MAIN_CLASS__ITEM)
        {
            // keys
            sprintf(pstReqParam->m_szKey, "%ld|%ld", ptbReqAction->m_bParam[0].m_stItem.m_ddwBufferId,
                ptbReqAction->m_bParam[0].m_stItem.m_ddwNum);

            // cmd
            pstReqParam->SetCmd("item_finish");

            pstReqParam->m_BIsNeedCb = TRUE;
        }
        else
        {
            //todo;
        }
    }
    else
    {
        TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
        if(ptbReqAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH)
        {
            ////TODO
            ////SActionMarchParam *pstActionParam = &ptbReqAction->m_bParam[0].m_stMarch;

            //TINT32 dwSourceAid = ptbSPlayer->m_nAlpos ? ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
            //TINT32 dwTargetAid = ptbTPlayer->m_nAlpos ? ptbTPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
            //// head
            ////udwCurLen = sprintf(pCur, "%ld|%ld|%d|%ld|%u|%d|%ld|%u|%ld",
            ////    ptbReqAction->m_nSuid, ptbReqAction->m_nScid, dwSourceAid,
            ////    0, pstActionParam->m_udwTargetCityId, dwTargetAid,
            ////    0, pstActionParam->m_ucTargetType, ptbReqAction->m_nSclass);//TODO

            //// head
            //udwCurLen = sprintf(pCur, "%ld|%ld|%d|%ld|%u|%d|%ld|%u|%ld",
            //    ptbReqAction->m_nSuid, ptbReqAction->m_nScid, dwSourceAid,
            //    0, 0, dwTargetAid,
            //    0, 0, ptbReqAction->m_nSclass);//TODO

            //pCur += udwCurLen;

            //switch(ptbReqAction->m_nSclass)
            //{
            //case EN_ACTION_SEC_CLASS__TRANSPORT:
            //    if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__UN_LOADING)
            //    {
            //        pstReqParam->SetCmd("transport_arrived");
            //        // resource
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstSession->m_stReqParam.m_udwResource, EN_RESOURCE_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        //以b打印
            //        pstReqParam->m_ucIsSuid = 0;

            //        pstReqParam->m_BIsNeedCb = TRUE;
            //    }
            //    else if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__RETURNING)
            //    {
            //        pstReqParam->SetCmd("transport_return");
            //        //// resource
            //        //CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stResource.m_audwNum, EN_RESOURCE_TYPE__END, "|", FALSE);
            //        //pCur += udwCurLen;
            //        //// troop raw
            //        //CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroopRaw.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        //pCur += udwCurLen;
            //        //// troop end
            //        //CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroop.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        //pCur += udwCurLen;

            //        pstReqParam->m_BIsNeedCb = TRUE;
            //    }
            //    break;
            //case EN_ACTION_SEC_CLASS__REINFORCE_NORMAL:
            //    if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__SETUP_CAMP)
            //    {
            //        pstReqParam->SetCmd("reinforce_arrived");
            //        // troop
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroop.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        //以b打印
            //        pstReqParam->m_ucIsSuid = 0;

            //        pstReqParam->m_BIsNeedCb = TRUE;
            //    }
            //    else if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__RETURNING)
            //    {
            //        pstReqParam->SetCmd("reinforce_return");
            //        // troop raw
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroopRaw.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        // troop end
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroop.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;

            //        pstReqParam->m_BIsNeedCb = TRUE;
            //    }
            //    break;
            //case EN_ACTION_SEC_CLASS__ATTACK:
            //case EN_ACTION_SEC_CLASS__OCCUPY:
            //    if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__RETURNING)
            //    {
            //        pstReqParam->SetCmd("attack_return");
            //        // resource
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stResource.m_audwNum, EN_RESOURCE_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        // troop raw
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroopRaw.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        // troop end
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroop.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        //target wild level
            //        udwCurLen = sprintf(pCur, "|%u", pstActionParam->m_ucTargetLevel);
            //        pCur += udwCurLen;
            //        //item list
            //        udwCurLen = sprintf(pCur, "|");
            //        pCur += udwCurLen;
            //        // troop wound:能进医院的兵
            //        udwCurLen = sprintf(pCur, "|");
            //        pCur += udwCurLen;

            //        pstReqParam->m_BIsNeedCb = TRUE;
            //    }
            //    else if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__LOADING)
            //    {
            //        pstReqParam->SetCmd("res_load_finish");
            //        // resource
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stResource.m_audwNum, EN_RESOURCE_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        // troop raw
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroopRaw.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        // troop end
            //        CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstActionParam->m_stTroop.m_audwNum, EN_TROOP_TYPE__END, "|", FALSE);
            //        pCur += udwCurLen;
            //        //target wild level
            //        udwCurLen = sprintf(pCur, "|%u", pstActionParam->m_ucTargetLevel);
            //        pCur += udwCurLen;
            //        // TODO
            //        // 获取的骑士经验
            //        udwCurLen = sprintf(pCur, "|%u", 0);
            //        pCur += udwCurLen;

            //        pstReqParam->m_BIsNeedCb = TRUE;
            //    }
            //    else if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__FIGHTING)
            //    {
            //        pstReqParam->m_BIsNeedCb = FALSE;
            //    }
            //    break;
            //case EN_ACTION_SEC_CLASS__SCOUT:
            //    if(pstSession->m_ucActionRawStatus == EN_MARCH_STATUS__MARCHING)
            //    {
            //        pstReqParam->SetCmd("scout_finish");

            //        pstReqParam->m_BIsNeedCb = TRUE;
            //    }
            //    break;
            //}
        }
    }
}
