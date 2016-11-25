#ifndef _JSON_RESULT_GENERATOR_H_
#define _JSON_RESULT_GENERATOR_H_

#include <string>
#include "jsoncpp/json/json.h"
#include "session.h"
#include "client_netio.pb.h"
#include "jsoncpp_utils.h"

using namespace client_netio_protocol;


class CJsonResultGenerator
{
public:
    CJsonResultGenerator();
    ~CJsonResultGenerator();


    Json::FastWriter m_jsonWriter;
    Json::Value m_jsonResult;

    CJsoncppSeri m_jSeri;

    TCHAR* m_szDataBuffer;
    TCHAR* m_szCompressBuffer;
    TCHAR* m_szEncodeBuffer;
    TCHAR* m_szResultBuffer;
    TCHAR* m_szMD5Buffer;

    TBOOL m_bNeedCompress;

    TINT32 dwResultLength;

public:
    ClientResponse m_objPbResponse;

    TVOID GenPushData_Pb(SSession *pstSession, TUINT16 uwServiceType, Json::Value& rJson);
    TVOID GenEmptyTableFroClientUpdt_Pb(const TCHAR *pszTableName);
};

#endif // !_JSON_RESULT_GENERATOR_H_