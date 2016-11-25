#ifndef _AWS_TABLE_TASK_H_
#define _AWS_TABLE_TASK_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_TASK "task"

enum ETbTaskField
{
	TbTASK_FIELD_UID = 0,
	TbTASK_FIELD_TASK_REFRESH_TIME = 1,
	TbTASK_FIELD_TASK_FINISH = 2,
	TbTASK_FIELD_TASK_NORMAL = 3,
	TbTASK_FIELD_TASK_TIME = 4,
	TbTASK_FIELD_TASK_STATUS = 5,
	TbTASK_FIELD_TASK_OPEN_WINDOW_FLAG = 6,
	TbTASK_FIELD_TASK_CHECK_ID = 7,
	TbTASK_FIELD_RECOMMAND_TASK_ID = 8,
	TbTASK_FIELD_SEQ = 9,
};

enum ETbTaskOpenType
{
	ETbTASK_OPEN_TYPE_PRIMARY = 0,
};

#define	TBTASK_TASK_FINISH_MAX_NUM	1
typedef	BinBuf<STaskFinsh, TBTASK_TASK_FINISH_MAX_NUM>		TbTask_Task_finish;

#define	TBTASK_TASK_NORMAL_MAX_NUM	50
typedef	BinBuf<STaskNodeNow, TBTASK_TASK_NORMAL_MAX_NUM>		TbTask_Task_normal;

#define	TBTASK_TASK_TIME_MAX_NUM	5
typedef	BinBuf<STaskNodeNow, TBTASK_TASK_TIME_MAX_NUM>		TbTask_Task_time;

class TbTask : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nTask_refresh_time;
	TbTask_Task_finish m_bTask_finish;
	TbTask_Task_normal m_bTask_normal;
	TbTask_Task_time m_bTask_time;
	TINT64 m_nTask_status;
	TINT64 m_nTask_open_window_flag;
	TINT64 m_nTask_check_id;
	TINT64 m_nRecommand_task_id;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbTask():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbTask()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nTask_refresh_time = 0;
		m_bTask_finish.Reset();
		m_bTask_normal.Reset();
		m_bTask_time.Reset();
		m_nTask_status = 0;
		m_nTask_open_window_flag = 0;
		m_nTask_check_id = 0;
		m_nRecommand_task_id = 0;
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid)
	{
		m_nUid = nUid;
	}
	const TINT64& Get_Task_refresh_time()
	{
		return m_nTask_refresh_time;
	}
	void Set_Task_refresh_time(const TINT64& nTask_refresh_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTask_refresh_time = nTask_refresh_time;
		m_mFlag[TbTASK_FIELD_TASK_REFRESH_TIME] = dwActionType;
	}
	const TINT64& Get_Task_status()
	{
		return m_nTask_status;
	}
	void Set_Task_status(const TINT64& nTask_status, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTask_status = nTask_status;
		m_mFlag[TbTASK_FIELD_TASK_STATUS] = dwActionType;
	}
	const TINT64& Get_Task_open_window_flag()
	{
		return m_nTask_open_window_flag;
	}
	void Set_Task_open_window_flag(const TINT64& nTask_open_window_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTask_open_window_flag = nTask_open_window_flag;
		m_mFlag[TbTASK_FIELD_TASK_OPEN_WINDOW_FLAG] = dwActionType;
	}
	const TINT64& Get_Task_check_id()
	{
		return m_nTask_check_id;
	}
	void Set_Task_check_id(const TINT64& nTask_check_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTask_check_id = nTask_check_id;
		m_mFlag[TbTASK_FIELD_TASK_CHECK_ID] = dwActionType;
	}
	const TINT64& Get_Recommand_task_id()
	{
		return m_nRecommand_task_id;
	}
	void Set_Recommand_task_id(const TINT64& nRecommand_task_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRecommand_task_id = nRecommand_task_id;
		m_mFlag[TbTASK_FIELD_RECOMMAND_TASK_ID] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbTASK_FIELD_SEQ] = dwActionType;
	}
	//Scan命令使用,返回完整的请求包
	AwsMap* OnScanReq(unsigned int udwIdxNo, bool bHasStartKey = false, bool bReturnConsumedCapacity = true,
		unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0);
	int OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey = false, bool bReturnConsumedCapacity = true,
		unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0);
	int OnScanRsp(const Json::Value& item);
	//Query命令使用,返回完整的请求包
	AwsMap* OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0, bool bCount = false);
	int OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);
	int OnQueryRsp(const Json::Value& item);
	//查找记录数量
	int OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);	//UpdateItem命令使用,返回完整的请求包
	AwsMap* OnUpdateItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnUpdateItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnUpdateItemRsp(const Json::Value& item);
	//BatchWriteItem命令使用
	AwsMap* OnWriteItemReq(int dwActionType = WRITE_ACTION_TYPE_PUT);
	void OnWriteItemReq(AwsMap* pWriteItem, int dwActionType = WRITE_ACTION_TYPE_PUT);
	//BatchGetItem命令使用
	AwsMap* OnReadItemReq(unsigned int udwIdxNo);
	void OnReadItemReq(AwsMap*pReadItem);
	int OnReadItemRsp(const Json::Value& item);
	//DeleteItem命令使用,返回完整的请求包
	AwsMap* OnDeleteItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnDeleteItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnDeleteItemRsp(const Json::Value& item);
	//GetItem命令使用,返回完整的请求包
	AwsMap* OnGetItemReq(unsigned int udwIdxNo,
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true);
	int OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true);
	int OnGetItemRsp(const Json::Value& item);
	//PutItem命令使用,返回完整的请求包
	AwsMap* OnPutItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnPutItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnPutItemRsp(const Json::Value& item);
	int OnResponse(const Json::Value& item);
public: 
	static AwsTable* NewObject();
	string GetTableName();
	TINT32 GetTableIdx();
public: 
	TINT64 GetSeq();
};

#endif 

