#ifndef _EVENT_REQ_INFO_H_
#define _EVENT_REQ_INFO_H_

#include <string>
#include "bussiness_struct.h"

using namespace std;

enum ERequestType
{
    EN_REQUEST_TYPE__UPDATE = 1,
    EN_REQUEST_TYPE__GET_INFO = 2,
    EN_REQUEST_TYPE__GET_ALL_INFO = 3,
    EN_REQUEST_TYPE__GEN_HISTORY_INFO = 4,
};

struct EventReqInfo
{
    unsigned int m_udwIdxNo; //打开方式,用于区分主动和被动action
	string m_sReqContent;
    TUINT32 m_udwRequestType;
    TUINT32 m_udwIsTheme;

    EventReqInfo(unsigned int udwIdx = 0, const string& sReq = "") :m_udwIdxNo(udwIdx), m_sReqContent(sReq)
    {
       
    }

    void SetVal(TUINT32 udwSid, TUINT32 udwUid, const string &sUname, TUINT64 uddwMight, TUINT64 uddwAlid,
        const string &sAlName, TUINT64 uddwAlMight, const string &sAlNickName, TUINT32 udwAlPos, 
        TUINT32 udwGemBuy, TUINT32 udwMaxBuy, TUINT32 udwLastBuy, TUINT32 udwLastBuyTime, TUINT32 udwTotalPay, TUINT32 udwMaxPay, TUINT32 udwLastPay,
        TUINT32 udwAlGiftLv, TUINT32 udwCTime,TUINT32 udwCastleLv, TUINT32 udwTrialLv, 
        TUINT32 udwReqType, TUINT64 uddwEventSeq = 0,
        TUINT32 udwScoreType = 0, TUINT32 udwScore = 0,TUINT32 udwScoreId = 0,TUINT32 udwIndex = 0,TUINT32 udwEventId = 0)
    {
        Json::Value rEventJson = Json::Value(Json::objectValue);

        rEventJson["sid"] = udwSid;
        rEventJson["uid"] = udwUid;
        rEventJson["uname"] = sUname;
        rEventJson["might"] = uddwMight;
        rEventJson["alid"] = uddwAlid;
        rEventJson["alname"] = sAlName;
        rEventJson["almight"] = uddwAlMight;
        rEventJson["request_type"] = udwReqType;
        rEventJson["score_type"] = udwScoreType;
        rEventJson["score"] = udwScore;
        rEventJson["score_id"] = udwScoreId;
        rEventJson["event_type"] = udwEventId;
        rEventJson["alnick"] = sAlNickName;
        rEventJson["al_pos"] = udwAlPos;
        rEventJson["al_gift_lv"] = udwAlGiftLv;
        rEventJson["ctime"] = udwCTime;
        rEventJson["castle_lv"] = udwCastleLv;
        rEventJson["event_seq"] = uddwEventSeq;
        rEventJson["gem_buy"] = udwGemBuy;
        rEventJson["max_buy"] = udwMaxBuy;
        rEventJson["last_buy"] = udwLastBuy;
        rEventJson["last_buy_time"] = udwLastBuyTime;
        rEventJson["total_pay"] = udwTotalPay;
        rEventJson["max_pay"] = udwMaxPay;
        rEventJson["last_pay"] = udwLastPay;
        rEventJson["trial_lv"] = udwTrialLv;

        Json::FastWriter rEventWriter;
        rEventWriter.omitEndingLineFeed();

        m_sReqContent = rEventWriter.write(rEventJson);
        m_udwIdxNo = udwIndex;
        m_udwRequestType = udwReqType;

    }

    void SetVal(TUINT32 udwSid, TUINT32 udwUid, const string &sUname, TUINT64 uddwAlid,
        TUINT32 udwScoreType, TUINT32 udwScore, TUINT32 udwScoreId, TUINT32 udwReqType, TUINT32 udwAlPos = 1, TUINT32 udwTrialLv = 0, TUINT32 udwIndex = 0)
    {
        Json::Value rEventJson = Json::Value(Json::objectValue);

        rEventJson["sid"] = udwSid;
        rEventJson["uid"] = udwUid;
        rEventJson["uname"] = sUname;
        rEventJson["alid"] = uddwAlid;
        rEventJson["score_type"] = udwScoreType;
        rEventJson["score"] = udwScore;
        rEventJson["score_id"] = udwScoreId;
        rEventJson["al_pos"] = udwAlPos;
        rEventJson["trial_lv"] = udwTrialLv;
        rEventJson["request_type"] = udwReqType;

        Json::FastWriter rEventWriter;
        rEventWriter.omitEndingLineFeed();

        m_sReqContent = rEventWriter.write(rEventJson);
        m_udwIdxNo = udwIndex;
        m_udwRequestType = udwReqType;
    }

    //获取主题活动
    void SetVal(TUINT32 udwSid, TUINT32 udwUid, TUINT32 udwAlid, const string &sUname, const string &sAlNickName, 
                TUINT32 udwTrialLv, TUINT32 udwIsTheme, TUINT32 udwReqType, TUINT32 udwIndex)
    {
        Json::Value rEventJson = Json::Value(Json::objectValue);
        rEventJson["sid"] = udwSid;
        rEventJson["uid"] = udwUid;
        rEventJson["uname"] = sUname;
        rEventJson["alid"] = udwAlid;
        rEventJson["alnick"] = sAlNickName;
        rEventJson["trial_lv"] = udwTrialLv;
        rEventJson["request_type"] = udwReqType;

        Json::FastWriter rEventWriter;
        rEventWriter.omitEndingLineFeed();

        m_sReqContent = rEventWriter.write(rEventJson);
        m_udwIdxNo = udwIndex;
        m_udwRequestType = udwReqType;
        m_udwIsTheme = udwIsTheme;
    }

    //获取主题活动历史
    void SetVal(TUINT32 udwSid, TUINT32 udwUid, TUINT32 udwIsTheme, TUINT32 udwReqType, TUINT32 udwIndex)
    {
        Json::Value rEventJson = Json::Value(Json::objectValue);
        rEventJson["sid"] = udwSid;
        rEventJson["uid"] = udwUid;
        rEventJson["request_type"] = udwReqType;

        Json::FastWriter rEventWriter;
        rEventWriter.omitEndingLineFeed();

        m_sReqContent = rEventWriter.write(rEventJson);
        m_udwIdxNo = udwIndex;
        m_udwRequestType = udwReqType;
        m_udwIsTheme = udwIsTheme;
    }

};

#endif
