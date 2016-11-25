#ifndef _PROCESS_BOOKMARK_H_
#define _PROCESS_BOOKMARK_H_


#include "session.h"



class CProcessBookmark
{
public: 

    // function  ==> 获取bookmark信息
    static TINT32 ProcessCmd_BookmarkGet(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ==> 增加bookmark信息
    static TINT32 ProcessCmd_BookmarkAdd(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ==> 更新bookmark信息
    static TINT32 ProcessCmd_BookmarkUpdate(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ==> 删除bookmark信息
    static TINT32 ProcessCmd_BookmarkDelete(SSession *pstSession, TBOOL &bNeedResponse);



    
};



#endif
