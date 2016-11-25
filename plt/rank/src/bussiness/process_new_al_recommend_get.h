#ifndef _PROCESS_NEW_AL_RECOMMEND_GET_H_
#define _PROCESS_NEW_AL_RECOMMEND_GET_H_

#include "session.h"

class CProcessNewAlRecommendGet
{
public:
    static TINT32 requestHandler(SSession *pstSession);

private:
    static const TINT32 k_max_al_recommand_people = 490;

    static TVOID NewWrapNewAlRecommendJson(SSession *pstSession);
};



#endif
