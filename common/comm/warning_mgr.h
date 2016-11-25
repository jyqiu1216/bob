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
// ˽����Ƕ�࣬�������ʱ���Զ��ͷ�
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
    static CGarbo Garbo; //����һ����̬��Ա�������������ʱ��ϵͳ���Զ����������������� 

// �����������ض���
private:   
    CWarningMgr()
    {}
    CWarningMgr(const CWarningMgr &);
    CWarningMgr & operator =(const CWarningMgr &);
    static CWarningMgr *m_poWarningMgr;                                 
        
// ��ȡ����Ĺ����ӿں���
public:
    ~CWarningMgr();
    
    // function  ===> ʵ���� 
    static CWarningMgr *GetInstance();                     
        
    // function  ===> ��ʼ��cahce������
    TINT32 Init(CTseLogger *poLog);

    // ���͸澯��Ϣ��agent
    TINT32 Send_Warning(string strWarningInfo);
    
// �ڲ�ʹ�õĺ���
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

// �ڲ���Ա����
private:
	CTseLogger *m_poServLog;

};

#endif

