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

// 流程
enum EProcedure
{
    // 流程起始状态
    EN_PROCEDURE__INIT = 0,

    // 流程结束标记
    EN_PROCEDURE__END,  // 80
    EN_PROCEDURE__SEND_RESULT_BACK,
};


// 命令步骤
enum ECommandStep
{
    EN_COMMAND_STEP__INIT = 0,

    // 命令字步骤
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
    TBOOL               m_bRefreshCode;//TRUE 表示数据正确  FALSE 表示数据错误

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
                        外部链接信息和序列号信息
                        *********************************************************************************************/
    TUINT32             m_udwSeqNo;
    LongConnHandle      m_stClientHandle;
    TUINT32             m_udwRequestType;

    /********************************************************************************************
                        session本身状态
                        *********************************************************************************************/
    TUINT8              m_ucIsUsing;
    TBOOL               m_bProcessing;

    /********************************************************************************************
                        当前使用的下游节点
                        *********************************************************************************************/
    SDownNode            *m_pstAwsProxyNode;
    TBOOL               m_bAwsProxyNodeExist;

    /********************************************************************************************
                        网络消息内存
                        *********************************************************************************************/
    TUCHAR              m_szClientReqBuf[MAX_HTTP_REQ_LEN];					// client请求Buf
    TUINT32             m_udwClientReqBufLen;								// client请求Buf长度

    /********************************************************************************************
                        内部保留信息(session中间保留信息)
                        *********************************************************************************************/
    //

    /********************************************************************************************
                        时间参数------凡是有上下游交互的地方，都需要记录耗时
                        *********************************************************************************************/
    TUINT64             m_uddwTimeBeg;     // 收到请求的时间
    TUINT64             m_uddwTimeEnd;     // 发送前端回应的时间 

    TUINT64             m_uddwTimeBeg_Process; //工作线程获取到的时间


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
