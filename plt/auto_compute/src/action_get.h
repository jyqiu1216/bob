#ifndef _ACTION_GET_H_
#define _ACTION_GET_H_

#include "std_header.h"
#include "session.h"
#include "action_map.h"
#include "aws_req_info.h"
#include "aws_rsp_info.h"
#include "svr_list_conf.h"
#include "zkutil/zk_util.h"

#define MAX_ACTION_NUM_IN_ONE_REQUEST	(200)
#define MAX_REPORT_ID_SKIP				(1)


class CActionGet
{
public:
    // ��ʼ��
    TINT32 Init(CConf *poConf, CZkRegConf* poZkConf, CTaskQueue *pTaskQueue, CTseLogger *poServLog);
    // �߳���ں���
    static void *Start(void *pParam);
    void StopQuery();
    // ��ʱ����action��ȡ����
    TINT32 WorkRoutine();
private:
    // Do work
    TINT32 DoWork(const SActionTable& stActionTable);

    // ����action���󲢻�ȡ��Ӧ
    TINT32 GetActionList(const SActionTable& stActionTable, TUINT32 udwEndTime);
    // ����Action list
    TINT32 ProcessActionList(const SActionTable& stActionTable);
    TINT32 ProcessBuffActionList(const SActionTable& stActionTable);
    TINT32 ProcessAlActionList(const SActionTable& stActionTable);
    TINT32 ProcessMarchActionList(const SActionTable& stActionTable);

    // ������Ӧ
    TINT32 ParseAwsResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo *pstHsResInfo);

    // ���ν���
    TINT32 ReconnetActionSvr(TINT32 dwActionType);
    // ������Ӧ
    TINT32 ParseActionSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo *pstHsResInfo);

private:
    CTaskQueue*	m_pTaskQueue;
    CConf*		m_poConf;
    CTseLogger*	m_poLog;
    TBOOL m_bStopFlag;

    AwsReqInfo	m_stAwsReqNode;
    AwsRspInfo	m_stAwsResInfo;
    TUINT32		m_udwSeqno;
    TUINT32		m_udwActionSeq;

    SDownNode*	m_pstAwsNode;
    TINT32		m_dwSocket;
    TBOOL		m_bHsIsOk;

    SDownNode* m_pastNodeList[EN_ACTION_END];
    TINT32 m_adwSocket[EN_ACTION_END];

    TUINT32 m_udwReqType;
    CBaseProtocolPack m_oPack;
    CBaseProtocolUnpack m_oUnPack;
    TUCHAR		m_pucBuf[MAX_NETIO_PACKAGE_BUF_LEN];
    TUINT32		m_udwBufLen;

    TUINT32		m_udwResActionNum;
    TbAction	m_astAction[MAX_ACTION_NUM_IN_ONE_REQUEST];
    TbAlliance_action m_astAlAction[MAX_ACTION_NUM_IN_ONE_REQUEST];
    TUINT32 m_udwAlActionNum;
    TbMarch_action m_astMarch[MAX_ACTION_NUM_IN_ONE_REQUEST];
    TUINT32 m_udwMarchNum;

    CZkRegConf* m_poZkConf;
};

#endif
