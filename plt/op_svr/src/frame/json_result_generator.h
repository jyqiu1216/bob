#ifndef _JSON_RESULT_GENERATOR_H_
#define _JSON_RESULT_GENERATOR_H_

#include <string>
#include "jsoncpp/json/json.h"
#include "session.h"
#include "base_json.h"
#include "client_netio.pb.h"
#include "jsoncpp_utils.h"
using namespace client_netio_protocol;


class CJsonResultGenerator
{
public:
    CJsonResultGenerator();
    ~CJsonResultGenerator();

    TUINT32 GetResultJsonLength() const;
    TCHAR* GenResult(SSession* pstSession);
    TCHAR* GenNobodyResult(SSession* pstSession);
    TVOID SetCompress(TBOOL bFilter);

    
    TBOOL IsNeedCompare(SSession* pstSession);

public:
    TINT32 GenResHeader(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos);
    TINT32 GenResData(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos);
    TINT32 GenData(SSession* pstSession);
    TINT32 GenPromoteData(SSession* pstSession);

    TINT32 Compress(TUINT32 udwVersion, TUINT32 udwRawDataLen, TCHAR* szRawData);
    TINT32 CompressZip(TUINT32 udwRawDataLen, TCHAR* szRawData);
    TINT32 JsonToKeyData(Json::Value& jsonData, string& strKey, TCHAR* pBeginPos, TCHAR* pEndPos);
    TINT32 JsonToKeyDataNew(Json::Value& jsonData, string& strKey, TCHAR* pBeginPos, TCHAR* pEndPos, TUINT64& uddwRawMd5, TINT32 dwOutputType);

    Json::FastWriter m_jsonWriter;
    Json::Value m_jsonResult;

    CJsoncppSeri m_jSeri;

    TCHAR* m_szDataBuffer;
    TCHAR* m_szCompressBuffer;
    TCHAR* m_szEncodeBuffer;
    TCHAR* m_szResultBuffer;
    TCHAR* m_szMD5Buffer;

    TBOOL m_bNeedCompress;

    CBaseJson* m_apJsonGen[EN_JSON_TYPE_END];

    TINT32 dwResultLength;

public:
    ClientResponse m_objPbResponse;
    TVOID GenResult_Pb(SSession* pstSession, TBOOL bNeedContent = TRUE);

    TVOID GenPushData_Pb(SSession *pstSession, TUINT16 uwServiceType, Json::Value& rJson, TINT32 dwSid = -1);
    TVOID GenEmptyTableFroClientUpdt_Pb(const TCHAR *pszTableName);
};

#endif // !_JSON_RESULT_GENERATOR_H_