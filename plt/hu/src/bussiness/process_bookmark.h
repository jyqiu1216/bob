#ifndef _PROCESS_BOOKMARK_H_
#define _PROCESS_BOOKMARK_H_


#include "session.h"



class CProcessBookmark
{
public: 

    // function  ==> ��ȡbookmark��Ϣ
    static TINT32 ProcessCmd_BookmarkGet(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ==> ����bookmark��Ϣ
    static TINT32 ProcessCmd_BookmarkAdd(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ==> ����bookmark��Ϣ
    static TINT32 ProcessCmd_BookmarkUpdate(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ==> ɾ��bookmark��Ϣ
    static TINT32 ProcessCmd_BookmarkDelete(SSession *pstSession, TBOOL &bNeedResponse);



    
};



#endif
