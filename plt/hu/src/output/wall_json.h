#ifndef _WALL_JSON_H_
#define _WALL_JSON_H_

#include "base_json.h"

class CWallJson : public CBaseJson
{
public:
    CWallJson();
    virtual ~CWallJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
private:
    static TBOOL TbAlComment_Compare(const TbAl_wall tbWall_A, const TbAl_wall tbWall_B);
};

#endif