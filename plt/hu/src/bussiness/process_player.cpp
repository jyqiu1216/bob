#include "process_player.h"
#include "db_request.h"
#include "db_response.h"
#include "global_serv.h"
#include "procedure_base.h"
#include "common_func.h"
#include "encode/src/encode/utf8_util.h"
#include "city_info.h"
#include "city_base.h"
#include "item_base.h"
#include "common_base.h"
#include "item_logic.h"
#include "action_base.h"
#include "sendmessage_base.h"
#include "process_player.h"
#include "quest_logic.h"
#include "globalres_logic.h"
#include "game_info.h"
#include "process_action.h"
#include "backpack_info.h"
#include "conf_base.h"
#include "quest_notic_logic.h"
#include "activities_logic.h"
#include "common_logic.h"
#include "wild_info.h"
#include "process_item.h"
#include "msg_base.h"
#include "document.h"
#include "rating_user.h"
#include "common_json.h"
#include "game_svr.h"
#include "map_logic.h"
#include "map_base.h"
#include "player_base.h"
#include "tool_base.h"
#include "backpack_logic.h"
#include "report_svr_request.h"
#include "pushdata_action.h"

TINT32 CProcessPlayer::ProcessCmd_LoginGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    return 0;
}


TINT32 CProcessPlayer::ProcessCmd_UserInfoCreate(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;
    TbLogin& tbLogin = pstUser->m_tbLogin;
    TbMap& tbTmpMap = pstSession->m_tbTmpMap;
    SCityInfo *pstCity = NULL;
    CGameSvrInfo *poGameSvr = CGameSvrInfo::GetInstance();
    SReqParam *pstReq = &pstSession->m_stReqParam;
    unsigned int udwSid = pstReq->m_udwSvrId;

    // 0. 进行流程判定
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(pstReq->m_ucIsNpc != 0 && pstReq->m_udwCityId != 0)
        {
            tbTmpMap.Reset();
            tbTmpMap.Set_Sid(udwSid);
            tbTmpMap.Set_Id(pstReq->m_udwCityId);
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        }
    }

    // 1. 从map中获取未被占领的plain作为新城市的id——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;

        pstSession->ResetMapSvrInfo();
        // set package
        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(pstSession->m_stReqParam.m_udwSvrId, "GetNewCityMap");
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        CJsoncppSeri jSeri;

        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["type"] = 0; // 0: new user 1: random move 2: attack move
        jTmp["zone"] = -1;
        jTmp["province"] = -1;

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

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_UserInfoCreate: send get map item request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }


    // 2. 解析获取的新city，并进行信息设置
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // c.set procedure
        TINT32 dwParseCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbTmpMap);
        if (dwParseCode <= 0 || tbTmpMap.m_nId == 0 || tbTmpMap.m_nUid != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_UserInfoCreate: map update failed, get again.[ret_code=%d res_info_ret_code=%u tbTmpMap_uid=%u login_uid=%u] [seq=%u]",
                dwRetCode,
                pstSession->m_stCommonResInfo.m_dwRetCode,
                tbTmpMap.m_nUid,
                tbLogin.m_nUid,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
    }

    // 3. 优先修改map数据，方便后续兼容插入失败情况——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        pstSession->ResetAwsInfo();

        // 容错
        pstUser->m_tbPlayer.Set_Cid(0);

        // 设置创建信息
        pstReq->m_udwCityId = tbTmpMap.Get_Id();
        if (pstUser->m_tbPlayer.Get_Uid() > 0)
        {
            CAwsRequest::AddCity(pstUser, udwSid, tbTmpMap.Get_Id());//原始已经有数据，只需增加城市即可
        }
        else// 同时创建用户和城市
        {
            CAwsRequest::CreateUserInfo(pstSession, pstUser, udwSid, tbTmpMap.Get_Id());
            pstUser->m_tbPlayer.Set_Npc(pstSession->m_stReqParam.m_ucIsNpc); //能不能合到CreateUserInfo中去?
        }

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("aws req: SendAwsRequest: [vaildtasksnum=%u] [seq=%u]", \
            pstSession->m_vecAwsReq.size(),
            pstSession->m_udwSeqNo));

        //重设请求参数
        pstSession->m_stReqParam.m_udwUserId = pstUser->m_tbPlayer.m_nUid;

        // 新用户 初始
        if (1U == pstSession->m_stReqParam.m_ucIsNewPlayer)
        {
            // account
            TbLogin *pTbLogin = &pstUser->m_tbLogin;
            if (pTbLogin->m_nGem > EN_GEM_REWARD__GUIDE_0_COST_GEM)
            {
                pTbLogin->Set_Gem(pTbLogin->m_nGem - EN_GEM_REWARD__GUIDE_0_COST_GEM);
            }
            else
            {
                pTbLogin->Set_Gem(0);
            }
        }


        TSE_LOG_DEBUG(pstSession->m_poServLog, ("aws req: SendAwsRequest: [vaildtasksnum=%u] [seq=%u]", \
            pstSession->m_vecAwsReq.size(),
            pstSession->m_udwSeqNo));

        TbLogin *pTbLogin = &pstUser->m_tbLogin;
        BITSET(pTbLogin->m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH);
        pTbLogin->SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);

        // 更新map数据，并设置其标记
        pstCity = &pstUser->m_stCityInfo;

        // 设置新手保护，使用和平和免侦查物品，时长72小时
        pstUser->m_udwActionNum = 0;
        UActionParam stParam;

        //新手保护时间配置化
        TUINT32 udwNewProtectTime = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_NEW_PROTECT_TIME].asUInt();

        stParam.m_stItem.SetValue(EN_BUFFER_INFO_PEACE_TIME, 0, udwNewProtectTime);
        CCommonBase::AddAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__ITEM, EN_ACTION_SEC_CLASS__ITEM,
            EN_ITEM_STATUS__USING, udwNewProtectTime, &stParam);

        tbPlayer.Set_Status(EN_CITY_STATUS__NEW_PROTECTION | EN_CITY_STATUS__NEW_USER_MISTERY);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("aws req: SendAwsRequest: [vaildtasksnum=%u] [seq=%u]", \
            pstSession->m_vecAwsReq.size(),
            pstSession->m_udwSeqNo));

        // 更新map数据
        tbTmpMap.Set_Utime(CTimeUtils::GetUnixTime());
        tbTmpMap.Set_Type(EN_WILD_TYPE__CITY);
        tbTmpMap.Set_Level(1);
        tbTmpMap.Set_Name(pstCity->m_stTblData.Get_Name());
        tbTmpMap.Set_Status(tbPlayer.m_nStatus);
        tbTmpMap.Set_Might(pstUser->m_tbPlayer.Get_Might());
        tbTmpMap.Set_Force_kill(0);
        tbTmpMap.Set_Cid(tbTmpMap.Get_Id());
        tbTmpMap.Set_Uid(pstUser->m_tbPlayer.Get_Uid());
        tbTmpMap.Set_Uname(pstUser->m_tbPlayer.Get_Uin());
        tbTmpMap.Set_Ulevel(pstUser->m_tbPlayer.Get_Level());
        tbTmpMap.Set_Alid(pstUser->m_tbPlayer.Get_Alid());
        tbTmpMap.Set_Alname(pstUser->m_tbPlayer.Get_Alname());
        tbTmpMap.Set_Npc(pstReq->m_ucIsNpc);
        tbTmpMap.Set_Center_pos(tbTmpMap.m_nId);
        tbTmpMap.Set_Name_update_time(CTimeUtils::GetUnixTime());
        tbTmpMap.Set_Bid(CMapBase::GetBlockIdFromPos(tbTmpMap.m_nId));
        tbTmpMap.Set_Prison_flag(0);
        tbTmpMap.Set_Expire_time(0);

        // get data
        CAwsRequest::UpdateItem(pstSession, &tbTmpMap, ExpectedDesc(), 0, true);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessReqCommand_UserInfoCreate:[map_pos=%ld][seq=%u]",
            tbTmpMap.m_nId,
            pstSession->m_udwSeqNo));

        // send request 
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_UserInfoCreate: map update request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 4. 处理响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->ResetAwsInfo();

        //// 更新统计数据
        //poGameSvr->m_aastProvinceInfo[pstReq->m_udwSvrId][pstReq->m_ucProvince].m_udwCityNum += CConfBase::GetInt("MirrorNum");
        //if (static_cast<TINT32>(poGameSvr->m_aastProvinceInfo[pstReq->m_udwSvrId][pstReq->m_ucProvince].m_udwPlainNum) > CConfBase::GetInt("MirrorNum"))
        //{
        //    poGameSvr->m_aastProvinceInfo[pstReq->m_udwSvrId][pstReq->m_ucProvince].m_udwPlainNum -= CConfBase::GetInt("MirrorNum");
        //}
        //else
        //{
        //    poGameSvr->m_aastProvinceInfo[pstReq->m_udwSvrId][pstReq->m_ucProvince].m_udwPlainNum = 0;
        //}

        // 检测是否要跳过新手教学，直接guide_finish
        CAwsRequest::UpdateUserAccountInfo(pstSession, &pstSession->m_stUserInfo.m_tbLogin, &pstSession->m_stReqParam);

        // 更新标记
        pstSession->m_stReqParam.m_ucIsNewSvrPlayer = TRUE;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);
        CAwsRequest::GetGlobalId(pstSession, EN_GLOBAL_PARAM__REPORT_ID);

        // send request 
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_UserInfoCreate: send global mail and report id req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }

        return 0;
    }


    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        TbUser_stat& tbUser_stat = pstSession->m_stUserInfo.m_tbUserStat;
        tbUser_stat.Set_Uid(pstUser->m_tbPlayer.m_nUid);
        for (unsigned int i = 0; i < pstSession->m_vecAwsRsp.size(); ++i)
        {
            TbParam tbParam;
            AwsRspInfo& rspInfo = *pstSession->m_vecAwsRsp[i];
            if (CCommonFunc::GetTableRawName(rspInfo.sTableName) == EN_AWS_TABLE_PARAM)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(rspInfo, &tbParam);
                if (dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_UserInfoCreate: get global mail and report id failed[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -8;
                }
                if (EN_GLOBAL_PARAM__MAIL_ID == tbParam.m_nKey)
                {
                    tbUser_stat.Set_Newest_mailid(tbParam.m_nVal); // 使其接收新邮件
                    tbUser_stat.Set_Birth_m(tbParam.m_nVal);
                    tbUser_stat.Set_Newest_mailid(tbParam.m_nVal);
                    tbUser_stat.Set_Return_mailid(tbParam.m_nVal);
                }
                else if (EN_GLOBAL_PARAM__REPORT_ID == tbParam.m_nKey)
                {
                    tbUser_stat.Set_Newest_reportid(tbParam.m_nVal);
                    tbUser_stat.Set_Birth_r(tbParam.m_nVal);
                    tbUser_stat.Set_Newest_reportid(tbParam.m_nVal);
                    tbUser_stat.Set_Return_reportid(tbParam.m_nVal);
                }
            }
            else
            {
                assert(0);
            }
        }

        pstSession->m_stUserInfo.m_udwWildNum = 0;
        return 0;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_NewLoginCreate(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    CGameSvrInfo *poGameSvr = CGameSvrInfo::GetInstance();

    TUINT32 udwSvrNum = CGameSvrInfo::GetInstance()->m_udwSvrNum;
    if(udwSvrNum <= 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("wrong server num [svr_num=%u] [seq=%u]", udwSvrNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    pstSession->m_udwTmpSvrNum = udwSvrNum;

    // 1. 发送创建请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(pstSession->m_stReqParam.m_udwUserId > 0 || pstSession->m_stReqParam.m_uddwDeviceId == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        pstSession->m_stReqParam.m_bLotteryRefresh = FALSE;

        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__USER_ID);

        TbSvr_stat tbSvrStat;
        TbMap tbThrone; //王座的map信息
        for(TUINT32 idx = 0; idx < udwSvrNum; idx++)
        {
            //拿到svr_stat表中的信息
            tbSvrStat.Reset();
            tbSvrStat.Set_Sid(idx);
            CAwsRequest::GetItem(pstSession, &tbSvrStat, ETbSVR_STAT_OPEN_TYPE_PRIMARY);

            //拿到王座的map信息
            //TODO
            tbThrone.Reset();
            tbThrone.Set_Sid(idx);
            tbThrone.Set_Id(THRONE_POS);
            CAwsRequest::GetItem(pstSession, &tbThrone, ETbMAP_OPEN_TYPE_PRIMARY);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate:: send login create req. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -2;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TUINT32 udwSvrStatNum = 0;
        TUINT32 udwThroneMapNum = 0;

        TbParam tbParamUserId;
        AwsConsumeInfo consumeInfo;

        //获取结果
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_vecAwsRsp.size(); ++dwIdx)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[dwIdx];
            string udwTableName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName.c_str());
            if(udwTableName == EN_AWS_TABLE_SVR_STAT)
            {
                TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_atbTmpSvrStat[udwSvrStatNum]);
                if(dwRetCode <= 0)
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("can't find corresponding svr_stat info [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -3;
                }
                udwSvrStatNum++;
                continue;
            }
            if(udwTableName == EN_AWS_TABLE_MAP)
            {
                TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_atbTmpThroneMap[udwThroneMapNum]);
                if(dwRetCode <= 0)
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("can't find corresponding throne map info [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -4;
                }
                udwThroneMapNum++;
                continue;
            }
            if(udwTableName == EN_AWS_TABLE_PARAM)
            {
                TINT32 dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbParamUserId, consumeInfo);
                if(dwRetCode < 0 || tbParamUserId.m_nVal == 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                    return -5;
                }
                continue;
            }
        }

        if(udwSvrStatNum != udwSvrNum || udwThroneMapNum != udwSvrNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("wrong data number [svr_stat_num=%u] [throne_map_num=%u] [svr_num=%u] [seq=%u]",
                udwSvrStatNum, udwThroneMapNum, udwSvrNum, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        pstSession->m_udwTmpSvrStatNum = udwSvrStatNum;
        pstSession->m_udwTmpThroneMapNum = udwThroneMapNum;

        TCHAR szRegisterResponse[MAX_JSON_LEN];
        TCHAR szUrl[1024];
        memset(szRegisterResponse, 0, sizeof(szRegisterResponse));
        const char *pszUrlBeg = strchr(pstSession->m_stReqParam.m_szReqUrl, '?');
        string strRequest = pszUrlBeg;
        string strUid = "uid=" + CCommonFunc::NumToString(tbParamUserId.m_nVal);
        CMsgBase::StringReplace(strRequest, "uid=0", strUid);
        std::size_t end = strRequest.find("command=");
        if(end != std::string::npos)
        {
            strRequest.resize(end + strlen("command="));
        }
        CMsgBase::StringReplace(strRequest, "command=", "command=new_visitor_register");

        memset(szUrl, 0, sizeof(szUrl));
        sprintf(szUrl, "%s%s", CConfBase::GetString("account_url_pre", "serv_url").c_str(), strRequest.c_str());

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate: Url:%s [seq=%u]", szUrl, pstSession->m_stUserInfo.m_udwBSeqNo));

        CURLcode res = CToolBase::ResFromUrl(szUrl, szRegisterResponse);
        if(CURLE_OK != res)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_RELATED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate: visitor_register failed[%d][seq=%u]", res, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        for(TUINT32 udwIdx = 0; udwIdx < strlen(szRegisterResponse); ++udwIdx)
        {
            if(szRegisterResponse[udwIdx] == '\n')
            {
                szRegisterResponse[udwIdx] = ' ';
            }
        }
        Json::Reader reader;
        pstSession->m_JsonValue.clear();
        if(FALSE == reader.parse(szRegisterResponse, pstSession->m_JsonValue))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_RELATED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate: parse visitor_register response failed. [seq=%u]", pstSession->m_udwSeqNo));
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate: visitor_register [response=%s] [seq=%u]", szRegisterResponse, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        if(pstSession->m_JsonValue.isObject()
        && pstSession->m_JsonValue.isMember("res_header")
        && pstSession->m_JsonValue["res_header"].isObject()
        && pstSession->m_JsonValue["res_header"].isMember("ret_code")
        && pstSession->m_JsonValue["res_header"]["ret_code"].asInt() == 0)
        {
            pstSession->m_JsonValue.clear();
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate: visitor_register succ [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        else
        {
            pstSession->m_JsonValue.clear();
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_RELATED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate: parse visitor_register failed. [seq=%u]", pstSession->m_udwSeqNo));
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate: visitor_register [response=%s] [seq=%u]", szRegisterResponse, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        // set next procedure--// 转入创建用户数据的流程
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__USER_INFO_CREATE;
        pstSession->m_stReqParam.m_ucIsNewPlayer = EN_NEW_PLAYER__CREATE;

        // 服务器选择
        if(pstSession->m_stReqParam.m_ucIsNpc == 0) // 真实用户
        {
            // @rock  真实用户创建
            if(pstSession->m_stReqParam.m_udwSvrId == (TUINT32)-1)
            {
                pstSession->m_stReqParam.m_udwSvrId = poGameSvr->GetNewPlayerSvr();     //wave@20160730: 流程已修改为login_center来为新用户选择sid，此处不应该走到
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_NewLoginCreate::Create user. get svr [sid=%u][seq=%u]",
                    pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }
        else
        {
            // npc用户通过指定的svr进行创建
            TCHAR *pSvrId = strstr(pstSession->m_stReqParam.m_szReqUrl, "sid=");
            if(pSvrId == NULL)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                return -7;
            }
            else
            {
                pstSession->m_stReqParam.m_udwSvrId = atoi(pSvrId + 4);
                if(pstSession->m_stReqParam.m_udwSvrId == (TUINT32)-1)
                {
                    pstSession->m_stReqParam.m_udwSvrId = poGameSvr->GetNewPlayerSvr();
                }
            }
        }

        // 检查服务器状态，如果已经满，则取消创建
//         if(poGameSvr->CheckSvrStatus(pstSession->m_stReqParam.m_udwSvrId) == FALSE)
//         {
//             pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_IS_FULL;
//             return -8;
//         }

        // get data
        CAwsRequest::LoginCreate(pstSession, tbParamUserId.m_nVal);

        // send request
        bNeedResponse = TRUE;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("send login create req. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -9;
        }
        return 0;
    }

    assert(0);
    return 0;
}

// function  ===> 返回客户端假的用户数据,假数据不做更新,属于新用户第一次登录的教学假数据
// 全新用户创建时客户端所传参数
// lg=1 & uid=0 & sid=-1 & cid=0 & aid=0 & pp=0 & pg=0 & lang=0
TINT32 CProcessPlayer::ProcessCmd_LoginFake(SSession *pstSession, TBOOL &bNeedResponse)
{

    CGameSvrInfo *poGameSvr = CGameSvrInfo::GetInstance();

    pstSession->m_stReqParam.m_bLotteryRefresh = FALSE;

    // 创建用户信息
    CAwsRequest::CreateFakeUserInfo(pstSession, &pstSession->m_stUserInfo, pstSession->m_stReqParam.m_udwSvrId, 100100);

    // 计算信息
    CCommonBase::ComputeUserMight(&pstSession->m_stUserInfo);

    // 设置fake标记
    pstSession->m_stReqParam.m_ucIsNewPlayer = EN_NEW_PLAYER__FAKE;
    pstSession->m_stReqParam.m_ucIsNewSvrPlayer = 0;

    // 服务器选择
    TUINT32 udwNewSid = 0;
    if(pstSession->m_stReqParam.m_udwSvrId == (TUINT32)-1)
    {
        udwNewSid = poGameSvr->GetNewPlayerSvr();
    }
    else
    {
        udwNewSid = pstSession->m_stReqParam.m_udwSvrId;
    }
    TbLogin& tbLogin = pstSession->m_stUserInfo.m_tbLogin;
    tbLogin.Set_Sid(udwNewSid);
    tbLogin.Set_Gem(EN_GEM_REWARD__LOGIN_CREATE);

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_LoginFake:[new svr id=%u] [seq=%u]", udwNewSid,pstSession->m_udwSeqNo));

    //fake
    pstSession->m_stUserInfo.m_tbPlayer.Set_Sid(udwNewSid);

    pstSession->m_stReqParam.m_udwCityId = pstSession->m_stUserInfo.m_stCityInfo.m_stTblData.m_nPos;
    pstSession->m_stReqParam.m_udwSvrId = udwNewSid;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_GuideFinishStage(SSession *pstSession, TBOOL &bNeedResponse)
{   
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[0]);

    //网络超时 重复请求保护
    if (BITTEST(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, dwType) != 0)
    {
        return 0;
    }

    if(EN_GUIDE_FINISH_STAGE_1_RESEARCH == dwType) //学会科技升级
    {
        pstCity->m_stTblData.m_bResearch[0].m_addwLevel[0] = 1;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESEARCH);

        BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, dwType);
        pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);

        //活动积分统计
        CActivitesLogic::ComputeResearchUpgradeScore(pstUser, EN_RESEARCH_TYPE__SCOUT, 1);
        CActivitesLogic::ComputeSpeedUpItemScore(pstUser, 11);

        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_ING_RESEARCH_TIME);

        //add_reward
        SSpGlobalRes stGlobalRes;
        stGlobalRes.Reset();
        TINT32 dwRetCode = 0;
        dwRetCode = CGlobalResLogic::GetResearchReward(0, 1, &stGlobalRes);

        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("BuildingActionDone:get  reward fail ret_code=%d type=%u lv=%u [seq=%u]",
                dwRetCode,
                EN_RESEARCH_TYPE__SCOUT,
                1,
                pstSession->m_udwSeqNo));
        }

        dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalRes);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("BuildingActionDone:add  reward fail ret_code=%d type=%u lv=%u [seq=%u]",
                dwRetCode,
                EN_RESEARCH_TYPE__SCOUT,
                1,
                pstSession->m_udwSeqNo));
        }

    }
    else if(EN_GUIDE_FINISH_STAGE_2_TRAIN_TROOP == dwType) //学会训练军队
    {
        SCommonTroop& stCommonTroop = pstCity->m_stTblData.m_bTroop[0];
        stCommonTroop[EN_TROOP_TYPE__T1_INFANTRY] += 5;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);

        BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, dwType);
        pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);

        pstSession->m_ddwTroopChange += 5;

        //活动积分统计
        CActivitesLogic::ComputeTrainTroopScore(pstUser, (TUINT32)EN_TROOP_TYPE__T1_INFANTRY, 5);

        //task
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_TROOP_TRAIN_NUM, 5, 0, CToolBase::GetTroopLvByTroopId(EN_TROOP_TYPE__T1_INFANTRY));
    }
//     else if(EN_GUIDE_FINISH_STAGE_3_ADD_KNIGHT_EXP == dwType)
//     {
//         BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, dwType);
//         pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);
// 
//         //增加英雄经验
//         CCityBase::AddKnightExp(pstCity, 0, 1000);
//         //英雄任职
//         pstCity->m_stTblData.m_bKnight[0].ddwPos = EN_KNIGHT_POS__TROOP;
//         pstCity->m_stTblData.m_bKnight[0].ddwStatus = EN_KNIGHT_STATUS__ASSIGNING;
//         pstCity->m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
//     }
//     else if(EN_GUIDE_FINISH_STAGE_4_OCCUPY_RESOURCE == dwType)
//     {
//         CCityBase::AddFood(pstCity, 10000);
//         BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, dwType);
//         pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__FOOD] = 10000;
//         pstUser->m_tbPlayer.SetFlag(TbPLAYER_FIELD_GAIN_RESOURCE);
// 
//         pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);
//     }
//     else if(EN_GUIDE_FINISH_STAGE_5_ASSIGN_HERO_SKILL == dwType)
//     {
//         BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, dwType);
//         pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);
//     }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GuideFinishStage::type[%u] not define [seq=%u]", dwType,pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_GuideFinish(SSession *pstSession, TBOOL &bNeedResponse)
{
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BuildingUpgrade(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    SBuildingInfo stBuildingInfo;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;

    // 输入参数
    // upgrade_type(0:normal;1:instant,buy:2) ==> ucUpgradeType(key0)
    // pos([0-79]:city;[80-99]:dragon_city) ==> ucPos(key1)
    // building_type ==> ucId(key2)
    // traget_level ==> ucTargetLevel(key3)
    // gem_or_time_cost ==> udwKey(key4)
    // exp ==> udwExp(key5)
    // is_need_help(目前没有用到) ==> ucIsNeedHelp(key6)
    // cidx ===>useless
    TUINT8 ucUpgradeType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT8 ucId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT8 ucTargetLevel = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TUINT32 udwKey = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TUINT32 udwExp = strtoul(pstSession->m_stReqParam.m_szKey[5], NULL, 10);
    TUINT8 ucIsNeedHelp = atoi(pstSession->m_stReqParam.m_szKey[6]);

    TUINT64 uddwClientAct = strtoull(pstSession->m_stReqParam.m_szKey[7], NULL, 10);

    SCityBuildingNode* pstBuildingNode = CCityBase::GetBuildingAtPos(&tbCity, udwPos);
    TUINT8 ucCurLevel = 0;
    if(pstBuildingNode)
    {
        ucCurLevel = pstBuildingNode->m_ddwLevel;
    }

    if(pstSession->m_stReqParam.m_szKey[0][0] == 0
    || pstSession->m_stReqParam.m_szKey[1][0] == 0
    || pstSession->m_stReqParam.m_szKey[2][0] == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: upgradetype or pos or building_type is null [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    // 防止客户端网络问题重试
    if(ucCurLevel != ucTargetLevel - 1) //llt add
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: (curlv != tragetlv -1) [curlv=%u] [tragetlv=%u] [seq=%u]", \
            ucCurLevel, ucTargetLevel, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    //防止客户端网络问题重试,lucien@20150902:即便出现网络延迟，CurLevel也不会立即变成TargetLevel，故以上判断无效
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; udwIdx++)
    {
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }

        if(pstUser->m_atbSelfAlAction[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING
        && (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE || pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
        && pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stBuilding.m_ddwPos == udwPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: duplicate building [existent pos=%u] [target pos=%u] [existent action id=%ld] [seq=%u]", \
                pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stBuilding.m_ddwPos, udwPos, pstUser->m_atbSelfAlAction[udwIdx].m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }

    // 后台校验建造数量是否超限制
    if (ucTargetLevel == 1 && CCityBase::HasBuildCategoryCapacity(pstUser, ucId) == FALSE)
    {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: building category full [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
    }

    TbAction* ptbBufferAction = CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, EN_BUFFER_INFO_SECOND_BUILDING_TASK);

    if(ucUpgradeType == 0
    && ptbBufferAction == NULL
    && pstCity->m_stActionStat.m_ucDoingBuildingNum > 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BUILDING_DOING;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: building or removing now [ucUpgradeType=%u] [ucPos=%u] [DoingBuildingNum=%u] [seq=%u]", \
            ucUpgradeType, udwPos, pstCity->m_stActionStat.m_ucDoingBuildingNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    if(ucUpgradeType == 0
    && ptbBufferAction != NULL
    && pstCity->m_stActionStat.m_ucDoingBuildingNum >= 2)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TWO_BUILDING_ACTION_DOING;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: building or removing now [ucUpgradeType=%u] [ucPos=%u] [DoingBuildingNum=%u] [seq=%u]", \
            ucUpgradeType, udwPos, pstCity->m_stActionStat.m_ucDoingBuildingNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    // 建筑升级时pos上的建筑类型判断
    if(ucTargetLevel > 1)
    {
        if(!pstBuildingNode)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -5;
        }
        if(pstBuildingNode->m_ddwType != ucId)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: pos[%u], raw_type[%u], new_type[%u] [seq=%u]", \
                udwPos, pstBuildingNode->m_ddwType, ucId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
    }
    else if(ucTargetLevel == 1)
    {
        SCityBuildingNode stNewBuilding;
        stNewBuilding.m_ddwPos = udwPos;
        stNewBuilding.m_ddwType = ucId;
        stNewBuilding.m_ddwLevel = ucTargetLevel;
        if(CCommonLogic::CheckBuildingCollision(&pstCity->m_stTblData, stNewBuilding) == TRUE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -7;
        }
    }

    // 判断是否能建多个的建筑
    TINT8 ucCanConstructMul = poGameInfo->m_oJsonRoot["game_building"][CCommonFunc::NumToString(ucId)]["a"]["a2"].asInt();

    if(0 == ucCanConstructMul
    && 1 == ucTargetLevel
    && 0 < CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, ucId))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: this building has been built and can't build more [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return 0;
    }

    poGameInfo->GetBuildingInfo(ucId, ucTargetLevel, &stBuildingInfo);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: m_audwBaseCost[%u:%u:%u:%u:%u:%u:%u:%u], m_stSpecialItem[%u:%u] [seq=%u]", \
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__GOLD],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__FOOD],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__WOOD],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__STONE],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__ORE],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__SPACE],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__POPULATION],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__TIME],
        stBuildingInfo.m_stSpecialItem.m_udwId,
        stBuildingInfo.m_stSpecialItem.m_udwNum,
        pstSession->m_stUserInfo.m_udwBSeqNo));


    // 检测消耗时间和宝石的合法性
    TINT32 dwBuff = 10000 + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_BUILDING_SPEED_UP_PERCENT].m_ddwBuffTotal;
    TINT32 dwRealTimeCost = stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__TIME] - pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_BUILDING_TIME_REDUCE].m_ddwBuffTotal;
    if (dwRealTimeCost <= 0)
    {
        dwRealTimeCost = 1;
    }
    dwRealTimeCost = dwRealTimeCost / (1.0 * dwBuff / 10000);
    TINT32 dwFreeTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
    if (ucUpgradeType == 0)
    {
        if (!CCommonBase::IsCorrectTime(udwKey, dwRealTimeCost))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildingUpgrade: time_cost is incorrect [raw_time:%d buff:%d given_time:%u, calc_time:%d] [seq=%u]", 
                stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__TIME], dwBuff, udwKey, dwRealTimeCost, pstSession->m_stUserInfo.m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }
    }
    else
    {
        dwRealTimeCost -= dwFreeTime;
        if (!CCommonBase::CheckInstantGem(udwKey, dwRealTimeCost))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildingUpgrade: instant gem is incorrect [raw_time:%d buff:%d given_gem_cost:%u, calc_time_cost:%d] [seq=%u]", 
                stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__TIME], dwBuff, udwKey, dwRealTimeCost, pstSession->m_stUserInfo.m_udwBSeqNo)); 
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }
    }

    // 判定资源/gem/物品是否足够:0-normal;1-instant;2-buy
    if(ucUpgradeType == 0 || ucUpgradeType == 1)
    {
        if(FALSE == CCityBase::HasEnoughResource(pstCity, &stBuildingInfo.m_addwBaseCost[0]))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: resource not enough [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
    }
    if(ucUpgradeType == 1 || ucUpgradeType == 2)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwKey))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: gem not enough [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }
    }
    if(stBuildingInfo.m_ucUpradeNeedItem == TRUE)   // item所有方式(0-normal;1-instant;2-buy)都需要
    {
        if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, stBuildingInfo.m_stSpecialItem.m_udwId, stBuildingInfo.m_stSpecialItem.m_udwNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: item not enough [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }
    }

    // 消耗资源/gem/物品
    if(ucUpgradeType == 0 || ucUpgradeType == 1)
    {
        CCityBase::CostResource(pstCity, &stBuildingInfo.m_addwBaseCost[0]);
        //active_quest
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
        {
            pstUser->m_udwCostResource += stBuildingInfo.m_addwBaseCost[udwIdx];
        }

    }
    if(ucUpgradeType == 1 || ucUpgradeType == 2)
    {
        pstSession->m_udwGemCost = udwKey;
        CPlayerBase::CostGem(pstUser, udwKey);
    }
    if(stBuildingInfo.m_ucUpradeNeedItem == TRUE)
    {
        dwRetCode = CItemBase::CostItem(&pstUser->m_tbBackpack, stBuildingInfo.m_stSpecialItem.m_udwId, stBuildingInfo.m_stSpecialItem.m_udwNum);

        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: item use failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -11;
        }
    }

    // 5. 执行upgrade
    UActionParam stParam;
    stParam.m_stBuilding.SetValue(udwPos, ucId, ucTargetLevel, udwExp, (TCHAR*)tbPlayer.m_sUin.c_str());
    if(ucUpgradeType == 0 && udwKey > 1) //耗时小于等于1s的走instant build
    {
        TUINT64 uddwActionId = 0;
        //判断客户端的action id
        if (uddwClientAct != 0)
        {
            uddwActionId = uddwClientAct;
            if (pstUser->m_tbLogin.m_nClient_seq == MAX_CLIENT_SEQ)
            {
                pstUser->m_tbLogin.m_nClient_seq = 0;
            }
            if (uddwActionId != CToolBase::GetClientBuildTaskId(tbPlayer.m_nUid, pstUser->m_tbLogin.m_nClient_seq))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: item use failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -13;
            }
            pstUser->m_tbLogin.Set_Client_seq(pstUser->m_tbLogin.m_nClient_seq + 1);
        }
        // 生成主城action
        CActionBase::AddAlAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__BUILDING,
            EN_ACTION_SEC_CLASS__BUILDING_UPGRADE,
            EN_BUILDING_STATUS__BUILDDING, udwKey, &stParam, 0, uddwActionId);

        if(ucIsNeedHelp && pstUser->m_tbPlayer.m_nAlpos && pstUser->m_udwActionNum)
        {
            TbAlliance_action &tbAction = pstUser->m_atbSelfAlAction[pstUser->m_udwSelfAlActionNum - 1];

            TINT32 dwCanHelpTimes = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALLIANCE_HELP_NUM].m_ddwBuffTotal;
            tbAction.Set_Sal(tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            tbAction.Set_Can_help_num(dwCanHelpTimes);
            tbAction.Set_Helped_num(0);
        }
    }
    else
    {
        CProcessAction::BuildingActionDone(pstSession, pstUser, pstCity, EN_ACTION_SEC_CLASS__BUILDING_UPGRADE, &stParam.m_stBuilding);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BuildingRemove(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;

    // 输入参数
    // remove_type(0:normal;1:instant(use),buy_and_use:2) ==> ucRemoveType(key0)
    // pos([0-79]:city;[80-99]:dragon_city) ==> ucPos(key1)
    // cost time ==> udwTimeCost(key2)
    // item(shatter wave) price ==> udwItemPrice(key3)
    // cidx useless(key4)
    TUINT8 ucRemoveType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwTimeCost = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TUINT32 udwItemPrice = strtoul(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TUINT32 udwDestroyItem = EN_ITEM_ID__DRAGONS_STOMP;//默认的瞬间拆除需要的物品id

    SCityBuildingNode* pstBuildingNode = CCityBase::GetBuildingAtPos(&tbCity, udwPos);

    // 判定合法性
    if(pstSession->m_stReqParam.m_szKey[0][0] == 0
    || pstSession->m_stReqParam.m_szKey[1][0] == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingRemove: removetype or pos is null [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    if(pstBuildingNode == NULL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    //防止客户端网络问题重试
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; udwIdx++)
    {
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }

        if(pstUser->m_atbSelfAlAction[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING
        && (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE || pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
        && pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stBuilding.m_ddwPos == udwPos)
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: duplicate removing [existent pos=%u] [target pos=%u] [existent action id=%ld] [seq=%u]", \
                pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stBuilding.m_ddwPos, udwPos, pstUser->m_atbSelfAlAction[udwIdx].m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return 0;
        }
    }

    TbAction* ptbAction = CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, EN_BUFFER_INFO_SECOND_BUILDING_TASK);

    if(0 == ucRemoveType
    && ptbAction == NULL
    && pstCity->m_stActionStat.m_ucDoingBuildingNum > 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BUILDING_DOING;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingRemove: building or removing now [ucRemoveType=%u] [ucPos=%u][seq=%u]", \
            ucRemoveType, udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }
    if(ucRemoveType == 0
    && ptbAction != NULL
    && pstCity->m_stActionStat.m_ucDoingBuildingNum >= 2)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TWO_BUILDING_ACTION_DOING;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingRemove: building or removing now [ucRemoveType=%u] [ucPos=%u][seq=%u]", \
            ucRemoveType, udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -6;
    }

    // (1-instant(use);2-buy_and_use情况下)判断拆除建筑的item是否足够: 0-normal;1-instant(use);2-buy_and_use
    if(ucRemoveType == 1)
    {
        if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwDestroyItem))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_DESTROY_ITEM;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingRemove: item not enough [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        dwRetCode = CItemBase::CostItem(&pstUser->m_tbBackpack, udwDestroyItem);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingRemove: item use failed [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
    }
    else if(ucRemoveType == 2)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwItemPrice))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingRemove: gem not enough [seq=%u]", pstSession->m_udwSeqNo));
            return -9;
        }

        pstSession->m_udwGemCost = udwItemPrice;
        CPlayerBase::CostGem(pstUser, udwItemPrice);
    }

    // 3. 执行remove
    UActionParam stParam;
    stParam.m_stBuilding.SetValue(udwPos, pstBuildingNode->m_ddwType, pstBuildingNode->m_ddwLevel, 0, (TCHAR*)tbPlayer.m_sUin.c_str());
    if (ucRemoveType == 0 && udwTimeCost > 1)   // (0-normal)情况下
    {
        // 生成主城action
        CActionBase::AddAlAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__BUILDING,
            EN_ACTION_SEC_CLASS__BUILDING_REMOVE,
            EN_BUILDING_STATUS__REMOVEING, udwTimeCost, &stParam);
    }
    else    // (1-instant(use);2-buy_and_use情况下)
    {
        CProcessAction::BuildingActionDone(pstSession, pstUser, pstCity, EN_ACTION_SEC_CLASS__BUILDING_REMOVE, &stParam.m_stBuilding);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BuildingMove(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TUINT32 udwSourcePos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwTargetPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SCityBuildingNode* pstBuildingNode = CCityBase::GetBuildingAtPos(&pstCity->m_stTblData, udwSourcePos);
    if(pstBuildingNode)
    {
        pstBuildingNode->m_ddwPos = udwTargetPos;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
    }
    for(TUINT32 udwActionIdx = 0; udwActionIdx < pstUser->m_udwSelfAlActionNum; ++udwActionIdx)
    {
        if(pstUser->m_aucSelfAlActionFlag[udwActionIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwActionIdx].m_nId))
        {
            continue;
        }
        TbAlliance_action* ptbBuildingAction = &pstUser->m_atbSelfAlAction[udwActionIdx];
        if(ptbBuildingAction->m_nMclass != EN_ACTION_MAIN_CLASS__BUILDING)
        {
            continue;
        }
        if(ptbBuildingAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_UPGRADE
        && ptbBuildingAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
        {
            continue;
        }
        if(ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwPos == udwSourcePos)
        {
            ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwPos = udwTargetPos;
            ptbBuildingAction->SetFlag(TbALLIANCE_ACTION_FIELD_PARAM);
            pstUser->m_aucSelfAlActionFlag[udwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        }
    }

    vector<SCityBuildingNode> vecNewBuilding;
    for(TUINT32 udwActionIdx = 0; udwActionIdx < pstUser->m_udwSelfAlActionNum; ++udwActionIdx)
    {
        if(pstUser->m_aucSelfAlActionFlag[udwActionIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwActionIdx].m_nId))
        {
            continue;
        }
        TbAlliance_action* ptbBuildingAction = &pstUser->m_atbSelfAlAction[udwActionIdx];
        if(ptbBuildingAction->m_nMclass != EN_ACTION_MAIN_CLASS__BUILDING)
        {
            continue;
        }
        if(ptbBuildingAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
        {
            continue;
        }
        if(ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel == 1)
        {
            SCityBuildingNode stNewNode;
            stNewNode.Reset();
            stNewNode.m_ddwType = ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwType;
            stNewNode.m_ddwPos = ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwPos;
            stNewNode.m_ddwLevel = ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel;
            vecNewBuilding.push_back(stNewNode);
        }
    }

    if(CCommonLogic::CheckBuildingCollision(&pstCity->m_stTblData, vecNewBuilding, pstSession->m_udwSeqNo) == TRUE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BuildingEdit(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    TbDecoration& tbDecoration = pstUser->m_tbDecoration;

    TINT32 dwRetCode = 0;
    TUINT32 udwCount = 0;
    TUINT32 udwDecoCount = 0;
    TINT32 adwSrcPos[MAX_BUILDING_NUM_IN_ONE_CITY];
    TINT32 adwTarPos[MAX_BUILDING_NUM_IN_ONE_CITY];
    memset((TVOID*)adwSrcPos, 0, sizeof(TINT32)*MAX_BUILDING_NUM_IN_ONE_CITY);
    memset((TVOID*)adwTarPos, 0, sizeof(TINT32)*MAX_BUILDING_NUM_IN_ONE_CITY);
    TINT32 adwDecoPos[MAX_BUILDING_NUM_IN_ONE_CITY];
    TINT32 adwDecoId[MAX_BUILDING_NUM_IN_ONE_CITY];
    string strBuildingId = "";
    memset((TVOID*)adwDecoPos, 0, sizeof(TINT32)*MAX_BUILDING_NUM_IN_ONE_CITY);
    memset((TVOID*)adwDecoId, 0, sizeof(TINT32)*MAX_BUILDING_NUM_IN_ONE_CITY);

    const TCHAR ucPair = ':';
    const TCHAR ucEntry = ',';
    //key0=src1:tar1,src2:tar2
    //key1=pos1:id1,pos2:id2
    const TCHAR* pCurPos = pstSession->m_stReqParam.m_szKey[0];
    const TCHAR* pDecoInfo = pstSession->m_stReqParam.m_szKey[1];

    while(pCurPos && *pCurPos)
    {
        adwSrcPos[udwCount] = atoi(pCurPos);
        pCurPos = strchr(pCurPos, ucPair);
        if(pCurPos != NULL)
        {
            pCurPos++;
            adwTarPos[udwCount] = atoi(pCurPos);
            udwCount++;
            if(udwCount == MAX_BUILDING_NUM_IN_ONE_CITY)
            {
                break;
            }
        }
        else
        {
            break;
        }
        pCurPos = strchr(pCurPos, ucEntry);
        if(pCurPos != NULL)
        {
            pCurPos++;
        }
    }

    while (pDecoInfo && *pDecoInfo)
    {
        adwDecoPos[udwDecoCount] = atoi(pDecoInfo);
        pDecoInfo = strchr(pDecoInfo, ucPair);
        if (pDecoInfo != NULL)
        {
            pDecoInfo++;
            adwDecoId[udwDecoCount] = atoi(pDecoInfo);
            udwDecoCount++;
            if (udwDecoCount == MAX_BUILDING_NUM_IN_ONE_CITY)
            {
                break;
            }
        }
        else
        {
            break;
        }
        pDecoInfo = strchr(pDecoInfo, ucEntry);
        if (pDecoInfo != NULL)
        {
            pDecoInfo++;
        }
    }
    //if (udwCount == 0 && udwDecoCount == 0)
    //{
    //    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
    //    return -2;
    //}

    //清除building list中的装饰物
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    Json::Value jTmp = pstGameInfo->m_oJsonRoot["game_building"];
    for (TUINT32 udwIdx = 0; udwIdx < tbCity.m_bBuilding.m_udwNum; udwIdx++)
    {
        strBuildingId = CCommonFunc::NumToString(tbCity.m_bBuilding[udwIdx].m_ddwType);
        if (jTmp[strBuildingId]["a"]["a12"].asInt() == 2)
        {   //装饰物
            CCityBase::DelBuildingAtPos(&tbCity, tbCity.m_bBuilding[udwIdx].m_ddwPos);
        }
    }
    pstSession->m_JsonValue = Json::Value(Json::objectValue);
    pstSession->m_JsonValue["building_move"] = Json::Value(Json::arrayValue);

    std::vector<std::pair<SCityBuildingNode*, TUINT32> > buildingToMove;
    std::vector<std::pair<TbAlliance_action*, TUINT32> > actionToModify;
    for(TUINT32 udwIdx = 0; udwIdx < udwCount; ++udwIdx)
    {
        SCityBuildingNode* pstNode = CCityBase::GetBuildingAtPos(&pstCity->m_stTblData, adwSrcPos[udwIdx]);
        //if (adwTarPos[udwIdx] == -1)    //收起装饰物
        //{
        //    dwRetCode = CPlayerBase::PickUpDecoration(&pstUser->m_tbDecoration, pstNode->m_ddwType);
        //    if (dwRetCode < 0)
        //    {
        //        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        //        TSE_LOG_ERROR(pstSession->m_poServLog, ("PickUpDecoration: pick up failed [id=%ld] [seq=%u]", pstNode->m_ddwType, pstSession->m_stUserInfo.m_udwBSeqNo));
        //        return -5;
        //    }
        //    CCityBase::DelBuildingAtPos(&tbCity, pstNode->m_ddwPos);
        //    continue;
        //}
        if (!pstNode || adwTarPos[udwIdx] == -1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        if(pstNode != NULL)
        {
            buildingToMove.push_back(std::make_pair(pstNode, adwTarPos[udwIdx]));
        }

        for(TUINT32 udwActionIdx = 0; udwActionIdx < pstUser->m_udwSelfAlActionNum; ++udwActionIdx)
        {
            if(pstUser->m_aucSelfAlActionFlag[udwActionIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwActionIdx].m_nId))
            {
                continue;
            }
            TbAlliance_action* ptbBuildingAction = &pstUser->m_atbSelfAlAction[udwActionIdx];
            if(ptbBuildingAction->m_nMclass != EN_ACTION_MAIN_CLASS__BUILDING)
            {
                continue;
            }
            if(ptbBuildingAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_UPGRADE
            && ptbBuildingAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
            {
                continue;
            }
            if(ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwPos == adwSrcPos[udwIdx])
            {
                actionToModify.push_back(std::make_pair(ptbBuildingAction, adwTarPos[udwIdx]));
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("BuildingEdit: [actionid=%ld][srcpos=%u][tarpos=%u][seq=%u]",
                    ptbBuildingAction->m_nId, adwSrcPos[udwIdx], adwTarPos[udwIdx], pstSession->m_udwSeqNo));
                pstUser->m_aucSelfAlActionFlag[udwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                break;
            }
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < buildingToMove.size(); ++udwIdx)
    {
        Json::Value temp = Json::Value(Json::arrayValue);
        temp.append(buildingToMove[udwIdx].first->m_ddwPos);
        temp.append(buildingToMove[udwIdx].second);
        pstSession->m_JsonValue["building_move"].append(temp);
        buildingToMove[udwIdx].first->m_ddwPos = buildingToMove[udwIdx].second;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
    }

    for(TUINT32 udwActionIdx = 0; udwActionIdx < actionToModify.size(); ++udwActionIdx)
    {
        Json::Value temp = Json::Value(Json::arrayValue);
        temp.append(actionToModify[udwActionIdx].first->m_bParam[0].m_stBuilding.m_ddwPos);
        temp.append(actionToModify[udwActionIdx].second);
        pstSession->m_JsonValue["building_move"].append(temp);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("BuildingTrueEdit: [actionid=%ld][srcpos=%u][tarpos=%u][seq=%u]",
            actionToModify[udwActionIdx].first->m_nId,
            actionToModify[udwActionIdx].first->m_bParam[0].m_stBuilding.m_ddwPos,
            actionToModify[udwActionIdx].second,
            pstSession->m_udwSeqNo));
        actionToModify[udwActionIdx].first->m_bParam[0].m_stBuilding.m_ddwPos =
            actionToModify[udwActionIdx].second;
        actionToModify[udwActionIdx].first->SetFlag(TbALLIANCE_ACTION_FIELD_PARAM);
    }

    vector<SCityBuildingNode> vecNewBuilding;
    for(TUINT32 udwActionIdx = 0; udwActionIdx < pstUser->m_udwSelfAlActionNum; ++udwActionIdx)
    {
        if(pstUser->m_aucSelfAlActionFlag[udwActionIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwActionIdx].m_nId))
        {
            continue;
        }
        TbAlliance_action* ptbBuildingAction = &pstUser->m_atbSelfAlAction[udwActionIdx];
        if(ptbBuildingAction->m_nMclass != EN_ACTION_MAIN_CLASS__BUILDING)
        {
            continue;
        }
        if(ptbBuildingAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
        {
            continue;
        }
        if(ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel == 1)
        {
            SCityBuildingNode stNewNode;
            stNewNode.Reset();
            stNewNode.m_ddwType = ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwType;
            stNewNode.m_ddwPos = ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwPos;
            stNewNode.m_ddwLevel = ptbBuildingAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel;
            vecNewBuilding.push_back(stNewNode);
        }
    }

    //添加装饰物
    Json::Value jDeco = Json::Value(Json::objectValue);
    for (TUINT32 udwIdx = 0; udwIdx < udwDecoCount; udwIdx++)
    {
        strBuildingId = CCommonFunc::NumToString(adwDecoId[udwIdx]);
        CCityBase::AddBuilding(adwDecoPos[udwIdx], adwDecoId[udwIdx], 1, tbCity);
        if (jDeco.isMember(strBuildingId))
        {
            jDeco[strBuildingId]["num"] = jDeco[strBuildingId]["num"].asInt() + 1;
        }
        else
        {
            jDeco[strBuildingId] = Json::Value(Json::objectValue);
            jDeco[strBuildingId]["num"] = 1;
        }
    }
    if (tbCity.m_bBuilding.m_udwNum >= MAX_BUILDING_NUM_IN_ONE_CITY)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildingEdit: the num of buildings has reached max [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;

    }
    if(CCommonLogic::CheckBuildingCollision(&pstCity->m_stTblData, vecNewBuilding, pstSession->m_udwSeqNo) == TRUE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }
    //修改db中装饰物表的信息
    Json::Value jDecoList = tbDecoration.m_jDecoration_list;
    Json::Value::Members jMember = jDecoList.getMemberNames();
    TUINT32 udwItemNum;
    for (TUINT32 udwIdx = 0; udwIdx < jMember.size(); udwIdx++)
    {
        udwItemNum = 0;
        if (jDeco.isMember(jMember[udwIdx]))
        {
            udwItemNum = jDeco[jMember[udwIdx]]["num"].asUInt();
        }
        dwRetCode = CPlayerBase::SetDecoration(&tbDecoration, jMember[udwIdx], udwItemNum);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -4;
        }
    }
    CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_EDIT_CITY);
    tbCity.SetFlag(TbCITY_FIELD_BUILDING);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_AgeUpgrade(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    /*
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    CGameInfo* pGameInfo = CGameInfo::GetInstance();
    TUINT32 dwNextAge = pstUser->m_tbPlayer.m_nAge + 1;
    if(dwNextAge >= pGameInfo->m_oJsonRoot["game_age"].size())
    {
        return 0;
    }

    SSpGlobalRes stGlobal;
    stGlobal.Reset();
    stGlobal.aRewardList[stGlobal.udwTotalNum].udwType = pGameInfo->m_oJsonRoot["game_age"][dwNextAge][0U].asUInt();
    stGlobal.aRewardList[stGlobal.udwTotalNum].udwId = pGameInfo->m_oJsonRoot["game_age"][dwNextAge][1U].asUInt();
    stGlobal.aRewardList[stGlobal.udwTotalNum].udwNum = pGameInfo->m_oJsonRoot["game_age"][dwNextAge][2U].asUInt();
    stGlobal.udwTotalNum++;

    if(CGlobalResLogic::HaveEnoughSpGlobalsRes(pstUser, pstCity, &stGlobal) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    //立马完成action
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if(pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }

        if(pstUser->m_atbSelfAlAction[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__BUILDING)
        {
            continue;
        }

        if(pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE
            || pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE
            || EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE == pstUser->m_atbSelfAlAction[udwIdx].m_nSclass)
        {
            CProcessAction::BuildingActionDone(pstSession, pstUser, pstCity,
                (EActionSecClass)pstUser->m_atbSelfAlAction[udwIdx].m_nSclass,
                &pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stBuilding,
                FALSE);
            pstUser->m_aucSelfAlActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &pstCity->m_stTblData.m_bBuilding[udwIdx];
        if(pstNode->m_ddwLevel == 0)
        {
            continue;
        }
        string strBuildingId = CCommonFunc::NumToString(pstNode->m_ddwType);
        TINT32 dwNextType = pGameInfo->m_oJsonRoot["game_building"][strBuildingId]["a"]["a10"].asInt();
        if(dwNextType > 0)
        {
            pstNode->m_ddwType = dwNextType;
            pstNode->m_ddwLevel = 1;
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
        }
    }

    //删除上个时代task
    CQuestLogic::RemoveAgeTask(pstUser,pstCity, pstUser->m_tbPlayer.m_nAge);
    //清掉上个时代的未完成kingdom quest
    CQuestLogic::RemoveAgeTopQuest(pstUser, pstUser->m_tbPlayer.m_nAge);
    */

    TINT64 ddwTargetAge = pstUser->m_tbPlayer.m_nAge + 1;
    if (ddwTargetAge < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AgeUpgrade: cur_age[%ld] error target_age[%ld] [seq=%u]",
            pstUser->m_tbPlayer.m_nAge, ddwTargetAge, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (ddwTargetAge <= 3)
    {
        pstUser->m_tbPlayer.Set_Age(pstUser->m_tbPlayer.m_nAge + 1);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ResearchUpgrade(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;
    SCommonResearch& stResearch = pstUser->m_stCityInfo.m_stTblData.m_bResearch[0];
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    // 输入参数
    // upgrade_type(0:normal;1:instant,buy:2) ==> ucUpgradeType
    // research_type ==> ucId
    // traget_level ==> ucTargetLevel
    // gem_or_time_cost ==> udwKey
    // exp ==> udwExp
    // is_need_help(目前没有用到) ==> ucIsNeedHelp
    TUINT8 ucUpgradeType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 udwId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwTargetlevel = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwKey = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TUINT32 udwExp = strtoul(pstSession->m_stReqParam.m_szKey[4], NULL, 10);
    TBOOL bIsNeedHelp = atoi(pstSession->m_stReqParam.m_szKey[5]);

    TUINT8 ucCurLevel = stResearch.m_addwLevel[udwId];

    // 1. 判定合法性
    if(pstCity == NULL || udwId >= EN_RESEARCH_TYPE__END || ucCurLevel != dwTargetlevel - 1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ResearchUpgrade: req param error [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
    if(ucUpgradeType == 0 && pstCity->m_stActionStat.m_ucDoingResearchNum > 0)//normal
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESEARCH_DOING;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: researching now [seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    // 检测消耗时间和宝石的合法性
    TINT32 dwRawTimeCost = CGameInfo::GetInstance()->m_oJsonRoot["game_research"][CCommonFunc::NumToString(udwId)]["r"]["r0"][dwTargetlevel - 1]["a0"].asInt();
    TINT32 dwBuff = 10000 + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_RESEARCH_SPEED_UP_PERCENT].m_ddwBuffTotal;
    TINT32 dwRealTimeCost = dwRawTimeCost - pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_RESEARCH_TIME_REDUCE].m_ddwBuffTotal;
    if (dwRealTimeCost <= 0)
    {
        dwRealTimeCost = 1;
    }
    dwRealTimeCost = dwRealTimeCost / (1.0 * dwBuff / 10000);
    TINT32 dwFreeTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
    if (ucUpgradeType == 0)
    {
        if (!CCommonBase::IsCorrectTime(udwKey, dwRealTimeCost))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ResearchUpgrade: time_cost is incorrect [raw_time:%d buff:%d given_time:%u calc_time:%d] [seq=%u]",
                dwRawTimeCost, dwBuff, udwKey, dwRealTimeCost, pstSession->m_stUserInfo.m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }
    }
    else
    {
        dwRealTimeCost -= dwFreeTime;
        if (!CCommonBase::CheckInstantGem(udwKey, dwRealTimeCost))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ResearchUpgrade: instant gem is incorrect [raw_time:%d buff:%d given_gem_cost:%u, calc_time_cost:%d] [seq=%u]",
                dwRawTimeCost, dwBuff, udwKey, dwRealTimeCost, pstSession->m_stUserInfo.m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }
    }

    // 2. 获取research相关信息
    SSpGlobalRes stCost;
    if(CCommonBase::GetResearchCost(udwId, dwTargetlevel, stCost) != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ResearchUpgrade: req param error [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }

    for(TUINT32 udwIdx = 0; udwIdx < stCost.udwTotalNum; ++udwIdx)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__ResearchUpgrade: [%d, %d, %d] [seq=%u]",
            stCost[udwIdx].udwType,
            stCost[udwIdx].udwId,
            stCost[udwIdx].udwNum,
            pstSession->m_udwSeqNo
            ));
    }

    // 3. 判定资源和物品是否足够:0-normal;1-instant;2-buy
    if(ucUpgradeType != 2)
    {
        if(FALSE == CGlobalResLogic::HaveEnoughSpGlobalsRes(pstUser, pstCity, &stCost))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ResearchUpgrade: resource not enough [seq=%u]", pstSession->m_udwSeqNo));
            return -4;
        }
    }
    if(ucUpgradeType != 0)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwKey))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ResearchUpgrade: gem not enough [seq=%u]", pstSession->m_udwSeqNo));
            return -5;
        }
    }

    // 4. 消耗资源
    if(ucUpgradeType != 2)
    {
        CGlobalResLogic::CostSpGlobalRes(pstUser, pstCity, &stCost);
    }
    if(ucUpgradeType != 0)
    {
        pstSession->m_udwGemCost = udwKey;
        CPlayerBase::CostGem(pstUser, udwKey);
    }

    // 5. 执行upgrade
    UActionParam stParam;
    stParam.m_stBuilding.SetValue(0, udwId, dwTargetlevel, udwExp, (TCHAR*)tbPlayer.m_sUin.c_str());
    if (ucUpgradeType == 0 && udwKey > 1)
    {
        CActionBase::AddAlAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__BUILDING,
            EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE, EN_BUILDING_STATUS__RESEARCHING, udwKey, &stParam);
        if(bIsNeedHelp && pstUser->m_tbPlayer.m_nAlpos && pstUser->m_udwSelfAlActionNum)
        {
            TbAlliance_action &tbAlAction = pstUser->m_atbSelfAlAction[pstUser->m_udwSelfAlActionNum - 1];

            TINT32 dwCanHelpTimes = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALLIANCE_HELP_NUM].m_ddwBuffTotal;
            tbAlAction.Set_Sal(tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            tbAlAction.Set_Can_help_num(dwCanHelpTimes);
            tbAlAction.Set_Helped_num(0);
        }
    }
    else
    {
        // 生成Action
        CProcessAction::BuildingActionDone(pstSession, pstUser, pstCity, EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE, &stParam.m_stBuilding);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TroopTrain(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    // 请求参数
    // troop_type ==> ucId(key0)
    // troop_num_trained ==> udwNum(key1)
    // cost_time ==> udwCostTime(key2)
    // exp ==> udwExp(key3)
    TUINT8  ucId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwCostTime = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwExp = strtoul(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TUINT32 udwNeedResource = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TUINT32 udwGemNum = atoi(pstSession->m_stReqParam.m_szKey[5]);

    TINT32 udwTroopCategory = CToolBase::GetTroopCategoryByTroopType(ucId);
    if(udwTroopCategory < EN_TROOP_CATEGORY__INFANTRY || udwTroopCategory >= EN_TROOP_CATEGORY__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if(pstCity->m_stActionStat.m_aucDoingTroopNum[udwTroopCategory] > 0 && udwCostTime > 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TROOP_SLOT_LACK;
        return -2;
    }

    dwRetCode = Train(pstUser, pstCity, EN_ACTION_SEC_CLASS__TROOP, ucId, udwNum, udwCostTime, udwExp, udwNeedResource, udwGemNum, pstSession->m_bIsNeedCheck);

    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__TroopTrain: train ret[%d] [seq=%u]", \
            dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    //active quest 
    pstSession->m_ddwTroopChange += udwNum;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_FortTrain(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    // 0. 请求参数
    TUINT8  ucId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TUINT32 udwExp = strtoul(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TUINT32 udwNeedResource = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TUINT32 udwGemNum = atoi(pstSession->m_stReqParam.m_szKey[5]);

    if(pstCity->m_stActionStat.m_ucDoingFortNum > 0 && udwCostTime > 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__FORT_SLOT_LACK;
        return -1;
    }

    // 1. train
    dwRetCode = Train(pstUser, pstCity, EN_ACTION_SEC_CLASS__FORT, ucId, udwNum, udwCostTime, udwExp, udwNeedResource, udwGemNum, pstSession->m_bIsNeedCheck);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__FortTrain: train ret[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    return 0;
}

TINT32 CProcessPlayer::Train(SUserInfo *pstUser, SCityInfo *pstCity, TUINT8 ucType, TUINT8 ucId, TUINT32 udwNum, TUINT32 udwCostTime, TUINT32 udwExp, TUINT32 udwNeedResource, TUINT32 udwGemNum, TBOOL bIsNeedCheck)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    STroopInfo stTroopInfo;
    TINT64 addwResource[EN_RESOURCE_TYPE__END] = {0};
    TUINT32 idx = 0;
    double fResRate = 1.0;

    if(udwGemNum > 0)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemNum))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessGetResult: Gem not enough [gem num=%u][seq=%u]",
                udwGemNum, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__GEM_LACK;
        }
    }

    TINT32 dwBuff = 10000;
    TINT32 dwRawTimeCost = 0;
    // get fort info
    if(ucType == EN_ACTION_SEC_CLASS__FORT)
    {
        if(FALSE == poGameInfo->GetFortInfo(ucId, &stTroopInfo))
        {
            return EN_RET_CODE__TARGET_NOT_EXIST;
        }
        dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_BUILD_WALL_SPEED].m_ddwBuffTotal;
        dwRawTimeCost = stTroopInfo.m_audwBaseCost[EN_BASE_COST_TYPE__TIME] * udwNum;
    }
    else if(ucType == EN_ACTION_SEC_CLASS__TROOP)
    {
        if(FALSE == poGameInfo->GetTroopInfo(ucId, &stTroopInfo))
        {
            return EN_RET_CODE__TARGET_NOT_EXIST;
        }
        dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALL_TROOP_TRAIN_SPEED].m_ddwBuffTotal;
        dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALL_TROOP_ONLY_TRAIN_SPEED].m_ddwBuffTotal;

        if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__INFANTRY)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INFANTRY_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INFANTRY_TRAIN_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__REMOTE)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_REMOTE_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_REMOTE_TRAIN_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__SOWAR)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SOWAR_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SOWAR_TRAIN_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__SIEGE)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SIEGE_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SIEGE_TRAIN_SPEED_ONLY].m_ddwBuffTotal;
        }
        dwRawTimeCost = stTroopInfo.m_audwBaseCost[EN_BASE_COST_TYPE__TIME] * udwNum;
    }
    else if(ucType == EN_ACTION_SEC_CLASS__HOS_TREAT)
    {
        if(FALSE == poGameInfo->GetHealTroopInfo(ucId, &stTroopInfo))
        {
            return EN_RET_CODE__TARGET_NOT_EXIST;
        }
        fResRate = 1.0;
        dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALL_TROOP_TRAIN_SPEED].m_ddwBuffTotal;
        dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALL_TROOP_ONLY_HEAL_SPEED].m_ddwBuffTotal;

        if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__INFANTRY)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INFANTRY_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INFANTRY_HEAL_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__REMOTE)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_REMOTE_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_REMOTE_HEAL_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__SOWAR)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SOWAR_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SOWAR_HEAL_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__SIEGE)
        {
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SIEGE_TRAIN_SPEED].m_ddwBuffTotal;
            dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_SIEGE_HEAL_SPEED_ONLY].m_ddwBuffTotal;
        }
        dwRawTimeCost = stTroopInfo.m_audwBaseCost[EN_BASE_COST_TYPE__TIME] * udwNum;
    }
    else if(ucType == EN_ACTION_SEC_CLASS__FORT_REPAIR)
    {
        if(FALSE == poGameInfo->GetHealFortInfo(ucId, &stTroopInfo))
        {
            return EN_RET_CODE__TARGET_NOT_EXIST;
        }
        fResRate = 1.0;
        dwBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FORT_HEAL_SPEED].m_ddwBuffTotal;
        dwRawTimeCost = stTroopInfo.m_audwBaseCost[EN_BASE_COST_TYPE__TIME] * udwNum;
    }

    // 检测消耗时间和宝石的合法性
    TINT32 dwRealTimeCost = dwRawTimeCost / (1.0 * dwBuff / 10000);
    if (udwGemNum == 0)
    {
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTimeCost))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Train: time_cost is incorrect [raw_time:%d buff:%d given_time:%u calc_time:%d] [seq=%u]",
                dwRawTimeCost, dwBuff, udwCostTime, dwRealTimeCost, pstUser->m_udwBSeqNo));
            if (bIsNeedCheck)
            {
                return EN_RET_CODE__BLACK_ACCOUNT;
            }
        }
    }
    else
    {
        if (!CCommonBase::CheckInstantGem(udwGemNum, dwRealTimeCost))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_BuildingUpgrade: instant gem is incorrect [raw_time:%d buff:%d given_gem_cost:%u calc_time_cost:%d] [seq=%u]",
                dwRawTimeCost, dwBuff, udwGemNum, dwRealTimeCost, pstUser->m_udwBSeqNo));
            if (bIsNeedCheck)
            {
                return EN_RET_CODE__BLACK_ACCOUNT;
            }
        }

    }

    // 重新计算需要的资源数量
    TINT32 dwCostBuff = 0;
    if (ucType == EN_ACTION_SEC_CLASS__TROOP)
    {
        if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__INFANTRY)
        {
            dwCostBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_TROOP1_TRAIN_COST].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__REMOTE)
        {
            dwCostBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_TROOP2_TRAIN_COST].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__SOWAR)
        {
            dwCostBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_TROOP3_TRAIN_COST].m_ddwBuffTotal;
        }
        else if (stTroopInfo.m_dwCategory == EN_TROOP_CATEGORY__SIEGE)
        {
            dwCostBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_TROOP4_TRAIN_COST].m_ddwBuffTotal;
        }
    }
    else if (ucType == EN_ACTION_SEC_CLASS__FORT)
    {
        dwCostBuff += pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FORT_TRAIN_COST].m_ddwBuffTotal;
    }
    for (idx = 0; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        addwResource[idx] = fResRate * stTroopInfo.m_audwBaseCost[idx] * udwNum * (10000.0 / (10000 + dwCostBuff));
    }
    
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CProcessPlayer::Train uid=%u type=%u cost:[gold=%ld food=%ld wood=%ld stone=%ld ore=%ld time=%u] [seq=%u]",
        pstUser->m_tbPlayer.m_nUid,
        ucType,
        addwResource[EN_RESOURCE_TYPE__GOLD],
        addwResource[EN_RESOURCE_TYPE__FOOD],
        addwResource[EN_RESOURCE_TYPE__WOOD],
        addwResource[EN_RESOURCE_TYPE__STONE],
        addwResource[EN_RESOURCE_TYPE__ORE],
        udwCostTime,
        pstUser->m_udwBSeqNo));

    if(udwNeedResource)
    {
        // 检测资源是否足够
        if(FALSE == CCityBase::HasEnoughResource(pstCity, &addwResource[0]))
        {
            return EN_RET_CODE__RESOURCE_LACK;
        }

        // 4. 消耗资源
        CCityBase::CostResource(pstCity, &addwResource[0]);

        //active quest 
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
        {
            pstUser->m_udwCostResource += addwResource[udwIdx];
        }
    }

    //消耗宝石
    if(udwGemNum > 0)
    {
        CPlayerBase::CostGem(pstUser, udwGemNum);
    }

    // 消耗物品
    if(0 < stTroopInfo.m_udwRelyNum)
    {
        for(TUINT32 udwIdx = 0; udwIdx < stTroopInfo.m_udwRelyNum; ++udwIdx)
        {
            if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack,
                stTroopInfo.m_astItemCost[udwIdx].m_udwId,
                stTroopInfo.m_astItemCost[udwIdx].m_udwNum*udwNum))
            {
                return EN_RET_CODE__ITEM_NOT_ENOUGH;
            }
            CItemBase::CostItem(&pstUser->m_tbBackpack, stTroopInfo.m_astItemCost[udwIdx].m_udwId, stTroopInfo.m_astItemCost[udwIdx].m_udwNum * udwNum);
        }
    }

    if(0 == udwCostTime)
    {
        SCommonTroop *pstTroop = &pstCity->m_stTblData.m_bTroop[0];
        SCommonFort *pstFort = &pstCity->m_stTblData.m_bFort[0];
        TbPlayer *pstPlayer = &pstUser->m_tbPlayer;
        TUINT64 uddwGainedMight = 0;

        // 1. set data
        switch(ucType)
        {
        case EN_ACTION_SEC_CLASS__TROOP:
            // tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TRAIN_TROOP_OK, pstPlayer->m_nUid, FALSE, ucId, udwNum);
            CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_TROOP_TRAIN_NUM, udwNum, 0, CToolBase::GetTroopLvByTroopId(ucId));

            break;
        case EN_ACTION_SEC_CLASS__FORT:
            // tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TRAIN_FORT_OK, pstPlayer->m_nUid, FALSE, ucId, udwNum);
            CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_FORT_TRAIN_NUM, udwNum, 0, CToolBase::GetFortLvByFortId(ucId));

            break;
        case EN_ACTION_SEC_CLASS__HOS_TREAT:
            // tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HEAL_TROOP_OK, pstPlayer->m_nUid, FALSE, ucId, udwNum);
            break;
        case EN_ACTION_SEC_CLASS__FORT_REPAIR:
            // tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HEAL_FORT_OK, pstPlayer->m_nUid, FALSE, ucId, udwNum);
            break;
        default:
            return -1;
            assert(0);
            break;
        }

        // 2. compute
        switch(ucType)
        {
        case EN_ACTION_SEC_CLASS__TROOP:
            pstTroop->m_addwNum[ucId] += udwNum;
            pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_TROOP] = UPDATE_ACTION_TYPE_PUT;
            // add might
            uddwGainedMight = udwNum * CToolBase::GetTroopSingleMight(ucId);
            pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);
            pstPlayer->Set_Mgain(pstPlayer->m_nMgain + uddwGainedMight);

            //活动积分统计
            CActivitesLogic::ComputeTrainTroopScore(pstUser, ucId, udwNum);

            break;
        case EN_ACTION_SEC_CLASS__FORT:
            pstFort->m_addwNum[ucId] += udwNum;
            pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_FORT] = UPDATE_ACTION_TYPE_PUT;
            // add might
            uddwGainedMight = udwNum * CToolBase::GetFortSingleMight(ucId);
            pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);
            pstPlayer->Set_Mgain(pstPlayer->m_nMgain + uddwGainedMight);

            //活动积分统计
            CActivitesLogic::ComputeTrainFortScore(pstUser, ucId, udwNum);

            break;
        case EN_ACTION_SEC_CLASS__HOS_TREAT:
            pstTroop->m_addwNum[ucId] += udwNum;
            pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_TROOP] = UPDATE_ACTION_TYPE_PUT;
            // add might
            uddwGainedMight = udwNum * CToolBase::GetTroopSingleMight(ucId);
            pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);

            //活动积分统计
            CActivitesLogic::ComputeHealTroopScore(pstUser, ucId, udwNum);

            break;
        case EN_ACTION_SEC_CLASS__FORT_REPAIR:
            pstFort->m_addwNum[ucId] += udwNum;
            pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_FORT] = UPDATE_ACTION_TYPE_PUT;
            // add might
            uddwGainedMight = udwNum * CToolBase::GetFortSingleMight(ucId);
            pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);

            //活动积分统计
            CActivitesLogic::ComputeHealFortScore(pstUser, ucId, udwNum);
            break;
        }
        // 用户exp增加
        CPlayerBase::AddLordExp(pstUser,pstCity, udwExp);
    }
    else
    {
        //add action
        UActionParam stParam;
        stParam.m_stTrain.SetValue(ucId, udwNum, udwExp, (TCHAR*)pstUser->m_tbPlayer.m_sUin.c_str());

        CActionBase::AddAlAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__TRAIN_NEW, ucType, EN_TRAIN_STATUS__TRAINING, udwCostTime, &stParam);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TroopDismiss(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    SCommonTroop& stTroop = tbCity.m_bTroop[0];

    // 请求参数
    // troop_type ==> ucId(key0)
    // troop_num_dismissed ==> udwNum(key1)
    TUINT8 ucId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    // dismiss
    if(stTroop.m_addwNum[ucId] < udwNum)
    {
        stTroop.m_addwNum[ucId] = 0;
    }
    else
    {
        stTroop.m_addwNum[ucId] -= udwNum;
    }
    tbCity.SetFlag(TbCITY_FIELD_TROOP);

    pstSession->m_ddwTroopChange += -1L * udwNum;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_FortDismiss(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    SCommonFort& stFort = tbCity.m_bFort[0];

    TUINT8 ucId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    // dismiss
    if(stFort.m_addwNum[ucId] < udwNum)
    {
        stFort.m_addwNum[ucId] = 0;
    }
    else
    {
        stFort.m_addwNum[ucId] -= udwNum;
    }
    tbCity.SetFlag(TbCITY_FIELD_FORT);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DeadFortHeal(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity* ptbCity = &pstCity->m_stTblData;
    TINT32 dwRetCode = 0;

    // 输入参数
    TUINT8  ucFortType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwFortNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwCostTime = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwExp = strtoul(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TUINT32 udwNeedResource = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TUINT32 udwGemNum = atoi(pstSession->m_stReqParam.m_szKey[5]);

    // 1. check param
    if(ucFortType >= EN_FORT_TYPE__END || ptbCity->m_bDead_fort[0].m_addwNum[ucFortType] < udwFortNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    // 2. train
    dwRetCode = Train(pstUser, pstCity, EN_ACTION_SEC_CLASS__FORT_REPAIR, ucFortType, udwFortNum, udwCostTime, udwExp, udwNeedResource, udwGemNum, pstSession->m_bIsNeedCheck);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DeadFortHeal: train ret[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    // 3. del from wait list
    ptbCity->m_bDead_fort[0].m_addwNum[ucFortType] -= udwFortNum;
    ptbCity->SetFlag(TbCITY_FIELD_DEAD_FORT);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DeadFortAbandon(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    SCommonFort& stFort = tbCity.m_bDead_fort[0];

    TUINT8 ucId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    if(stFort[ucId] < udwNum)
    {
        stFort[ucId] = 0;
    }
    else
    {
        stFort[ucId] -= udwNum;
    }
    tbCity.SetFlag(TbCITY_FIELD_DEAD_FORT);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_HospitalTroopTreat(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity* ptbCity = &pstCity->m_stTblData;
    TINT32 dwRetCode = 0;

    // 输入参数
    // troop_type ==> ucTroopType(key0)
    // troop_num ==> udwTroopNum(key1)
    // cost_time ==> udwCostTime(key2)
    // exp ==> udwExp(key3)
    TUINT8  ucTroopType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwTroopNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwCostTime = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwExp = strtoul(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TUINT32 udwNeedResource = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TUINT32 udwGemNum = atoi(pstSession->m_stReqParam.m_szKey[5]);

    // 1. check param
    if(ucTroopType >= EN_TROOP_TYPE__END || ptbCity->m_bHos_wait[0].m_addwNum[ucTroopType] < udwTroopNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_HospitalTroopTreat: train troop_type[%u] treat_troop_num[%u] wait_num[%u] ret[%d] [seq=%u]",
            ucTroopType,
            udwTroopNum,
            ptbCity->m_bHos_wait[0].m_addwNum[ucTroopType],
            dwRetCode, 
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    // 2. train
    dwRetCode = Train(pstUser, pstCity, EN_ACTION_SEC_CLASS__HOS_TREAT, ucTroopType, udwTroopNum, udwCostTime, udwExp, udwNeedResource, udwGemNum, pstSession->m_bIsNeedCheck);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_HospitalTroopTreat: train ret[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    // 3. del from wait list
    ptbCity->m_bHos_wait[0].m_addwNum[ucTroopType] -= udwTroopNum;
    ptbCity->SetFlag(TbCITY_FIELD_HOS_WAIT);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_AbandWoundTroop(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    SCommonTroop& bTroop = tbCity.m_bHos_wait[0];

    // 请求参数
    // troop_type ==> ucId(key0)
    // troop_num_abanded ==> udwNum(key1)
    TUINT8 ucId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    if(bTroop[ucId] < udwNum)
    {
        bTroop[ucId] = 0;
    }
    else
    {
        bTroop[ucId] -= udwNum;
    }
    tbCity.SetFlag(TbCITY_FIELD_HOS_WAIT);

    pstSession->m_ddwTroopChange += -1L * udwNum;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ChangePlayerName(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;

    // 0. 请求参数
    TCHAR *pszPlayerName = pstSession->m_stReqParam.m_szKey[0];
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. 第一步操作
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TCHAR szTmpName[MAX_TABLE_NAME_LEN];
        strcpy(szTmpName, pszPlayerName);
        CUtf8Util::strtolower(szTmpName);
        if(!CToolBase::IsValidName(szTmpName, EN_PLAYER_NAME))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_NAME_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: reserve name [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }

        // 1.1 检验资源是否足够
        if(dwGemNum > 0)
        {
            if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
        }
        else if(dwItemId >= 0)
        {
            if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        // update data first
        pstSession->ResetAwsInfo();

        TbUnique_name tbUniqueName;
        tbUniqueName.Set_Type(EN_PLAYER_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(szTmpName));
        tbUniqueName.Set_Exist(1);
        tbUniqueName.Set_Id(pstUser->m_tbPlayer.m_nUid);
        ExpectedDesc expectedDesc;
        expectedDesc.clear();
        ExpectedItem expectedItem;
        expectedItem.SetVal(TbUNIQUE_NAME_FIELD_EXIST, FALSE);
        expectedDesc.push_back(expectedItem);

        CAwsRequest::UpdateItem(pstSession, &tbUniqueName, expectedDesc, RETURN_VALUES_ALL_NEW);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    // 2. 处理查询响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Reset();
        int dwCnt = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbUniqueName);
        if(dwCnt <= 0 || tbUniqueName.m_nId != pstUser->m_tbPlayer.m_nUid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_NAME_EXIST;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: name has been used [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        pstSession->ResetAwsInfo();
        tbUniqueName.Reset();
        tbUniqueName.Set_Type(EN_PLAYER_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(pstUser->m_tbPlayer.m_sUin));
        CAwsRequest::DeleteItem(pstSession, &tbUniqueName);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // 2.1 消耗资源
        if(dwGemNum > 0)
        {
            pstSession->m_udwGemCost = dwGemNum;
            CPlayerBase::CostGem(pstUser, dwGemNum);
        }
        else if(dwItemId > 0)
        {
            CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
        }

        // 2.2. 设置用户名
        char szName[MAX_TABLE_NAME_LEN];
        snprintf(szName, sizeof(szName), "%s", pszPlayerName);
        tbPlayer.Set_Uin(szName);

        pstUser->m_tbSelfName.Set_Name(CToolBase::ToLower(szName));  //防止在之后的流程中把旧名字更新回去

        // 2.3 如果是盟主，修改alliance属性
        if(tbPlayer.m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
        {
            pstUser->m_tbAlliance.Set_Oname(tbPlayer.m_sUin);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ChangeBaseName(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    // 0. 请求参数
    TCHAR *pszName = pstSession->m_stReqParam.m_szKey[0];
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. 检验资源是否足够
    if(!CToolBase::IsValidName(pszName, EN_CITY_NAME))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__OTHER_NAME_INVALID;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ChangeCityName: reserve name [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    if(dwGemNum > 0)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ChangeCityName: gem not enough [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }
    }
    else if(dwItemId >= 0)
    {
        if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ChangeCityName: item not enough [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ChangeCityName: req item and gem all empty [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }

    // 2. 消耗资源
    if(dwGemNum > 0)
    {
        pstSession->m_udwGemCost = dwGemNum;
        CPlayerBase::CostGem(pstUser, dwGemNum);
    }
    else if(dwItemId > 0)
    {
        CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId);

    }
    // 3. 设置城市名
    TbCity& tbCity = pstCity->m_stTblData;
    tbCity.Set_Name(pszName);
    if(tbCity.m_sName.size() >= MAX_TABLE_NAME_LEN)
    {
        tbCity.m_sName.resize(MAX_TABLE_NAME_LEN - 1);
    }

    // 4. 修改map中对应地点的名称
//     TbMap& tbTmpMap = pstSession->m_tbTmpMap;
//     if (CMapLogic::IfPlayerCity(pstUser, &tbTmpMap))
//     {
//         tbTmpMap.Reset();
//         tbTmpMap.Set_Id(tbCity.m_nPos);
//         tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
//         tbTmpMap.Set_Name(tbCity.m_sName);
//         pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
//     }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_PlayerInfoGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwUid = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (udwUid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PlayerInfoGet:uid is zero [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsReq();
        CAwsRequest::UserGetByUid(pstSession, udwUid);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PlayerInfoGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    // 2. 解析响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        // parse data
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpPlayer);
        if (dwRetCode <= 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PlayerInfoGet: not such uid user uid[%u] get failed [seq=%u]", udwUid, pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        pstSession->m_tbTmpPlayer.m_nMkill = pstSession->m_tbTmpPlayer.m_nKfort + pstSession->m_tbTmpPlayer.m_nKtroop;

        //get rank info
        pstSession->ResetRankSvrInfo();
        RankSvrReqInfo *ptbRankReq = new RankSvrReqInfo;
        pstSession->m_vecRankSvrReq.push_back(ptbRankReq);
        ptbRankReq->SetVal(pstSession->m_tbTmpPlayer.m_nSid, udwUid, "get_player_self_rank");

        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__RANK_SVR;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendRankSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PlayerInfoGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            //return -1;
            bNeedResponse = FALSE;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_LIST;
            return 0;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecRankSvrRsp.size(); udwIdx++)
        {
            if (pstSession->m_vecRankSvrRsp[udwIdx]->m_udwUid == udwUid)
            {
                Json::Reader reader;
                reader.parse(pstSession->m_vecRankSvrRsp[udwIdx]->m_sRspJson, pstSession->m_jTmpPlayerRankInfo);
                break;
            }
        }
    }

    /*
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if (pstSession->m_tbTmpPlayer.m_nUid)
        {
            // send request
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

            CAwsRequest::EquipQueryByStatus(pstSession, pstSession->m_tbTmpPlayer.m_nUid, EN_EQUIPMENT_STATUS_ON_DRAGON);
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PlayerInfoGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
            return 0;
        }
    }
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        for (TUINT32 dwIdx = 0; dwIdx < pstSession->m_vecAwsRsp.size(); dwIdx++)
        {
            AwsRspInfo *pRspInfo = pstSession->m_vecAwsRsp[dwIdx];
            dwRetCode = CAwsResponse::OnQueryRsp(*pRspInfo, &pstSession->m_atbTmpEquip[pstSession->m_udwTmpEquipNum],
                sizeof(TbEquip), MAX_USER_EQUIP_NUM - pstSession->m_udwTmpEquipNum);
            if (dwRetCode > 0)
            {
                pstSession->m_udwTmpEquipNum += dwRetCode;
            }
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_LIST;
        return 0;
    }
    */
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_LIST;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_SetGuideFlag(SSession *pstSession, TBOOL &bNeedResponse)
{
    TCHAR *pszFlags = &pstSession->m_stReqParam.m_szKey[0][0];
    TUINT32 udwSize = 20;
    TUINT32 audwFlagIds[20];
    CCommonFunc::GetArrayFromString(pszFlags, ':', &audwFlagIds[0], udwSize);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbLogin* ptbLogin = &pstUser->m_tbLogin;
    SBitFlag& stGuideFlag = ptbLogin->m_bGuide_flag[0];

    for(TUINT32 udwIdx = 0; udwIdx < udwSize; ++udwIdx)
    {
        if(audwFlagIds[udwIdx] >= TBLOGIN_GUIDE_FLAG_MAX_NUM)
        {
            continue;//do not return error
        }
        BITSET(stGuideFlag.m_bitFlag, audwFlagIds[udwIdx] - 1);
        ptbLogin->SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);
    }
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_QuestStart(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwQuestType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwQuestIdx = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SQuestNode *pstQusetNode = NULL;

    if (ddwQuestType == EN_TIME_QUEST_TYPE_DAILY)
    {
        pstQusetNode = &pstUserInfo->m_tbQuest.m_bDaily_quest[0];
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE)
    {
        pstQusetNode = &pstUserInfo->m_tbQuest.m_bAl_quest[0];
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_VIP)
    {
        pstQusetNode = &pstUserInfo->m_tbQuest.m_bVip_quest[0];
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:err quest type=%u idx=%u [seq=%u]",
            ddwQuestType, ddwQuestIdx, pstUserInfo->m_udwBSeqNo));
        return -2;
    }

    if (pstQusetNode->m_ddwQuestNum <= ddwQuestIdx)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:quest idx err type=%u idx=%u [seq=%u]",
            ddwQuestType, ddwQuestIdx, pstUserInfo->m_udwBSeqNo));
        return -1;
    }

    SQuestComm *pstQuestComm = &pstQusetNode->m_stQuestCom[ddwQuestIdx];
    for (TUINT32 udwIdx = 0; udwIdx < pstQusetNode->m_ddwQuestNum; ++udwIdx)
    {
        if (pstQuestComm->m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:quest is runing type=%u idx=%u [seq=%u]",
                ddwQuestType, udwIdx, pstUserInfo->m_udwBSeqNo));
            return 0;
        }
    }

    if (pstQuestComm->m_ddwStatus != EN_TIME_QUEST_STATUS_WAIT_START)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:quest is not waiting type=%u idx=%u [seq=%u]",
            ddwQuestType, ddwQuestIdx, pstUserInfo->m_udwBSeqNo));
        return 0;
    }

    pstQuestComm->m_ddwBTime = CTimeUtils::GetUnixTime();
    pstQuestComm->m_ddwStatus = EN_TIME_QUEST_STATUS_RUNNING;

    if (ddwQuestType == EN_TIME_QUEST_TYPE_DAILY)
    {
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_DAILY_QUEST);
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE)
    {
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_AL_QUEST);
    }

    else if (ddwQuestType == EN_TIME_QUEST_TYPE_VIP)
    {
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_VIP_QUEST);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_QuestRewardCollect(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32  dwRetCode = 0;
    TINT64 ddwQuestType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwQuestIdx = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SQuestNode *pstQusetNode = NULL;

    if (ddwQuestType == EN_TIME_QUEST_TYPE_DAILY)
    {
        pstQusetNode = &pstUserInfo->m_tbQuest.m_bDaily_quest[0];
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE)
    {
        pstQusetNode = &pstUserInfo->m_tbQuest.m_bAl_quest[0];
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_VIP)
    {
        pstQusetNode = &pstUserInfo->m_tbQuest.m_bVip_quest[0];
    }

    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessQuestRewardCollect:err quest type=%u idx=%u [seq=%u]",
            ddwQuestType, ddwQuestIdx, pstUserInfo->m_udwBSeqNo));
        return -1;
    }

    if (pstQusetNode->m_ddwQuestNum <= ddwQuestIdx)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:quest idx err  type=%u quest_num=%u idx=%u [seq=%u]",
            ddwQuestType, pstQusetNode->m_ddwQuestNum, ddwQuestIdx, pstUserInfo->m_udwBSeqNo));
        return -3;
    }

    SQuestComm *pstQuestComm = &pstQusetNode->m_stQuestCom[ddwQuestIdx];
    if (pstQuestComm->m_ddwStatus != EN_TIME_QUEST_STATUS_FINISH && pstQuestComm->m_ddwStatus != EN_TIME_QUEST_STATUS_AUTO_FINISH)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:quest can not collect type=%u idx=%u status=%u [seq=%u]",
            ddwQuestType, ddwQuestIdx, pstQuestComm->m_ddwStatus, pstUserInfo->m_udwBSeqNo));
        return -2;
    }

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    SGlobalRes *pstGlobalRes = &pstQuestComm->m_stReward;
    dwRetCode = CGlobalResLogic::AddGlobalRes(pstUserInfo, pstCity, pstGlobalRes);
    if (dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:Add reward fail  type=%u quest_num=%u idx=%u [seq=%u]",
            ddwQuestType, pstQusetNode->m_ddwQuestNum, ddwQuestIdx, pstUserInfo->m_udwBSeqNo));
        return -4;
    }

    //ll add for cb log
    TUINT32 udwLen = 0;
    TBOOL bHead = TRUE;
    sprintf(pstSession->m_stReqParam.m_szKey[2], "%ld", pstQuestComm->m_ddwLv);
    for (TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
    {
        if (bHead)
        {
            udwLen = snprintf(pstSession->m_stReqParam.m_szKey[3], 1024, "%ld#%ld#%ld", 
                pstGlobalRes->aRewardList[udwIdx].ddwType, pstGlobalRes->aRewardList[udwIdx].ddwId, pstGlobalRes->aRewardList[udwIdx].ddwNum);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(pstSession->m_stReqParam.m_szKey[3] + udwLen, 1024 - udwLen, ":%ld#%ld#%ld", 
                pstGlobalRes->aRewardList[udwIdx].ddwType, pstGlobalRes->aRewardList[udwIdx].ddwId, pstGlobalRes->aRewardList[udwIdx].ddwNum);
        }
    }

    pstQuestComm->m_ddwStatus = EN_TIME_QUEST_STATUS_DONE;
    pstQusetNode->m_ddwCollectNum++;

    if (ddwQuestType == EN_TIME_QUEST_TYPE_DAILY)
    {
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_DAILY_QUEST);
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE)
    {
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_AL_QUEST);
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_VIP)
    {
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_VIP_QUEST);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_RefreshTimeQuest(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwQuestType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwGemCost = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT64 ddwItemId = strtoll(pstSession->m_stReqParam.m_szKey[2], NULL, 10);;

    if (ddwQuestType != EN_TIME_QUEST_TYPE_ALLIANCE && ddwQuestType != EN_TIME_QUEST_TYPE_DAILY && ddwQuestType != EN_TIME_QUEST_TYPE_VIP)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessRefreshTimeQuest:quest type err type=%u [seq=%u]",
            ddwQuestType, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (ddwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE && pstSession->m_stUserInfo.m_tbPlayer.m_nAlid == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessRefreshTimeQuest:not in alliance [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    if (ddwGemCost > 0)
    {
        if (!CPlayerBase::HasEnoughGem(&pstSession->m_stUserInfo.m_tbLogin, ddwGemCost))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessRefreshTimeQuest:gem not enough gemcost=%u gem_owm=%u [seq=%u]",
                ddwGemCost, pstSession->m_stUserInfo.m_tbLogin.Get_Gem(), pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        CPlayerBase::CostGem(&pstSession->m_stUserInfo, ddwGemCost);
        pstSession->m_udwGemCost = ddwGemCost;
    }
    else
    {
        if (!CItemBase::HasEnoughItem(&pstSession->m_stUserInfo.m_tbBackpack, ddwItemId, 1))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessRefreshTimeQuest:item not enough item_id=%u [seq=%u]",
                ddwItemId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, ddwItemId, 1);
    }

    
    SCityInfo *pstCityInfo = &pstSession->m_stUserInfo.m_stCityInfo;
    if (ddwQuestType == EN_TIME_QUEST_TYPE_DAILY)
    {
        CQuestLogic::NewRefreshTimequest(&pstSession->m_stUserInfo, pstCityInfo, ddwQuestType, &pstSession->m_stUserInfo.m_tbQuest.m_bDaily_quest[0], EN_QUEST_REFRESH_TYPE_FORCE);
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE)
    {
        CQuestLogic::NewRefreshTimequest(&pstSession->m_stUserInfo, pstCityInfo, ddwQuestType, &pstSession->m_stUserInfo.m_tbQuest.m_bAl_quest[0], EN_QUEST_REFRESH_TYPE_FORCE);
    }

    else if (ddwQuestType == EN_TIME_QUEST_TYPE_VIP)
    {
        CQuestLogic::NewRefreshTimequest(&pstSession->m_stUserInfo, pstCityInfo, ddwQuestType, &pstSession->m_stUserInfo.m_tbQuest.m_bVip_quest[0], EN_QUEST_REFRESH_TYPE_FORCE);
    }

    return 0;

    /*
    SCityInfo *pstCityInfo = &pstSession->m_stUserInfo.m_stCityInfo;
    if (ddwQuestType == EN_TIME_QUEST_TYPE_DAILY)
    {
        TUINT32 udwBreakInitDailyLv = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][30U].asUInt();
        TUINT32 udwCastleLv = CCityBase::GetBuildingLevelByFuncType(pstCityInfo, EN_BUILDING_TYPE__CASTLE);
        if (udwCastleLv < udwBreakInitDailyLv)
        {
            CQuestLogic::RefreshTimequest(&pstSession->m_stUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_NEW_USER_DAILY, &pstSession->m_stUserInfo.m_tbQuest.m_bDaily_quest[0], TRUE);
        }
        else
        {
            CQuestLogic::RefreshTimequest(&pstSession->m_stUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_DAILY, &pstSession->m_stUserInfo.m_tbQuest.m_bDaily_quest[0]);
        }

        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_DAILY_QUEST);
    }
    else if (ddwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE)
    {
        CQuestLogic::RefreshTimequest(&pstSession->m_stUserInfo, pstCityInfo, ddwQuestType, &pstSession->m_stUserInfo.m_tbQuest.m_bAl_quest[0]);
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_AL_QUEST);
    }

    else if (ddwQuestType == EN_TIME_QUEST_TYPE_VIP)
    {
        CQuestLogic::RefreshTimequest(&pstSession->m_stUserInfo, pstCityInfo, ddwQuestType, &pstSession->m_stUserInfo.m_tbQuest.m_bVip_quest[0]);
        pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_VIP_QUEST);
    }

    return 0;
    */
}

TINT32 CProcessPlayer::ProcessCmd_GetTimerGift(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32  dwRetCode = 0;
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SQuestNode *pstQusetNode = &pstUserInfo->m_tbQuest.m_bTimer_gift[0];

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (pstQusetNode->m_stQuestCom[0].m_ddwStatus != EN_TIME_QUEST_STATUS_FINISH
            && pstQusetNode->m_stQuestCom[0].m_ddwBTime + pstQusetNode->m_stQuestCom[0].m_ddwCTime > CTimeUtils::GetUnixTime())
        {
            //客户端数据为拉取成功导致的重复领取不报错....
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessMisteryGiftCollect:mistery quest not finish status=%u time=%ld [seq=%u]",
                pstQusetNode->m_stQuestCom[0].m_ddwStatus, \
                pstQusetNode->m_stQuestCom[0].m_ddwBTime + pstQusetNode->m_stQuestCom[0].m_ddwCTime,
                pstUserInfo->m_udwBSeqNo));
            return 0;
        }

        SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
        SGlobalRes *pstGlobalRes = &pstQusetNode->m_stQuestCom[0].m_stReward;
        dwRetCode = CGlobalResLogic::AddGlobalRes(pstUserInfo, pstCity, pstGlobalRes);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd__QuestClaim:Add reward fail [seq=%u]", \
                                                              pstUserInfo->m_udwBSeqNo));
            return -2;
        }

        Json::Value tipContent = Json::Value(Json::arrayValue);
        for (TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
        {
            tipContent[udwIdx] = Json::Value(Json::arrayValue);
            tipContent[udwIdx].append(pstGlobalRes->aRewardList[udwIdx].ddwType);
            tipContent[udwIdx].append(pstGlobalRes->aRewardList[udwIdx].ddwId);
            tipContent[udwIdx].append(pstGlobalRes->aRewardList[udwIdx].ddwNum);
        }
        CSendMessageBase::AddTips(pstUserInfo, pstUserInfo->m_tbPlayer.m_nUid, FALSE, EN_TIPS_TYPE__MYSTEY_GIFT, tipContent.toStyledString());


        pstQusetNode->m_ddwQuestNum = 1;
        pstQusetNode->m_ddwCollectNum++;
 
        
        if (pstUserInfo->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_USER_MISTERY)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GetTimerGift: new player mistery [seq=%u]",
                                                    pstSession->m_udwSeqNo));
            pstUserInfo->m_tbPlayer.Set_Status(pstUserInfo->m_tbPlayer.m_nStatus - EN_CITY_STATUS__NEW_USER_MISTERY);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
        pstSession->ResetDataCenterReq();


        DataCenterReqInfo* pstReq = new DataCenterReqInfo;
        pstReq->m_udwType = EN_REFRESH_DATA_TYPE__MISTERY_GIFT;
        Json::Value rDataReqJson = Json::Value(Json::objectValue);

        rDataReqJson["sid"] = pstUserInfo->m_tbLogin.m_nSid;
        rDataReqJson["uid"] = pstUserInfo->m_tbPlayer.m_nUid;
        rDataReqJson["request"] = Json::Value(Json::objectValue);
        rDataReqJson["request"]["user_status"] = pstUserInfo->m_tbPlayer.m_nStatus;
        rDataReqJson["request"]["collect_num"] = pstQusetNode->m_ddwCollectNum;
        rDataReqJson["request"]["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUserInfo->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
        rDataReqJson["request"]["request_type"] = 1L;

        Json::FastWriter rEventWriter;
        pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
        pstSession->m_vecDataCenterReq.push_back(pstReq);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GetTimerGift: data center req: [json=%s] [type=%u] [uid=%ld][seq=%u]",
                                                pstReq->m_sReqContent.c_str(), \
                                                pstReq->m_udwType, \
                                                pstUserInfo->m_tbPlayer.m_nUid, \
                                                pstSession->m_udwSeqNo));
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
        if (dwRetCode == 0)
        {
            return 0;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetTimerGift: send request to data center failed. [json=%s] [ret=%d] [seq=%u]",
                                                    pstReq->m_sReqContent.c_str(), dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        
    }


    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;
        if (vecRsp.size() > 0)
        {

            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__MISTERY_GIFT)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GetTimerGift: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                                                            pstDataCenterRsp->m_sRspJson.c_str(), pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetTimerGift: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -6;
                    }
                    TINT32 dwRetCode = stRefreshData.m_stMisteryGiftRsp.setVal(oRspDataJson);
                    if (0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetTimerGift: response data format error. [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -7;
                    }
                    break;
                }
            }

            if(pstUserInfo->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_USER_MISTERY)
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetTimerGift: new player mistery error [seq=%u]",
                                                        pstSession->m_udwSeqNo));
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
            }
            else
            {
                pstQusetNode->m_ddwRTime = CTimeUtils::GetUnixTime();
                pstQusetNode->m_stQuestCom[0].m_ddwBTime = CTimeUtils::GetUnixTime();
                pstQusetNode->m_stQuestCom[0].m_ddwStatus = EN_TIME_QUEST_STATUS_RUNNING;
                pstQusetNode->m_stQuestCom[0].m_ddwLv = 1;
                pstQusetNode->m_stQuestCom[0].m_ddwCTime = stRefreshData.m_stMisteryGiftRsp.m_dwCostTime;
                pstQusetNode->m_stQuestCom[0].m_stReward.ddwTotalNum = 0;

                for (TUINT32 udwIdx = 0; udwIdx < 1 && udwIdx < stRefreshData.m_stMisteryGiftRsp.m_vecReward.size(); ++udwIdx)
                {
                    pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwType = stRefreshData.m_stMisteryGiftRsp.m_vecReward[udwIdx]->ddwType;
                    pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwId = stRefreshData.m_stMisteryGiftRsp.m_vecReward[udwIdx]->ddwId;
                    pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwNum = stRefreshData.m_stMisteryGiftRsp.m_vecReward[udwIdx]->ddwNum;
                    pstQusetNode->m_stQuestCom[0].m_stReward.ddwTotalNum++;
                    if(MAX_REWARD_ITEM_NUM <= pstQusetNode->m_stQuestCom[0].m_stReward.ddwTotalNum)
                    {
                        break;
                    }
    
                }
                
            }
            pstSession->m_stUserInfo.m_tbQuest.SetFlag(TbQUEST_FIELD_TIMER_GIFT);
            

            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_GetTimerGift:before uid=%u collect_num=%u status=%u r_time=%u cur_time=%u seq=%u",
                                                              pstUserInfo->m_tbPlayer.m_nUid,
                                                              pstQusetNode->m_ddwCollectNum,
                                                              pstUserInfo->m_tbPlayer.m_nStatus,
                                                              pstQusetNode->m_ddwRTime,
                                                              CTimeUtils::GetUnixTime(), \
                                                              pstSession->m_stUserInfo.m_udwBSeqNo));

           
            //ll add for cb log
            TUINT32 udwLen = 0;
            TBOOL bHead = TRUE;
            for (TUINT32 udwIdx = 0; udwIdx < pstQusetNode->m_stQuestCom[0].m_stReward.ddwTotalNum; ++udwIdx)
            {
                if (bHead)
                {
                    udwLen = snprintf(pstSession->m_stReqParam.m_szKey[0], 1024, "%ld#%ld#%ld", pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwType, pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwId, pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwNum);
                    bHead = FALSE;
                }
                else
                {
                    udwLen += snprintf(pstSession->m_stReqParam.m_szKey[0] + udwLen, 1024 - udwLen, ":%ld#%ld#%ld", pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwType, pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwId, pstQusetNode->m_stQuestCom[0].m_stReward.aRewardList[udwIdx].ddwNum);
                }
            }
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
            
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_LordSkillUpgrade(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbUser_stat* ptbUserStat = &pstUser->m_tbUserStat;

    // 0. 请求参数
    TINT32 dwSkillId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwSkillPointAdd = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwSkillPointOrigin = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check
    if (dwSkillId >= EN_SKILL_TYPE__END || dwSkillId < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    // 校验，防止网络超时引起的多次误加点
    if (0 != dwSkillPointOrigin && dwSkillPointOrigin != ptbUserStat->m_bLord_skill[0].m_addwLevel[dwSkillId])
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_LordSkillUpgrade: origin skill point error[ori=%d,parm=%d] [seq=%u]",
            ptbUserStat->m_bDragon_skill[0].m_addwLevel[dwSkillId], dwSkillPointOrigin, pstSession->m_udwSeqNo));
        return 0;
    }

    TINT32 dwSkillPointLeft = CPlayerBase::GetLeftLordSkillPoint(pstUser);
    if (dwSkillPointLeft < dwSkillPointAdd || dwSkillPointLeft <= 0)
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_LordSkillUpgrade: no enough skill point[left=%d,parm=%d] [seq=%u]",
            dwSkillPointLeft, dwSkillPointAdd, pstSession->m_udwSeqNo));
        return 0;
    }

    if (ptbUserStat->m_bLord_skill[0].m_addwLevel[dwSkillId] + dwSkillPointAdd > CPlayerBase::GetLordSkillPointLimit(dwSkillId))
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill point over max [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    if (!CPlayerBase::IsMeetLordSkillReliance(pstUser, dwSkillId, ptbUserStat->m_bLord_skill[0].m_addwLevel[dwSkillId] + dwSkillPointAdd))
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill rely not meet [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    // 2. process
    ptbUserStat->m_bLord_skill[0].m_addwLevel[dwSkillId] += dwSkillPointAdd;
    ptbUserStat->SetFlag(TbUSER_STAT_FIELD_LORD_SKILL);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_LordSkillReset(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbBackpack* ptbBackpack = &pstUser->m_tbBackpack;

    // 0. 请求参数
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    if (udwType == 0)
    {
        if (CItemBase::HasEnoughItem(ptbBackpack, udwItemId, 1) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            return -1;
        }
    }
    else if (udwType == 1)
    {
        if (CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemNum) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    // 2. process
    pstUser->m_tbUserStat.m_bLord_skill[0].Reset();
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_LORD_SKILL);

    if (udwType == 0)
    {
        CItemBase::CostItem(ptbBackpack, udwItemId, 1);
    }
    else if (udwType == 1)
    {
        pstSession->m_udwGemCost = udwGemNum;
        CPlayerBase::CostGem(pstUser, udwGemNum);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonSkillUpgrade(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    TbUser_stat* ptbUserStat = &pstUser->m_tbUserStat;

    // 0. 请求参数
    TINT32 dwSkillId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwSkillPointAdd = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwSkillPointOrigin = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check
    if (ptbPlayer->m_nHas_dragon == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
//     if (ptbPlayer->m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         return -2;
//     }
    if(dwSkillId >= EN_SKILL_TYPE__END || dwSkillId < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    // 校验，防止网络超时引起的多次误加点
    if(0 != dwSkillPointOrigin && dwSkillPointOrigin != ptbUserStat->m_bDragon_skill[0].m_addwLevel[dwSkillId])
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: origin skill point error[ori=%d,parm=%d] [seq=%u]",
            ptbUserStat->m_bDragon_skill[0].m_addwLevel[dwSkillId], dwSkillPointOrigin, pstSession->m_udwSeqNo));
        return 0;
    }

    TINT32 dwSkillPointLeft = CPlayerBase::GetLeftDragonSkillPoint(pstUser);
    if(dwSkillPointLeft < dwSkillPointAdd || dwSkillPointLeft <= 0)
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: no enough skill point[left=%d,parm=%d] [seq=%u]",
            dwSkillPointLeft, dwSkillPointAdd, pstSession->m_udwSeqNo));
        return 0;
    }

    if (ptbUserStat->m_bDragon_skill[0].m_addwLevel[dwSkillId] + dwSkillPointAdd > CPlayerBase::GetDragonSkillPointLimit(dwSkillId))
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill point over max [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    if (!CPlayerBase::IsMeetDragonSkillReliance(pstUser, dwSkillId, ptbUserStat->m_bDragon_skill[0].m_addwLevel[dwSkillId] + dwSkillPointAdd))
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill rely not meet [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    // 2. process
    ptbUserStat->m_bDragon_skill[0].m_addwLevel[dwSkillId] += dwSkillPointAdd;
    ptbUserStat->SetFlag(TbUSER_STAT_FIELD_DRAGON_SKILL);

    BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH_STAGE_5_ASSIGN_HERO_SKILL);
    pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonSkillReset(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbBackpack* ptbBackpack = &pstUser->m_tbBackpack;

    // 0. 请求参数
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check
    if (ptbPlayer->m_nHas_dragon == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
//     if (ptbPlayer->m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         return -2;
//     }
    if(udwType == 0)
    {
        if(CItemBase::HasEnoughItem(ptbBackpack, udwItemId, 1) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            return -1;
        }
    }
    else if(udwType == 1)
    {
        if(CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemNum) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    // 2. process
    pstUser->m_tbUserStat.m_bDragon_skill[0].Reset();
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_DRAGON_SKILL);

    if(udwType == 0)
    {
        CItemBase::CostItem(ptbBackpack, udwItemId, 1);
    }
    else if(udwType == 1)
    {
        pstSession->m_udwGemCost = udwGemNum;
        CPlayerBase::CostGem(pstUser, udwGemNum);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonMonsterSkillUpgrade(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    TbUser_stat* ptbUserStat = &pstUser->m_tbUserStat;

    // 0. 请求参数
    TINT32 dwSkillId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwSkillPointAdd = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwSkillPointOrigin = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check
    if (ptbPlayer->m_nHas_dragon == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
//     if (ptbPlayer->m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         return -2;
//     }
    if (dwSkillId >= EN_SKILL_TYPE__END || dwSkillId < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    // 校验，防止网络超时引起的多次误加点
    if(0 != dwSkillPointOrigin && dwSkillPointOrigin != ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[dwSkillId])
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MonsterSkillUpgrade: origin skill point error[ori=%d,parm=%d] [seq=%u]",
            ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[dwSkillId], dwSkillPointOrigin, pstSession->m_udwSeqNo));
        return 0;
    }

    TINT32 dwSkillPointLeft = CPlayerBase::GetLeftDragonMonsterSkillPoint(pstUser);
    if(dwSkillPointLeft < dwSkillPointAdd || dwSkillPointLeft <= 0)
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MonsterSkillUpgrade: no enough skill point[left=%d,parm=%d] [seq=%u]",
            dwSkillPointLeft, dwSkillPointAdd, pstSession->m_udwSeqNo));
        return 0;
    }

    if(ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[dwSkillId] + dwSkillPointAdd > CPlayerBase::GetDragonMonsterSkillPointLimit(dwSkillId))
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MonsterSkillUpgrade: skill point over max [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    if (!CPlayerBase::IsMeetDragonMonsterSkillReliance(pstUser, dwSkillId, ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[dwSkillId] + dwSkillPointAdd))
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MonsterSkillUpgrade: skill rely not meet [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    // 2. process
    ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[dwSkillId] += dwSkillPointAdd;
    ptbUserStat->SetFlag(TbUSER_STAT_FIELD_DRAGON_MONSTER_SKILL);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonMonsterSkillReset(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbBackpack* ptbBackpack = &pstUser->m_tbBackpack;

    // 0. 请求参数
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check
    if (ptbPlayer->m_nHas_dragon == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
//     if (ptbPlayer->m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         return -2;
//     }
    if(udwType == 0)
    {
        if(CItemBase::HasEnoughItem(ptbBackpack, udwItemId, 1) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            return -1;
        }
    }
    else if(udwType == 1)
    {
        if(CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemNum) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    // 2. process
    pstUser->m_tbUserStat.m_bDragon_monster_skill[0].Reset();
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_DRAGON_MONSTER_SKILL);

    if(udwType == 0)
    {
        CItemBase::CostItem(ptbBackpack, udwItemId, 1);
    }
    else if(udwType == 1)
    {
        pstSession->m_udwGemCost = udwGemNum;
        CPlayerBase::CostGem(pstUser, udwGemNum);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_PlayerAvatarChange(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    // 0. 请求参数
    TINT32 dwAvatar = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[3]);
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    // 1.1 检验资源是否足够
    if(dwType == 1)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_AvatarChange: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else if(dwType == 0)
    {
        if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_AvatarChange: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_AvatarChange: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    // 2.1 消耗资源
    if(dwType == 1)
    {
        pstSession->m_udwGemCost = dwGemNum;
        CPlayerBase::CostGem(pstUser, dwGemNum);
    }
    else if(dwType == 0)
    {
        CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
    }

    pstUser->m_tbPlayer.Set_Avatar(dwAvatar);

    //task count
    CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_CHANGE_PLAYER_ICON);
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_QuestClaim(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    STopQuest& stQuest = pstUser->m_tbUserStat.m_bTop_quest[0];

    TUINT32 udwQuestId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if(udwQuestId >= EN_TOP_QUEST_NUM_LIMIT)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd__QuestClaim: no such quest [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }

    if(BITTEST(stQuest.m_bitQuest, udwQuestId))
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd__QuestClaim: already claimed [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    TCHAR szKey[32];
    snprintf(szKey, 32, "%u", udwQuestId);
    if(!pstGameInfo->m_oJsonRoot["game_quest"].isMember(szKey))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd__QuestClaim: not quest [%s] [seq=%u]", szKey, pstSession->m_udwSeqNo));
        return -2;
    }
    const Json::Value &pQuest = pstGameInfo->m_oJsonRoot["game_quest"][szKey];
    if(pQuest.isNull())
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd__QuestClaim: wrong game json [%s] [seq=%u]", szKey, pstSession->m_udwSeqNo));
        return -3;
    }
    const Json::Value &oReward = pstGameInfo->m_oJsonRoot["game_quest"][szKey]["r"]["a1"];
    TINT32 dwRewardType = pstGameInfo->m_oJsonRoot["game_quest"][szKey]["r"]["a0"].asInt();
    SSpGlobalRes stResInfo;
    stResInfo.Reset();
    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(oReward, dwRewardType, &stResInfo);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessQuestClaim: get reward info fail [uid=%ld quest_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwQuestId, dwRetCode));
        return dwRetCode;
    }

    // 3、获取奖励
    TINT32 dwLevelBefore = pstUser->m_tbPlayer.m_nLevel;
    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stResInfo);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessQuestClaim: take reward fail [uid=%ld quest_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwQuestId, dwRetCode));
        return dwRetCode;
    }
    TINT32 dwLevelAfter = pstUser->m_tbPlayer.m_nLevel;
    snprintf(pstSession->m_stReqParam.m_szKey[1], sizeof(pstSession->m_stReqParam.m_szKey[1]), "%d", dwLevelBefore < dwLevelAfter ? 1 : 0);

    // 5、设置完成标记
    BITSET(stQuest.m_bitQuest, udwQuestId);
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_TOP_QUEST);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_LordLevelUp(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    SLevelQuest& stLevel = pstUser->m_tbUserStat.m_bLord_level_finish[0];

    TUINT32 udwQuestId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if(udwQuestId >= MAX_LEVEL_LIMIT)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_LordLevelUp: no such quest [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }

    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    TCHAR szKey[32];
    snprintf(szKey, 32, "%u", udwQuestId);
    if(!pstGameInfo->m_oJsonRoot["game_lord_level_up"].isMember(szKey))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_LordLevelUp: not quest [%s] [seq=%u]", szKey, pstSession->m_udwSeqNo));
        return -2;
    }
    const Json::Value &pQuest = pstGameInfo->m_oJsonRoot["game_lord_level_up"][szKey];
    if(pQuest.isNull())
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_LordLevelUp: wrong game json [%s] [seq=%u]", szKey, pstSession->m_udwSeqNo));
        return -3;
    }
    const Json::Value &oReward = pstGameInfo->m_oJsonRoot["game_lord_level_up"][szKey]["r"]["a1"];
    TINT32 dwRewardType = pstGameInfo->m_oJsonRoot["game_lord_level_up"][szKey]["r"]["a0"].asInt();
    SSpGlobalRes stResInfo;
    stResInfo.Reset();
    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(oReward, dwRewardType, &stResInfo);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_LordLevelUp: get reward info fail [uid=%ld quest_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwQuestId, dwRetCode));
        return dwRetCode;
    }

    if (BITTEST(stLevel.m_bitLevel, udwQuestId))
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_LordLevelUp: already claimed [seq=%u]", pstSession->m_udwSeqNo));
        for (TUINT32 udwIdx = 0; udwIdx < stResInfo.udwTotalNum; ++udwIdx)
        {
            if (stResInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_LORD_LV)
            {
                if (stResInfo.aRewardList[udwIdx].udwNum > pstUser->m_tbPlayer.m_nLevel)
                {
                    pstUser->m_tbPlayer.Set_Level(stResInfo.aRewardList[udwIdx].udwNum);
                }
                break;
            }
        }
        return 0;
    }

    // 3、获取奖励
    TINT32 dwLevelBefore = pstUser->m_tbPlayer.m_nLevel;
    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stResInfo);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessQuestClaim: take reward fail [uid=%ld quest_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwQuestId, dwRetCode));
        return dwRetCode;
    }
    TINT32 dwLevelAfter = pstUser->m_tbPlayer.m_nLevel;
    snprintf(pstSession->m_stReqParam.m_szKey[1], sizeof(pstSession->m_stReqParam.m_szKey[1]), "%d", dwLevelBefore < dwLevelAfter ? 1 : 0);

    // 5、设置完成标记
    BITSET(stLevel.m_bitLevel, udwQuestId);
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_LORD_LEVEL_FINISH);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonLevelUp(SSession *pstSession, TBOOL& bNeedResponse)
{
    //TODO 等级依赖限制

    TINT32 dwRetCode = 0;

    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    SLevelQuest& stLevel = pstUser->m_tbUserStat.m_bDragon_level_finish[0];

    TUINT32 udwQuestId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (udwQuestId >= MAX_LEVEL_LIMIT)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_DragonLevelUp: no such quest [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }

    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    TCHAR szKey[32];
    snprintf(szKey, 32, "%u", udwQuestId);
    if (!pstGameInfo->m_oJsonRoot["game_dragon_level_up"].isMember(szKey))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_DragonLevelUp: not quest [%s] [seq=%u]", szKey, pstSession->m_udwSeqNo));
        return -2;
    }
    const Json::Value &pQuest = pstGameInfo->m_oJsonRoot["game_dragon_level_up"][szKey];
    if (pQuest.isNull())
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_DragonLevelUp: wrong game json [%s] [seq=%u]", szKey, pstSession->m_udwSeqNo));
        return -3;
    }
    const Json::Value &oReward = pstGameInfo->m_oJsonRoot["game_dragon_level_up"][szKey]["r"]["a1"];
    TINT32 dwRewardType = pstGameInfo->m_oJsonRoot["game_dragon_level_up"][szKey]["r"]["a0"].asInt();
    SSpGlobalRes stResInfo;
    stResInfo.Reset();
    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(oReward, dwRewardType, &stResInfo);
    if (dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_DragonLevelUp: get reward info fail [uid=%ld quest_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwQuestId, dwRetCode));
        return dwRetCode;
    }

    if (BITTEST(stLevel.m_bitLevel, udwQuestId))
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("ProcessCmd_DragonLevelUp: already claimed [seq=%u]", pstSession->m_udwSeqNo));

        for (TUINT32 udwIdx = 0; udwIdx < stResInfo.udwTotalNum; ++udwIdx)
        {
            if (stResInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_LV)
            {
                if (stResInfo.aRewardList[udwIdx].udwNum > pstUser->m_tbPlayer.m_nDragon_level)
                {
                    pstUser->m_tbPlayer.Set_Dragon_level(stResInfo.aRewardList[udwIdx].udwNum);
                }
                break;
            }
        }
        return 0;
    }

    // 3、获取奖励
    TINT32 dwLevelBefore = pstUser->m_tbPlayer.m_nDragon_level;
    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stResInfo);
    if (dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_DragonLevelUp: take reward fail [uid=%ld quest_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwQuestId, dwRetCode));
        return dwRetCode;
    }
    TINT32 dwLevelAfter = pstUser->m_tbPlayer.m_nDragon_level;
    snprintf(pstSession->m_stReqParam.m_szKey[1], sizeof(pstSession->m_stReqParam.m_szKey[1]), "%d", dwLevelBefore < dwLevelAfter ? 1 : 0);

    // 5、设置完成标记
    BITSET(stLevel.m_bitLevel, udwQuestId);
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_DRAGON_LEVEL_FINISH);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonNameChange(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;

    // 0. 请求参数
    TCHAR *pszDragonName = pstSession->m_stReqParam.m_szKey[0];
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    // 1. 第一步操作
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (tbPlayer.m_nHas_dragon == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }
//         if (tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//         {
//             pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//             return -2;
//         }
        if (!CToolBase::IsValidName(pszDragonName, EN_DRAGON_NAME))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__OTHER_NAME_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: reserve name [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        // 1.1 检验资源是否足够
        if (dwGemNum > 0)
        {
            if (FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
        }
        else if (dwItemId >= 0)
        {
            if (FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // 2.1 消耗资源
        if(dwGemNum > 0)
        {
            pstSession->m_udwGemCost = dwGemNum;
            CPlayerBase::CostGem(pstUser, dwGemNum);
        }
        else if(dwItemId > 0)
        {
            CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
        }

        tbPlayer.Set_Dragon_name(pszDragonName);

        //task count
        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_CHANGE_HERO_NAME);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonAvatarChange(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    // 0. 请求参数
    TINT32 dwDragonAvatar = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[3]);
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    if (ptbPlayer->m_nHas_dragon == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    // 1.1 检验资源是否足够
    if(dwType == 1)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_AvatarChange: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else if(dwType == 0)
    {
        if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_AvatarChange: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_AvatarChange: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    // 2.1 消耗资源
    if(dwType == 1)
    {
        pstSession->m_udwGemCost = dwGemNum;
        CPlayerBase::CostGem(pstUser, dwGemNum);
    }
    else if(dwType == 0)
    {
        CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
    }

    //task count
    CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_CHANGE_HERO_ICON);

    pstUser->m_tbPlayer.Set_Dragon_avatar(dwDragonAvatar);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TaskClearFlag(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbTask *ptbTask = &pstSession->m_stUserInfo.m_tbTask;
    TBOOL bTimeNodeUpdate = FALSE;
    TBOOL bNomnalNodeUpdate = FALSE;
    std::vector<TUINT32> vecTaskId;

    CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecTaskId);

    for(TUINT32 udwIdx = 0; udwIdx < vecTaskId.size(); ++udwIdx)
    {
        TUINT32 udwTaskId = vecTaskId[udwIdx];
        if(BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId))
        {
            continue;
        }
        const Json::Value &oTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];
        if(!oTaskJson.isMember(CCommonFunc::NumToString(udwTaskId)))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_TaskClearFlag:game json not such quest id [quest_id=%u] [seq=%u]",
                udwTaskId,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        TUINT32 udwTaskType = oTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a7"].asUInt();
        TINT32 dwTaskIdx = -1;
        if(udwTaskType == 1)
        {
            for(TUINT32 udwTimeIdx = 0; udwTimeIdx < ptbTask->m_bTask_time.m_udwNum; ++udwTimeIdx)
            {
                if(ptbTask->m_bTask_time[udwTimeIdx].m_ddwId == 0)
                {
                    continue;
                }
                if(ptbTask->m_bTask_time[udwTimeIdx].m_ddwId == udwTaskId)
                {
                    dwTaskIdx = udwTimeIdx;
                    break;
                }
            }
        }
        else
        {
            for(TUINT32 udwTimeIdx = 0; udwTimeIdx < ptbTask->m_bTask_normal.m_udwNum; ++udwTimeIdx)
            {
                if(ptbTask->m_bTask_normal[udwTimeIdx].m_ddwId == 0)
                {
                    continue;
                }
                if(ptbTask->m_bTask_normal[udwTimeIdx].m_ddwId == udwTaskId)
                {
                    dwTaskIdx = udwTimeIdx;
                    break;
                }
            }
        }
        if(dwTaskIdx == -1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_TaskClearFlag:not such id in task list [task_id=%u] [seq=%u]",
                udwTaskId,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        if(udwTaskType == 1)
        {
            ptbTask->m_bTask_time[dwTaskIdx].m_bIsNew = FALSE;
            //pstQuest->m_bTask_time[dwTaskIdx].m_bIsProgress = FALSE;
            bTimeNodeUpdate = TRUE;
        }
        else
        {
            ptbTask->m_bTask_normal[dwTaskIdx].m_bIsNew = FALSE;
            //pstQuest->m_bTask_normal[dwTaskIdx].m_bIsProgress = FALSE;
            bNomnalNodeUpdate = TRUE;
        }
    }
    if(bTimeNodeUpdate)
    {
        ptbTask->SetFlag(TbTASK_FIELD_TASK_TIME);
    }
    if(bNomnalNodeUpdate)
    {
        ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
    }
    
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TaskOperate(SSession *pstSession, TBOOL &bNeedResponse)
{
    //1 查看世界地图
    SUserInfo *pstUser= &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    // 0. 请求参数
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    switch(udwType)
    {
    case EN_TASK_OPERATE_TYPE_MAP:
        CQuestLogic::SetTaskCurValue(pstUser,pstCity, EN_TASK_TYPE_CHECK_WORLD_MAP_TILE, 1, udwId);
        break;
    case EN_TASK_OPERATE_TYPE_CHECK_MAIL:
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_CHECK_MAIL);
        break;
    case EN_TASK_OPERATE_TYPE_CHECK_REPORT:
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_CHECK_REPORT);
        break;
    case EN_TASK_OPERATE_TYPE_GET_AL_HELP:
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_CHECK_ALLIANCE_HELP);
        break;
    case EN_TASK_OPERATE_TYPE_MAP_TITLE:
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_VISIT_WORLD_MAP);
        break;
    case EN_TASK_OPERATE_TYPE_ALLIANCE_CHAT:
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ALLIANCE_CHAT);
        break;
    default:
        break;
    }
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_RandomReward(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TCHAR* pszReward = pstSession->m_stReqParam.m_szKey[0];
    Json::Value jsonReward;
    Json::Reader jsonReader;
    if(jsonReader.parse(pszReward, jsonReward) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    SSpGlobalRes stGlobalRes;
    stGlobalRes.Reset();
    for(TUINT32 udwIdx = 0; udwIdx < jsonReward.size(); ++udwIdx)
    {
        stGlobalRes.aRewardList[udwIdx].udwType = jsonReward[udwIdx][0U].asUInt();
        stGlobalRes.aRewardList[udwIdx].udwId = jsonReward[udwIdx][1U].asUInt();
        stGlobalRes.aRewardList[udwIdx].udwNum = jsonReward[udwIdx][2U].asUInt();
        stGlobalRes.udwTotalNum++;

        if(stGlobalRes.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_RESOURCE)
        {
            CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_RANDOM_REWARD_RESOURCE_NUM, stGlobalRes.aRewardList[udwIdx].udwNum, stGlobalRes.aRewardList[udwIdx].udwId);
        }
        if(stGlobalRes.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_GEM)
        {
            CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_RANDOM_REWARD_GEM_NUM, stGlobalRes.aRewardList[udwIdx].udwNum);
        }

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_RandomReward: [type=%u, id=%u, num=%u][seq=%u]",
            stGlobalRes.aRewardList[udwIdx].udwType,
            stGlobalRes.aRewardList[udwIdx].udwId,
            stGlobalRes.aRewardList[udwIdx].udwNum,
            pstSession->m_udwSeqNo));
    }

    CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalRes);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_OpenTaskWinDow(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbTask *ptbTask = &pstSession->m_stUserInfo.m_tbTask;

    ptbTask->Set_Task_check_id(0);
    ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);
    ptbTask->Set_Task_open_window_flag(TRUE);

    pstSession->m_stUserInfo.m_bTaskClearCmd = TRUE;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_RemoveObstacle(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    SBuildingInfo stBuildingInfo;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;

    CGameInfo *pstGameInfo = CGameInfo::GetInstance();

    /*
    key0 = 操作类型(0:normal 1 : instant  2 : bug)
    key1 = pos
    key2 = building_id
    key3 = cost_time or gem
    */

    TUINT32 udwUpgradeType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwBuildingId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwCost = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TUINT32 udwIsNeedHelp = atoi(pstSession->m_stReqParam.m_szKey[4]);


    SCityBuildingNode* pstBuildingNode = CCityBase::GetBuildingAtPos(&tbCity, udwPos);
    TUINT8 ucCurLevel = 0;
    if(pstBuildingNode)
    {
        ucCurLevel = pstBuildingNode->m_ddwLevel;
    }
    else
    {
        return 0;
    }
    if(pstSession->m_stReqParam.m_szKey[0][0] == 0
        || pstSession->m_stReqParam.m_szKey[1][0] == 0
        || pstSession->m_stReqParam.m_szKey[2][0] == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RemoveObstacle: remove_obstacle type or pos or building_type is null [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    TbAction* ptbAction = CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, EN_BUFFER_INFO_SECOND_BUILDING_TASK);

    if(udwUpgradeType == 0
        && ptbAction == NULL
        &&pstCity->m_stActionStat.m_ucDoingBuildingNum > 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BUILDING_DOING;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: building or removing now [ucUpgradeType=%u] [ucPos=%u] [DoingBuildingNum=%u] [seq=%u]", \
            udwUpgradeType, udwPos, pstCity->m_stActionStat.m_ucDoingBuildingNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    if(udwUpgradeType == 0
        && ptbAction != NULL
        &&pstCity->m_stActionStat.m_ucDoingBuildingNum >= 2)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TWO_BUILDING_ACTION_DOING;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: building or removing now [ucUpgradeType=%u] [ucPos=%u] [DoingBuildingNum=%u] [seq=%u]", \
            udwUpgradeType, udwPos, pstCity->m_stActionStat.m_ucDoingBuildingNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }

    // 建筑pos上的建筑类型判断
    if(ucCurLevel == 1)
    {
        if(!pstBuildingNode)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -5;
        }
        if(pstBuildingNode->m_ddwType != udwBuildingId)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: pos[%u], raw_type[%u], new_type[%u] [seq=%u]", \
                udwPos, pstBuildingNode->m_ddwType, udwBuildingId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        if(pstGameInfo->m_oJsonRoot["game_building"][CCommonFunc::NumToString(udwBuildingId)]["a"]["a12"].asUInt() != 1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: pos[%u], raw_type[%u], new_type[%u]  is not obstalse [seq=%u]", \
                udwPos, pstBuildingNode->m_ddwType, udwBuildingId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
    }
    poGameInfo->GetBuildingInfo(udwBuildingId, 1, &stBuildingInfo);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: m_audwBaseCost[%u:%u:%u:%u:%u:%u:%u:%u], m_stSpecialItem[%u:%u] [seq=%u]", \
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__GOLD],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__FOOD],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__WOOD],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__STONE],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__ORE],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__SPACE],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__POPULATION],
        stBuildingInfo.m_addwBaseCost[EN_BASE_COST_TYPE__TIME],
        stBuildingInfo.m_stSpecialItem.m_udwId,
        stBuildingInfo.m_stSpecialItem.m_udwNum,
        pstSession->m_stUserInfo.m_udwBSeqNo));

    // 判定资源/gem/物品是否足够:0-normal;1-instant;2-buy
    if(udwUpgradeType == 0 || udwUpgradeType == 1)
    {
        if(FALSE == CCityBase::HasEnoughResource(pstCity, &stBuildingInfo.m_addwBaseCost[0]))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: resource not enough [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
    }
    if(udwUpgradeType == 1 || udwUpgradeType == 2)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwCost))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: gem not enough [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
    }
    if(stBuildingInfo.m_ucUpradeNeedItem == TRUE)   // item所有方式(0-normal;1-instant;2-buy)都需要
    {
        if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, stBuildingInfo.m_stSpecialItem.m_udwId, stBuildingInfo.m_stSpecialItem.m_udwNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: item not enough [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }
    }

    // 消耗资源/gem/物品
    if(udwUpgradeType == 0 || udwUpgradeType == 1)
    {
        CCityBase::CostResource(pstCity, &stBuildingInfo.m_addwBaseCost[0]);

    }
    if(udwUpgradeType == 1 || udwUpgradeType == 2)
    {
        pstSession->m_udwGemCost = udwCost;
        CPlayerBase::CostGem(pstUser, udwCost);
    }
    if(stBuildingInfo.m_ucUpradeNeedItem == TRUE)
    {
        dwRetCode = CItemBase::CostItem(&pstUser->m_tbBackpack, stBuildingInfo.m_stSpecialItem.m_udwId, stBuildingInfo.m_stSpecialItem.m_udwNum);

        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: item use failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }
    }

    // 5. 执行remove
    UActionParam stParam;
    stParam.m_stBuilding.SetValue(udwPos, udwBuildingId, 1, 0, (TCHAR*)tbPlayer.m_sUin.c_str());
    if (udwUpgradeType == 0 && udwCost > 1)
    {
        // 生成主城action
        CActionBase::AddAlAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__BUILDING,
            EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE, EN_BUILDING_STATUS__BUILDDING, udwCost, &stParam);

        if(udwIsNeedHelp && pstUser->m_tbPlayer.m_nAlpos && pstUser->m_udwSelfAlActionNum)
        {
            TbAlliance_action &tbAction = pstUser->m_atbSelfAlAction[pstUser->m_udwSelfAlActionNum - 1];

            TINT32 dwCanHelpTimes = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALLIANCE_HELP_NUM].m_ddwBuffTotal;

            tbAction.Set_Sal(tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            tbAction.Set_Can_help_num(dwCanHelpTimes);
            tbAction.Set_Helped_num(0);
        }
    }
    else
    {
        CProcessAction::BuildingActionDone(pstSession, pstUser, pstCity, EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE, &stParam.m_stBuilding);
    }

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ActiveSecondBuildAction(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0 = gem_cost
    key1 = add_time
    */
    TUINT32 udwGemCost = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwAddTime = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    //has enough gem
    if(FALSE == CPlayerBase::HasEnoughGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemCost))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: gem not enough [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    //process
    TbAction *pstAction = CActionBase::GetActionByBufferId(&pstSession->m_stUserInfo.m_atbAction[0], pstSession->m_stUserInfo.m_udwActionNum, EN_BUFFER_INFO_SECOND_BUILDING_TASK);
    if(pstAction != NULL)
    {
        pstAction->Set_Etime(pstAction->Get_Etime() + udwAddTime);
        pstAction->Set_Ctime(pstAction->Get_Ctime() + udwAddTime);

        TUINT32 udwIdx = CActionBase::GetActionIndex(&pstSession->m_stUserInfo.m_atbAction[0], pstSession->m_stUserInfo.m_udwActionNum, pstAction->m_nId);
        pstSession->m_stUserInfo.m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else
    {
        UActionParam stParam;
        stParam.Reset();
        stParam.m_stItem.SetValue(EN_BUFFER_INFO_SECOND_BUILDING_TASK, 0, udwAddTime, 0);

        CCommonBase::AddAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__ITEM, EN_ACTION_SEC_CLASS__ITEM,
            EN_ITEM_STATUS__USING, udwAddTime, &stParam);

    }
    //cost gem
    CPlayerBase::CostGem(pstUser, udwGemCost);

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}


TINT32 CProcessPlayer::ProcessCmd_ComposeEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=equip_type //装备game type
    key1=scroll_id//消耗的卷轴id
    key2=material_id_list//材料id 以：分隔
    key3=cost_time
    key4=cost_gem//如果不为0 立马完成
    key5=cost_gold
    */
    TINT32 dwRetCode = 0;
    TUINT32 udwEquipType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwScrollId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TCHAR *pszMaterialList = pstSession->m_stReqParam.m_szKey[2];
    TUINT32 udwCostTime = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TUINT32 udwCostGem = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TUINT32 udwCostGold = atoi(pstSession->m_stReqParam.m_szKey[5]);


	//wave@20161003: get gold cost
	const Json::Value &jEquip = CBackpackInfo::GetInstance()->m_oJsonRoot;
	const char* pszEquipType = pstSession->m_stReqParam.m_szKey[0];
	if (!jEquip["equip_equipment"].isMember(pszEquipType))
	{
		pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
		TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:param err EquipType[%s] [seq=%u]",
			pszEquipType, pstSession->m_stUserInfo.m_udwBSeqNo));
		return -1;
	}
	else
	{
		TUINT32 udwTmpGoldCost = jEquip["equip_equipment"][pszEquipType]["b"][6].asUInt();
		if(udwTmpGoldCost != udwCostGold)
		{
			TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip[ForDebug]:gold cost: client=%u, num=%u EquipType[%s] [seq=%u]",
				udwCostGold, udwTmpGoldCost, pszEquipType, pstSession->m_stUserInfo.m_udwBSeqNo));
			udwCostGold = udwTmpGoldCost;
		}		
	}


    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    //const Json::Value &jEquip = CBackpackInfo::GetInstance()->m_oJsonRoot;

    vector<TUINT32> vecMaterialId;
    vector<TUINT32> vecMaterialLv;
    vecMaterialId.clear();
    vecMaterialLv.clear();
    CCommonFunc::GetVectorFromString(pszMaterialList, ':', vecMaterialId);

    //TODO
//     TINT32 dwBuff = 10000 + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_EQUIPMENT_CRAFT_SPEED].m_ddwBuffTotal;
// 
//     TINT32 dwRawTime = 0;
//     for (TUINT32 udwIdx = 0; udwIdx < vecMaterialId.size(); ++udwIdx)
//     {
//         const Json::Value &oMaterialJson = jEquip["equip_material"];
// 
//         if (oMaterialJson.isMember(CCommonFunc::NumToString(vecMaterialId[udwIdx])))
//         {
//             dwRawTime += oMaterialJson[CCommonFunc::NumToString(vecMaterialId[udwIdx])]["c"][0U].asUInt();
//         }
//     }
// 
//     TINT32 dwFreeTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
//     TINT32 dwRealTimeCost = dwRawTime / (1.0 * dwBuff / 10000);
//     if (udwCostGem == 0)
//     {
//         if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTimeCost))
//         {
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip: time_cost is incorrect [raw_time:%d buff:%d given_time:%u calc_time:%d] [seq=%u]",
//                 dwRawTime, dwBuff, udwCostTime, dwRealTimeCost, pstSession->m_stUserInfo.m_udwBSeqNo));
//             if (pstSession->m_bIsNeedCheck)
//             {
//                 pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
//                 return -1;
//             }
//         }
//     }
//     else
//     {
//         dwRealTimeCost -= dwFreeTime;
//         if (!CCommonBase::CheckInstantGem(udwCostGem, dwRealTimeCost))
//         {
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip: instant gem is incorrect [raw_time:%d buff:%d given_gem_cost:%u, calc_time_cost:%d] [seq=%u]",
//                 dwRawTime, dwBuff, udwCostGem, dwRealTimeCost, pstSession->m_stUserInfo.m_udwBSeqNo));
//             if (pstSession->m_bIsNeedCheck)
//             {
//                 pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
//                 return -1;
//             }
//         }
//     }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (udwCostTime == 0 && udwCostGem == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:param err cost and cost gem 0 [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        if (udwCostGem != 0 && !CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwCostGem))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:param err gem not enough [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        if (!CCityBase::HasEnoughGold(pstCity, udwCostGold))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:param err gold not enough [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }
        if (!CBackpack::HasEnoughScroll(pstUser, udwScrollId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:param err scroll not enough [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }

        map<TUINT32, TUINT32> mapMaterialNum;
        mapMaterialNum.clear();
        for (TUINT32 udwIdx = 0; udwIdx < vecMaterialId.size(); ++udwIdx)
        {
            if (mapMaterialNum.find(vecMaterialId[udwIdx]) != mapMaterialNum.end())
            {
                mapMaterialNum[vecMaterialId[udwIdx]]++;
            }
            else
            {
                mapMaterialNum[vecMaterialId[udwIdx]] = 1;
            }
        }
        for (map<TUINT32, TUINT32>::iterator it = mapMaterialNum.begin(); it != mapMaterialNum.end(); it++)
        {
            if (!CBackpack::HasEnoughMaterial(pstUser, it->first, it->second))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:[material_id=%u nun=%u] material not enough [seq=%u]",
                    it->first, it->second, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -10;
            }
        }

        TINT32 dwCost = 0;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwEquipNum; ++udwIdx)
        {
            if (pstSession->m_stUserInfo.m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            dwCost++;
        }
        if (pstUser->m_tbUserStat.m_nEquip_gride - dwCost <= 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_GRIDE_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:equip gride not enough [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -11;
        }

        TINT32 dwPos = CBackpack::GetEquipPos(udwScrollId);
        if (dwPos <= 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip: scroll[%u] GetEquipPos[%d] pos invalid [seq=%u]",
                udwScrollId, dwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -12;
        }

        const Json::Value &jEquip = CBackpackInfo::GetInstance()->m_oJsonRoot;
        for (TUINT32 udwIdx = 0; udwIdx < vecMaterialId.size(); ++udwIdx)
        {
            vecMaterialLv.push_back(jEquip["equip_material"][CCommonFunc::NumToString(vecMaterialId[udwIdx])]["b"][1U].asUInt());
        }
        string sCraft;
        CBackpack::GetMaterialLvStr(vecMaterialLv, sCraft);
        if (!jEquip["equip_craft_new"].isMember(sCraft))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip: equip.json->equip_craft_new not have member[%s] [seq=%u]",
                sCraft.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
            return -13;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
        pstSession->ResetDataCenterReq();

        DataCenterReqInfo* pstReq = new DataCenterReqInfo;
        pstReq->m_udwType = EN_REFRESH_DATA_TYPE__EQUIP_COMPOSE;
        Json::Value rDataReqJson = Json::Value(Json::objectValue);

        rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
        rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
        rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
        rDataReqJson["request"] = Json::Value(Json::objectValue);
        rDataReqJson["request"]["equip_id"] = udwScrollId;
        rDataReqJson["request"]["position"] = dwPos;
        rDataReqJson["request"]["material_list"] = Json::Value(Json::arrayValue);
        for (TUINT32 udwIdx = 0; udwIdx < vecMaterialId.size(); ++udwIdx)
        {
            Json::Value jsonMaterial = Json::Value(Json::arrayValue);
            jsonMaterial[0U] = jEquip["equip_material"][CCommonFunc::NumToString(vecMaterialId[udwIdx])]["b"][0U].asUInt();
            jsonMaterial[1U] = jEquip["equip_material"][CCommonFunc::NumToString(vecMaterialId[udwIdx])]["b"][1U].asUInt();
            assert(jEquip["equip_material"][CCommonFunc::NumToString(vecMaterialId[udwIdx])]["b"].size() >= 6);
            jsonMaterial[2U] = jEquip["equip_material"][CCommonFunc::NumToString(vecMaterialId[udwIdx])]["b"][5U].asInt();
            rDataReqJson["request"]["material_list"][udwIdx] = jsonMaterial;
        }
        rDataReqJson["request"]["chances"] = Json::Value(Json::arrayValue);
        for (TUINT32 udwIdx = 0; udwIdx < MAX_CARD_MATERIAL_LV; ++udwIdx)
        {
            rDataReqJson["request"]["chances"][udwIdx] = jEquip["equip_craft_new"][sCraft]["p"][udwIdx].asUInt();
        }

        Json::FastWriter rEventWriter;
        pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
        pstSession->m_vecDataCenterReq.push_back(pstReq);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]",
            pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: send request to data center failed. [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -14;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;

        TINT32 dwLv = -1;

        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            oRspDataJson.clear();
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__EQUIP_COMPOSE)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -15;
                    }
                    break;
                }
            }
            if (oRspDataJson.isMember("forge_equip") && oRspDataJson["forge_equip"].isMember("lv"))
            {
                dwLv = oRspDataJson["forge_equip"]["lv"].asInt();
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: rsp from data center format error [json=%s]. [seq=%u]",
                    pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -16;
            }
        }

        if (dwLv == -1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: rsp from data center format error [json=%s] lv[%d]. [seq=%u]",
                pstDataCenterRsp->m_sRspJson.c_str(), dwLv, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -17;
        }

        SEquipMentInfo stNewEquip;
        stNewEquip.Reset();
        dwRetCode = CBackpack::ComposeNormalEquip(pstUser, udwEquipType, udwScrollId, vecMaterialId, &stNewEquip, udwCostGem > 0, dwLv);
        if (dwRetCode != 0)
        {
            if (dwRetCode == EN_RET_CODE__COMPOSE_DRAGON_LV_NOT_ENOUGH)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__COMPOSE_DRAGON_LV_NOT_ENOUGH;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:player lv not enough [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -11;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip:compose fail [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -12;
            }

        }

        if (udwCostGem)
        {
            CPlayerBase::CostGem(pstUser, udwCostGem);

            CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_ING_COMPOSE_EQUIP);

            CMsgBase::SendEncourageMail(&pstUser->m_tbUserStat, pstUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_COMPOSE_EQUIP, 0, CCommonFunc::NumToString(stNewEquip.stBaseInfo.udwEType));
        }
        else
        {
            // tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__EQUIP_COMPOSE_BEGIN, pstUser->m_tbPlayer.m_nUid, FALSE, stNewEquip.stBaseInfo.udwEType);

            UActionParam stParam;
            stParam.m_stEquip.SetValue(stNewEquip.uddwId, stNewEquip.stBaseInfo.udwEType, udwScrollId, stNewEquip.stBaseInfo.udwLv, udwCostGold, pszMaterialList, (TCHAR*)pstUser->m_tbPlayer.m_sUin.c_str());

            CActionBase::AddAlAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__EQUIP,
                EN_ACTION_SEC_CLASS__EQUIP_UPGRADE,
                EN_SMELT_STATUS__COMPOSING, udwCostTime, &stParam);
        }

        CCityBase::CostGold(pstCity, udwCostGold);
        pstSession->m_stUserInfo.m_udwCostResource += udwCostGold;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ComposeMaterial(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=id
    */
    TUINT32 udwSourceId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwIsComposeAll = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    //check 
    const Json::Value &oMaterialJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_material"];

    if (!oMaterialJson.isMember(CCommonFunc::NumToString(udwSourceId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeMaterial:not such material [id=%u] [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (oMaterialJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asInt() == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeMaterial:material is highest lv [id=%u] [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    TUINT32 udwCostNum = MAX_UPGRADE_ITEM_USE;
    if (udwIsComposeAll)
    {
        TUINT32 udwOwnNum = CBackpack::GetMaterialNumById(pstUser, udwSourceId);

        TUINT32 udwComposeTime = udwOwnNum / 4;
        if (udwComposeTime == 1)
        {
            udwCostNum = MAX_UPGRADE_ITEM_USE;
        }
        else if (udwComposeTime > 1)
        {
            udwCostNum = MAX_UPGRADE_ITEM_USE * udwComposeTime;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeMaterial:not enough material [id=%u] [seq=%u]", 
                udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    if (!CBackpack::HasEnoughMaterial(pstUser, udwSourceId, udwCostNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeMaterial:not enough material [id=%u] [seq=%u]", 
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    TUINT32 udwTargetId = oMaterialJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asUInt();
    CBackpack::AddMaterial(pstUser, udwTargetId, udwCostNum / 4);
    CBackpack::CostMaterial(pstUser, udwSourceId, udwCostNum);

    // tips
    //CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__COMPOSE_RESULT, pstUser->m_tbPlayer.m_nUid, FALSE, EN_GLOBALRES_TYPE_EQUIP_MATERIAL, udwTargetId);

    //task count
    CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_COMPRESS_MATERIAL_NUM);

    //返回给客户端
    SGlobalRes stReward;
    stReward.ddwTotalNum = 1;
    stReward[0].ddwType = EN_GLOBALRES_TYPE_EQUIP_MATERIAL;
    stReward[0].ddwId = udwTargetId;
    stReward[0].ddwNum = udwCostNum / 4;

    CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WINDOW_TYPE_ONE_MATERIAL, EN_REWARD_WINDOW_GET_TYPE_COMPOSE, 0, &stReward, FALSE);

    pstSession->m_stUserInfo.udwRewardWinType = EN_REWARD_WINDOW_TYPE_ONE_MATERIAL;
    pstSession->m_stUserInfo.udwRewardWinGetType = EN_REWARD_WINDOW_GET_TYPE_COMPOSE;
    pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwType = EN_GLOBALRES_TYPE_EQUIP_MATERIAL;
    pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwId = udwTargetId;
    pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwNum = udwCostNum / 4;
    pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum++;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ComposeSoul(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=id
    */
    TUINT32 udwSourceId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwIsComposeAll = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    //check 
    const Json::Value &oSoulJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_soul"];

    if (!oSoulJson.isMember(CCommonFunc::NumToString(udwSourceId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeSoul:not such soul [id=%u] [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (oSoulJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asInt() == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeSoul:soul is highest lv [id=%u] [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    TUINT32 udwCostNum = MAX_UPGRADE_ITEM_USE;
    if (udwIsComposeAll)
    {
        TUINT32 udwOwnNum = CBackpack::GetSoulNumById(pstUser, udwSourceId);

        TUINT32 udwComposeTime = udwOwnNum / 4;
        if (udwComposeTime == 1)
        {
            //do nothing
        }
        else if (udwComposeTime > 1)
        {
            udwCostNum = MAX_UPGRADE_ITEM_USE * udwComposeTime;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeMaterial:not enough material [id=%u] [seq=%u]", \
                udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    if (!CBackpack::HasEnoughSoul(pstUser, udwSourceId, udwCostNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeSoul:not enough soul [id=%u] [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    TUINT32 udwTargetId = oSoulJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asUInt();
    CBackpack::AddSoul(pstUser, udwTargetId, udwCostNum / 4);
    CBackpack::CostSoul(pstUser, udwSourceId, udwCostNum);

    // tips
    //CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__COMPOSE_RESULT, pstUser->m_tbPlayer.m_nUid, FALSE, EN_GLOBALRES_TYPE_EQUIP_SOUL, udwTargetId);

    //返回给客户端
//     pstSession->m_stUserInfo.udwRewardWinType = EN_REWARD_WINDOW_TYPE_ONE_SOUL;
//     pstSession->m_stUserInfo.udwRewardWinGetType = EN_REWARD_WINDOW_GET_TYPE_COMPOSE;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwType = EN_GLOBALRES_TYPE_EQUIP_SOUL;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwId = udwTargetId;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwNum = udwCostNum / 4;
//     pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum++;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ComposeParts(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=id
    */
    TUINT32 udwSourceId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwIsComposeAll = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    //check 
    const Json::Value &oPartsJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_parts"];

    if (!oPartsJson.isMember(CCommonFunc::NumToString(udwSourceId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeParts:[id=%u]  not such parts [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (oPartsJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asInt() == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeParts:[id=%u]  parts is highest lv  [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    TUINT32 udwCostNum = MAX_UPGRADE_ITEM_USE;
    if (udwIsComposeAll)
    {
        TUINT32 udwOwnNum = CBackpack::GetPartsNumById(pstUser, udwSourceId);

        TUINT32 udwComposeTime = udwOwnNum / 4;
        if (udwComposeTime == 1)
        {
            //do nothing
        }
        else if (udwComposeTime > 1)
        {
            udwCostNum = MAX_UPGRADE_ITEM_USE * udwComposeTime;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeMaterial:not enough material [id=%u] [seq=%u]", \
                udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    if (!CBackpack::HasEnoughParts(pstUser, udwSourceId, udwCostNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeParts:[id=%u]  not enough parts [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    TUINT32 udwTargetId = oPartsJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asUInt();
    CBackpack::AddParts(pstUser, udwTargetId, udwCostNum / 4);
    CBackpack::CostParts(pstUser, udwSourceId, udwCostNum);

    // tips
    //CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__COMPOSE_RESULT, pstUser->m_tbPlayer.m_nUid, FALSE, EN_GLOBALRES_TYPE_EQUIP_PARTS, udwTargetId);

    //返回给客户端
//     pstSession->m_stUserInfo.udwRewardWinType = EN_REWARD_WINDOW_TYPE_ONE_PARTS;
//     pstSession->m_stUserInfo.udwRewardWinGetType = EN_REWARD_WINDOW_GET_TYPE_COMPOSE;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwType = EN_GLOBALRES_TYPE_EQUIP_PARTS;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwId = udwTargetId;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwNum = udwCostNum / 4;
//     pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum++;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ComposeCrystal(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=id
    */
    TUINT32 udwSourceId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwIsComposeAll = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    //check 
    const Json::Value &oCrystalJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_crystal"];

    if (!oCrystalJson.isMember(CCommonFunc::NumToString(udwSourceId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeCrystal:[id=%u] not such crystal  [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (oCrystalJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asInt() == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeCrystal:[id=%u]parts is crystal lv  [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    TUINT32 udwCostNum = MAX_UPGRADE_ITEM_USE;
    if (udwIsComposeAll)
    {
        TUINT32 udwOwnNum = CBackpack::GetCrystalNumById(pstUser, udwSourceId);

        TUINT32 udwComposeTime = udwOwnNum / 4;
        if (udwComposeTime == 1)
        {
            //do nothing
        }
        else if (udwComposeTime > 1)
        {
            udwCostNum = MAX_UPGRADE_ITEM_USE * udwComposeTime;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeMaterial:not enough material [id=%u] [seq=%u]", \
                udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    if (!CBackpack::HasEnoughCrystal(pstUser, udwSourceId, udwCostNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeCrystal:[id=%u] not enough crystal  [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    TUINT32 udwTargetId = oCrystalJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asUInt();
    CBackpack::AddCrystal(pstUser, udwTargetId, udwCostNum / 4);
    CBackpack::CostCrystal(pstUser, udwSourceId, udwCostNum);

    // tips
    //CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__COMPOSE_RESULT, pstUser->m_tbPlayer.m_nUid, FALSE, EN_GLOBALRES_TYPE_EQUIP_CRYSTAL, udwTargetId);

    //返回给客户端
    SGlobalRes stReward;
    stReward.ddwTotalNum = 1;
    stReward[0].ddwType = EN_GLOBALRES_TYPE_EQUIP_CRYSTAL;
    stReward[0].ddwId = udwTargetId;
    stReward[0].ddwNum = udwCostNum / 4;

    CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WINDOW_TYPE_ONE_CRYSTAL, EN_REWARD_WINDOW_GET_TYPE_COMPOSE, 0, &stReward, FALSE);

    pstSession->m_stUserInfo.udwRewardWinType = EN_REWARD_WINDOW_TYPE_ONE_CRYSTAL;
    pstSession->m_stUserInfo.udwRewardWinGetType = EN_REWARD_WINDOW_GET_TYPE_COMPOSE;
    pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwType = EN_GLOBALRES_TYPE_EQUIP_CRYSTAL;
    pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwId = udwTargetId;
    pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwNum = udwCostNum / 4;
    pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum++;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ComposeSpCrystal(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=id
    */
    TUINT32 udwSourceId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwIsComposeAll = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    //check 
    const Json::Value &oCrystalJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_special_crystal"];

    if (!oCrystalJson.isMember(CCommonFunc::NumToString(udwSourceId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeSpCrystal:[id=%u] not such crystal  [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (oCrystalJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asInt() == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeSpCrystal:[id=%u]parts is crystal lv  [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    TUINT32 udwCostNum = MAX_UPGRADE_ITEM_USE;
    if (udwIsComposeAll)
    {
        TUINT32 udwOwnNum = CBackpack::GetSpCrystalNumById(pstUser, udwSourceId);

        TUINT32 udwComposeTime = udwOwnNum / 4;
        if (udwComposeTime == 1)
        {
            //do nothing
        }
        else if (udwComposeTime > 1)
        {
            udwCostNum = MAX_UPGRADE_ITEM_USE * udwComposeTime;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeSpCrystal:not enough material [id=%u] [seq=%u]", \
                udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    if (!CBackpack::HasEnoughSpCrystal(pstUser, udwSourceId, udwCostNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeSpCrystal:[id=%u] not enough crystal  [seq=%u]", \
            udwSourceId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    TUINT32 udwTargetId = oCrystalJson[CCommonFunc::NumToString(udwSourceId)]["b"][4U].asUInt();
    CBackpack::AddSpCrystal(pstUser, udwTargetId, udwCostNum / 4);
    CBackpack::CostSpCrystal(pstUser, udwSourceId, udwCostNum);

    //返回给客户端
//     pstSession->m_stUserInfo.udwRewardWinType = EN_REWARD_WIMDOW_TYPE_ONE_SP_CRYSTAL;
//     pstSession->m_stUserInfo.udwRewardWinGetType = EN_REWARD_WINDOW_GET_TYPE_COMPOSE;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwType = EN_GLOBALRES_TYPE_SPECIAL_CRYSTAL;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwId = udwTargetId;
//     pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwNum = udwCostNum / 4;
//     pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum++;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_AddEquipGride(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
        key0=gem_cost
        key1=num
        key2=item_id
    */
    TUINT32 udwGemCost = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwAddNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    //gem
    if(udwGemCost == 0 && udwItemId == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddEquipGrid: gem and item id zero [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if(udwGemCost > 0 && !CPlayerBase::HasEnoughGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemCost))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddEquipGrid: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    if(udwItemId > 0 && !CItemBase::HasEnoughItem(&pstSession->m_stUserInfo.m_tbBackpack, udwItemId, 1))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddEquipGrid: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    //cost
    if(udwGemCost > 0)
    {
        pstSession->m_udwGemCost = udwGemCost;
        CPlayerBase::CostGem(&pstSession->m_stUserInfo, udwGemCost);
    }
    else if(udwItemId > 0)
    {
        CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, udwItemId, 1);
    }
    pstUser->m_tbUserStat.Set_Equip_gride(pstUser->m_tbUserStat.m_nEquip_gride + udwAddNum);

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_CrystalInsert(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=equip_id
    key1=crystal_id
    key2=pos(装备的第几个槽位)
    */
    TUINT64 uddwEquipId = strtol(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwCrystalId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[2]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    //check
    TINT32 dwEquipIdx = -1;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL &&
            pstUser->m_atbEquip[udwIdx].m_nId == static_cast<TINT64>(uddwEquipId))
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
    if (dwEquipIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NOT_EXIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert: [equip_id=%ld] not such equip [seq=%u]", uddwEquipId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (udwPos >= MAX_CRYSTAL_IN_EQUIPMENT)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__INSERT_IDX_NOT_RIGHT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert: [udwPos=%u] pos err [seq=%u]", udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    SEquipMentInfo stEquip;
    stEquip.Reset();
    CBackpack::GetEquipInfoById(pstUser->m_atbEquip, pstUser->m_udwEquipNum, uddwEquipId, &stEquip);
//     if (stEquip.stStatusInfo.udwStatus == EN_EQUIPMENT_STATUS_ON_DRAGON &&
//         pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert:  hero_status=%u not right [seq=%u]", pstUser->m_tbPlayer.m_nDragon_status, pstSession->m_stUserInfo.m_udwBSeqNo));
//         return -6;
//     }
    if (stEquip.stStatusInfo.audwSlot[udwPos] != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__INSERT_IDX_NOT_RIGHT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert:  [udwPos=%u] pos has crystal already [seq=%u]", udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }
    if (!CBackpack::HasEnoughCrystal(pstUser, udwCrystalId))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CRYSTAL_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert: [crystal_id=%u] not enough crystal [seq=%u]", udwCrystalId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    SCrystalInfo stCrystal;
    stCrystal.Reset();
    CBackpack::GetCrystalInfoById(udwCrystalId, &stCrystal);
    SCrystalInfo stCrystalOnEquip;
    stCrystalOnEquip.Reset();
    for (TUINT32 udwIdx = 0; udwIdx < MAX_CRYSTAL_IN_EQUIPMENT; ++udwIdx)
    {
        TUINT32 udwCrystalOnEquipId = stEquip.stStatusInfo.audwSlot[udwIdx];
        if (udwCrystalOnEquipId == 0)
        {
            continue;
        }
        CBackpack::GetCrystalInfoById(udwCrystalOnEquipId, &stCrystalOnEquip);
        if (stCrystalOnEquip.udwType == stCrystal.udwType)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SAME_TYPE_CRYSTAL;
            return -6;
        }
    }

    pstUser->m_atbEquip[dwEquipIdx].m_bCrystal[0].m_addwNum[udwPos] = udwCrystalId;

    pstUser->m_aucEquipFlag[dwEquipIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    pstUser->m_atbEquip[dwEquipIdx].SetFlag(TbEQUIP_FIELD_CRYSTAL);

    CBackpack::CostCrystal(pstUser, udwCrystalId);

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_CrystalRemove(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=equip_id
    key1= pos(装备的第几个槽位)
    key2=gem_cost
    key3=item_id
    */
    TUINT64 uddwEquipId = strtol(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwGemCost = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[3]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    //check
    TINT32 dwEquipIdx = -1;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL &&
            pstUser->m_atbEquip[udwIdx].m_nId == static_cast<TINT64>(uddwEquipId))
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
    if (dwEquipIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NOT_EXIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalRemove: [equip_id=%ld] not such equip [seq=%u]", uddwEquipId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (udwPos >= MAX_CRYSTAL_IN_EQUIPMENT)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__INSERT_IDX_NOT_RIGHT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalRemove: [udwPos=%u] pos err [seq=%u]", udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    SEquipMentInfo stEquip;
    stEquip.Reset();
    CBackpack::GetEquipInfoById(pstUser->m_atbEquip, pstUser->m_udwEquipNum, uddwEquipId, &stEquip);
//     if (stEquip.stStatusInfo.udwStatus == EN_EQUIPMENT_STATUS_ON_DRAGON &&
//         pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert:  hero_status=%u not right [seq=%u]", pstUser->m_tbPlayer.m_nDragon_status, pstSession->m_stUserInfo.m_udwBSeqNo));
//         return -6;
//     }
    if (stEquip.stStatusInfo.audwSlot[udwPos] == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__INSERT_IDX_NOT_RIGHT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert:  [udwPos=%u] pos has not crystal [seq=%u]", udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    //gem 是否足够
    if (udwGemCost > 0)
    {
        if (!CPlayerBase::HasEnoughGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemCost))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalRemove: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
    }

    if (udwItemId > 0 && udwGemCost == 0)
    {
        if (!CItemBase::HasEnoughItem(&pstSession->m_stUserInfo.m_tbBackpack, udwItemId, 1))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalRemove: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
    }
    if (udwItemId > 0)
    {
        //check item is ok
        SCrystalInfo stCrystal;
        stCrystal.Reset();
        CBackpack::GetCrystalInfoById(stEquip.stStatusInfo.audwSlot[udwPos], &stCrystal);

        if (!CItemLogic::IsCanMoveCrystal(&pstSession->m_stUserInfo, udwItemId, stCrystal.udwLv))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CAN_NOT_UDW_TO_MOVE_CRYSTAL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalRemove: can not use to move this crystal [item_id=%u crystal_lv] [seq=%u]",
                udwItemId, stCrystal.udwLv, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
    }

    //process
    if (udwGemCost > 0 || udwItemId > 0)
    {
        //保留水晶
        CBackpack::AddCrystal(pstUser, stEquip.stStatusInfo.audwSlot[udwPos]);
    }
    else
    {
        //不保留水晶，弹tip
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__CRYSTAL_IS_DESTROYED, pstUser->m_tbPlayer.m_nUid, FALSE, stEquip.stStatusInfo.audwSlot[udwPos], 0, 0);
    }

    pstUser->m_atbEquip[dwEquipIdx].m_bCrystal[0].m_addwNum[udwPos] = 0;
    pstUser->m_aucEquipFlag[dwEquipIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    pstUser->m_atbEquip[dwEquipIdx].SetFlag(TbEQUIP_FIELD_CRYSTAL);

    //cost Gem
    if (udwGemCost > 0)
    {
        pstSession->m_udwGemCost = udwGemCost;
        CPlayerBase::CostGem(&pstSession->m_stUserInfo, udwGemCost);
    }
    else if (udwItemId > 0)
    {
        CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, udwItemId, 1);
    }

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_PutOnEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=equip_id
    key1=pos
    */
    TINT32 dwRetCode = 0;
    TUINT64 uddwEquipId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *pstPlayer = &pstUser->m_tbPlayer;

    //check
    TINT32 dwEquipIdx = -1;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL &&
            pstUser->m_atbEquip[udwIdx].m_nId == static_cast<TINT64>(uddwEquipId))
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
//     if (pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert:  dragon_status=%u not in city [seq=%u]",
//             pstUser->m_tbPlayer.m_nDragon_status, pstSession->m_stUserInfo.m_udwBSeqNo));
//         return -6;
//     }

    if (dwEquipIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NOT_EXIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PutOnEquip: [equip_id=%ld] not such equip [seq=%u]", uddwEquipId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (udwPos > MAX_EQUIP_POS_ON_PLAYER || udwPos == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PutOnEquip: [udwPos=%u] pos err [seq=%u]", udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    SEquipMentInfo stEquipInfo;
    stEquipInfo.Reset();
    dwRetCode = CBackpack::GetEquipInfoById(pstUser->m_atbEquip, pstUser->m_udwEquipNum, uddwEquipId, &stEquipInfo);
    if (dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PutOnEquip: [uddwEquipId=%ld] get equip info fail [ret=%d] [seq=%u]",
            uddwEquipId, dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    if (stEquipInfo.stBaseInfo.udwOnLv > pstPlayer->m_nDragon_level)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_LV_NOT_ENOUGH_PUT_ON_EQUIP;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PutOnEquip: [uddwEquipId=%ld on_lv=%u player_lv=%u] player lv not enough [seq=%u]",
            stEquipInfo.stBaseInfo.udwOnLv, pstPlayer->m_nDragon_level, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    //process
    //是否已有装备在身上
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL &&
            pstUser->m_atbEquip[udwIdx].m_nPut_on_pos == udwPos)
        {
            pstUser->m_atbEquip[udwIdx].Set_Status(EN_EQUIPMENT_STATUS_NORMAL);
            pstUser->m_atbEquip[udwIdx].Set_Put_on_pos(0);
            pstUser->m_aucEquipFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        }
    }

    pstUser->m_atbEquip[dwEquipIdx].Set_Status(EN_EQUIPMENT_STATUS_ON_DRAGON);
    pstUser->m_atbEquip[dwEquipIdx].Set_Put_on_pos(udwPos);
    pstUser->m_atbEquip[dwEquipIdx].Set_Put_on_time(pstUser->m_atbEquip[dwEquipIdx].m_nPut_on_time == 0 ? CTimeUtils::GetUnixTime() : pstUser->m_atbEquip[dwEquipIdx].m_nPut_on_time);
    pstUser->m_aucEquipFlag[dwEquipIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_PutOffEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=equip_id
    key1=pos
    */
    TUINT64 uddwEquipId = strtol(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    //check
    TINT32 dwEquipIdx = -1;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL &&
            pstUser->m_atbEquip[udwIdx].m_nId == static_cast<TINT64>(uddwEquipId))
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
//     if (pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//         TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert:  dragon_status=%u not in city [seq=%u]",
//             pstUser->m_tbPlayer.m_nDragon_status, pstSession->m_stUserInfo.m_udwBSeqNo));
//         return -6;
//     }
    if (dwEquipIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NOT_EXIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PutOffEquip: [equip_id=%ld] not such equip [seq=%u]", uddwEquipId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (udwPos > MAX_EQUIP_POS_ON_PLAYER || udwPos == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PutOffEquip: [udwPos=%u] pos err [seq=%u]", udwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    if (pstUser->m_atbEquip[dwEquipIdx].m_nStatus != EN_EQUIPMENT_STATUS_ON_DRAGON ||
        pstUser->m_atbEquip[dwEquipIdx].m_nPut_on_pos != udwPos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PutOffEquip: [equip_id=%ld] not on player [seq=%u]", uddwEquipId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    //process
    pstUser->m_atbEquip[dwEquipIdx].Set_Status(EN_EQUIPMENT_STATUS_NORMAL);
    pstUser->m_atbEquip[dwEquipIdx].Set_Put_on_pos(0);
    pstUser->m_aucEquipFlag[dwEquipIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DestroyEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=equip_id
    */
    TINT32 dwRetCode = 0;
    TUINT64 uddwEquipId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    //check
    TINT32 dwEquipIdx = -1;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL &&
            pstUser->m_atbEquip[udwIdx].m_nId == static_cast<TINT64>(uddwEquipId))
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
    if (dwEquipIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NOT_EXIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DestroyEquip: [equip_id=%ld] not such equip [seq=%u]", uddwEquipId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {

        SEquipMentInfo stEquipInfo;
        stEquipInfo.Reset();
        dwRetCode = CBackpack::GetEquipInfoById(pstUser->m_atbEquip, pstUser->m_udwEquipNum, uddwEquipId, &stEquipInfo);

        TUINT32 udwEquipType = stEquipInfo.stBaseInfo.udwEType;
        TUINT32 udwEquipLv = stEquipInfo.stBaseInfo.udwLv;

        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DestroyEquip: [uddwEquipId=%ld] get equip info fail [ret=%d] [seq=%u]", uddwEquipId, dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

//         if (stEquipInfo.stStatusInfo.udwStatus == EN_EQUIPMENT_STATUS_ON_DRAGON &&
//             pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//         {
//             pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_NOT_IN_CITY;
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CrystalInsert:  hero_status=%u not right [seq=%u]", pstUser->m_tbPlayer.m_nDragon_status, pstSession->m_stUserInfo.m_udwBSeqNo));
//             return -3;
//         }
        if (stEquipInfo.stBaseInfo.udwCategory == EN_EQUIP_CATEGORY_SOUL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DestroyEquip: [uddwEquipId=%ld] the equip is soul  [seq=%u]", uddwEquipId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        const Json::Value &jEquip = CBackpackInfo::GetInstance()->m_oJsonRoot;
        TINT32 dwPos = CBackpack::GetEquipPos(udwEquipType);
        if (dwPos <= 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ComposeEquip: uddwEquipId[%ld] GetEquipPos[%d] pos invalid [seq=%u]",
                uddwEquipId, dwPos, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        vector<TUINT32> vecMaterialType;
        const Json::Value &jCompose = jEquip["equip_equipment"][CCommonFunc::NumToString(udwEquipType)]["c"];
        for (TUINT32 udwIdx = 0; udwIdx < jCompose.size(); ++udwIdx)
        {
            if (jCompose[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_MATERIAL_TYPE)
            {
                vecMaterialType.push_back(jCompose[udwIdx][1U].asUInt());
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
        pstSession->ResetDataCenterReq();

        DataCenterReqInfo* pstReq = new DataCenterReqInfo;
        pstReq->m_udwType = EN_REFRESH_DATA_TYPE__EQUIP_DESTROY;
        Json::Value rDataReqJson = Json::Value(Json::objectValue);

        rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
        rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
        rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
        rDataReqJson["request"] = Json::Value(Json::objectValue);
        rDataReqJson["request"]["equip_id"] = udwEquipType;
        rDataReqJson["request"]["position"] = dwPos;
        rDataReqJson["request"]["equip_lv"] = udwEquipLv;
        rDataReqJson["request"]["material_list"] = Json::Value(Json::arrayValue);
        for (TUINT32 udwIdx = 0; udwIdx < vecMaterialType.size(); ++udwIdx)
        {
            TUINT32 udwMaterialId = CBackpack::GetMaterialIdByTypeAndLv(vecMaterialType[udwIdx], udwEquipLv);
            Json::Value jsonMaterial = Json::Value(Json::arrayValue);
            jsonMaterial[0U] = vecMaterialType[udwIdx];
            assert(jEquip["equip_material"][CCommonFunc::NumToString(udwMaterialId)]["b"].size() >= 6);
            jsonMaterial[1U] = jEquip["equip_material"][CCommonFunc::NumToString(udwMaterialId)]["b"][5U].asInt();
            rDataReqJson["request"]["material_list"][udwIdx] = jsonMaterial;
        }

        Json::FastWriter rEventWriter;
        pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
        pstSession->m_vecDataCenterReq.push_back(pstReq);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]",
            pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: send request to data center failed. [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;

        TINT32 dwMaterialId = -1;

        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            oRspDataJson.clear();
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__EQUIP_DESTROY)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -7;
                    }
                    break;
                }
            }
            if (oRspDataJson.isMember("dismantle_equip") && oRspDataJson["dismantle_equip"].isMember("material"))
            {
                dwMaterialId = oRspDataJson["dismantle_equip"]["material"].asInt();
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: rsp from data center format error [json=%s]. [seq=%u]",
                    pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -8;
            }
        }

        if (dwMaterialId == -1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: rsp from data center format error [json=%s] dwMaterialId[%d]. [seq=%u]",
                pstDataCenterRsp->m_sRspJson.c_str(), dwMaterialId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }

        //TINT32 dwMaterialType = CBackpack::GetMaterialTypeByEquipType(udwEquipType);
        //TUINT32 udwMaterialId = CBackpack::GetMaterialIdByTypeAndLv(dwMaterialType, udwEquipLv); // 废弃，从DC获取
        TUINT32 udwMaterialId = dwMaterialId;
        if (udwMaterialId == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DestroyEquip: [udwMaterialId=%u] not fund such material [seq=%u]",
                udwMaterialId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }

        pstUser->m_aucEquipFlag[dwEquipIdx] = EN_TABLE_UPDT_FLAG__DEL;

        //add material 
        CBackpack::AddMaterial(pstUser, udwMaterialId);

        //返回给客户端
        SGlobalRes stReward;
        stReward.ddwTotalNum = 1;
        stReward[0].ddwType = EN_GLOBALRES_TYPE_EQUIP_MATERIAL;
        stReward[0].ddwId = udwMaterialId;
        stReward[0].ddwNum = 1;

        CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WINDOW_TYPE_ONE_MATERIAL, EN_REWARD_WINDOW_GET_TYPE_DESTROY, 0, &stReward, FALSE);

        pstSession->m_stUserInfo.udwRewardWinType = EN_REWARD_WINDOW_TYPE_ONE_MATERIAL;
        pstSession->m_stUserInfo.udwRewardWinGetType = EN_REWARD_WINDOW_GET_TYPE_DESTROY;
        pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwType = EN_GLOBALRES_TYPE_EQUIP_MATERIAL;
        pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwId = udwMaterialId;
        pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum].udwNum = 1;
        pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum++;
    }

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BuyScroll(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=scroll_id
    key1=gold_cost
    */
    TUINT32 udwScrollId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT64 ddwGoldNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    const Json::Value &oScrollJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_scroll"];

    if (!oScrollJson.isMember(CCommonFunc::NumToString(udwScrollId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuyScroll:not such scroll [id=%lu] [seq=%u]", 
            udwScrollId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (oScrollJson[CCommonFunc::NumToString(udwScrollId)]["a2"].asInt64() > ddwGoldNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuyScroll:gold_cost not right [%ld:%ld] [seq=%u]",
            ddwGoldNum, oScrollJson[CCommonFunc::NumToString(udwScrollId)]["a2"].asInt64(), 
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (!CCityBase::HasEnoughGold(&pstUser->m_stCityInfo, ddwGoldNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuyScroll:gold not enough [%ld:%ld] [seq=%u]",
            ddwGoldNum, pstUser->m_stCityInfo.m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD],
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    CBackpack::AddScroll(pstUser, udwScrollId, 1);

    CCityBase::CostGold(&pstUser->m_stCityInfo, ddwGoldNum);

    pstUser->m_udwCostResource += ddwGoldNum;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BuyScrollNew(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=scroll_id
    key1=gold_cost
    */
    TUINT32 udwScrollId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT64 ddwGemNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    const Json::Value &oScrollJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_scroll"];

    if (!oScrollJson.isMember(CCommonFunc::NumToString(udwScrollId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuyScroll:not such scroll [id=%lu] [seq=%u]",
            udwScrollId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (oScrollJson[CCommonFunc::NumToString(udwScrollId)]["a7"].asInt64() > ddwGemNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuyScroll:gold_cost not right [%ld:%ld] [seq=%u]",
            ddwGemNum, oScrollJson[CCommonFunc::NumToString(udwScrollId)]["a7"].asInt64(),
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (!CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, ddwGemNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuyScroll:gold not enough [%ld:%ld] [seq=%u]",
            ddwGemNum, pstUser->m_tbLogin.m_nGem, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    CBackpack::AddScroll(pstUser, udwScrollId, 1);

    CPlayerBase::CostGem(pstUser, ddwGemNum);

    pstUser->m_udwCostResource += ddwGemNum;

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DropScroll(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=scroll_id
    */
    TUINT32 udwScrollId = atoi(pstSession->m_stReqParam.m_szKey[0]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    const Json::Value &oScrollJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_scroll"];

    if (!oScrollJson.isMember(CCommonFunc::NumToString(udwScrollId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuyScroll:not such scroll [id=%lu] [seq=%u]",
            udwScrollId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    CBackpack::DropScroll(pstUser, udwScrollId);

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ReleaseDragon(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwTargetUid = atoi(pstSession->m_stReqParam.m_szKey[0]);

    SUserInfo* pstUser = &pstSession->m_stUserInfo;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TbMarch_action* ptbPrisonTimer = NULL;
        TINT32 dwIndex = -1;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            ptbPrisonTimer = &pstUser->m_atbPassiveMarch[udwIdx];
            if(ptbPrisonTimer->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            && ptbPrisonTimer->m_nSuid == udwTargetUid)
            {
                dwIndex = udwIdx;
                break;
            }
        }
        if(dwIndex == -1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
            return -1;
        }

        //发给被抓者tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__HERO_IS_RELEASED, udwTargetUid, TRUE, 0, 0, 0, pstUser->m_tbPlayer.m_sUin.c_str());

        CCommonLogic::GenPrisonReport(ptbPrisonTimer, &pstUser->m_tbPlayer, EN_REPORT_TYPE_DRAGON_RELEASE, EN_REPORT_RESULT_DRAGON_RELEASE, &pstSession->m_tbTmpReport);
        ptbPrisonTimer->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
        //ptbPrisonTimer->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_BE_RELEASED;
        ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
        ptbPrisonTimer->Set_Etime(CTimeUtils::GetUnixTime());
        ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__CALCING);
        pstUser->m_aucPassiveMarchFlag[dwIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__REPORT_ID);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReleaseHero: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbParam tbReportId;
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbReportId);
        if(dwRetCode <= 0 || tbReportId.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_RELEASED,
            "", "",
            0, 0,
            0, 0,
            0, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, udwTargetUid, stNoticInfo);

        pstSession->ResetAwsInfo();
        pstSession->m_tbTmpReport.Set_Id(tbReportId.m_nVal);
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpReport);

        pstSession->ResetReportInfo();
        CReportSvrRequest::UserReportPut(pstSession, udwTargetUid, tbReportId.m_nVal, pstSession->m_tbTmpReport.m_nType, EN_ALLIANCE_REPORT_GET_TYPE__ALL);
        CReportSvrRequest::UserReportPut(pstSession, pstUser->m_tbPlayer.m_nUid, tbReportId.m_nVal, pstSession->m_tbTmpReport.m_nType, EN_ALLIANCE_REPORT_GET_TYPE__ALL);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReleaseHero: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;

    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Repatriate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_KillDragon(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwTargetUid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwGemCost = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(NULL == pstCity)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_KillHero: no city info [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        if(dwGemCost > 0 && !CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemCost))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            return -2;
        }

        if(dwItemId > 0 && !CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            return -2;
        }

        TbMarch_action* ptbPrisonTimer = NULL;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass != EN_ACTION_SEC_CLASS__PRISON_TIMER
            || pstUser->m_atbPassiveMarch[udwIdx].m_nSuid != udwTargetUid)
            {
                continue;
            }
            ptbPrisonTimer = &pstUser->m_atbPassiveMarch[udwIdx];
            ptbPrisonTimer->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
            ptbPrisonTimer->Set_Tbid(0, UPDATE_ACTION_TYPE_DELETE);
            //ptbPrisonTimer->DeleteField(TbMARCH_ACTION_FIELD_TAL);
            //ptbPrisonTimer->DeleteField(TbMARCH_ACTION_FIELD_TBID);
            ptbPrisonTimer->Set_Etime(CTimeUtils::GetUnixTime() + 1);
            ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_BE_KILLED;
            ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
            ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__CALCING);
            pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        }
        if(ptbPrisonTimer == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
            return -3;
        }

        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        CCommonLogic::GenPrisonReport(ptbPrisonTimer, &pstUser->m_tbPlayer, EN_REPORT_TYPE_DRAGON_KILLED, EN_REPORT_RESULT_WIN, &pstSession->m_tbTmpReport);
        //tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__KILL_HERO, pstUser->m_tbPlayer.m_nUid, FALSE, ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwLevel,
            0, 0, ptbPrisonTimer->m_bPrison_param[0].stDragon.m_szName); //杀者
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HERO_IS_KILLED, udwTargetUid, TRUE, 0, 0, 0, pstUser->m_tbPlayer.m_sUin.c_str()); //被杀者

        TbCity* ptbCity = &pstCity->m_stTblData;
        TUINT32 udwAltarLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__ALTAR);
        if(udwAltarLevel > 0)
        {
            ptbCity->Set_Altar_dragon_name(ptbPrisonTimer->m_bPrison_param[0].stDragon.m_szName);
            ptbCity->Set_Altar_dragon_lv(ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwLevel);
            ptbCity->Set_Altar_buff_btime(udwCurTime);
            ptbCity->Set_Altar_buff_etime(udwCurTime + CPlayerBase::GetAltarBuffTime(ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwLevel));
            ptbCity->m_bAltar_buff.Reset();
            ptbCity->m_bAltar_buff.m_udwNum = 0;;
            CGameInfo* pstGameInfo = CGameInfo::GetInstance();
            string szAltarId = CCommonFunc::NumToString(CCityBase::GetBuildingIdByFuncType(&pstCity->m_stTblData, EN_BUILDING_TYPE__ALTAR));
            const Json::Value& jsonBuff = pstGameInfo->m_oJsonRoot["game_building"][szAltarId]["b"]["b3"][udwAltarLevel - 1];
            Json::Value::Members members = jsonBuff.getMemberNames();
            TINT64 ddwFactor = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_ALTAR_DRAGON_BUFF_FACTOR);
            for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
            {
                TUINT32 udwBuffId = jsonBuff[*it][0u].asUInt();
                TUINT32 udwBuffNum = jsonBuff[*it][1u].asUInt();

                ptbCity->m_bAltar_buff[ptbCity->m_bAltar_buff.m_udwNum].ddwBuffId = udwBuffId;
                ptbCity->m_bAltar_buff[ptbCity->m_bAltar_buff.m_udwNum].ddwBuffNum = udwBuffNum + ddwFactor * ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwLevel;
                ptbCity->m_bAltar_buff.m_udwNum++;
            }
            ptbCity->SetFlag(TbCITY_FIELD_ALTAR_BUFF);
        }

        pstSession->ResetAwsInfo();

        Json::Value jsonContent = Json::Value(Json::objectValue);
        jsonContent["killer"] = Json::Value(Json::arrayValue);
        jsonContent["killer"].append(pstUser->m_tbPlayer.m_nUid);
        jsonContent["killer"].append(pstUser->m_tbAlliance.m_sAl_nick_name);
        jsonContent["killer"].append(pstUser->m_tbPlayer.m_sUin);
        jsonContent["owner"] = Json::Value(Json::arrayValue);
        jsonContent["owner"].append(ptbPrisonTimer->m_nSuid);
        jsonContent["owner"].append(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick);
        jsonContent["owner"].append(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName);
        jsonContent["bekilled_dragon"] = Json::Value(Json::arrayValue);
        jsonContent["bekilled_dragon"].append(ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwIconId);
        jsonContent["bekilled_dragon"].append(ptbPrisonTimer->m_bPrison_param[0].stDragon.m_szName);
        jsonContent["bekilled_dragon"].append(ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwLevel);
        jsonContent["kill_time"] = udwCurTime;

        TbAltar_history tbAltarHistory;
        tbAltarHistory.Set_Time(udwCurTime);
        tbAltarHistory.Set_Content(jsonContent);

        tbAltarHistory.Set_Uid(ptbPrisonTimer->m_nSuid);
        CAwsRequest::UpdateItem(pstSession, &tbAltarHistory);

        tbAltarHistory.Set_Uid(pstUser->m_tbPlayer.m_nUid);
        CAwsRequest::UpdateItem(pstSession, &tbAltarHistory);

        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__REPORT_ID);

        TbPlayer tbTargetPlayer;
        tbTargetPlayer.Set_Uid(udwTargetUid);
        CAwsRequest::GetItem(pstSession, &tbTargetPlayer, ETbPLAYER_OPEN_TYPE_PRIMARY);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_KillHero send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbParam tbReportId;
        tbReportId.Reset();
        TbPlayer tbTargetPlayer;
        tbTargetPlayer.Reset();
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo* pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_PARAM)
            {
                CAwsResponse::OnUpdateItemRsp(*pstAwsRspInfo, &tbReportId);
                continue;
            }
            if(strTableRawName == EN_AWS_TABLE_PLAYER)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbTargetPlayer);
                continue;
            }
        }

        if(tbReportId.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -5;
        }

        if(tbTargetPlayer.m_nUid != udwTargetUid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_KillHero: no such target player [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        pstSession->ResetAwsInfo();
        pstSession->m_tbTmpReport.Set_Id(tbReportId.m_nVal);
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpReport);
        pstSession->ResetReportInfo();
        CReportSvrRequest::UserReportPut(pstSession, udwTargetUid, tbReportId.m_nVal, pstSession->m_tbTmpReport.m_nType, EN_ALLIANCE_REPORT_GET_TYPE__ALL);
        CReportSvrRequest::UserReportPut(pstSession, pstUser->m_tbPlayer.m_nUid, tbReportId.m_nVal, pstSession->m_tbTmpReport.m_nType, EN_ALLIANCE_REPORT_GET_TYPE__ALL);

        //自己添加处决英雄次数
        pstUser->m_tbPlayer.m_bDragon_statistics[0].ddwExecuteDragonNum++;
        pstUser->m_tbPlayer.SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
        //对方添加被处决次数
        tbTargetPlayer.m_bDragon_statistics[0].ddwMyDragonExecutedNum++;
        tbTargetPlayer.SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
        CAwsRequest::UpdateItem(pstSession, &tbTargetPlayer);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_KillHero: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_KILLED,
            pstSession->m_stUserInfo.m_tbPlayer.m_sUin, "",
            0, 0,
            0, 0,
            0, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, udwTargetUid, stNoticInfo);
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_KILL_HERO);

        CMsgBase::SendEncourageMail(&pstSession->m_stUserInfo.m_tbUserStat, pstSession->m_stUserInfo.m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_EXECUTE_HERO);

        if(dwGemCost > 0)
        {
            CPlayerBase::CostGem(pstUser, dwGemCost);
        }
        if(dwItemId > 0)
        {
            CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Repatriate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ReviveDragon(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT32 dwGemCost = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo* pstUser = &pstSession->m_stUserInfo;

    if (dwGemCost > 0 && !CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemCost))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
        return -2;
    }

    if (dwItemId > 0 && !CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
        return -2;
    }

    if (pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_DEAD)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    pstUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_NORMAL);

    if (dwGemCost > 0)
    {
        CPlayerBase::CostGem(pstUser, dwGemCost);
    }
    if (dwItemId > 0)
    {
        CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_SvrChange(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
        key0=target_svr
        key1=item_id
        key2=gem_cost
        key3=targe_pos
    */
    // next procedure
    TINT32 dwRetCode = 0;
    TUINT32 udwTargetSvr = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwGemCost = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TUINT32 udwOldPos = atoi(pstSession->m_stReqParam.m_szKey[4]);

    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbAlliance *pstAlliance = &pstUser->m_tbAlliance;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        //如果服务器处于合服保护状态，则阻止用户进行svr change
        SGameSvrInfo *pstSvrInfo = CGameSvrInfo::GetInstance()->GetSidInfo(udwTargetSvr);
        if(pstSvrInfo->m_dwMergeStatus != EN_SVR_MERGE_STATUS__NORMAL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_MERGE_PROTECTED;
            return -1;
        }
        pstSvrInfo = CGameSvrInfo::GetInstance()->GetSidInfo(pstSession->m_stReqParam.m_udwSvrId);
        if(pstSvrInfo->m_dwMergeStatus != EN_SVR_MERGE_STATUS__NORMAL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_MERGE_PROTECTED;
            return -1;
        }

        if(pstAlliance->m_nAid > 0 && pstUser->m_tbPlayer.m_nAlpos > 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_CHANGE_SVR_IN_ALLIANCE;
            return -1;
        }

        if(pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_DEAD)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CAPTURE_HERO;
                return -3;
            }
        }

        TUINT32 udwMarchNum = 0;
        TBOOL bIsRallyWar = FALSE;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
            {
                continue;
            }
            if(pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TIMER)
            {
                continue;
            }
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
            {
                bIsRallyWar = TRUE;
                break;
            }
            udwMarchNum++;
        }
        if (udwMarchNum || bIsRallyWar)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_MOVE_CITY_BY_MARCH_ACTION;
            if (bIsRallyWar)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_MOVE_CITY_ON_RALLY_WAR;
            }
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: march action nut null [num=%u] [rally_war=%d] [seq=%u]", 
                udwMarchNum, bIsRallyWar, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;

        }
        TUINT32 udwCanNotMoveNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL &&
                (pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING || pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__SETUP_CAMP))
            {
                udwCanNotMoveNum++;
            }
            if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TRANSPORT &&
                pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__UN_LOADING)
            {
                udwCanNotMoveNum++;
            }
        }
        
        if(udwCanNotMoveNum)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_CHANGE_ACTION_ERR;

            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: passive action not null [num=%u] [seq=%u]", udwCanNotMoveNum,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }

        //gem 是否足够
        if(udwGemCost > 0)
        {
            if(!CPlayerBase::HasEnoughGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemCost))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -5;
            }
        }

        if(udwItemId > 0 && udwGemCost == 0)
        {
            if(!CItemBase::HasEnoughItem(&pstSession->m_stUserInfo.m_tbBackpack, udwItemId, 1))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
        }

        for (TUINT32 idx = 0; idx < pstUser->m_udwActionNum; idx++)
        {
            TbAction *ptbAction = &pstUser->m_atbAction[idx];
            if (ptbAction->m_nMclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE
                && ptbAction->m_bParam[0].m_stAttackMove.m_ddwSpecailMoveFlag == 1)
            {
                //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BE_KICKING_OUT;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: be kicking out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return 0;
            }
        }
        
        if (udwOldPos != 0 && udwOldPos != pstCity->m_stTblData.m_nPos)
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAS_KICKED_OUT;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: has kicked out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return 0;
        }

        pstSession->ResetMapSvrReq();
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;

        pstSession->ResetMapSvrReq();

        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(udwTargetSvr, "GetNewCityMap");
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        CJsoncppSeri jSeri;
        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["type"] = 3; // 0: new user 1: random move 2: attack move 3: move city
        jTmp["id"] = udwPos;

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

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }
    
    if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0],
            &pstSession->m_tbTmpMap);

        if (dwRetCode <= 0 || pstSession->m_tbTmpMap.m_nId == 0 || pstSession->m_tbTmpMap.m_nUid != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MOVE_CITY_COORD_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("GetMapInfo: wild belong someone else. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
    } 
    
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if(!pstCity)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MoveCity: city id error [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if (pstSession->m_tbTmpMap.m_nType == EN_WILD_TYPE__SPECIAL_LAKE)
        {
            if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NEW_PROTECT_CANNOT_MOVE_TO_SPECAIL;
                return -10;
            }

            CCommonBase::UpdatePlayerStatusOnSpecailWild(pstUser);
        }
        else
        {
            if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD)
            {
                pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__ON_SPECIAL_WILD));
            }
        }

        // 设置新map数据
        CCommonBase::SetMapToNewCity(&pstSession->m_tbTmpMap, &pstUser->m_tbPlayer, pstCity, pstAlliance);
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
        {
            TbMap* ptbMap = &pstUser->m_atbWild[udwIdx];
            if (EN_WILD_TYPE__CITY != ptbMap->m_nType)
            {
                continue;
            }
            pstSession->m_tbTmpMap.Set_City_info(ptbMap->m_jCity_info);
            break;
        }

        //设置保护时间
        pstSession->m_tbTmpMap.Set_Time_end(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime);

        pstSession->ResetAwsReq();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap, ExpectedDesc(), 0, true);

        //wave@push_data
        CPushDataBasic::PushDataMap_SingleWild(pstSession, &pstSession->m_tbTmpMap);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    // 5. update action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsReq();

        TUINT32 idx = 0;
        // 更新action数据
        for(idx = 0; idx < pstUser->m_udwActionNum; idx++)
        {
            TbAction *ptbAction = &pstUser->m_atbAction[idx];
            //是否存在被动移城action
            if(ptbAction->m_nMclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE)
            {
                pstUser->m_aucActionFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
                continue;
            }
            if(ptbAction->m_nSuid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
            {
                ptbAction->Set_Sid(udwTargetSvr);
                //ptbAction->Set_Scid(pstSession->m_tbTmpMap.m_nId);
                CAwsRequest::UpdateItem(pstSession, ptbAction);
            }
        }

        for(idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
        {
            TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];
            ptbAlAction->Set_Sid(udwTargetSvr);
            ptbAlAction->Set_Scid(pstSession->m_tbTmpMap.m_nId);
            CAwsRequest::UpdateItem(pstSession, ptbAlAction);
        }

        pstSession->m_dwOldRequestAid = pstUser->m_tbPlayer.m_nReq_al >> 32;
        if(pstSession->m_dwOldRequestAid > 0)
        {
            TbAl_member tbAlmember;
            tbAlmember.Set_Aid(-1 * pstSession->m_dwOldRequestAid);
            tbAlmember.Set_Uid(pstUser->m_tbPlayer.m_nUid);
            CAwsRequest::DeleteItem(pstSession, &tbAlmember);
        }

        // send request
        if(pstSession->m_vecAwsReq.size() > 0)
        {
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
        else
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_SvrChange: no action and wild need update [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
    }

    // 6. 最后更新本地city、city所在map和player信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        TINT64 ddwOldSid = pstUser->m_tbPlayer.m_nSid;

        pstUser->m_tbPlayer.Set_Invite_mail_time(CTimeUtils::GetUnixTime());
        pstUser->m_tbPlayer.Set_Svr_change_time(CTimeUtils::GetUnixTime());
        pstUser->m_tbPlayer.Set_Sid(udwTargetSvr);
        pstUser->m_tbLogin.Set_Sid(udwTargetSvr);

        pstUser->m_tbPlayer.Set_Req_al(0);

        TUINT32 udwRawCityId = pstSession->m_stReqParam.m_udwCityId;
        TUINT32 udwNewCityId = pstSession->m_tbTmpMap.m_nId;

        // 更新成功之后设置新city数据
        pstSession->m_stReqParam.m_udwCityId = udwNewCityId;

        // 更新city的id
        pstCity->m_stTblData.Set_Pos(udwNewCityId);

        // 更新player数据
        pstUser->m_tbPlayer.Set_Cid(udwNewCityId);
        
        // 更新原始map数据
        pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        pstSession->m_tbTmpMap.Reset();
        pstSession->m_tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        pstSession->m_tbTmpMap.Set_Id(udwRawCityId);
        pstSession->m_tbTmpMap.Set_Type(EN_WILD_TYPE__CITY);

        CCommonBase::AbandonWild(pstCity, &pstSession->m_tbTmpMap);

        for(TUINT32 udwIdx = 0; udwIdx < MAX_WILD_RETURN_NUM; ++udwIdx)
        {
            pstSession->m_atbTmpWild[udwIdx].Reset();
            pstSession->m_aucTmpWildFlag[udwIdx] = 0;
        }

        //wave@push_data
        CPushDataBasic::PushDataMap_SingleWild(pstSession, &pstSession->m_tbTmpMap);
        
        //禁止 after更新
        pstUser->m_udwWildNum = 0;

        if(udwGemCost > 0)
        {
            CPlayerBase::CostGem(pstUser, udwGemCost);
        }
        else
        {
            CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId);
        }

        if(pstUser->m_tbPlayer.m_nHas_change_svr == 0)
        {
            pstUser->m_tbPlayer.Set_Has_change_svr(1);
        }

        TCHAR szSvrChangeResponse[MAX_JSON_LEN];
        TCHAR szUrl[1024];
        memset(szSvrChangeResponse, 0, sizeof(szSvrChangeResponse));
        const char *pszUrlBeg = strchr(pstSession->m_stReqParam.m_szReqUrl, '?');
        string strRequest = pszUrlBeg;
        std::size_t end = strRequest.find("command=");
        if (end != std::string::npos)
        {
            strRequest.resize(end + strlen("command="));
        }
        CMsgBase::StringReplace(strRequest, "command=", "command=change_sid&key0=");

        memset(szUrl, 0, sizeof(szUrl));
        sprintf(szUrl, "%s%s%u", CConfBase::GetString("account_url_pre", "serv_url").c_str(), strRequest.c_str(), udwTargetSvr);

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: Url:%s [seq=%u]", szUrl, pstSession->m_stUserInfo.m_udwBSeqNo));

        CURLcode res = CToolBase::ResFromUrl(szUrl, szSvrChangeResponse);
        if (CURLE_OK != res)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: change_sid failed[%d][seq=%u]", res, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        for (TUINT32 udwIdx = 0; udwIdx < strlen(szSvrChangeResponse); ++udwIdx)
        {
            if (szSvrChangeResponse[udwIdx] == '\n')
            {
                szSvrChangeResponse[udwIdx] = ' ';
            }
        }
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: account_svr change_sid response: %s. [seq=%u]", szSvrChangeResponse, pstSession->m_udwSeqNo));

        Json::Reader reader;
        pstSession->m_JsonValue.clear();
        if (FALSE == reader.parse(szSvrChangeResponse, pstSession->m_JsonValue))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: parse change_sid response failed. [seq=%u]", pstSession->m_udwSeqNo));
            return -5;
        }

        if (pstSession->m_JsonValue.isObject()
            && pstSession->m_JsonValue.isMember("res_header")
            && pstSession->m_JsonValue["res_header"].isObject()
            && pstSession->m_JsonValue["res_header"].isMember("ret_code")
            && pstSession->m_JsonValue["res_header"]["ret_code"].asInt() == 0)
        {
            pstSession->m_JsonValue.clear();
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: change_sid succ [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        else
        {
            pstSession->m_JsonValue.clear();
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: parse change_sid failed. [seq=%u]", pstSession->m_udwSeqNo));
            return -6;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_RatingRewardCollect( SSession *pstSession, TBOOL& bNeedResponse )
{
    TINT32 dwRewardGem = atoi(pstSession->m_stReqParam.m_szKey[0]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    CRatingUserInfo *pRatingUsreInfo = CRatingUserInfo::GetInstance();
    if (!pRatingUsreInfo->m_oJsonRoot.isMember("rating_user_list") || !pRatingUsreInfo->m_oJsonRoot["rating_user_list"].isObject())
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RatingRewardCollect: rating json error [seq=%u]",
            pstSession->m_udwSeqNo));
        return -3;
    }

    const Json::Value &stRatingRoot = pRatingUsreInfo->m_oJsonRoot["rating_user_list"];
//     TUINT32 udwRatingUserNum = stRatingRoot.size();
//     TINT32 dwRatingIdx = -1;
//     for (TUINT32 udwIdx = 0; udwIdx < udwRatingUserNum; udwIdx++)
//     {
//         if (pstUser->m_tbLogin.Get_Uid() == stRatingRoot[udwIdx][0U].asUInt())
//         {
//             dwRatingIdx = udwIdx;
//             break;
//         }
//     }
     TUINT32 udwRatingTime = 0;
//     if (dwRatingIdx >= 0)

    string szUid = CCommonFunc::NumToString(pstUser->m_tbLogin.m_nUid);
    if (stRatingRoot.isMember(szUid))
    {
        udwRatingTime = stRatingRoot[szUid][0U].asUInt();
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RatingRewardCollect: can not find uid[%ld] [seq=%u]", 
            pstUser->m_tbLogin.Get_Uid(), pstSession->m_udwSeqNo));
        return -3;
    }

    if (dwRewardGem <= 0 || dwRewardGem != stRatingRoot[szUid][1U].asInt())
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RatingRewardCollect: reward gem zero [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }

    if (pstUser->m_bRatingSwitch == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RatingRewardCollect: rating switch off [seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_RatingRewardCollect:[uid=%u],[rating_switch=%u] [last_lg_time=%u] [rating_time=%u] [rating_sersion=%f] [seq=%u]",
        pstUser->m_tbLogin.Get_Uid(),
        pstUser->m_bRatingSwitch,
        pstUser->m_tbLogin.Get_Last_lg_time(),
        pstUser->m_tbLogin.Get_Rating_time(),
        pstSession->m_stReqParam.m_udwVersion,
        pstSession->m_udwSeqNo
        ));

    TUINT32 udwUserGemNum = pstUser->m_tbLogin.Get_Gem() + dwRewardGem;
    pstUser->m_tbLogin.Set_Gem(udwUserGemNum);
    pstUser->m_tbLogin.Set_Rating_time(udwRatingTime);

    pstUser->m_bRatingSwitch = 0;
    pstUser->m_udwRatingGem = 0;

    TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_RatingRewardCollect:[uid=%u],[rating_switch=%u] [last_lg_time=%u] [rating_time=%u] [rating_sersion=%f] [seq=%u]",
        pstUser->m_tbLogin.Get_Uid(),
        pstUser->m_bRatingSwitch,
        pstUser->m_tbLogin.Get_Last_lg_time(),
        pstUser->m_tbLogin.Get_Rating_time(),
        pstSession->m_stReqParam.m_udwVersion,
        pstSession->m_udwSeqNo
        ));

    // next procedure
    pstSession->m_udwCommandStep = EN_PROCEDURE__INIT;
    pstSession->m_udwNextProcedure = EN_PROCEDURE__COMMON_HANDLE_AFTER;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_MysteryStoreBuy(SSession *pstSession, TBOOL& bNeedResponse)
{
    return -1;
}

TINT32 CProcessPlayer::ProcessCmd_MysteryStorePass(SSession *pstSession, TBOOL& bNeedResponse)
{
    return -1;
}

TINT32 CProcessPlayer::ProcessCmd_BlackUserAdd(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbBlacklist *atbBlackList = pstUser->m_atbBlackList;

    TINT64 ddwUserId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    string szUserName = pstSession->m_stReqParam.m_szKey[1];
    TINT64 ddwAvatarId = strtoll(pstSession->m_stReqParam.m_szKey[2], NULL, 10);

    TUINT32 udwMaxNum = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_BLACKLIST_NUM_LIMIT);
    if (udwMaxNum > MAX_BLACKLIST_NUM)
    {
        udwMaxNum = MAX_BLACKLIST_NUM;
    }

    if (pstUser->m_udwBlackListNum >= udwMaxNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACKLIST_IS_FULL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BlackUserAdd: black list num[%u] is max[%u] [seq=%u]", pstUser->m_udwBlackListNum, udwMaxNum, pstUser->m_udwBSeqNo));
        return -1;
    }

    TUINT32 udwIdx = 0;
    for (udwIdx = 0; udwIdx < pstUser->m_udwBlackListNum; udwIdx++)
    {
        if (atbBlackList[udwIdx].m_nTarget_uid == ddwUserId)
        {
            atbBlackList[udwIdx].Set_Uid(ptbPlayer->m_nUid);
            atbBlackList[udwIdx].Set_Target_uid(ddwUserId);
            atbBlackList[udwIdx].Set_Target_uname(szUserName);
            atbBlackList[udwIdx].Set_Target_avatar(ddwAvatarId);
            atbBlackList[udwIdx].Set_Utime(CTimeUtils::GetUnixTime());
            pstUser->m_aucBlackListFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        }
    }
    if (udwIdx >= pstUser->m_udwBlackListNum)
    {
        atbBlackList[pstUser->m_udwBlackListNum].Set_Uid(ptbPlayer->m_nUid);
        atbBlackList[pstUser->m_udwBlackListNum].Set_Target_uid(ddwUserId);
        atbBlackList[pstUser->m_udwBlackListNum].Set_Target_uname(szUserName);
        atbBlackList[pstUser->m_udwBlackListNum].Set_Target_avatar(ddwAvatarId);
        atbBlackList[pstUser->m_udwBlackListNum].Set_Utime(CTimeUtils::GetUnixTime());
        pstUser->m_aucBlackListFlag[pstUser->m_udwBlackListNum] = EN_TABLE_UPDT_FLAG__NEW;
        pstUser->m_udwBlackListNum++;
    }
    
    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BlackUserDel(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbBlacklist *atbBlackList = pstUser->m_atbBlackList;

    TINT64 ddwUserId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwBlackListNum; udwIdx++)
    {
        if (atbBlackList[udwIdx].m_nTarget_uid == ddwUserId)
        {
            pstUser->m_aucBlackListFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__BLACKUSER_DEL, pstUser->m_tbPlayer.m_nUid, FALSE, 0, 0, 0,atbBlackList[udwIdx].m_sTarget_uname.c_str());
        }
    }

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ClearHelpBubble(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    pstUser->m_tbUserStat.Set_Last_help_bubble_time_out(udwCurTime - 1);
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_SetHelpBubble: Help bubble status off. [seq=%u]", pstSession->m_udwSeqNo));

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;

}

TINT32 CProcessPlayer::ProcessCmd_RecoveryMergeUser(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbLogin *ptbLogin = &pstUser->m_tbLogin;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;
    CGameSvrInfo *poGameSvr = CGameSvrInfo::GetInstance();
    SReqParam *pstReq = &pstSession->m_stReqParam;
    TINT32 dwSid = pstReq->m_udwSvrId;
    TINT32 dwNewSvrId = CGameSvrInfo::GetInstance()->GetTargeSid(ptbLogin->m_nSid);

    // 1. 从map中获取未被占领的plain作为新城市的id——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (dwSid == dwNewSvrId)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;

        pstSession->ResetMapSvrInfo();
        // set package
        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(pstSession->m_stReqParam.m_udwSvrId, "GetNewCityMap");
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        CJsoncppSeri jSeri;

        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["type"] = 2; // 0: new user 1: random move 2: attack move
        jTmp["zone"] = -1;
        jTmp["province"] = -1;

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

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RecoveryMergeUser: send get map item request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }
    // 2. 解析获取的新city，并进行信息设置
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TINT32 dwParseCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if (dwParseCode <= 0 || ptbTmpMap->m_nId == 0 || ptbTmpMap->m_nUid != 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RecoveryMergeUser: can not find a map for move [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -4;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
    }

    // 3. 优先修改map数据，方便后续兼容插入失败情况——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        pstSession->ResetAwsInfo();

        // 更新map数据
        ptbTmpMap->Set_Uid(pstUser->m_tbPlayer.m_nUid);

        // get data
        CAwsRequest::UpdateItem(pstSession, ptbTmpMap, ExpectedDesc(), 0, true);

        // send request 
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RecoveryMergeUser: map update request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 4. 处理响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        TINT32 dwNewCityPos = ptbTmpMap->m_nId;
        ptbTmpMap->Reset();
        pstSession->ResetAwsInfo();

        // 更新统计数据
        //poGameSvr->m_aastProvinceInfo[dwNewSvrId][pstReq->m_ucProvince].m_udwCityNum += CConfBase::GetInt("MirrorNum");
        //if (static_cast<TINT32>(poGameSvr->m_aastProvinceInfo[dwNewSvrId][pstReq->m_ucProvince].m_udwPlainNum) > CConfBase::GetInt("MirrorNum"))
        //{
        //    poGameSvr->m_aastProvinceInfo[dwNewSvrId][pstReq->m_ucProvince].m_udwPlainNum -= CConfBase::GetInt("MirrorNum");
        //}
        //else
        //{
        //    poGameSvr->m_aastProvinceInfo[dwNewSvrId][pstReq->m_ucProvince].m_udwPlainNum = 0;
        //}

        // next procedure
        snprintf(pstSession->m_stReqParam.m_szKey[0], 1024, "%d", dwNewSvrId);
        snprintf(pstSession->m_stReqParam.m_szKey[1], 1024, "%d", dwNewCityPos);
        pstSession->m_udwCommand = EN_CLIENT_OP_CMD__CHANGE_SVR;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
        pstSession->m_bGotoOtherCmd = TRUE;

        return 0;
    }

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_KnightAssign( SSession *pstSession, TBOOL& bNeedResponse )
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    TbCity_Knight& bKnight = tbCity.m_bKnight;
    TBOOL bState = TRUE;

    /*
    任职 / 取消任职
    Command = knight_assign
    Key0 = knight id
    Key1 = pos(0代表取消任职)
    */
    // 0. 请求参数
    TUINT8	ucId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT8	ucPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    // 1. check param
    if(ucId >= bKnight.m_udwNum || ucPos >= EN_KNIGHT_POS__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__KnightAssign: req param error [seq=%u]",pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    // 1.1 unassign check
    if(ucPos == 0 && bKnight[ucId].ddwPos == EN_KNIGHT_POS__UNASSIGN)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__KnightAssign: unassign failed, knight pos error [seq=%u]",pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if(ucPos == EN_KNIGHT_POS__UNASSIGN && bKnight[ucId].ddwStatus != EN_KNIGHT_STATUS__ASSIGNING)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__KnightAssign: assign or unassign failed, knight stat error [seq=%u]",pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if(ucPos > EN_KNIGHT_POS__UNASSIGN)
    {
        if(bKnight[ucId].ddwStatus != EN_KNIGHT_STATUS__NORMAL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__KnightAssign: assign or unassign failed, knight stat error [seq=%u]",pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        TUINT32 udwSignleUpkeep = CGameInfo::GetInstance()->GetBasicVal(EN_GAME_BASIC_KNIGHT_ASSIGN_COST_GOLD);
        TUINT32 udwLevel = CGameInfo::GetInstance()->ComputeKnightLevelByExp(bKnight[ucId].ddwExp);
        if(tbCity.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] <= 0 &&
            udwSignleUpkeep * udwLevel > pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_ddwCurProduction) //最小要5个金币
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__KnightAssign: gold not enough [seq=%u]",pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }

    // 1.2 assign check
    if(ucPos > EN_KNIGHT_POS__UNASSIGN)
    {
        if(bKnight[ucId].ddwPos != EN_KNIGHT_POS__UNASSIGN)
        {
            bState = FALSE;
        }
        else
        {
            for(TUINT32 idx = 0; idx < bKnight.m_udwNum; idx++)
            {
                if(bKnight[idx].ddwPos == ucPos)
                {
                    bState = FALSE;
                    break;
                }
            }
        }

        if(bState == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__KnightAssign: assign failed, knight stat error [seq=%u]",pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }

    // 2. assign and unassign
    bKnight[ucId].ddwPos = ucPos;
    if(ucPos == EN_KNIGHT_POS__UNASSIGN)
    {
        bKnight[ucId].ddwStatus = EN_KNIGHT_STATUS__NORMAL;
    }
    else
    {
        bKnight[ucId].ddwStatus = EN_KNIGHT_STATUS__ASSIGNING;
    }    
    tbCity.SetFlag(TbCITY_FIELD_KNIGHT);

    pstSession->m_udwNextProcedure  = EN_PROCEDURE__COMMON_HANDLE_AFTER;

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_PersonGuideClaim( SSession *pstSession, TBOOL& bNeedResponse )
{
    TUINT32 udwGuideId = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    BITSET(ptbPlayer->m_bPerson_guide[0].m_bitGuide, udwGuideId);
    ptbPlayer->SetFlag(TbPLAYER_FIELD_PERSON_GUIDE);

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonUnlock(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwCostTime = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwCostGem = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TUINT32 udwCastleLv = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);

    if (ptbPlayer->m_nDragon_max_lv == 0 || ptbPlayer->m_nHas_dragon > 0
        || udwCastleLv < 1 /*CCommonBase::GetGameBasicVal(EN_GAME_BASIC_CITY_LV_DRAGON_UNLOCK)*/)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DragonUnlock: status not right [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; udwIdx++)
    {
        if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[udwIdx];
        if (!CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, ptbAlAction->m_nId))
        {
            continue;
        }

        if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__UNLOCK_DRAGON)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DragonUnlock: has unlock dragon action [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }

    if (udwCostGem > 0)
    {
        if (!CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwCostGem))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DragonUnlock: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }

    TINT64 addwResource[EN_RESOURCE_TYPE__END] = {0};
    
    const Json::Value& jResCost = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon"]["a9"]["r0"][0U]["a1"];
    for (TUINT32 udwIdx = 0; udwIdx < jResCost.size(); udwIdx++)
    {
        if (jResCost[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            addwResource[jResCost[udwIdx][1U].asInt()] += jResCost[udwIdx][2U].asInt();
        }
    }

    if (!CCityBase::HasEnoughResource(pstCity, addwResource))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DragonUnlock: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    CCityBase::CostResource(pstCity, addwResource);

    if (udwCostGem > 0)
    {
        pstSession->m_udwGemCost = udwCostGem;
        CPlayerBase::CostGem(pstUser, udwCostGem);
        TINT64 ddwMaxEnergy = CPlayerBase::GetCurDragonMaxEnergy(pstUser);
        CPlayerBase::AddDragon(ptbPlayer, ddwMaxEnergy);
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__UNLOCK_DRAGON, ptbPlayer->m_nUid, FALSE);
    }
    else
    {
        UActionParam stParam;
        stParam.Reset();
        CActionBase::AddAlAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__DRAGON, EN_ACTION_SEC_CLASS__UNLOCK_DRAGON, EN_DRAGON_STATUS_UNLOCKING, udwCostTime, &stParam);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_AddRandomReward(SSession *pstSession, TBOOL& bNeesResponse)
{
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwEtime = atoi(pstSession->m_stReqParam.m_szKey[2]);

    if (udwType > 5)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TbRandom_reward tbRandomReward;
        tbRandomReward.Set_Uid(pstSession->m_stReqParam.m_udwUserId);
        tbRandomReward.Set_Type(udwType);
        tbRandomReward.Set_Num(udwNum);
        tbRandomReward.Set_Etime(udwEtime);

        CAwsRequest::UpdateItem(pstSession, &tbRandomReward);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeesResponse = TRUE;
        TINT32 dwRet = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRet < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddRandomReward: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_GetRandomReward(SSession *pstSession, TBOOL& bNeesResponse)
{
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        CAwsRequest::RandomRewardQuery(pstSession, pstSession->m_stReqParam.m_udwUserId);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeesResponse = TRUE;
        TINT32 dwRet = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRet < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetRandomReward: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TINT32 dwRet = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0], pstSession->m_stUserInfo.m_atbRandomReward, sizeof(TbRandom_reward), MAX_RANDOM_REWARD_NUM);
        if (dwRet < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetRandomReward: get random reward failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        else
        {
            pstSession->m_stUserInfo.m_udwRandomRewardNum = dwRet;
        }

        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_RANDOM_REWARD;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_ClaimRandomReward(SSession *pstSession, TBOOL& bNeesResponse)
{
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwRandomRewardNum; udwIdx++)
    {
        if (pstUser->m_atbRandomReward[udwIdx].m_nType == udwType 
            && pstUser->m_atbRandomReward[udwIdx].m_nEtime > udwCurTime)
        {
            if (udwType < EN_RESOURCE_TYPE__END)
            {
                CCityBase::AddResource(pstCity, pstUser->m_atbRandomReward[udwIdx].m_nType, pstUser->m_atbRandomReward[udwIdx].m_nNum);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_RANDOM_REWARD_RESOURCE_NUM, pstUser->m_atbRandomReward[udwIdx].m_nNum, pstUser->m_atbRandomReward[udwIdx].m_nType);
            }
            else if (udwType == 5)
            {
                CPlayerBase::AddGem(&pstUser->m_tbLogin, pstUser->m_atbRandomReward[udwIdx].m_nNum);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_RANDOM_REWARD_GEM_NUM, pstUser->m_atbRandomReward[udwIdx].m_nNum);
            }
            pstUser->m_aucRandomRewardFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TrialInit(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    if (ptbPlayer->m_nTrial_init != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: trial_init[%ld] != 0. just return 0 [seq=%u]",
            ptbPlayer->m_nTrial_init, pstSession->m_udwSeqNo));
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
        pstSession->ResetDataCenterReq();

        DataCenterReqInfo* pstReq = new DataCenterReqInfo;
        pstReq->m_udwType = EN_REFRESH_DATA_TYPE__TRIAL;
        Json::Value rDataReqJson = Json::Value(Json::objectValue);

        rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
        rDataReqJson["uid"] = ptbPlayer->m_nUid;
        rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
        rDataReqJson["dragon_lv"] = ptbPlayer->m_nDragon_level;
        rDataReqJson["request"] = Json::Value(Json::objectValue);
        rDataReqJson["request"]["type"] = 0; // normal

        Json::FastWriter rEventWriter;
        pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
        pstSession->m_vecDataCenterReq.push_back(pstReq);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialInit: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]",
            pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: send request to data center failed. [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -14;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;

        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            oRspDataJson.clear();
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__TRIAL)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialInit: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -15;
                    }
                    break;
                }
            }

            if (!oRspDataJson.isMember("trail"))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error [json=%s]. [seq=%u]",
                    pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -16;
            }
            else
            {
                TUINT32 udwTrialType = oRspDataJson["trail"]["type"].asUInt();

                if (udwTrialType != 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error. udwTrialType[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if (oRspDataJson["trail"]["reward"].size() < MAX_TRIAL_ATK_TIME - 1)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error. reward_size[%u] < %d [json=%s]. [seq=%u]",
                        oRspDataJson["trail"]["reward"].size(), MAX_TRIAL_ATK_TIME - 1, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if ((udwTrialType == 0 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_NORMAL_NUM) ||
                    (udwTrialType == 1 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_RAGE_NUM))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error. type[%u] chest_size[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, oRspDataJson["trail"]["chest"].size(), pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }

                ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime = 0;
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_ATK_TIME - 1; ++udwIdx)
                {
                    Json::Value& jReward = oRspDataJson["trail"]["reward"][udwIdx];
                    ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwType = jReward[0U].asInt();
                    ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwId = jReward[1U].asInt();
                    ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwNum = jReward[2U].asInt();
                }
                ptbPlayer->m_bTrial_monster_normal[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwType = 0;
                ptbPlayer->m_bTrial_monster_normal[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwId = 0;
                ptbPlayer->m_bTrial_monster_normal[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwNum = 0;
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_NORMAL);
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_LUCKY_BAG_NORMAL_NUM; ++udwIdx)
                {
                    Json::Value& jChest = oRspDataJson["trail"]["chest"][udwIdx];
                    ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwType = jChest[0U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwId = jChest[1U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwNum = jChest[2U].asInt();
                }
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_LUCKY_BAG_NORMAL);

                pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
                pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
                pstSession->ResetDataCenterReq();

                DataCenterReqInfo* pstReq = new DataCenterReqInfo;
                pstReq->m_udwType = EN_REFRESH_DATA_TYPE__TRIAL;
                Json::Value rDataReqJson = Json::Value(Json::objectValue);

                rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
                rDataReqJson["uid"] = ptbPlayer->m_nUid;
                rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
                rDataReqJson["dragon_lv"] = ptbPlayer->m_nDragon_level;
                rDataReqJson["request"] = Json::Value(Json::objectValue);
                rDataReqJson["request"]["type"] = 1; // rage

                Json::FastWriter rEventWriter;
                pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
                pstSession->m_vecDataCenterReq.push_back(pstReq);

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialInit: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]",
                    pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
                bNeedResponse = TRUE;
                TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
                if (dwRetCode != 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: send request to data center failed. [ret=%d] [seq=%u]",
                        dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -14;
                }
                return 0;
            }
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;

        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            oRspDataJson.clear();
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__TRIAL)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialInit: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -15;
                    }
                    break;
                }
            }

            if (!oRspDataJson.isMember("trail"))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error [json=%s]. [seq=%u]",
                    pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -16;
            }
            else
            {
                TUINT32 udwTrialType = oRspDataJson["trail"]["type"].asUInt();

                if (udwTrialType != 1)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error. udwTrialType[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if (oRspDataJson["trail"]["reward"].size() < MAX_TRIAL_ATK_TIME - 1)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error. reward_size[%u] < %d [json=%s]. [seq=%u]",
                        oRspDataJson["trail"]["reward"].size(), MAX_TRIAL_ATK_TIME - 1, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if ((udwTrialType == 0 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_NORMAL_NUM) ||
                    (udwTrialType == 1 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_RAGE_NUM))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialInit: rsp from data center format error. type[%u] chest_size[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, oRspDataJson["trail"]["chest"].size(), pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }

                ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime = 0;
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_ATK_TIME - 1; ++udwIdx)
                {
                    Json::Value& jReward = oRspDataJson["trail"]["reward"][udwIdx];
                    ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwType = jReward[0U].asInt();
                    ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwId = jReward[1U].asInt();
                    ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwNum = jReward[2U].asInt();
                }
                ptbPlayer->m_bTrial_monster_rage[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwType = 0;
                ptbPlayer->m_bTrial_monster_rage[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwId = 0;
                ptbPlayer->m_bTrial_monster_rage[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwNum = 0;
                ptbPlayer->m_bTrial_lucky_bag_rage[0].udwOpenNum = 0;
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_LUCKY_BAG_RAGE_NUM; ++udwIdx)
                {
                    Json::Value& jChest = oRspDataJson["trail"]["chest"][udwIdx];
                    ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwType = jChest[0U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwId = jChest[1U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwNum = jChest[2U].asInt();
                }
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_RAGE);
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_LUCKY_BAG_RAGE);

                pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            }
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        ptbPlayer->Set_Trial_init(1);
        ptbPlayer->Set_Trial_rage_open(0);
        ptbPlayer->Set_Trial_rage_mode(0);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TrialRageMode(SSession *pstSession, TBOOL& bNeesResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwAtkTime = atoi(pstSession->m_stReqParam.m_szKey[0]);

    const TUINT32 udwTrialAtkItemId = 836;
    const TUINT32 udwRageItemId = 835; // 狂暴药水

    if (ptbPlayer->m_nTrial_init == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_NOT_INIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialRageMode: trial not init trial_init[%ld] [seq=%u]",
            ptbPlayer->m_nTrial_init, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (ptbPlayer->m_nTrial_rage_open == 0 && FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwRageItemId, 1))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialRageMode: rage_mode not open & item[%d] not enough [seq=%u]",
            udwRageItemId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    if (udwAtkTime < 0 || udwAtkTime > MAX_TRIAL_ATK_TIME)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialRageMode: atk_time[%u] not between 0 and 10 [seq=%u]",
            udwAtkTime, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }
    if (ptbPlayer->m_nTrial_rage_mode == 1)
    {
        if (CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwRageItemId))
        {
            CItemBase::SetItem(&pstUser->m_tbBackpack, udwRageItemId, 0);
            return 0;
        }
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialRageMode: cur mode[%d] error [seq=%u]",
            ptbPlayer->m_nTrial_rage_mode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    TUINT32 udwItemNum = 0;
    TUINT32 udwShardNum = 0;
    if (ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime < udwAtkTime)
    {
        CPlayerBase::GetTrialAttackCost(pstUser, udwAtkTime - ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime, udwItemNum, udwShardNum);
    }
    if (!CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwTrialAtkItemId, udwItemNum) || !CPlayerBase::HasEnoughDragonShard(pstUser, udwShardNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_SHARD_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialRageMode: have free_item[%u] & shard[%u] not enough. need free_item[%u] shard[%u] [seq=%u]",
            CItemBase::GetItemNum(&pstUser->m_tbBackpack, udwTrialAtkItemId), ptbPlayer->m_nDragon_shard, udwItemNum, udwShardNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }

    if (ptbPlayer->m_nTrial_rage_open == 0)
    {
        CItemBase::CostItem(&pstUser->m_tbBackpack, udwRageItemId);
        ptbPlayer->Set_Trial_rage_open(1);

        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TRIAL_ACTIVATE_RAGE_MODE, ptbPlayer->m_nUid, FALSE);
    }
    ptbPlayer->Set_Trial_rage_mode(1);
    CItemBase::CostItem(&pstUser->m_tbBackpack, udwTrialAtkItemId, udwItemNum);
    CPlayerBase::CostDragonShard(pstUser, udwShardNum);

    if (ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime < udwAtkTime)
    {
        CActivitesLogic::ComputeTrialAttackScore(pstUser, EN_TRIAL_MODE__NORMAL, udwAtkTime - ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime);

        for (TUINT32 udwIdx = ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime + 1; udwIdx <= udwAtkTime; ++udwIdx)
        {
            SOneGlobalRes sReward = ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx - 1];
            CGlobalResLogic::AddGlobalRes(pstUser, pstCity, sReward.ddwType, sReward.ddwId, sReward.ddwNum);
        }
        ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime = udwAtkTime;
        ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_NORMAL);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TrialNormalMode(SSession *pstSession, TBOOL& bNeesResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwAtkTime = atoi(pstSession->m_stReqParam.m_szKey[0]);

    const TUINT32 udwTrialAtkItemId = 836;

    if (ptbPlayer->m_nTrial_init == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_NOT_INIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialNormalMode: trial not init trial_init[%ld] [seq=%u]",
            ptbPlayer->m_nTrial_init, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (udwAtkTime < 0 || udwAtkTime > MAX_TRIAL_ATK_TIME)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialNormalMode: atk_time[%u] not between 0 and 10 [seq=%u]",
            udwAtkTime, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }
    if (ptbPlayer->m_nTrial_rage_mode == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialNormalMode: cur mode[%d] error [seq=%u]",
            ptbPlayer->m_nTrial_rage_mode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    TUINT32 udwItemNum = 0;
    TUINT32 udwShardNum = 0;
    if (ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime < udwAtkTime)
    {
        CPlayerBase::GetTrialAttackCost(pstUser, udwAtkTime - ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime, udwItemNum, udwShardNum);
    }
    if (!CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwTrialAtkItemId, udwItemNum) || !CPlayerBase::HasEnoughDragonShard(pstUser, udwShardNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_SHARD_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialNormalMode: have free_item[%u] & shard[%u] not enough: need free_item[%u] shard[%u] [seq=%u]",
            CItemBase::GetItemNum(&pstUser->m_tbBackpack, udwTrialAtkItemId), ptbPlayer->m_nDragon_shard, udwItemNum, udwShardNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    ptbPlayer->Set_Trial_rage_mode(0);
    CItemBase::CostItem(&pstUser->m_tbBackpack, udwTrialAtkItemId, udwItemNum);
    CPlayerBase::CostDragonShard(pstUser, udwShardNum);

    if (ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime < udwAtkTime)
    {
        CActivitesLogic::ComputeTrialAttackScore(pstUser, EN_TRIAL_MODE__RAGE, udwAtkTime - ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime);

        for (TUINT32 udwIdx = ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime + 1; udwIdx <= udwAtkTime; ++udwIdx)
        {
            SOneGlobalRes sReward = ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx - 1];
            CGlobalResLogic::AddGlobalRes(pstUser, pstCity, sReward.ddwType, sReward.ddwId, sReward.ddwNum);
        }
        ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime = udwAtkTime;
        ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_RAGE);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TrialAttack(SSession *pstSession, TBOOL& bNeesResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwMode = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwAtkTime = atoi(pstSession->m_stReqParam.m_szKey[1]);

    const TUINT32 udwTrialAtkItemId = 836;

    if (ptbPlayer->m_nTrial_init == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_NOT_INIT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialAttack: trial not init trial_init[%ld] [seq=%u]",
            ptbPlayer->m_nTrial_init, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (udwMode != ptbPlayer->m_nTrial_rage_mode)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialAttack: req_mode[%lu] != user_mode[%ld] [seq=%u]",
            udwMode, ptbPlayer->m_nTrial_rage_mode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (udwMode != 0 && udwMode != 1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialAttack: req_mode[%lu] not 0 or 1 [seq=%u]",
            udwMode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (udwAtkTime <= 0 || udwAtkTime > MAX_TRIAL_ATK_TIME)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialAttack: atk_time[%u] not between 1 and 10 [seq=%u]",
            udwAtkTime, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    TUINT32 udwRealAtkTime = ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime;
    TUINT32 udwAtkNum = 0;
    if (udwMode == 1)
    {
        udwRealAtkTime = ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime;
    }
    if (udwRealAtkTime < udwAtkTime)
    {
        udwAtkNum = udwAtkTime - udwRealAtkTime;
    }
    TUINT32 udwItemNum = 0;
    TUINT32 udwShardNum = 0;
    CPlayerBase::GetTrialAttackCost(pstUser, udwAtkNum, udwItemNum, udwShardNum);
    if (!CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwTrialAtkItemId, udwItemNum) || !CPlayerBase::HasEnoughDragonShard(pstUser, udwShardNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DRAGON_SHARD_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialAttack: have free_item[%u] & shard[%u] not enough. need free_item[%u] shard[%u] [seq=%u]",
            CItemBase::GetItemNum(&pstUser->m_tbBackpack, udwTrialAtkItemId), ptbPlayer->m_nDragon_shard, udwItemNum, udwShardNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    CItemBase::CostItem(&pstUser->m_tbBackpack, udwTrialAtkItemId, udwItemNum);
    CPlayerBase::CostDragonShard(pstUser, udwShardNum);

    if (udwRealAtkTime < udwAtkTime)
    {
        CActivitesLogic::ComputeTrialAttackScore(pstUser, udwMode, udwAtkTime - udwRealAtkTime);

        if (udwMode == 0)
        {
            for (TUINT32 udwIdx = ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime + 1; udwIdx <= udwAtkTime; ++udwIdx)
            {
                SOneGlobalRes sReward = ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx - 1];
                CGlobalResLogic::AddGlobalRes(pstUser, pstCity, sReward.ddwType, sReward.ddwId, sReward.ddwNum);
            }
            ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime = udwAtkTime;
            ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_NORMAL);
        }
        else if (udwMode == 1)
        {
            for (TUINT32 udwIdx = ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime + 1; udwIdx <= udwAtkTime; ++udwIdx)
            {
                SOneGlobalRes sReward = ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx - 1];
                CGlobalResLogic::AddGlobalRes(pstUser, pstCity, sReward.ddwType, sReward.ddwId, sReward.ddwNum);
            }
            ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime = udwAtkTime;
            ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_RAGE);
        }
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TrialLuckyBagNormal(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (ptbPlayer->m_nTrial_init == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_NOT_INIT;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: trial not init trial_init[%ld] [seq=%u]",
                ptbPlayer->m_nTrial_init, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        if (ptbPlayer->m_nTrial_rage_mode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_MODE_NOT_NORMAL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: trial mode[%ld] not normal [seq=%u]",
                ptbPlayer->m_nTrial_rage_mode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        if (ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime != MAX_TRIAL_ATK_TIME)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_ATK_TIME_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: attack time[%ld] != %d [seq=%u]",
                ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime, MAX_TRIAL_ATK_TIME, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        SOneGlobalRes& sReward = ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[0];
        CGlobalResLogic::AddGlobalRes(pstUser, pstCity, sReward.ddwType, sReward.ddwId, sReward.ddwNum);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
        pstSession->ResetDataCenterReq();

        DataCenterReqInfo* pstReq = new DataCenterReqInfo;
        pstReq->m_udwType = EN_REFRESH_DATA_TYPE__TRIAL;
        Json::Value rDataReqJson = Json::Value(Json::objectValue);

        rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
        rDataReqJson["uid"] = ptbPlayer->m_nUid;
        rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
        rDataReqJson["dragon_lv"] = ptbPlayer->m_nDragon_level;
        rDataReqJson["request"] = Json::Value(Json::objectValue);
        rDataReqJson["request"]["type"] = 0; // normal

        Json::FastWriter rEventWriter;
        pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
        pstSession->m_vecDataCenterReq.push_back(pstReq);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]",
            pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: send request to data center failed. [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -14;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;

        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            oRspDataJson.clear();
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__TRIAL)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -15;
                    }
                    break;
                }
            }

            if (!oRspDataJson.isMember("trail"))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: rsp from data center format error [json=%s]. [seq=%u]",
                    pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -16;
            }
            else
            {
                TUINT32 udwTrialType = oRspDataJson["trail"]["type"].asUInt();

                if (udwTrialType != 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: rsp from data center format error. udwTrialType[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if (oRspDataJson["trail"]["reward"].size() < MAX_TRIAL_ATK_TIME - 1)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: rsp from data center format error. reward_size[%u] < %d [json=%s]. [seq=%u]",
                        oRspDataJson["trail"]["reward"].size(), MAX_TRIAL_ATK_TIME - 1, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if ((udwTrialType == 0 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_NORMAL_NUM) ||
                    (udwTrialType == 1 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_RAGE_NUM))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagNormal: rsp from data center format error. type[%u] chest_size[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, oRspDataJson["trail"]["chest"].size(), pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }

                ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime = 0;
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_ATK_TIME - 1; ++udwIdx)
                {
                    Json::Value& jReward = oRspDataJson["trail"]["reward"][udwIdx];
                    ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwType = jReward[0U].asInt();
                    ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwId = jReward[1U].asInt();
                    ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwNum = jReward[2U].asInt();
                }
                ptbPlayer->m_bTrial_monster_normal[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwType = 0;
                ptbPlayer->m_bTrial_monster_normal[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwId = 0;
                ptbPlayer->m_bTrial_monster_normal[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwNum = 0;
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_LUCKY_BAG_NORMAL_NUM; ++udwIdx)
                {
                    Json::Value& jChest = oRspDataJson["trail"]["chest"][udwIdx];
                    ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwType = jChest[0U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwId = jChest[1U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwNum = jChest[2U].asInt();
                }
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_NORMAL);
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_LUCKY_BAG_NORMAL);
            }

            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TrialLuckyBagRage(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwTime = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (ptbPlayer->m_nTrial_init == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_NOT_INIT;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: trial not init trial_init[%ld] [seq=%u]",
                ptbPlayer->m_nTrial_init, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        if (ptbPlayer->m_nTrial_rage_mode != 1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_MODE_NOT_RAGE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: trial mode[%ld] not rage [seq=%u]",
                ptbPlayer->m_nTrial_rage_mode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        if (ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime != MAX_TRIAL_ATK_TIME)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_ATK_TIME_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: attack time[%ld] != %d [seq=%u]",
                ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime, MAX_TRIAL_ATK_TIME, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        if (udwTime <= 0 || udwTime > MAX_TRIAL_LUCKY_BAG_RAGE_NUM)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: udwTime[%u] not between 1 and 25 [seq=%u]",
                udwTime, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        TBOOL bNeedUpdate = FALSE;
        if (udwTime > ptbPlayer->m_bTrial_lucky_bag_rage[0].udwOpenNum)
        {
            for (TUINT32 udwIdx = ptbPlayer->m_bTrial_lucky_bag_rage[0].udwOpenNum + 1; udwIdx <= udwTime; ++udwIdx)
            {
                if (ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx - 1].ddwType != EN_GLOBALRES_TYPE_TRIAL_GOBLIN)
                {
                    SOneGlobalRes& sReward = ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx - 1];
                    CGlobalResLogic::AddGlobalRes(pstUser, pstCity, sReward.ddwType, sReward.ddwId, sReward.ddwNum);
                }
                else
                {
                    bNeedUpdate = TRUE;
                    break;
                }
            }

            if (udwTime == MAX_TRIAL_LUCKY_BAG_RAGE_NUM)
            {
                bNeedUpdate = TRUE;
            }
        }
        if (bNeedUpdate == FALSE)
        {
            ptbPlayer->m_bTrial_lucky_bag_rage[0].udwOpenNum = udwTime;
            ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_LUCKY_BAG_RAGE);
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: not need to refresh rage data [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
        pstSession->ResetDataCenterReq();

        DataCenterReqInfo* pstReq = new DataCenterReqInfo;
        pstReq->m_udwType = EN_REFRESH_DATA_TYPE__TRIAL;
        Json::Value rDataReqJson = Json::Value(Json::objectValue);

        rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
        rDataReqJson["uid"] = ptbPlayer->m_nUid;
        rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
        rDataReqJson["dragon_lv"] = ptbPlayer->m_nDragon_level;
        rDataReqJson["request"] = Json::Value(Json::objectValue);
        rDataReqJson["request"]["type"] = 1; // rage

        Json::FastWriter rEventWriter;
        pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
        pstSession->m_vecDataCenterReq.push_back(pstReq);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]",
            pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: send request to data center failed. [ret=%d] [seq=%u]",
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -14;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;

        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            oRspDataJson.clear();
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__TRIAL)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -15;
                    }
                    break;
                }
            }

            if (!oRspDataJson.isMember("trail"))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: rsp from data center format error [json=%s]. [seq=%u]",
                    pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -16;
            }
            else
            {
                TUINT32 udwTrialType = oRspDataJson["trail"]["type"].asUInt();

                if (udwTrialType != 1)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: rsp from data center format error. udwTrialType[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if (oRspDataJson["trail"]["reward"].size() <MAX_TRIAL_ATK_TIME - 1)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: rsp from data center format error. reward_size[%u] < %d [json=%s]. [seq=%u]",
                        oRspDataJson["trail"]["reward"].size(), MAX_TRIAL_ATK_TIME - 1, pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }
                if ((udwTrialType == 0 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_NORMAL_NUM) ||
                    (udwTrialType == 1 && oRspDataJson["trail"]["chest"].size() != MAX_TRIAL_LUCKY_BAG_RAGE_NUM))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialLuckyBagRage: rsp from data center format error. type[%u] chest_size[%u] incorrect [json=%s]. [seq=%u]",
                        udwTrialType, oRspDataJson["trail"]["chest"].size(), pstDataCenterRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -16;
                }

                ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime = 0;
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_ATK_TIME - 1; ++udwIdx)
                {
                    Json::Value& jReward = oRspDataJson["trail"]["reward"][udwIdx];
                    ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwType = jReward[0U].asInt();
                    ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwId = jReward[1U].asInt();
                    ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwNum = jReward[2U].asInt();
                }
                ptbPlayer->m_bTrial_monster_rage[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwType = 0;
                ptbPlayer->m_bTrial_monster_rage[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwId = 0;
                ptbPlayer->m_bTrial_monster_rage[0].aRewardList[MAX_TRIAL_ATK_TIME - 1].ddwNum = 0;
                ptbPlayer->m_bTrial_lucky_bag_rage[0].udwOpenNum = 0;
                for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_LUCKY_BAG_RAGE_NUM; ++udwIdx)
                {
                    Json::Value& jChest = oRspDataJson["trail"]["chest"][udwIdx];
                    ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwType = jChest[0U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwId = jChest[1U].asInt();
                    ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwNum = jChest[2U].asInt();
                }
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_MONSTER_RAGE);
                ptbPlayer->SetFlag(TbPLAYER_FIELD_TRIAL_LUCKY_BAG_RAGE);

                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            }
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_TrialGiftCollect(SSession *pstSession, TBOOL& bNeesResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwHasGift = 0;
    TUINT32 udwEndTime = 0;
    SOneGlobalRes sReward;
    sReward.Reset();

    CCommonBase::GetTrialGift(ptbPlayer, CTimeUtils::GetUnixTime(), udwHasGift, udwEndTime, sReward);

    if (!udwHasGift)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TRIAL_NO_GIFT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TrialGiftCollect: trial no global_gift available [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    CGlobalResLogic::AddGlobalRes(pstUser, pstCity, sReward.ddwType, sReward.ddwId, sReward.ddwNum);
    ptbPlayer->Set_Trial_gift_last_etime(udwEndTime);

    // tips
    Json::Value tipContent = Json::Value(Json::arrayValue);
    Json::Value jsonTips = Json::Value(Json::arrayValue);
    jsonTips[0U] = sReward.ddwType;
    jsonTips[1U] = sReward.ddwId;
    jsonTips[2U] = sReward.ddwNum;
    tipContent.append(jsonTips);
    CSendMessageBase::AddTips(pstUser, ptbPlayer->m_nUid, FALSE, EN_TIPS_TYPE__TRIAL_GIFT, tipContent.toStyledString());

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_FinishGuide(SSession *pstSession, TBOOL& bNeesResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    TINT32 dwFinishGuide = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (ptbPlayer->m_bFinish_guide_list[0].udwNum >= MAX_FINISH_GUIDE_LIST_NUM)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_FinishGuide: finish guide num[%u] >= %d [seq=%u]",
            ptbPlayer->m_bFinish_guide_list[0].udwNum, MAX_FINISH_GUIDE_LIST_NUM, pstSession->m_udwSeqNo));
        return -1;
    }

    TBOOL bFind = FALSE;
    for (TUINT32 udwIdx = 0; udwIdx < ptbPlayer->m_bFinish_guide_list[0].udwNum; ++udwIdx)
    {
        if (dwFinishGuide == ptbPlayer->m_bFinish_guide_list[0].addwFinishGuide[udwIdx])
        {
            bFind = TRUE;
            break;
        }
    }

    if (!bFind)
    {
        ptbPlayer->m_bFinish_guide_list[0].addwFinishGuide[ptbPlayer->m_bFinish_guide_list[0].udwNum] = dwFinishGuide;
        ++ptbPlayer->m_bFinish_guide_list[0].udwNum;
        ptbPlayer->SetFlag(TbPLAYER_FIELD_FINISH_GUIDE_LIST);
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_BuildDecoration(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    SBuildingInfo stBuildingInfo;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TbDecoration& tbDecoration = pstSession->m_stUserInfo.m_tbDecoration;
    // 输入参数
    //key0=decoration_id
    //key1=pos
    TUINT32 udwDecoId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwLevel = 1;
    TUINT32 udwExp = 0;

    if (pstSession->m_stReqParam.m_szKey[0][0] == 0
        || pstSession->m_stReqParam.m_szKey[1][0] == 0
        )
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildDecoration: upgradetype or pos or building_type is null [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (tbCity.m_bBuilding.m_udwNum >= MAX_BUILDING_NUM_IN_ONE_CITY)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildDecoration: the num of buildings has reached max [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;

    }
    SCityBuildingNode* pstBuildingNode = CCityBase::GetBuildingAtPos(&tbCity, udwPos);
    if (pstBuildingNode)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildDecoration: a building has existed on the pos  [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    
    SCityBuildingNode stNewBuilding;
    stNewBuilding.m_ddwPos = udwPos;
    stNewBuilding.m_ddwType = udwDecoId;
    stNewBuilding.m_ddwLevel = udwLevel;
    if (CCommonLogic::CheckBuildingCollision(&pstCity->m_stTblData, stNewBuilding) == TRUE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildDecoration: building collision [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    // 判定装饰物是否足够
    dwRetCode = CPlayerBase::HasEnoughDecoration(&pstUser->m_tbDecoration, udwDecoId);
    if (dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RESOURCE_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildDecoration: decoration not enough [ret=%d] [seq=%u]", \
            dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    // 消耗装饰物
    dwRetCode = CPlayerBase::CostDecoration(&pstUser->m_tbDecoration, udwDecoId);
    if (dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BuildDecoration: decoration use failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }
    // 5. 执行upgrade
    CCityBase::AddBuilding(udwPos, udwDecoId, udwLevel, tbCity);
    CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__BUILDING_OK, tbPlayer.m_nUid, FALSE, udwDecoId, udwLevel);
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_OpenDecorationList(SSession *pstSession, TBOOL &bNeedResponse)
{
    //key0=series_list(:分隔)
    string strSeries = pstSession->m_stReqParam.m_szKey[0];

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbDecoration& tbDecoration = pstSession->m_stUserInfo.m_tbDecoration;

    std::vector<TUINT32> vecSeries;
    CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ':', vecSeries);
    Json::Value jSeries = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < vecSeries.size(); udwIdx++)
    {
        jSeries.append(vecSeries[udwIdx]);
    }
    tbDecoration.Set_Series_list(jSeries);
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DeleteDecoration(SSession *pstSession, TBOOL &bNeedResponse)
{
    //key0=id_list(:分隔)
    string strDecoId = pstSession->m_stReqParam.m_szKey[0];

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;
    TbDecoration *tbDecoration = &pstUser->m_tbDecoration;

    TINT64 ddwDecoId = 0;
    std::vector<string> vecDecoId;
    CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ':', vecDecoId);
    Json::Value jTmp = tbDecoration->m_jDecoration_list;
    for (TUINT32 udwIdx = 0; udwIdx < vecDecoId.size(); udwIdx++)
    {
        ddwDecoId = strtoll(vecDecoId[udwIdx].c_str(), NULL, 10);
        if (jTmp[vecDecoId[udwIdx]]["still_have_num"].asUInt() == jTmp[vecDecoId[udwIdx]]["total_num"].asUInt())
        {
            jTmp.removeMember(vecDecoId[udwIdx]);
            continue;
        }
        else
        {
            //删除building list
            for (TUINT32 udwIdy = 0; udwIdy < tbCity.m_bBuilding.m_udwNum; udwIdy++)
            {
                if (ddwDecoId == tbCity.m_bBuilding[udwIdy].m_ddwType)
                {
                    CCityBase::DelBuildingAtPos(&tbCity, tbCity.m_bBuilding[udwIdy].m_ddwPos);
                }
            }
            //删除db
            jTmp.removeMember(vecDecoId[udwIdx]);
        }
    }
    tbDecoration->Set_Decoration_list(jTmp);
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_GiveGift(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TUINT32 udwGiftId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    string szContent = pstSession->m_stReqParam.m_szKey[1];
    TCHAR *szUidList = pstSession->m_stReqParam.m_szKey[2];

    TUINT32 audwUidList[MAX_ALLIANCE_MEMBER_NUM];
    TUINT32 udwUidNum = MAX_ALLIANCE_MEMBER_NUM;
    CCommonFunc::GetArrayFromString(szUidList, ':', audwUidList, udwUidNum);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;

        if (udwUidNum == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: user num[%u] is zero [seq=%u]",
                udwUidNum, pstSession->m_udwSeqNo));
            return -1;
        }

        if (!CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwGiftId, udwUidNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: gift[%u] not enough [seq=%u]",
                udwUidNum, pstSession->m_udwSeqNo));
            return -1;
        }

        SSpGlobalRes stGlobalRes;
        stGlobalRes.Reset();
        TINT32 dwRetCode = CGlobalResLogic::GetLotteryChestGlobalResInfo(udwGiftId, &stGlobalRes);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: get gift reward fail[gift_id: %u] [seq=%u]",
                udwGiftId, pstSession->m_udwSeqNo));
            return -1;
        }
    }
    
    if (EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {
        TINT32 dwRetCode = 0;

        // reset req
        pstSession->ResetTranslateInfo();
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        TBOOL bTranslate = FALSE;
        if (bTranslate && !szContent.empty() && 0 != CDocument::GetInstance()->GetSupportLangNum())
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__TRANSLATE;
            // send request
            bNeedResponse = TRUE;
            Json::FastWriter oJsonWriter;
            oJsonWriter.omitEndingLineFeed();
            Json::Value jTranslateJson = Json::Value(Json::objectValue);
            for (TINT32 dwIdx = 0; dwIdx < CDocument::GetInstance()->GetSupportLangNum(); ++dwIdx)
            {
                jTranslateJson.clear();
                TranslateReqInfo *pstTranslateReq = new TranslateReqInfo;
                jTranslateJson["0"]["from"] = "";
                jTranslateJson["0"]["to"] = CDocument::GetInstance()->GetShortLangName(dwIdx);
                jTranslateJson["0"]["content"] = szContent;
                pstTranslateReq->SetVal("mail", "translate", oJsonWriter.write(jTranslateJson));
                pstSession->m_vecTranslateReq.push_back(pstTranslateReq);

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GiveGift: [TranslateType=%s] [TranslateOperate=%s] [TranslateReqContent=%s] [seq:%u]", 
                    pstTranslateReq->m_strTranslateType.c_str(), 
                    pstTranslateReq->m_strTranslateOperate.c_str(), 
                    pstTranslateReq->m_strTranslateContent.c_str(), 
                    pstSession->m_udwSeqNo));
            }

            dwRetCode = CBaseProcedure::SendTranslateRequest(pstSession, EN_SERVICE_TYPE_TRANSLATE_REQ);
            if (dwRetCode != 0)
            {
                bNeedResponse = FALSE;
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: send translate req fail [ret=%d] [seq=%u]", 
                    dwRetCode, pstSession->m_udwSeqNo));
            }
            else
            {
                return 0;
            }
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        CGameInfo *poGameInfo = CGameInfo::GetInstance();
        TCHAR szGiftId[64];
        sprintf(szGiftId, "%u", udwGiftId);
        if (!poGameInfo->m_oJsonRoot["game_item"].isMember(szGiftId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_GiveGift:game.json not include gift_id=%u [ret=-2]", udwGiftId));
            return -1;
        }
        //判断是否走运营
        TINT32 dwIsDataCenter = poGameInfo->m_oJsonRoot["game_item"][szGiftId]["a21"].asInt();

        if (dwIsDataCenter == 1)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
            pstSession->ResetDataCenterInfo();

            DataCenterReqInfo* pstReq = new DataCenterReqInfo;
            pstReq->m_udwType = EN_REFRESH_DATA_TYPE__CHEST;
            Json::Value rDataReqJson = Json::Value(Json::objectValue);

            rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
            rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
            rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
            rDataReqJson["request"] = Json::Value(Json::objectValue);
            rDataReqJson["request"]["chest_id"] = udwGiftId;
            rDataReqJson["request"]["open_num"] = 1;

            Json::FastWriter writer;
            writer.omitEndingLineFeed();
            pstReq->m_sReqContent = writer.write(rDataReqJson);
            pstSession->m_vecDataCenterReq.push_back(pstReq);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GiveGift: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]", 
                pstReq->m_udwType, 
                pstUser->m_tbPlayer.m_nUid, 
                pstSession->m_udwSeqNo, 
                pstReq->m_sReqContent.c_str()));
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
            if (dwRetCode == 0)
            {
                return 0;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: send request to data center failed. [json=%s] [ret=%d] [seq=%u]", 
                    pstReq->m_sReqContent.c_str(), dwRetCode, pstUser->m_udwBSeqNo));
                return -5;
            }
        }
    }

    if (EN_COMMAND_STEP__3 == pstSession->m_udwCommandStep)
    {
        TBOOL bTranslateFalg = FALSE;
        if ((TUINT32)CDocument::GetInstance()->GetSupportLangNum() == pstSession->m_vecTranslateRsp.size()
            && 0 != pstSession->m_vecTranslateRsp.size())
        {
            bTranslateFalg = TRUE;
        }
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
        {
            if (0 != pstSession->m_vecTranslateRsp[udwIdx]->m_dwRetCode)
            {
                bTranslateFalg = FALSE;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: parse translate info fail [seq=%u]",
                    pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GiveGift: [TranslateType=%s] [TranslateOperate=%s] [TranslateContent=%s] [TranslateResult=%s] [seq=%u]",
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateType.c_str(),
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateOperate.c_str(),
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateContent.c_str(),
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(),
                    pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }

        TINT32 dwLanguageId = -1;
        string strTranslateContent = "";
        Json::FastWriter oJsonWriter;
        oJsonWriter.omitEndingLineFeed();
        Json::Value jContent = Json::Value(Json::objectValue);
        jContent["raw_content"] = szContent;
        Json::Value &jTranslateJson = jContent["translate_info"];
        jTranslateJson = Json::Value(Json::objectValue);
        if (TRUE == bTranslateFalg)
        {
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
            {
                Json::Reader jsonReader;
                Json::Value jResultBody;
                if (jsonReader.parse(pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), jResultBody))
                {
                    if (0 == udwIdx)
                    {
                        dwLanguageId = atoi(CDocument::GetInstance()->GetLanguageId(jResultBody["0"]["from"].asString().c_str(), pstSession->m_stReqParam.m_udwLang).c_str());
                    }
                    Json::Value::Members vecMembers = jResultBody.getMemberNames();
                    for (TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                    {
                        jTranslateJson[CDocument::GetInstance()->GetLangId(jResultBody[vecMembers[0]]["to"].asString()).c_str()][vecMembers[udwIdy]] = jResultBody[vecMembers[udwIdy]]["content"];
                    }
                }
            }
        }
        jContent["raw_lang"] = dwLanguageId;

        strTranslateContent = oJsonWriter.write(jContent);

        CGameInfo *poGameInfo = CGameInfo::GetInstance();
        TCHAR szGiftId[64];
        sprintf(szGiftId, "%u", udwGiftId);
        //判断是否走运营
        TINT32 dwIsDataCenter = poGameInfo->m_oJsonRoot["game_item"][szGiftId]["a21"].asInt();

        SSpGlobalRes stGlobalRes;
        stGlobalRes.Reset();

        if (dwIsDataCenter == 0)
        {
            TINT32 dwRetCode = CGlobalResLogic::GetLotteryChestGlobalResInfo(udwGiftId, &stGlobalRes);
            if (dwRetCode != 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: GetLotteryChestGlobalResInfo ret[%d] [seq=%u]",
                    dwRetCode, pstUser->m_udwBSeqNo));
                return -8;
            }
        }
        else
        {
            SRefreshData stRefreshData;
            vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
            DataCenterRspInfo *pstDataCenterRsp = NULL;
            if (vecRsp.size() > 0)
            {
                stRefreshData.Reset();
                Json::Reader jsonReader;
                Json::Value oRspDataJson;
                for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
                {
                    pstDataCenterRsp = vecRsp[udwIdx];
                    if (EN_REFRESH_DATA_TYPE__CHEST == pstDataCenterRsp->m_udwType)
                    {
                        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GiveGift: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                            pstDataCenterRsp->m_sRspJson.c_str(), 
                            pstUser->m_tbPlayer.m_nUid, 
                            pstUser->m_udwBSeqNo));

                        if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                        {
                            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: prase rsp from data center failed. [seq=%u]", 
                                pstUser->m_udwBSeqNo));
                            return -6;
                        }
                        TINT32 dwRetCode = stRefreshData.m_stChestRsp.setVal(oRspDataJson);
                        if (0 != dwRetCode)
                        {
                            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: response data format error. [ret=%d][seq=%u]", 
                                dwRetCode, pstUser->m_udwBSeqNo));
                            return -7;
                        }
                        break;
                    }
                }
                if (MAX_SP_REWARD_ITEM_NUM < stRefreshData.m_stChestRsp.m_vecReward.size()
                    || 0 >= stRefreshData.m_stChestRsp.m_vecReward.size())
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: chest reward size is over. [size=%ld] [seq=%u]", 
                        stRefreshData.m_stChestRsp.m_vecReward.size(), pstUser->m_udwBSeqNo));
                    return -8;
                }

                for (TINT32 dwIdx = 0; dwIdx < stRefreshData.m_stChestRsp.m_vecReward.size(); ++dwIdx)
                {
                    SOneGlobalRes *pstOneGlobalRes = stRefreshData.m_stChestRsp.m_vecReward[dwIdx];
                    stGlobalRes.aRewardList[dwIdx].udwType = pstOneGlobalRes->ddwType;
                    stGlobalRes.aRewardList[dwIdx].udwId = pstOneGlobalRes->ddwId;
                    stGlobalRes.aRewardList[dwIdx].udwNum = pstOneGlobalRes->ddwNum;
                    ++stGlobalRes.udwTotalNum;
                    if (stGlobalRes.udwTotalNum >= TBMAIL_EX_REWARD_MAX_NUM)
                    {
                        break;
                    }
                }
            }
        }

        if (stGlobalRes.udwTotalNum == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiveGift: reward size == 0 [seq=%u]",
                pstUser->m_udwBSeqNo));
            return -8;
        }

        CMsgBase::GiveGift(pstUser->m_tbPlayer.m_nUid, audwUidList, udwUidNum, udwGiftId, strTranslateContent, &stGlobalRes);

        CItemBase::CostItem(&pstUser->m_tbBackpack, udwGiftId, udwUidNum);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_LordSkillUpgradeNew(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbUser_stat* ptbUserStat = &pstUser->m_tbUserStat;

    TUINT32 udwCount = 0;
    TINT32 dwSkillId = 0;
    TINT32 dwPoint = 0;
    TINT32 dwTotalPoint = 0;
    const TCHAR ucPair = ':';
    const TCHAR ucEntry = ',';
    TINT32 adwIdList[EN_SKILL_TYPE__END];
    TINT32 adwPointList[EN_SKILL_TYPE__END];
    //key0=Id1:Lv1,Id2:Lv2
    const TCHAR* pSkillInfo = pstSession->m_stReqParam.m_szKey[0];
    //处理参数
    while (pSkillInfo && *pSkillInfo)
    {
        adwIdList[udwCount] = atoi(pSkillInfo);
        pSkillInfo = strchr(pSkillInfo, ucPair);
        if (pSkillInfo != NULL)
        {
            pSkillInfo++;
            adwPointList[udwCount] = atoi(pSkillInfo);
            udwCount++;
            if (udwCount == EN_SKILL_TYPE__END)
            {
                break;
            }
        }
        else
        {
            break;
        }
        pSkillInfo = strchr(pSkillInfo, ucEntry);
        if (pSkillInfo != NULL)
        {
            pSkillInfo++;
        }
    }
    if (udwCount == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    //reset skill
    pstUser->m_tbUserStat.m_bLord_skill[0].Reset();
    for (TUINT32 udwIdx = 0; udwIdx < udwCount; udwIdx++)
    {
        dwSkillId = adwIdList[udwIdx];
        dwPoint = adwPointList[udwIdx];
        if (dwSkillId >= EN_SKILL_TYPE__END || dwSkillId < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }
        if (dwPoint > CPlayerBase::GetLordSkillPointLimit(dwSkillId))
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill point over max [seq=%u]", pstSession->m_udwSeqNo));
            return 0;
        }
        ptbUserStat->m_bLord_skill[0].m_addwLevel[dwSkillId] = dwPoint;
        dwTotalPoint += dwPoint;
    }
    //技能依赖
    for (TUINT32 udwIdx = 0; udwIdx < udwCount; udwIdx++)
    {
        dwSkillId = adwIdList[udwIdx];
        dwPoint = adwPointList[udwIdx];
        if (!CPlayerBase::IsMeetLordSkillReliance(pstUser, dwSkillId, dwPoint))
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill rely not meet [seq=%u]", pstSession->m_udwSeqNo));
            return 0;
        }
    }
    if (dwTotalPoint > pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT].m_ddwBuffTotal)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }
    ptbUserStat->SetFlag(TbUSER_STAT_FIELD_LORD_SKILL);
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonSkillUpgradeNew(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbUser_stat* ptbUserStat = &pstUser->m_tbUserStat;

    TUINT32 udwCount = 0;
    TINT32 dwSkillId = 0;
    TINT32 dwPoint = 0;
    TINT32 dwTotalPoint = 0;
    const TCHAR ucPair = ':';
    const TCHAR ucEntry = ',';
    TINT32 adwIdList[EN_SKILL_TYPE__END];
    TINT32 adwPointList[EN_SKILL_TYPE__END];
    //key0=Id1:Lv1,Id2:Lv2
    const TCHAR* pSkillInfo = pstSession->m_stReqParam.m_szKey[0];
    //处理参数
    while (pSkillInfo && *pSkillInfo)
    {
        adwIdList[udwCount] = atoi(pSkillInfo);
        pSkillInfo = strchr(pSkillInfo, ucPair);
        if (pSkillInfo != NULL)
        {
            pSkillInfo++;
            adwPointList[udwCount] = atoi(pSkillInfo);
            udwCount++;
            if (udwCount == EN_SKILL_TYPE__END)
            {
                break;
            }
        }
        else
        {
            break;
        }
        pSkillInfo = strchr(pSkillInfo, ucEntry);
        if (pSkillInfo != NULL)
        {
            pSkillInfo++;
        }
    }
    if (udwCount == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if (pstUser->m_tbPlayer.m_nHas_dragon == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    //reset skill
    pstUser->m_tbUserStat.m_bDragon_skill[0].Reset();
    for (TUINT32 udwIdx = 0; udwIdx < udwCount; udwIdx++)
    {
        dwSkillId = adwIdList[udwIdx];
        dwPoint = adwPointList[udwIdx];
        if (dwSkillId >= EN_SKILL_TYPE__END || dwSkillId < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }
        if (dwPoint > CPlayerBase::GetDragonSkillPointLimit(dwSkillId))
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill point over max [seq=%u]", pstSession->m_udwSeqNo));
            return 0;
        }
        ptbUserStat->m_bDragon_skill[0].m_addwLevel[dwSkillId] = dwPoint;
        dwTotalPoint += dwPoint;
    }
    //技能依赖
    for (TUINT32 udwIdx = 0; udwIdx < udwCount; udwIdx++)
    {
        dwSkillId = adwIdList[udwIdx];
        dwPoint = adwPointList[udwIdx];
        if (!CPlayerBase::IsMeetDragonMonsterSkillReliance(pstUser, dwSkillId, dwPoint))
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill rely not meet [seq=%u]", pstSession->m_udwSeqNo));
            return 0;
        }
    }
    if (dwTotalPoint > pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT].m_ddwBuffTotal)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }
    ptbUserStat->SetFlag(TbUSER_STAT_FIELD_DRAGON_SKILL);

    BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH_STAGE_5_ASSIGN_HERO_SKILL);
    pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);
    return 0;
}

TINT32 CProcessPlayer::ProcessCmd_DragonMonsterSkillUpgradeNew(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbUser_stat* ptbUserStat = &pstUser->m_tbUserStat;

    TUINT32 udwCount = 0;
    TINT32 dwSkillId = 0;
    TINT32 dwPoint = 0;
    TINT32 dwTotalPoint = 0;
    const TCHAR ucPair = ':';
    const TCHAR ucEntry = ',';
    TINT32 adwIdList[EN_SKILL_TYPE__END];
    TINT32 adwPointList[EN_SKILL_TYPE__END];
    //key0=Id1:Lv1,Id2:Lv2
    const TCHAR* pSkillInfo = pstSession->m_stReqParam.m_szKey[0];
    //处理参数
    while (pSkillInfo && *pSkillInfo)
    {
        adwIdList[udwCount] = atoi(pSkillInfo);
        pSkillInfo = strchr(pSkillInfo, ucPair);
        if (pSkillInfo != NULL)
        {
            pSkillInfo++;
            adwPointList[udwCount] = atoi(pSkillInfo);
            udwCount++;
            if (udwCount == EN_SKILL_TYPE__END)
            {
                break;
            }
        }
        else
        {
            break;
        }
        pSkillInfo = strchr(pSkillInfo, ucEntry);
        if (pSkillInfo != NULL)
        {
            pSkillInfo++;
        }
    }
    if (udwCount == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if (pstUser->m_tbPlayer.m_nHas_dragon == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    //reset skill
    pstUser->m_tbUserStat.m_bDragon_monster_skill[0].Reset();
    for (TUINT32 udwIdx = 0; udwIdx < udwCount; udwIdx++)
    {
        dwSkillId = adwIdList[udwIdx];
        dwPoint = adwPointList[udwIdx];
        if (dwSkillId >= EN_SKILL_TYPE__END || dwSkillId < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }
        if (dwPoint > CPlayerBase::GetDragonMonsterSkillPointLimit(dwSkillId))
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill point over max [seq=%u]", pstSession->m_udwSeqNo));
            return 0;
        }
        ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[dwSkillId] = dwPoint;
        dwTotalPoint += dwPoint;
    }
    //技能依赖
    for (TUINT32 udwIdx = 0; udwIdx < udwCount; udwIdx++)
    {
        dwSkillId = adwIdList[udwIdx];
        dwPoint = adwPointList[udwIdx];
        if (!CPlayerBase::IsMeetDragonMonsterSkillReliance(pstUser, dwSkillId, dwPoint))
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_SkillUpgrade: skill rely not meet [seq=%u]", pstSession->m_udwSeqNo));
            return 0;
        }
    }
    if (dwTotalPoint > pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT].m_ddwBuffTotal)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }
    ptbUserStat->SetFlag(TbUSER_STAT_FIELD_DRAGON_MONSTER_SKILL);
    return 0;
}