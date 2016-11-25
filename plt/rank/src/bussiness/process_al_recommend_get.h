#ifndef _PROCESS_AL_RECOMMEND_GET_H_
#define _PROCESS_AL_RECOMMEND_GET_H_

#include "session.h"

class CProcessAlRecommendGet
{
public:
    static TINT32 requestHandler(SSession *pstSession);

private:
    static TVOID NewWrapAlRecommendJson(SSession *pstSession);

    static TBOOL AllianceGreater(const TbAlliance* stA, const TbAlliance* stB);

};



#endif
