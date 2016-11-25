#ifndef _MY_DEFINE_H_
#define _MY_DEFINE_H_

// common
#define MAX_IP_LEN					(64)	// 可能有多个ip
#define DEFAULT_IP_LEN				(16)
#define DEFAULT_PAGE_NUM			(1)
#define DEFAULT_PERPAGE_NUM			(20)
#define DEFAULT_EVENT_PERPAGE_NUM           (100)

#define MAX_PERPAGE_NUM				(1000)

// 分页限制
#define MAX_ALLIANCE_PERPAGE_NUM		(20)
#define MAX_MAIL_PERPAGE_NUM			(20)
#define MAX_REPORT_PERPAGE_NUM			(20)
#define MAX_PLAYER_PERPAGE_NUM			(20)
//可以帮助的任务列表
#define MAX_CAN_HELP_TASK_PERPAGE_NUM   (20)
//未操作过的iap gift数量
#define MAX_AL_IAP_GIFT_PERPAGE_NUM     (20)

//用户名、pwd等参数的通用长度
#define DEFAULT_PARAM_STR_LEN	(10240)
#define DEFAULT_NAME_STR_LEN	(128)
#define DEFAULT_SVR_LIST_STR_LEN	(512)

//请求中关键参数个数
#define	MAX_REQ_PARAM_KEY_NUM	(20)

//网络交互包的最大长度
#define MAX_HTTP_REQ_LEN				(30<<10)
#define MAX_NETIO_PACKAGE_BUF_LEN		(2048<<10)
#define MAX_CHAT_PACKAGE_BUF_LEN		(2<<10)
#define MAX_GLOBAL_HS_PACKAGE_BUF_LEN	(5<<10)
#define MAX_LOCAL_HS_PACKAGE_BUF_LEN	(5<<10)
#define MAX_GLOBAL_HS_TASK_NUM			(1)
#define MAX_LOCAL_HS_TASK_NUM			(200)
#define MAX_AWS_REQ_TASK_NUM            (200)
#define MAX_REQ_BUF_LEN					(1<<10)
#define MAX_AUCTION_RES_PACKAGE_BUF_LEN (5<<10)

//最大的长连接session数量
#define	MAX_NETIO_LONGCONN_SESSION_NUM	(1024)

//长连接设置的通用收发超时时间，单位ms
#define MAX_NETIO_LONGCONN_TIMEOUT_MS	(5000)

//mirror检索节点的超时时间
#define DOWN_SEARCH_NODE_TIMEOUT_MS	(10000)

// cache节点的超时时间
#define CACHE_VAILD_TIME_S         (10 * 60)

// lock超时时间设置
#define DOWN_LOCK_NODE_TIMEOUT_MS	(3000)

//初始化同步数据的超时时间
#define INIT_SYN_DATA_TIMEOUT_MS	(600000)

//最多的向HS下游请求的包个数
#define MAX_HS_REQ_NUM				(10)

//临时字符串长度
#define MAX_TMP_STR_LEN				(1<<10)

// 每次请求锁最大的key数量，必须和lock_svr中的设置保持一致
#define MAX_LOCK_KEY_NUM_IN_ONE_REQ		(10)

#define MAX_AWS_RES_DATA_LEN	(2000<<10)


// au性能统计的模块数
#define MAX_AU_PREFORMANCE_ANALYSIS_GET	(20)        // 拉取action
#define MAX_AU_PREFORMANCE_ANALYSIS_PROCESS	(100)    // 处理action


#endif
