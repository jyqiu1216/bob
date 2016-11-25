#ifndef _JSON_RESULT_H_
#define _JSON_RESULT_H_

#include <string>
#include "jsoncpp/json/json.h"
#include "session.h"
#include "client_netio.pb.h"
#include "jsoncpp_utils.h"

using namespace client_netio_protocol;

class CJsonResult
{
public:
    CJsonResult();
    ~CJsonResult();

    TUINT32 GetResultJsonLength() const;
    TCHAR* GenResult(SSession* pstSession);
    TVOID SetCompress(TBOOL bFilter);

private:
    TINT32 GenResHeader(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos);
    TINT32 GenResData(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos);
    TINT32 GenData(SSession* pstSession);

    TINT32 Compress(TUINT32 udwVersion, TUINT32 udwRawDataLen, TCHAR* szRawData);
    TINT32 JsonToKeyData(Json::Value& jsonData, string& strKey, TCHAR* pBeginPos, TCHAR* pEndPos);

    Json::FastWriter m_jsonWriter;
    Json::Value m_jsonResult;

    TCHAR* m_szDataBuffer;
    TCHAR* m_szCompressBuffer;
    TCHAR* m_szEncodeBuffer;
    TCHAR* m_szResultBuffer;
    TCHAR* m_szMD5Buffer;

    TBOOL m_bNeedCompress;

    TINT32 dwResultLength;

public:
    CJsoncppSeri m_jSeri;
    ClientResponse m_objPbResponse;
    TVOID GenResult_Pb(SSession* pstSession, TBOOL bNeedContent = TRUE);
};

#endif