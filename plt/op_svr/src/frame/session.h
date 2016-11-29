#ifndef _SSC_SESSION_H_
#define _SSC_SESSION_H_

#include "queue_t.h"
#include "base/common/wtsetypedef.h"
#include "down_mgr.h"
#include "my_define.h"
#include "service_key.h"
#include <vector>


using namespace std;

#pragma pack(1)

// ����
enum EProcedure
{
    // ������ʼ״̬
    EN_PROCEDURE__INIT = 0,

    // ���̽������
    EN_PROCEDURE__END,  // 80
    EN_PROCEDURE__SEND_RESULT_BACK,
};


// �����
enum ECommandStep
{
    EN_COMMAND_STEP__INIT = 0,

    // �����ֲ���
    EN_COMMAND_STEP__1,
    EN_COMMAND_STEP__2,
    EN_COMMAND_STEP__3,
    EN_COMMAND_STEP__4,
    EN_COMMAND_STEP__5,
    EN_COMMAND_STEP__6,
    EN_COMMAND_STEP__7,
    EN_COMMAND_STEP__8,
    EN_COMMAND_STEP__9,
    EN_COMMAND_STEP__10,
    EN_COMMAND_STEP__11,

    EN_COMMAND_STEP__END = 9999,

};


enum EExpectProcedure
{
    EN_EXPECT_PROCEDURE__INIT = 0,
    EN_EXPECT_PROCEDURE__LOCK_GET,
    EN_EXPECT_PROCEDURE__LOCK_RELEASE,
    EN_EXPECT_PROCEDURE__AWS,
    EN_EXPECT_PROCEDURE__DB,
    EN_EXPECT_PROCEDURE__EVENT = 10,
    EN_EXPECT_PROCEDURE__CACHE = 11,
    EN_EXPECT_PROCEDURE__TRANSLATE = 12,
    EN_EXPECT_PROCEDURE__DATA_CENTER = 13,
};


struct SCommonResInfo
{
    TINT32              m_dwRetCode;
    TUINT64             m_uddwCostTime;
    TUINT8              m_ucJsonType;
    TBOOL               m_bRefreshCode;//TRUE ��ʾ������ȷ  FALSE ��ʾ���ݴ���

    void Reset()
    {
        m_dwRetCode = 0;
        m_uddwCostTime = 0;
        m_ucJsonType = EN_JSON_TYPE_USER_JSON;
        m_bRefreshCode = FALSE;
    }
};

struct SSession
{
    /********************************************************************************************
                        �ⲿ������Ϣ�����к���Ϣ
                        *********************************************************************************************/
    TUINT32             m_udwSeqNo;
    LongConnHandle      m_stClientHandle;
    TUINT32             m_udwRequestType;

    /********************************************************************************************
                        session����״̬
                        *********************************************************************************************/
    TUINT8              m_ucIsUsing;
    TBOOL               m_bProcessing;

    /********************************************************************************************
                        ��ǰʹ�õ����νڵ�
                        *********************************************************************************************/
    SDownNode            *m_pstAwsProxyNode;
    TBOOL               m_bAwsProxyNodeExist;

    /********************************************************************************************
                        ������Ϣ�ڴ�
                        *********************************************************************************************/
    TUCHAR              m_szClientReqBuf[MAX_HTTP_REQ_LEN];					// client����Buf
    TUINT32             m_udwClientReqBufLen;								// client����Buf����

    /********************************************************************************************
                        �ڲ�������Ϣ(session�м䱣����Ϣ)
                        *********************************************************************************************/
    //

    /********************************************************************************************
                        ʱ�����------�����������ν����ĵط�������Ҫ��¼��ʱ
                        *********************************************************************************************/
    TUINT64             m_uddwTimeBeg;     // �յ������ʱ��
    TUINT64             m_uddwTimeEnd;     // ����ǰ�˻�Ӧ��ʱ�� 

    TUINT64             m_uddwTimeBeg_Process; //�����̻߳�ȡ����ʱ��


    void Reset()
    {
        m_udwSeqNo = 0;
        memset((char*)&m_stClientHandle, 0, sizeof(LongConnHandle));
        m_udwRequestType = 0;


        m_ucIsUsing = 0;
        m_bProcessing = FALSE;


        m_bAwsProxyNodeExist = FALSE;
        m_pstAwsProxyNode = NULL;


        m_szClientReqBuf[0] = 0;
        m_udwClientReqBufLen = 0;


        m_uddwTimeBeg = 0;
        m_uddwTimeEnd = 0;
    }
};

#pragma pack()

typedef CQueueT<SSession *> CTaskQueue;

#endif
