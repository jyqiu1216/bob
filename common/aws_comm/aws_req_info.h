#ifndef _AWS_REQ_INFO_H_
#define _AWS_REQ_INFO_H_
#include <string>
#include "aws_table_common.h"
using namespace std;

struct AwsReqInfo
{
	//AwsTable* pAwsTbl;
	string sTableName;
	string sOperatorName;
	unsigned int udwIdxNo; //打开方式,用于区分主动和被动action
	string sReqContent;
    int dwLock;
    int dwUnlock;
    bool bIsDefaultLock;

    TUINT32 m_udwCacheMode;
    string m_strRouteFld;
    TUINT64 m_uddwRouteThrd;

	string	m_strHashKey;
	string	m_strRangeKey;

	AwsReqInfo(AwsTable* pTbl, const string& sTable="", const string& sOperator="", unsigned int udwIdx = 0, const string& sReq=""):sTableName(sTable),sOperatorName(sOperator),udwIdxNo(udwIdx),sReqContent(sReq)
	{
        dwLock = 0;
        dwUnlock = 0;
        bIsDefaultLock = true;

        if (pTbl)
        {
            m_udwCacheMode = pTbl->pTableDesc->udwCacheMode;
            m_strRouteFld = pTbl->pTableDesc->mFieldDesc[pTbl->pTableDesc->udwRouteFld].sName;
            m_uddwRouteThrd = pTbl->pTableDesc->uddwRouteThrd;
			m_strHashKey = pTbl->pTableDesc->mFieldDesc[pTbl->pTableDesc->udwHashKeyFldNo].sName;
			if( -1 != (TINT32)pTbl->pTableDesc->udwRangeKeyFldNo )
				m_strRangeKey = pTbl->pTableDesc->mFieldDesc[pTbl->pTableDesc->udwRangeKeyFldNo].sName;
        }
        else
        {
            m_udwCacheMode = 0;
            m_strRouteFld = "";
            m_uddwRouteThrd = 0;
			m_strHashKey = "";
			m_strRangeKey= "";
        }
    }

    AwsReqInfo()
    {
        //pAwsTbl = NULL;
        sTableName.clear();
        sOperatorName.clear();
        udwIdxNo = 0;
        sReqContent.clear();
        dwLock = 0;
        dwUnlock = 0;
        bIsDefaultLock = true;
        m_udwCacheMode = 0;
        m_strRouteFld = "";
        m_uddwRouteThrd = 0;
		m_strHashKey = "";
		m_strRangeKey= "";
    }

    void SetVal(AwsTable* pTbl, const string sTable = "", const string sOperator = "", unsigned int udwIndex = 0, const string &sReq="")
    {
		//pAwsTbl = pTbl;
        sTableName = sTable;
        sOperatorName = sOperator;
        udwIdxNo = udwIndex;
        sReqContent = sReq;
        if (pTbl)
        {
            m_udwCacheMode = pTbl->pTableDesc->udwCacheMode;
            m_strRouteFld = pTbl->pTableDesc->mFieldDesc[pTbl->pTableDesc->udwRouteFld].sName;
            m_uddwRouteThrd = pTbl->pTableDesc->uddwRouteThrd;
			m_strHashKey = pTbl->pTableDesc->mFieldDesc[pTbl->pTableDesc->udwHashKeyFldNo].sName;
            if (-1 != (TINT32)pTbl->pTableDesc->udwRangeKeyFldNo)
				m_strRangeKey = pTbl->pTableDesc->mFieldDesc[pTbl->pTableDesc->udwRangeKeyFldNo].sName;
        }
        else
        {
            m_udwCacheMode = 0;
            m_strRouteFld = "";
            m_uddwRouteThrd = 0;
			m_strHashKey = "";
			m_strRangeKey= "";
        }
    }	
};

#endif
