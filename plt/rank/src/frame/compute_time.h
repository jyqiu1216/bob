#ifndef _COMPUTE_TIME_H_
#define _COMPUTE_TIME_H_

#include <string>
using namespace std;

typedef __int64_t               TINT64;


enum EComputeType
{
    EN_COMPUTE_TYPE_RANK = 1,
};


enum EComputeTimeFld
{
	EN_COMPUTE_TIME_FLD_TYPE = 0,
    EN_COMPUTE_TIME_FLD_TIME = 1,
};

class ComputeTime
{
public:
	unsigned int	udwType;
    TINT64          ddwTime;
};

#endif
