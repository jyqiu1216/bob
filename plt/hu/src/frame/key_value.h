#ifndef TSHP_HU_KEY_VALUE_H
#define TSHP_HU_KEY_VALUE_H

// 请求的URL中最大的参数(key=value)个数
const unsigned int kMaxReqParam         = 50;
// 请求的参数中key的最大长度
const unsigned int kMaxReqKeyLen        = 64;
// 请求的参数中value的最大长度
const unsigned int kMaxReqValueLen      = 10240;
// http请求包的最大长度 (5 KB)
const unsigned int kMaxHttpReqLen       = 10 << 10;
// http请求包中url长度 (2 KB) (<kMaxHttpReqLen)
const unsigned int kMaxHttpUrlLen       = 10 << 10;

#pragma pack(1)

struct Key2Value
{
    char key[kMaxReqKeyLen];
    char value[kMaxReqValueLen];
};

struct RequestParam
{
    Key2Value param[kMaxReqParam];
    int param_num;

	void Reset()
	{
		param_num = 0;
	}
};

#pragma pack()
#endif

