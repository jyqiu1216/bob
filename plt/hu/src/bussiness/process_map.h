#ifndef _PROCESS_MAP_H_
#define _PROCESS_MAP_H_


#include "session.h"


class CProcessMap
{

public: 

    // function  ===> ��ȡ��ͼ����(�Ե�ͼ��posΪ���ٵ�λ)
    static TINT32 ProcessCmd_MapGet(SSession *pstSession, TBOOL &bNeedResponse);
    
    // function  ===> ��ȡ��ͼ����(�Ե�ͼ��blockΪ���ٵ�λ)(todo: �����Ի���ô)
    static TINT32 ProcessCmd_MapBlockGet(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ManorInfo(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_PrisionInfo(SSession* pstSession, TBOOL& bNeedResponse);
    
};



#endif
