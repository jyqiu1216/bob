#ifndef _WARNING_MGR_H_
#define _WARNING_MGR_H_

#include <string>
#include "base/common/wtsetypedef.h"
#include "base/log/wtselogger.h"
#include "game_define.h"

using namespace std;
using namespace wtse::log;

class CWarningMgr
{
// 私有内嵌类，程序结束时的自动释放
private:
    class CGarbo
    {
    public:
        ~CGarbo()
        {   
            if(CWarningMgr::m_poWarningMgr)
            {
                delete CWarningMgr::m_poWarningMgr;
                CWarningMgr::m_poWarningMgr = NULL;
            }
        }
    };
    static CGarbo Garbo; //定义一个静态成员变量，程序结束时，系统会自动调用它的析构函数 

// 单例服务的相关定义
private:   
    CWarningMgr()
    {}
    CWarningMgr(const CWarningMgr &);
    CWarningMgr & operator =(const CWarningMgr &);
    static CWarningMgr *m_poWarningMgr;                                 
        
// 获取服务的公共接口函数
public:
    ~CWarningMgr();
    
    // function  ===> 实例化 
    static CWarningMgr *GetInstance();                     
        
    // function  ===> 初始化cahce管理器
    TINT32 Init(CTseLogger *poLog);

    // 发送告警信息到agent
    TINT32 Send_Warning(string strWarningInfo);
    
// 内部使用的函数
private:
    static size_t WriteData(TVOID *buffer, size_t size, size_t nmemb, TVOID *userp)
    {        
        if (size * nmemb + strlen((char *)userp) >= MAX_JSON_LEN)
        {
            return 0;
        }

        memcpy((char *)userp + strlen((char *)userp), (char *)buffer, size * nmemb);
        *((char *)userp+strlen((char *)userp)+size * nmemb) = '\0';
        return size * nmemb;
    }

// 内部成员变量
private:
	CTseLogger *m_poServLog;

};

#endif

