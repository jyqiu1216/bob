#ifndef _BUFFER_INFO_JSON_H_
#define _BUFFER_INFO_JSON_H_

#include "json_result_generator.h"
class CBufferInfoJson : public CBaseJson
{
public:
	CBufferInfoJson();
	virtual ~CBufferInfoJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
};

#endif // !_REPORTLIST_JSON_H_
