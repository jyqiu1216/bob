#include "db_response.h"

int CDbResponse::OnCountResponse(const DbRspInfo& rspInfo)
{
    unsigned int udwNum = 0;
    unsigned int udwLen = 0;
    unsigned short uwFldNo = 0;
    unsigned int udwItemLen = 0;

    if (rspInfo.sRspContent.size() <= 10)
    {
        return -1;
    }
    memcpy(&udwLen, rspInfo.sRspContent.c_str(), 4);
    memcpy(&uwFldNo, rspInfo.sRspContent.c_str()+4, 2);
    if (uwFldNo != 0)
    {
        return -2;
    }
    memcpy(&udwItemLen, rspInfo.sRspContent.c_str()+6, 4);
    if (udwLen != udwItemLen+6)
    {
        return -3;
    }
    udwNum = atoi(rspInfo.sRspContent.c_str()+10);
    return udwNum;
}
