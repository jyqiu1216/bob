#include "hu_work.h"
#include "encode/src/encode/utf8_util.h"
#include "template_fun.h"
#include "zlib.h"
#include "util_base64.h"
#include "rc4.h"
#include "game_info.h"
#include "game_command.h"


using namespace base::encode;

#ifdef _DEBUG_
#define CHECK_POINTER_VALID(ptr, ret) {if(ptr == NULL){ ret--;assert(0);}}
#else
#define CHECK_POINTER_VALID(ptr, ret) {if(ptr == NULL){ ret--;break;}}
#endif

// request buffer should be end with NULL
// Request:[GET /?p1=123&p2=4567 HTTP/1.1]
int CHuWork::GetRequestUrl(const char *req_buf, char *url_buf)
{
    char *start_pos = NULL;
    char *end_pos = NULL;
    size_t url_len = 0;
    int ret_flag = -1;

    if((start_pos = strstr((char *)req_buf, "GET ")) != NULL)
    {
        start_pos += 4;
        if((end_pos = CStringUtils::strchr(start_pos, ' ')) != NULL)
        {
            url_len = end_pos - start_pos;
            if(url_len < kMaxHttpUrlLen)
            {
                memcpy(url_buf, start_pos, url_len);
                url_buf[url_len] = '\0';
                ret_flag = 0;
            }
            else if(url_len == 0)
            {
                ret_flag = -2;
            }
            else
            {
                ret_flag = -3;
            }
        }
    }

    return ret_flag;
}

int CHuWork::GetHttpParam(const char *url_buf, RequestParam *req_param)
{
    req_param->param_num = 0;

    // request should be /?p1=a&p2=&p3=c
    char *start_pos = CStringUtils::strchr((char *)url_buf, '?');
    char *split_pos = NULL;
    char *end_pos = NULL;
    if(start_pos == NULL || start_pos + 1 == NULL)
        return -1;

    size_t key_copy_len = 0;
    size_t value_copy_len = 0;
    unsigned int param_num = 0;
    start_pos++;

    do
    {
        split_pos = CStringUtils::strchr(start_pos, '=');
        end_pos = CStringUtils::strchr(start_pos, '&');

        if(split_pos == NULL && end_pos == NULL)
            key_copy_len = strlen(start_pos);
        else if(split_pos == NULL && end_pos != NULL)
            key_copy_len = end_pos - start_pos;
        else if(end_pos != NULL && split_pos > end_pos)
            key_copy_len = end_pos - start_pos;
        else
            key_copy_len = split_pos - start_pos;

        if(split_pos != NULL && end_pos == NULL)
            value_copy_len = strlen(split_pos + 1);
        else if(split_pos != NULL && end_pos != NULL && end_pos > split_pos)
            value_copy_len = end_pos - split_pos - 1;
        else
            value_copy_len = 0;

        if(key_copy_len > 0)
        {
            // copy key
            CStringUtils::StringCopy(start_pos, req_param->param[param_num].key,
                kMaxReqKeyLen, key_copy_len);
            // copy value
            CStringUtils::StringCopy(split_pos + 1, req_param->param[param_num].value,
                kMaxReqValueLen, value_copy_len);
            CHttpUtils::url_decode(req_param->param[param_num].value);
            param_num++;

            //进行处理
            CUtf8Util::QuanJiaoToBanJiao(req_param->param[param_num].key, req_param->param[param_num].key);
            CUtf8Util::QuanJiaoToBanJiao(req_param->param[param_num].value, req_param->param[param_num].value);
        }
        // TODO(tonghuang): too many param number, notify caller?
        if(param_num >= kMaxReqParam)
            break;
    } while((end_pos != NULL) && ((start_pos = end_pos + 1) != NULL));

    req_param->param_num = param_num;

    return param_num > 0 ? 0 : -1;
}

TVOID CHuWork::ProcessString(char *pszStr)
{
    //1、全半角
    CUtf8Util::QuanJiaoToBanJiao(pszStr, pszStr);

    //2、删除标点
    CUtf8Util::strtrim(pszStr);

    //3、转大写
    CUtf8Util::strtoupper(pszStr);
}

TUINT64 CHuWork::GetRawDataMd5( const TUCHAR *pszData, const TUINT32 udwDataLen )
{
    TUINT64 uddwKey = 0;
    if(NULL == pszData || 0 == udwDataLen)
    {
        uddwKey = 0;
    }
    else
    {
        MD5Segment(pszData, udwDataLen, (TUCHAR*)&uddwKey, sizeof(uddwKey));
    }
    return uddwKey;
}

TUINT64 CHuWork::GetStringMD5(char* pszStr)
{
    TUINT64 uddwKey = 0;

    ProcessString(pszStr);

    if(NULL == pszStr || 0 == pszStr[0])
    {
        uddwKey = 0;
    }
    else
    {
        MD5String(pszStr, (TUCHAR*)&uddwKey, sizeof(uddwKey));
    }
    return uddwKey;
}

TUINT64 CHuWork::GetStringMD5(const TCHAR* pszStr, TCHAR* pMD5Buffer)
{
    TUINT64 uddwKey = 0;
    strcpy(pMD5Buffer, pszStr);
    ProcessString(pMD5Buffer);
    MD5String(pMD5Buffer, (TUCHAR*)&uddwKey, sizeof(uddwKey));
    return uddwKey;
}

TUINT64 CHuWork::GetUserMd5(TCHAR *pszStr)
{
    TUINT64 uddwKey = 0;
    if(NULL == pszStr || 0 == pszStr[0])
    {
        return 0;
    }

    //1、全半角
    CUtf8Util::QuanJiaoToBanJiao(pszStr, pszStr);

    //2、删除标点
    CUtf8Util::strtrim(pszStr);

    //3、计算MD5
    if(0 == pszStr[0])
    {
        uddwKey = 0;
    }
    else
    {
        MD5String(pszStr, (TUCHAR*)&uddwKey, sizeof(uddwKey));
    }
    return uddwKey;
}

int CHuWork::GetRequestParam(SSession *session, CTseLogger *serv_log, RequestParam *req_param)
{
    int dwRetVal = 0;

    req_param->Reset();

    dwRetVal = GetHttpParam(session->m_stReqParam.m_szReqUrl, req_param);
    if(0 > dwRetVal)
    {
        TSE_LOG_ERROR(serv_log, ("Get http param failed! [seq=%u]", session->m_udwSeqNo));
        return -1;
    }

    dwRetVal = ProcessReqParam(req_param, session);
    if(0 > dwRetVal)
    {
        TSE_LOG_ERROR(serv_log, ("Get key param failed! [seq=%u]", session->m_udwSeqNo));
        return -2;
    }

    if(session->m_stReqParam.m_udwCommandID <= EN_CLIENT_REQ_COMMAND__UNKNOW || session->m_stReqParam.m_udwCommandID >= EN_CLIENT_REQ_COMMAND__END)
    {
        TSE_LOG_ERROR(serv_log, ("error command[%d]! [seq=%u]", session->m_stReqParam.m_udwCommandID, session->m_udwSeqNo));
        return -3;
    }

    return 0;
}

int CHuWork::ProcessReqParam(RequestParam *pstReqParam, SSession *pstSession)
{
    TINT32 dwIdx = 0, dwIdy = 0;
    Key2Value *pstHttpParam = NULL;
    SReqParam *pstReq = &pstSession->m_stReqParam;
    TCHAR	szTmpKey[16];

    if(pstReqParam->param_num <= 0)
    {
        return -1;
    }

    // wavewang@20130423: 获取用户ip
    const TCHAR *pszIpBeg = strstr((const char *)pstSession->m_szClientReqBuf, "\r\nX-Forwarded-For: ");
    const TCHAR *pszIpEnd = NULL;
    if(pszIpBeg)
    {
        pszIpBeg += strlen("\r\nX-Forwarded-For: ");
        pszIpEnd = strchr(pszIpBeg, '\r');
        if(pszIpEnd)
        {
            if(pszIpEnd - pszIpBeg >= MAX_IP_LEN)
            {
                memcpy(pstReq->m_szIp, pszIpBeg, MAX_IP_LEN - 1);
                pstReq->m_szIp[MAX_IP_LEN - 1] = 0;
            }
            else
            {
                memcpy(pstReq->m_szIp, pszIpBeg, pszIpEnd - pszIpBeg);
                pstReq->m_szIp[pszIpEnd - pszIpBeg] = 0;
            }
        }
    }

    for(dwIdx = pstReqParam->param_num - 1; dwIdx >= 0; dwIdx--)
    {
        pstHttpParam = &pstReqParam->param[dwIdx];

        if(0 == strcmp(pstHttpParam->key, "did"))
        {
            strncpy(pstReq->m_szDevice, pstHttpParam->value, MAX_DEVICE_ID_LEN - 1);
            pstReq->m_szDevice[MAX_DEVICE_ID_LEN - 1] = 0;
            pstReq->m_uddwDeviceId = GetUserMd5(pstHttpParam->value);
        }
        else if(0 == strcmp(pstHttpParam->key, "os_version"))
        {
            strncpy(pstReq->m_szIosVer, pstHttpParam->value, MAX_OS_VER_LEN-1);
            pstReq->m_szIosVer[MAX_OS_VER_LEN - 1] = 0;
        }
        else if(0 == strcmp(pstHttpParam->key, "pid"))
        {
            strncpy(pstReq->m_szPid, pstHttpParam->value, MAX_PRODUCT_ID_LEN-1);
            pstReq->m_szPid[MAX_PRODUCT_ID_LEN - 1] = 0;
        }
        else if(0 == strcmp(pstHttpParam->key, "idfa"))
        {
            strncpy(pstReq->m_szIdfa, pstHttpParam->value, MAX_DEVICE_ID_LEN-1);
            pstReq->m_szIdfa[MAX_DEVICE_ID_LEN - 1] = 0;
        }
        else if(0 == strcmp(pstHttpParam->key, "vs"))
        {
            strncpy(pstReq->m_szVs, pstHttpParam->value, MAX_PRODUCT_ID_LEN - 1);
            pstReq->m_szVs[MAX_PRODUCT_ID_LEN - 1] = '\0';
        }
        else if(0 == strcmp(pstHttpParam->key, "sy"))
        {
            strncpy(pstReq->m_szSy, pstHttpParam->value, MAX_PRODUCT_ID_LEN - 1);
            pstReq->m_szSy[MAX_PRODUCT_ID_LEN - 1] = '\0';
        }
        else if(0 == strcmp(pstHttpParam->key, "platform"))
        {
            strncpy(pstReq->m_szPlatForm, pstHttpParam->value, MAX_PRODUCT_ID_LEN - 1);
            pstReq->m_szPlatForm[MAX_PRODUCT_ID_LEN - 1] = '\0';
        }
        else if(0 == strcmp(pstHttpParam->key, "sid"))
        {
            if(pstHttpParam->value[0])
            {
                pstReq->m_udwSvrId = strtoul(pstHttpParam->value, NULL, 10);
            }
        }
        else if(0 == strcmp(pstHttpParam->key, "gid"))
        {
            strncpy(pstReq->m_szGameCenterID, pstHttpParam->value, MAX_GAME_CENTER_ID_LEN - 1);
            pstReq->m_szGameCenterID[MAX_GAME_CENTER_ID_LEN - 1] = 0;
            pstReq->m_uddwGId = GetUserMd5(pstReq->m_szGameCenterID);
        }
        else if(0 == strcmp(pstHttpParam->key, "uid"))
        {
            pstReq->m_udwUserId = strtoul(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "aid"))
        {
            pstReq->m_udwAllianceId = strtoul(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "cid"))
        {
            pstReq->m_udwCityId = strtoul(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "rid"))
        {
            pstReq->m_ddwReportId = strtoll(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "mid"))
        {
            pstReq->m_ddwMailId = strtoll(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "sn"))
        {
            pstReq->m_udwSeqNo = strtoul(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "version"))
        {
            pstReq->m_udwVersion = atof(pstHttpParam->value);
        }
        else if(0 == strcmp(pstHttpParam->key, "lg"))
        {
            pstReq->m_ucLoginStatus = atoi(pstHttpParam->value);
            if(pstReq->m_ucLoginStatus != EN_LOGIN_STATUS__USING)
            {
                pstReq->m_ucLoginStatus = EN_LOGIN_STATUS__LOGIN;
            }
        }
        else if(0 == strcmp(pstHttpParam->key, "lang"))
        {
            pstReq->m_udwLang = strtoul(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "pg"))
        {
            pstReq->m_udwPage = strtoul(pstHttpParam->value, NULL, 10);
            if(pstReq->m_udwPage == 0)
            {
                pstReq->m_udwPage = 1;
            }
        }
        else if(0 == strcmp(pstHttpParam->key, "pp"))
        {
            pstReq->m_udwPerpage = strtoul(pstHttpParam->value, NULL, 10);
            if(pstReq->m_udwPerpage == 0 || pstReq->m_udwPerpage > DEFAULT_PERPAGE_NUM)
            {
                pstReq->m_udwPerpage = DEFAULT_PERPAGE_NUM;
            }
        }
        else if(0 == strcmp(pstHttpParam->key, "npc"))
        {
            pstReq->m_ucIsNpc = atoi(pstHttpParam->value) == 1 ? 1 : 0;
        }
        else if(0 == strcmp(pstHttpParam->key, "command"))
        {
            strncpy(pstReq->m_szCommand, pstHttpParam->value, DEFAULT_NAME_STR_LEN - 1);
            pstReq->m_szCommand[DEFAULT_NAME_STR_LEN - 1] = 0;
            pstReq->m_udwCommandID = CClientCmd::GetInstance()->GetCommandID(pstReq->m_szCommand);

        }
        else if(0 == strcmp(pstHttpParam->key, "sandbox"))
        {
            pstReq->m_ucIsSandBox = atoi(pstHttpParam->value);
        }
        else if(0 == strcmp(pstHttpParam->key, "checkac"))
        {
            pstReq->m_bNeedLoginCheck = strtoul(pstHttpParam->value, NULL, 10);
        }
        else if(0 == strcmp(pstHttpParam->key, "reqcost"))
        {  
            pstReq->m_ddwReqCost = strtoll(pstHttpParam->value, NULL, 10);
        }
        //else if(0 == strcmp(pstHttpParam->key, "rtype"))
        //{
        //    TINT32 dwRType = atoi(pstHttpParam->value);
        //    if(dwRType == 0)
        //    {
        //        pstReq->m_ucResType = EN_CONTENT_UPDATE_TYPE__TABLE_INC;
        //    }
        //    else
        //    {
        //        pstReq->m_ucResType = EN_CONTENT_UPDATE_TYPE__ALL;
        //    }
        //}
        else if (0 == strcmp(pstHttpParam->key, "purchase_token")
            || 0 == strcmp(pstHttpParam->key, "receipt"))
        {
            pstReq->m_szPurchaseToken = pstHttpParam->value;
        }
        else if (0 == strcmp(pstHttpParam->key, "package_name"))
        {
            pstReq->m_szPackageName = pstHttpParam->value;
        }
        else if (0 == strcmp(pstHttpParam->key, "item_id"))
        {
            pstReq->m_szItemId = pstHttpParam->value;
        }
        else if (0 == strcmp(pstHttpParam->key, "purchase_uid")
            || 0 == strcmp(pstHttpParam->key, "transaction"))
        {
            pstReq->m_szPurchaseUid = pstHttpParam->value;
        }
        else
        {
            for(dwIdy = 0; dwIdy < MAX_REQ_PARAM_KEY_NUM; dwIdy++)
            {
                sprintf(szTmpKey, "key%u", dwIdy);
                if(0 == strcmp(pstHttpParam->key, szTmpKey))
                {
                    strncpy(&pstReq->m_szKey[dwIdy][0], pstHttpParam->value, DEFAULT_PARAM_STR_LEN - 1);
                    pstReq->m_szKey[dwIdy][DEFAULT_PARAM_STR_LEN - 1] = 0;
                    break;
                }
            }
        }
    }

    return 0;
}

