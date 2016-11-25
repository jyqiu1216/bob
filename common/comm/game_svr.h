#ifndef _GAME_SVR_H_
#define _GAME_SVR_H_

#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>

using namespace wtse::log;
using namespace std;

#define UPDATE_DATA_FLAG_FILE			("../data/update_data_flag")
#define CLIENT_VERSION_NEWEST_STR		("client_version_newest=")
#define CLIENT_VERSION_MIN_STR			("client_version_min=")
#define SERVER_UPDT_BEG_TIME			("svr_updt_beg_time=")
#define SERVER_UPDT_END_TIME			("svr_updt_end_time=")
#define UPDATE_BLACK_ACCOUNT_LIST_FLAG  ("../data/update_black_account_list_flag")
#define BLACK_ACCOUNT_LIST_FILE         ("../data/black.list")
#define SVR_MERGE_FILE                  ("../data/svr_merge.conf")

#define SVR_FULL_THRD					(0.95)
#define SVR_SUM_NEW_IN_RANK             (100)

enum SvrMergeStatus
{
    EN_SVR_MERGE_STATUS__NORMAL = 0,
    EN_SVR_MERGE_STATUS__PROTECTED,
    EN_SVR_MERGE_STATUS__DOING,
    EN_SVR_MERGE_STATUS__DONE,
};

struct SGameSvrInfo
{
	TUINT32 m_udwId;
	TCHAR	m_szName[MAX_TABLE_NAME_LEN];
	TCHAR	m_szLanguage[MAX_TABLE_NAME_LEN];
	TUINT32	m_udwOpenTime;
	TUINT32	m_dwStatus;

	TUINT32 m_udwPlainNum;		// 平原数目：表示空闲的plain总量，即无人占领的
	TUINT32 m_udwCityNum;		// 用户数量：表示城市总量
	TFLOAT32 m_fThrdRate;
    TUINT32 m_udwNewInChance;     // 新用户进入该服务器的概率
    TUINT32 m_udwSvrPos;          // server position
    TUINT32 m_udwSvrAvatar;       // server avatar

	TBOOL	m_bValidFlag;

    //wave@20160323
    TINT32  m_dwMergeStatus;        //SvrMergeStatus
    TINT32  m_dwMergeTargetSid;

	TVOID Reset()
	{
		memset((char*)this, 0, sizeof(*this));
        m_dwMergeTargetSid = -1;
	}
};

struct SProvinceInfo
{
	TUINT32 m_udwProbability;
	TUINT32 m_udwPlainNum;		// 平原数目：表示空闲的plain总量，即无人占领的
	TUINT32 m_udwCityNum;		// 用户数量：表示城市总量
};

class CGameSvrInfo
{
public:
	CGameSvrInfo();
	~CGameSvrInfo();
	static CGameSvrInfo* m_poGameSvrInfo;
	static CGameSvrInfo* GetInstance();
	static TINT32 Update(CTseLogger *poLog);

	TINT32 Init(const TCHAR *pszClientConf, const TCHAR *pszSvrUpdtConf, const TCHAR *pszGameSvrConf, CTseLogger *poLog);
public:
	// 客户端版本更新
	TFLOAT32 m_fNewestClientVersion;
	TFLOAT32 m_fMinClientVersion;
	TINT32 LoadClientVersion(const TCHAR *pszConfFile, CTseLogger *poLog);

public:
	// 后台维护参数更新
	TUINT32 m_udwSvrUpdtBegTime;
	TUINT32 m_udwSvrUpdtEndTime;
	TINT32 LoadSvrUpdtParam(const TCHAR *pszConfFile, CTseLogger *poLog);

public:
	// 每个svr中用户的最大数量、现有数量、阈值
	// 格式：id\tplain_num\tplayer_num\tthrd\n
	TUINT32 m_udwSvrNum;
	SGameSvrInfo m_astSvrInfo[MAX_GAME_SVR_NUM];
	TINT32 LoadSvrInfo(const TCHAR *pszConf, CTseLogger *poLog);

    // function ===> 获取全新用户创建用户时该导向的服号
	TUINT32 GetNewPlayerSvr();
    
	TBOOL	CheckSvrStatus(TUINT32 udwSvrId);
    TUINT32 GetSvrNewChanceInBySid(TUINT32 udwSvrId);
public:
	// 每个svr中每个省份用于分配新用户的概率、现有用户数量（含npc）、plain数（最大允许的可分配的数量）
	// 格式：province_id\tprobability\tplain_num\tplayer_num\n
	SProvinceInfo m_aastProvinceInfo[MAX_GAME_SVR_NUM][MAX_PROVINCE_NUM];
	TINT32 LoadProvinceInfo(const TCHAR *pszConf, TUINT32 udwSvrId, CTseLogger *poLog);
	TUINT32 GetNewPlayerProvince(TUINT32 udwSvrId, TUINT8 &ucProvince, TUINT32 &udwSkip);

public:
	// 黑名单
	set<TUINT32> m_objBlackList;
	TINT32 LoadBlackList(const TCHAR *pszFile);
    set<string> m_objBlackAccountListA;
    set<string> m_objBlackAccountListB;
    set<string>* m_pobjMasterBlackAccount;
    set<string>* m_pobjBufferBlackAccount;
    TINT32 LoadBlackAccountList(const TCHAR *pszFile, TBOOL bIsInit = FALSE);

public:
    // 加载空闲的plain列表——分svr分省
    TINT32 LoadPlainList(TUINT32 udwSvrId);
    // 根据省份获取空闲的plain以作建城使用
    TUINT32 GetEmptyPlain(TUINT32 udwSvrId, TUINT8 ucProvince);
    TCHAR* GetSvrNameBySid(TUINT32 udwSvrId);

public:
    TINT32 LoadSvrMergeInfo(const TCHAR *pszConf, CTseLogger *poLog);
    TINT32 GetTargeSid(TUINT32 udwRawSid);
    SGameSvrInfo* GetSidInfo(TUINT32 udwSid);
};

#endif


