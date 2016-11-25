#include "tbl_comm.h"


TINT64 CTableComm::TblParam_GetRealKey( TUINT32 udwSid, TUINT32 udwKey )
{
    TINT64 ddwRealKey = udwSid;
    return (ddwRealKey<<32) + udwKey;
}

