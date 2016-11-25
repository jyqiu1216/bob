#include "process_map.h"

#include "db_request.h"
#include "db_response.h"
#include "procedure_base.h"
#include "common_func.h"
#include "map_base.h"
#include "alliance_mapping.h"
#include "common_json.h"
#include "wild_info.h"
#include "map_logic.h"
#include "common_logic.h"
#include "game_svr.h"
#include "tool_base.h"
#include "player_base.h"

TINT32 CProcessMap::ProcessCmd_MapGet( SSession *pstSession, TBOOL &bNeedResponse )
{
    TINT32 dwRetCode = 0;
    TUINT32 idx = 0;

    TCHAR *pszPos = pstSession->m_stReqParam.m_szKey[0];
    TUINT32 udwSvrId = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        SGameSvrInfo *pstSvrInfo = CGameSvrInfo::GetInstance()->GetSidInfo(udwSvrId);
        TBOOL bIsNeedGetMap = TRUE;
        if (pstSvrInfo->m_dwMergeStatus != EN_SVR_MERGE_STATUS__NORMAL && pstSvrInfo->m_dwMergeStatus != EN_SVR_MERGE_STATUS__PROTECTED)
        {
            bIsNeedGetMap = FALSE;
        }

        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MapGet: NEMO need get map[%d][svr=%u][id=%u,target=%d status=%d] [seq=%u]", 
            bIsNeedGetMap, udwSvrId, pstSvrInfo->m_udwId, pstSvrInfo->m_dwMergeTargetSid, pstSvrInfo->m_dwMergeStatus, pstSession->m_stUserInfo.m_udwBSeqNo));

        TCHAR *pCur = pszPos;
        TCHAR *pWild = pszPos;
        TBOOL bError = FALSE;
        //TBOOL bFull = FALSE;
        TUINT32 audwThrd[4] = {0, 0, 0, 0};

        map<TUINT64, TUINT32>& mBlockId = pstSession->m_mBlockId;
        set<TINT32> setPosId;
        mBlockId.clear();
        setPosId.clear();

        pstSession->ResetAwsReq();

        while(pCur && bError == FALSE)
        {
            // cur block
            pWild = pCur;
            for(idx = 0; idx < 4; idx++)
            {
                audwThrd[idx] = atoi(pWild);
                pWild = strchr(pWild, ',');
                if(pWild)
                {
                    pWild++;
                }
                else if(pWild == NULL && idx != 3)
                {
                    bError = TRUE;
                    break;
                }
            }

            TINT64 ddwBlockId = CMapBase::GetBlockIdFromPos(MAP_X_Y_POS_COMPUTE_OFFSET * (audwThrd[0] + audwThrd[1]) / 2 + (audwThrd[2] + audwThrd[3]) / 2);

            if(ddwBlockId > 0 && mBlockId.find(ddwBlockId) == mBlockId.end())
            {
                TINT32 dwBX = ddwBlockId / 1000 * 10 + 1;
                TINT32 dwEX = (ddwBlockId / 1000 + 1) * 10;
                TINT32 dwBY = (ddwBlockId - 1) % 1000 * 10 + 1;
                TINT32 dwEY = ((ddwBlockId - 1) % 1000 + 1) * 10;

                for (TINT32 dwIdx = dwBX; dwIdx <= dwEX; dwIdx++)
                {
                    for (TINT32 dwIdy = dwBY; dwIdy <= dwEY; dwIdy++)
                    {
                        if (dwIdx % 2 == dwIdy % 2)
                        {
                            setPosId.insert(dwIdx * MAP_X_Y_POS_COMPUTE_OFFSET + dwIdy);
                        }
                    }
                }
                mBlockId[ddwBlockId] = 1;
            }
            // next block
            pCur = strchr(pCur, ';');
            if(pCur)
            {
                pCur++;
            }
        }

        if (bIsNeedGetMap == FALSE)
        {
            pstSession->ResetAwsInfo();
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        if (setPosId.size() > 0)
        {
            MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(udwSvrId, "GetMapList");
            Json::FastWriter writer;
            writer.omitEndingLineFeed();

            CJsoncppSeri jSeri;

            Json::Value jTmp = Json::Value(Json::objectValue);
            jTmp["map_id_list"] = Json::Value(Json::arrayValue);
            for (set<TINT32>::iterator it = setPosId.begin(); it != setPosId.end(); it++)
            {
                jTmp["map_id_list"].append(*it);
            }
            if (pstSession->m_udwContentType == EN_CONTENT_TYPE__STRING)
            {
                pMapSvrReq->sReqContent = writer.write(jTmp);
            }
            else
            {
                const TCHAR *pszRes = NULL;
                TUINT32 udwResLen = 0;
                pszRes = jSeri.serializeToBuffer(jTmp, udwResLen);
                pMapSvrReq->sReqContent.resize(udwResLen);
                memcpy((char*)pMapSvrReq->sReqContent.c_str(), pszRes, udwResLen);
            }
            pstSession->m_vecMapSvrReq.push_back(pMapSvrReq);
        }


        if (bError == TRUE || pstSession->m_vecMapSvrReq.size() == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // parse data
        pstSession->m_udwTmpWildNum = 0;
        for(idx = 0; idx < pstSession->m_vecAwsRsp.size(); idx++)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[idx],
                &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum],
                sizeof(TbMap), MAX_WILD_RETURN_NUM - pstSession->m_udwTmpWildNum);
            if(dwRetCode > 0)
            {
                pstSession->m_udwTmpWildNum += dwRetCode;
            }

            TSE_LOG_INFO(pstSession->m_poServLog, ("send map test [c_wild_num=%d, t_wild_num=%u] [seq=%u]",
                dwRetCode, pstSession->m_udwTmpWildNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        }

        //修正map中的alliance name信息
        const Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
        for(TUINT32 udwMapIdx = 0; udwMapIdx < pstSession->m_udwTmpWildNum; ++udwMapIdx)
        {
            TbMap *pstMap = &pstSession->m_atbTmpWild[udwMapIdx];
            if(pstMap->m_nAlid > 0 && jAlMapping["update_time"].asInt64() > pstMap->m_nName_update_time)
            {
                if(jAlMapping.isMember(CCommonFunc::NumToString(pstMap->m_nAlid)))
                {
                    //这里不要改AWS表里的信息，因为alliance_mapping的内容可能滞后
                    pstMap->Set_Alname(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["al_name"].asString());
                    pstMap->Set_Al_nick(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["nick_name"].asString());
                    pstMap->Set_Al_flag(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["avatar"].asInt64());
                    //TODO
                    if (pstMap->m_nId == THRONE_POS) //王座要同步oname
                    {
                        pstMap->Set_Uname(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["oname"].asString());
                    }
                }
            }
        }

        std::map<TUINT64, TUINT32>& mBlockId = pstSession->m_mBlockId;
        TbMarch_action tbMarch;
        tbMarch.Set_Sid(udwSvrId);
        for(map<TUINT64, TUINT32>::iterator iter = mBlockId.begin(); iter != mBlockId.end(); ++iter)
        {
            tbMarch.Set_Sid(udwSvrId);
            tbMarch.Set_Sbid(iter->first);
            CAwsRequest::Query(pstSession, &tbMarch, ETbMARCH_OPEN_TYPE_GLB_SBID, CompareDesc(), false);
            tbMarch.Set_Tbid(iter->first);
            CAwsRequest::Query(pstSession, &tbMarch, ETbMARCH_OPEN_TYPE_GLB_TBID, CompareDesc(), false);
        }

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    //解析map_action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        //// next procedure
        //pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        //pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        map<TUINT64, TUINT32> mapMarchId;
        TUINT32& udwNum = pstSession->m_udwTmpMarchNum;
        udwNum = 0;
        TbMarch_action* ptbMarch = NULL;
        for(unsigned int i = 0; i < pstSession->m_vecAwsRsp.size(); i++)
        {
            ptbMarch = &pstSession->m_atbTmpMarch[udwNum];
            dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[i], ptbMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM - udwNum);
            if(dwRetCode > 0)
            {
                for(int j = 0; j < dwRetCode;)
                {
                    //只返回打战的action
                    if(EN_ACTION_MAIN_CLASS__MARCH != ptbMarch[j].m_nMclass || mapMarchId.find(ptbMarch[j].m_nId) != mapMarchId.end() ||
                        (ptbMarch[j].m_nStatus != EN_MARCH_STATUS__MARCHING
                        && ptbMarch[j].m_nStatus != EN_MARCH_STATUS__FIGHTING
                        && ptbMarch[j].m_nStatus != EN_MARCH_STATUS__RETURNING
                        && ptbMarch[j].m_nStatus != EN_MARCH_STATUS__TRADING
                        && ptbMarch[j].m_nStatus != EN_MARCH_STATUS__LOADING
                        && ptbMarch[j].m_nStatus != EN_MARCH_STATUS__PRE_LOADING)) //去重
                    {
                        if(j < dwRetCode - 1)
                        {
                            ptbMarch[j] = ptbMarch[dwRetCode - 1];
                        }
                        dwRetCode--;
                    }
                    else
                    {
                        mapMarchId[ptbMarch[j].m_nId] = 1;
                        j++;
                    }
                }
                udwNum += dwRetCode;
            }
        }
        
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpMarchNum; udwIdx++)
        {
            ptbMarch = &pstSession->m_atbTmpMarch[udwIdx];
            if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__DRAGON_ATTACK 
                && ptbMarch->m_nStatus == EN_MARCH_STATUS__FIGHTING)
            {
                for (TUINT32 udwIdy = 0; udwIdy < pstSession->m_udwTmpWildNum; udwIdy++)
                {
                    if (ptbMarch->m_nTpos == pstSession->m_atbTmpWild[udwIdy].m_nId)
                    {
                        pstSession->m_atbTmpWild[udwIdy].Set_Type(ptbMarch->m_bMonster_info[0].ddwType);
                        pstSession->m_atbTmpWild[udwIdy].Set_Level(ptbMarch->m_bMonster_info[0].ddwLv);
                        pstSession->m_atbTmpWild[udwIdy].Set_Boss_life(ptbMarch->m_bMonster_info[0].ddwRawHp);
                        break;
                    }
                }
            }
        }

    }

    // next procedure
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMap::ProcessCmd_MapBlockGet( SSession *pstSession, TBOOL &bNeedResponse )
{
    TINT32 dwRetCode;
    //获取map
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        map<TUINT64, TUINT32>& mBlockId = pstSession->m_mBlockId;
        mBlockId.clear();
        vector<TUINT32> vecBlockId;
        CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecBlockId);
        for(unsigned int i = 0; i < vecBlockId.size(); ++i)
        {
            if(mBlockId.find(vecBlockId[i]) == mBlockId.end())
            {
                pstSession->m_tbTmpMap.Reset();
                pstSession->m_tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
                pstSession->m_tbTmpMap.Set_Bid(vecBlockId[i]);
                CAwsRequest::Query(pstSession, &pstSession->m_tbTmpMap, ETbMAP_OPEN_TYPE_GLB_BID, CompareDesc(), false);
                mBlockId[vecBlockId[i]] = 1;
            }
        }
        if(pstSession->m_vecAwsReq.size() == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }
    //解析map响应,获取map_action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // parse data
        pstSession->m_udwTmpWildNum = 0;
        for(unsigned int i = 0; i < pstSession->m_vecAwsRsp.size(); i++)
        {
            dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[i],
                &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum],
                sizeof(TbMap), MAX_WILD_RETURN_NUM - pstSession->m_udwTmpWildNum);
            if(dwRetCode > 0)
            {
                pstSession->m_udwTmpWildNum += dwRetCode;
            }
        }

        map<TUINT64, TUINT32>& mapBlockId = pstSession->m_mBlockId;
        TUINT32 udwSvrId = pstSession->m_stReqParam.m_udwSvrId;
        TbMarch_action tbMarch;
        tbMarch.Set_Sid(udwSvrId);
        for(map<TUINT64, TUINT32>::iterator iter = mapBlockId.begin(); iter != mapBlockId.end(); ++iter)
        {
            tbMarch.Set_Sbid(iter->first);
            CAwsRequest::Query(pstSession, &tbMarch, ETbMARCH_OPEN_TYPE_GLB_SBID, CompareDesc(), false);
            tbMarch.Set_Tbid(iter->first);
            CAwsRequest::Query(pstSession, &tbMarch, ETbMARCH_OPEN_TYPE_GLB_TBID, CompareDesc(), false);
        }

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }
    //解析map_action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        map<TUINT64, TUINT32> mapMarchId;
        TUINT32& udwNum = pstSession->m_udwTmpMarchNum;
        udwNum = 0;
        TbMarch_action* ptbMarch = NULL;
        for(unsigned int i = 0; i < pstSession->m_vecAwsRsp.size(); i++)
        {
            ptbMarch = &pstSession->m_atbTmpMarch[udwNum];
            dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[i], ptbMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM - udwNum);
            if(dwRetCode > 0)
            {
                for(int j = 0; j < dwRetCode;)
                {
                    //只返回打战的action
                    if(EN_ACTION_MAIN_CLASS__MARCH != ptbMarch[j].m_nMclass || mapMarchId.find(ptbMarch[j].m_nId) != mapMarchId.end()) //去重
                    {
                        if(j < dwRetCode - 1)
                        {
                            ptbMarch[j] = ptbMarch[dwRetCode - 1];
                        }
                        dwRetCode--;
                    }
                    else
                    {
                        mapMarchId[ptbMarch[j].m_nId] = 1;
                        j++;
                    }
                }
                udwNum += dwRetCode;
            }
        }
        // next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        return 0;
    }
    return 0;
}

TINT32 CProcessMap::ProcessCmd_ManorInfo(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        CAwsRequest::MapGet(pstSession, dwPos);
        TbMarch_action tbMarch;
        tbMarch.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        tbMarch.Set_Tbid(CMapBase::GetBlockIdFromPos(dwPos));
        CAwsRequest::Query(pstSession, &tbMarch, ETbMARCH_OPEN_TYPE_GLB_TBID, CompareDesc(), false);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ManorInfo send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_MAP)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpMap);
                continue;
            }
            if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_atbTmpMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRetCode > 0)
                {
                    pstSession->m_udwTmpMarchNum = dwRetCode;
                }
                continue;
            }
        }

        SCommonTroop stTotolTroop;
        stTotolTroop.Reset();
        TINT64 ddwTotalTroopNum = 0;
        TINT64 ddwTotalTroopForce = 0;

        pstSession->m_JsonValue.clear();
        pstSession->m_JsonValue = Json::Value(Json::objectValue);
        //TODO删掉
        pstSession->m_JsonValue["dragon"] = Json::Value(Json::objectValue);
        pstSession->m_JsonValue["dragon"]["uid"] = 0;
        pstSession->m_JsonValue["dragon"]["alnick"] = "";
        pstSession->m_JsonValue["dragon"]["uname"] = "";
        pstSession->m_JsonValue["dragon"]["dragon_name"] = "";
        pstSession->m_JsonValue["dragon"]["dragon_lv"] = 0;
        pstSession->m_JsonValue["dragon"]["dragon_avatar"] = 0;
        pstSession->m_JsonValue["dragon"]["action_id"] = 0;

        pstSession->m_JsonValue["main_assign"] = Json::Value(Json::objectValue);
        pstSession->m_JsonValue["main_assign"]["uid"] = 0;
        pstSession->m_JsonValue["main_assign"]["alnick"] = "";
        pstSession->m_JsonValue["main_assign"]["uname"] = "";
        pstSession->m_JsonValue["main_assign"]["dragon_name"] = "";
        pstSession->m_JsonValue["main_assign"]["dragon_lv"] = 0;
        pstSession->m_JsonValue["main_assign"]["dragon_avatar"] = 0;
        pstSession->m_JsonValue["main_assign"]["action_id"] = 0;
        pstSession->m_JsonValue["main_assign"]["knight"] = Json::Value(Json::arrayValue);
        pstSession->m_JsonValue["main_assign"]["knight"].append(-1);
        pstSession->m_JsonValue["main_assign"]["knight"].append(0);
        pstSession->m_JsonValue["main_assign"]["knight"].append(0);

        pstSession->m_JsonValue["detail"] = Json::Value(Json::arrayValue);

        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpMarchNum; ++udwIdx)
        {
            if(pstSession->m_atbTmpMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__DRAGON_ASSIGN
            && pstSession->m_atbTmpMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                //TODO删掉
                pstSession->m_JsonValue["dragon"]["uid"] = pstSession->m_atbTmpMarch[udwIdx].m_nSuid;
                pstSession->m_JsonValue["dragon"]["alnick"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_szSourceAlNick;
                pstSession->m_JsonValue["dragon"]["uname"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_szSourceUserName;
                pstSession->m_JsonValue["dragon"]["dragon_name"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stDragon.m_szName;
                pstSession->m_JsonValue["dragon"]["dragon_lv"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stDragon.m_ddwLevel;
                pstSession->m_JsonValue["dragon"]["dragon_avatar"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stDragon.m_ddwIconId;
                pstSession->m_JsonValue["dragon"]["action_id"] = pstSession->m_atbTmpMarch[udwIdx].m_nId;

                pstSession->m_JsonValue["main_assign"]["uid"] = pstSession->m_atbTmpMarch[udwIdx].m_nSuid;
                pstSession->m_JsonValue["main_assign"]["alnick"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_szSourceAlNick;
                pstSession->m_JsonValue["main_assign"]["uname"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_szSourceUserName;
                pstSession->m_JsonValue["main_assign"]["dragon_name"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stDragon.m_szName;
                pstSession->m_JsonValue["main_assign"]["dragon_lv"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stDragon.m_ddwLevel;
                pstSession->m_JsonValue["main_assign"]["dragon_avatar"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stDragon.m_ddwIconId;
                pstSession->m_JsonValue["main_assign"]["action_id"] = pstSession->m_atbTmpMarch[udwIdx].m_nId;
                pstSession->m_JsonValue["main_assign"]["knight"][0] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stKnight.ddwId;
                pstSession->m_JsonValue["main_assign"]["knight"][1] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stKnight.ddwLevel;
                pstSession->m_JsonValue["main_assign"]["knight"][2] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stKnight.ddwExpAdd;
            }

            if(pstSession->m_atbTmpMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && pstSession->m_atbTmpMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                Json::Value jsonReinforce = Json::Value(Json::objectValue);
                jsonReinforce["uid"] = pstSession->m_atbTmpMarch[udwIdx].m_nSuid;
                jsonReinforce["alnick"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_szSourceAlNick;
                jsonReinforce["uname"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_szSourceUserName;
                jsonReinforce["troop_num"] = pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_ddwTroopNum;
                CCommJson::GenTroopJson(&pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stTroop, jsonReinforce["troop"]);
                jsonReinforce["time"] = pstSession->m_atbTmpMarch[udwIdx].m_nBtime;
                jsonReinforce["action_id"] = pstSession->m_atbTmpMarch[udwIdx].m_nId;
                jsonReinforce["knight"] = Json::Value(Json::arrayValue);
                jsonReinforce["knight"].append(pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stKnight.ddwId);
                jsonReinforce["knight"].append(pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stKnight.ddwLevel);
                jsonReinforce["knight"].append(pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stKnight.ddwExpAdd);
                CToolBase::AddTroop(pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_stTroop, stTotolTroop);
                ddwTotalTroopForce += pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_ddwForce;
                ddwTotalTroopNum += pstSession->m_atbTmpMarch[udwIdx].m_bParam[0].m_ddwTroopNum;

                pstSession->m_JsonValue["detail"].append(jsonReinforce);
            }
        }

        CCommJson::GenTroopJson(&stTotolTroop, pstSession->m_JsonValue["total_troop"]);
        pstSession->m_JsonValue["troop_total_num"] = ddwTotalTroopNum;
        pstSession->m_JsonValue["troop_total_force"] = ddwTotalTroopForce;

        if(pstSession->m_tbTmpMap.m_nId > 0)
        {
            CCommJson::GenMapBaseJson(&pstSession->m_tbTmpMap, pstSession->m_JsonValue["map"]);
        }

        pstSession->m_JsonValue["owner"] = Json::Value(Json::objectValue);
        pstSession->m_JsonValue["owner"]["uid"] = 0;
        pstSession->m_JsonValue["owner"]["avatar"] = 0;
        pstSession->m_JsonValue["owner"]["vip_level"] = 0;
        pstSession->m_JsonValue["owner"]["city_pos"] = 0;

        if(pstSession->m_tbTmpMap.m_nUid > 0)
        {
            pstSession->ResetAwsInfo();
            CAwsRequest::UserGetByUid(pstSession, pstSession->m_tbTmpMap.m_nUid);
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ManorInfo send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }

        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        TbPlayer tbOwner;
        tbOwner.Reset();
        CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbOwner);
        if(tbOwner.m_nUid > 0)
        {
            pstSession->m_JsonValue["owner"] = Json::Value(Json::objectValue);
            pstSession->m_JsonValue["owner"]["uid"] = tbOwner.m_nUid;
            pstSession->m_JsonValue["owner"]["avatar"] = tbOwner.m_nAvatar;
            pstSession->m_JsonValue["owner"]["vip_level"] = CPlayerBase::GetVipLevel(&tbOwner);
            pstSession->m_JsonValue["owner"]["city_pos"] = tbOwner.m_nCid;

            if(pstSession->m_JsonValue["map"].isArray())
            {
                pstSession->m_JsonValue["map"][8U] = tbOwner.m_sUin;
                if (pstSession->m_JsonValue["map"][32U].asInt() == 0)
                {
                    pstSession->m_JsonValue["map"][32U] = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_tax"]["default"].asInt();
                }

                //TODO 兼容   后面删掉..
                pstSession->m_JsonValue["map"][38U] = 2;
            }
        }

        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMap::ProcessCmd_PrisionInfo(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwTargetUid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[2]);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        TbMarch_action tbPrisonTimer;
        tbPrisonTimer.Set_Sid(dwSid);
        tbPrisonTimer.Set_Tbid(CMapBase::GetBlockIdFromPos(dwPos));
        CAwsRequest::Query(pstSession, &tbPrisonTimer, ETbMARCH_OPEN_TYPE_GLB_TBID, CompareDesc(), false);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ManorInfo send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_atbTmpMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRetCode > 0)
                {
                    pstSession->m_udwTmpMarchNum = dwRetCode;
                }
                continue;
            }
        }

        pstSession->m_JsonValue.clear();
        pstSession->m_JsonValue = Json::Value(Json::objectValue);
        pstSession->m_JsonValue["pos"] = dwPos;
        pstSession->m_JsonValue["uid"] = dwTargetUid;
        pstSession->m_JsonValue["prison"] = Json::Value(Json::arrayValue);

        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpMarchNum; ++udwIdx)
        {
            if(pstSession->m_atbTmpMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            && pstSession->m_atbTmpMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING
            && pstSession->m_atbTmpMarch[udwIdx].m_nTuid == dwTargetUid)
            {
                Json::Value jsonPrison = Json::Value(Json::objectValue);
                jsonPrison["can_kill"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwCaptured;
                jsonPrison["uid"] = pstSession->m_atbTmpMarch[udwIdx].m_nSuid;
                jsonPrison["al_nick"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].szSourceAlNick;
                jsonPrison["uname"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].szSourceUserName;
                jsonPrison["dragon_avatar"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwIconId;
                jsonPrison["dragon_name"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].stDragon.m_szName;
                jsonPrison["dragon_lv"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwLevel;
                jsonPrison["dragon_status"] = EN_DRAGON_STATUS_WAIT_RELEASE;
                jsonPrison["join_time"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].ddwJoinTimeStamp;
                jsonPrison["can_kill_time"] = pstSession->m_atbTmpMarch[udwIdx].m_nEtime - pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].ddwReleaseWait + pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].ddwExcuteWait;
                jsonPrison["auto_release_time"] = pstSession->m_atbTmpMarch[udwIdx].m_nEtime;
                jsonPrison["action_id"] = pstSession->m_atbTmpMarch[udwIdx].m_bPrison_param[0].ddwEscortActionId;

                pstSession->m_JsonValue["prison"].append(jsonPrison);
            }
        }

        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAP;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}
