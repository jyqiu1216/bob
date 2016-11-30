
#include "process_base.h"
#include "game_command.h"

TINT32 CBaseCmdProcess::ProcedureCmd(SSession *pstSession, TBOOL &bNeedResponse)
{   
    TINT32 dwRetCode = 0;

PROCESS_CMD:
    pstSession->m_udwProcessSeq++;
    
    CClientCmd *poCClientCmd = CClientCmd::GetInstance();
    struct SFunctionSet stProcessFunctionSet;
    map<EClientReqCommand, struct SFunctionSet>::iterator itCmdFunctionSet;
    itCmdFunctionSet = poCClientCmd->Get_CmdMap()->find((EClientReqCommand)pstSession->m_udwCommand);            
    if(itCmdFunctionSet != poCClientCmd->Get_CmdMap()->end())
    {
        stProcessFunctionSet = itCmdFunctionSet->second;
    }

    bNeedResponse = FALSE;

    TSE_LOG_INFO(pstSession->m_poServLog, ("process_by_cmd: step[%u] begin process: cmd=%s,next=%u,step=%u, [seq=%u]", \
                                            pstSession->m_udwProcessSeq, stProcessFunctionSet.szCmdName, 
                                            pstSession->m_udwNextProcedure, pstSession->m_udwCommandStep,
                                            pstSession->m_udwSeqNo));
                                        
    if(NULL != stProcessFunctionSet.ProcessProc)
    {
        pstSession->m_bGotoOtherCmd = FALSE;
        dwRetCode = stProcessFunctionSet.ProcessProc(pstSession, bNeedResponse);
    }

    
    /*
    TSE_LOG_INFO(pstSession->m_poServLog, ("process_by_cmd: step[%u] end process: ret[%d][seq=%u]",
    pstSession->m_udwProcessSeq, dwRetCode, pstSession->m_udwSeqNo));
    */

    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("process_by_cmd: reqcmd=%u, nowcom=%u, curret=%d, comret=%d [seq=%u]", \
                                                pstSession->m_stReqParam.m_udwCommandID, pstSession->m_udwCommand, dwRetCode, \
                                                pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwSeqNo));
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
        }
    }

    if (pstSession->m_bGotoOtherCmd)
    {
        pstSession->m_udwProcessSeq = 0;
        goto PROCESS_CMD;
    }

    return dwRetCode;
}



