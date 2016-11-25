#ifndef _MY_DEFINE_H_
#define _MY_DEFINE_H_

// common
#define MAX_IP_LEN					(64)	// �����ж��ip
#define DEFAULT_IP_LEN				(16)
#define DEFAULT_PAGE_NUM			(1)
#define DEFAULT_PERPAGE_NUM			(20)
#define DEFAULT_EVENT_PERPAGE_NUM           (100)

#define MAX_PERPAGE_NUM				(1000)

// ��ҳ����
#define MAX_ALLIANCE_PERPAGE_NUM		(20)
#define MAX_MAIL_PERPAGE_NUM			(20)
#define MAX_REPORT_PERPAGE_NUM			(20)
#define MAX_PLAYER_PERPAGE_NUM			(20)
//���԰����������б�
#define MAX_CAN_HELP_TASK_PERPAGE_NUM   (20)
//δ��������iap gift����
#define MAX_AL_IAP_GIFT_PERPAGE_NUM     (20)

//�û�����pwd�Ȳ�����ͨ�ó���
#define DEFAULT_PARAM_STR_LEN	(10240)
#define DEFAULT_NAME_STR_LEN	(128)
#define DEFAULT_SVR_LIST_STR_LEN	(512)

//�����йؼ���������
#define	MAX_REQ_PARAM_KEY_NUM	(20)

//���罻��������󳤶�
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

//���ĳ�����session����
#define	MAX_NETIO_LONGCONN_SESSION_NUM	(1024)

//���������õ�ͨ���շ���ʱʱ�䣬��λms
#define MAX_NETIO_LONGCONN_TIMEOUT_MS	(5000)

//mirror�����ڵ�ĳ�ʱʱ��
#define DOWN_SEARCH_NODE_TIMEOUT_MS	(10000)

// cache�ڵ�ĳ�ʱʱ��
#define CACHE_VAILD_TIME_S         (10 * 60)

// lock��ʱʱ������
#define DOWN_LOCK_NODE_TIMEOUT_MS	(3000)

//��ʼ��ͬ�����ݵĳ�ʱʱ��
#define INIT_SYN_DATA_TIMEOUT_MS	(600000)

//������HS��������İ�����
#define MAX_HS_REQ_NUM				(10)

//��ʱ�ַ�������
#define MAX_TMP_STR_LEN				(1<<10)

// ÿ������������key�����������lock_svr�е����ñ���һ��
#define MAX_LOCK_KEY_NUM_IN_ONE_REQ		(10)

#define MAX_AWS_RES_DATA_LEN	(2000<<10)


// au����ͳ�Ƶ�ģ����
#define MAX_AU_PREFORMANCE_ANALYSIS_GET	(20)        // ��ȡaction
#define MAX_AU_PREFORMANCE_ANALYSIS_PROCESS	(100)    // ����action


#endif
