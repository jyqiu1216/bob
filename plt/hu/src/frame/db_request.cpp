#include "db_request.h"
#include "session.h"
#include "common_func.h"
#include "db_struct_define.h"

int CDbRequest::Select(SSession *pstSession, const string& sSql, const string& sTableName, const string& sOperName)
{
	DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, sTableName, sOperName, sSql);
	pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}


int CDbRequest::UpdateAllianceRank(SSession *pstSession, AllianceRank* pAllianceRank)
{
    if(pAllianceRank->sDesc.size() > 255)
    {
        pAllianceRank->sDesc.resize(255);
    }
    char szDesc[1024] = {0};
    CCommonFunc::SqlEncode(pAllianceRank->sDesc.c_str(), szDesc, sizeof(szDesc));
    ostringstream oss;
    oss << "replace into alliance_rank(aid,alname,oid,oname,might,member,policy,descript,al_language,rank0,rank1,rank2,rank3,rank4,rank5,rank6,rank7,rank8,rank9,rank10,rank11,rank12,rank13,"
        << "value0,value1,value2,value3,value4,value5,value6,value7,value8,value9,value10,value11,value12,value13,alnick,new_player_al_flag) values('" << pAllianceRank->udwAid
        << "','" << pAllianceRank->sAlName << "','" << pAllianceRank->udwOid << "','" << pAllianceRank->sOname << "','"
        << pAllianceRank->ddwMight << "','" << pAllianceRank->udwMember << "','" << pAllianceRank->udwPolicy << "','" << szDesc;
    oss << "','" << pAllianceRank->udwLanguage;
    for(unsigned int i = EN_ALLIANCE_RANK_FLD_RANK0; i <= EN_ALLIANCE_RANK_FLD_RANK13; ++i)
    {
        oss << "','" << pAllianceRank->audwRank[i - EN_ALLIANCE_RANK_FLD_RANK0];
    }
    for(unsigned int i = EN_ALLIANCE_RANK_FLD_VALUE0; i <= EN_ALLIANCE_RANK_FLD_VALUE13; ++i)
    {
        oss << "','" << pAllianceRank->addwValue[i - EN_ALLIANCE_RANK_FLD_VALUE0];
    }
    oss << "','" << pAllianceRank->sAlNick;
    oss << "','" << pAllianceRank->ddwNewPlayerAlFlag;
    oss << "')";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "alliance_rank", "replace");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;

}

int CDbRequest::UpdateAlliancePolicy(SSession *pstSession, AllianceRank* pAllianceRank)
{
    ostringstream oss;
    oss << "update alliance_rank set policy = '" << pAllianceRank->udwPolicy << "'"
        << " where aid = '" << pAllianceRank->udwAid << "'";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "alliance_rank", "replace_policy");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::UpdateAllianceName(SSession *pstSession, AllianceRank* pAllianceRank)
{
    ostringstream oss;
    oss << "update alliance_rank set alname = '" << pAllianceRank->sAlName << "'"
        << " where aid = '" << pAllianceRank->udwAid << "'";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "alliance_rank", "replace_alname");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::UpdateAllianceNick(SSession *pstSession, AllianceRank* pAllianceRank)
{
    ostringstream oss;
    oss << "update alliance_rank set alnick = '" << pAllianceRank->sAlNick << "'"
        << " where aid = '" << pAllianceRank->udwAid << "'";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "alliance_rank", "replace_alnick");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::DeleteAllianceRank(SSession *pstSession)
{
    ostringstream oss;
    oss << "delete from alliance_rank where aid=" << pstSession->m_stReqParam.m_udwAllianceId;
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "alliance_rank", "delete");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::SelectAllianceRank(SSession *pstSession, TUINT32 udwAid)
{
    ostringstream oss;
    oss << "select * from alliance_rank where aid=" << udwAid;
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "alliance_rank", "select");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::DeletePlayerRank(SSession *pstSession)
{
    ostringstream oss;
    oss << "replace into player_move values(" << pstSession->m_stUserInfo.m_tbPlayer.m_nUid << ")";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_move", "insert");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::UpdatePlayerName(SSession *pstSession, const string& sName)
{
    ostringstream oss;
    oss << "update player_rank set uname = '" << sName << "'"
        << " where uid = '" << pstSession->m_stUserInfo.m_tbPlayer.m_nUid << "'";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_rank", "replace_uname");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::SelectRecommendCount(SSession *pstSession, TUINT32 udwAid)
{
    ostringstream oss;
    oss << "select count(*) from player_recommend where aid=" << udwAid;
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_recommend", "count");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::DeletePlayerRecommend(SSession* pstSession, TUINT32 udwAid, TINT32 dwUid)
{
    ostringstream oss;
    oss << "delete from player_recommend where uid='" << dwUid << "'";
    //oss << "delete from player_recommend where aid='" << udwAid;
    //oss << "' and uid='" << dwUid << "'";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_recommend", "delete");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::DeleteAlPlayerRecommend(SSession* pstSession, TUINT32 udwAid, TINT32 dwUid)
{
    ostringstream oss;
    oss << "delete from player_recommend where aid='" << udwAid << "'";
    //oss << "delete from player_recommend where aid='" << udwAid;
    //oss << "' and uid='" << dwUid << "'";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_recommend", "delete_al");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::UpdatePlayerRecommendToInvited(SSession* pstSession, TUINT32 udwAid, TINT32 dwUid)
{
    ostringstream oss;
    oss << "update player_recommend set invited=1" 
        << " where uid=" << dwUid << " and aid=" << udwAid;
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_recommend", "update");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::SelectRecommendTime(SSession *pstSession)
{
    ostringstream oss;
    oss << "select * from compute_time where type=" << EN_COMPUTE_TIME_RECOMMEND;
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "compute_time", "select");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::SelectRecommendPlayer(SSession *pstSession, TUINT32 udwAid)
{
    ostringstream oss;
    oss << "select * from player_recommend where aid=" << udwAid << " order by 100*invited+hero_lv limit 10";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_recommend", "select");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::AddRecommendPlayer(SSession *pstSession, TUINT32 udwAid, TbPlayer* ptbPlayer)
{
    ostringstream oss;
    oss << "replace into player_recommend values(" << udwAid << ","
        << ptbPlayer->m_nSid << ","
        << ptbPlayer->m_nUid << ","
        << "'" << ptbPlayer->m_sUin << "',"
        << ptbPlayer->m_nAvatar << ","
        << ptbPlayer->m_nLevel << ","
        << 0 << ","
        << ptbPlayer->m_nMight << ","
        << 0 << ")";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "player_recommend", "add");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::UpdateRecommendTime(SSession *pstSession, TUINT32 udwTime)
{
    ostringstream oss;
    oss << "replace into compute_time values('" << EN_COMPUTE_TIME_RECOMMEND << "','" << udwTime << "')";
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "compute_time", "update");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::SelectTradeCityInfoInRange(SSession *pstSession, TINT32 dwSid, TINT32 dwX, TINT32 dwY, TINT32 dwX1, TINT32 dwX2, TINT32 dwY1, TINT32 dwY2, TUINT32 udwNum)
{
    ostringstream oss;
    oss << "select * from trade_city_list where sid= " << dwSid
        << " and (x >= " << dwX1 << " and x <= " << dwX2
        << " and y >= " << dwY1 << " and y <= " << dwY2
        << ") and not (x >= " << dwX - 200 << " and x <= " << dwX + 200
        << " and y >= " << dwY - 200 << " and y <= " << dwY + 200
        << ") order by rand() limit " << udwNum;
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "trade_city_list", "select_in_range");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}

int CDbRequest::SelectTradeCityInfoInRandom(SSession *pstSession, TINT32 dwSid, TUINT32 udwNum)
{
    ostringstream oss;
    oss << "select * from trade_city_list where sid= " << dwSid
        << " order by rand() limit " << udwNum;
    DbReqInfo* pDbReqInfo = new DbReqInfo(pstSession->m_stReqParam.m_udwSvrId, "trade_city_list", "select_in_random");
    pDbReqInfo->sReqContent = oss.str();
    pstSession->m_vecDbReq.push_back(pDbReqInfo);
    return 0;
}