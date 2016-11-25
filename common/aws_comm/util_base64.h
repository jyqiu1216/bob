#ifndef _BASE64_HPP
#define _BASE64_HPP

#include <algorithm>
#include <string.h>

typedef unsigned uint32;
typedef unsigned char uint8;

class CUtilBase64
{
private:
	static char g_to_table[64];
	static char *g_to_table_beg;
	static char *g_to_table_end;
	static char g_from_table[128];

public:
	static void encode(char* begin, char* end, char* out, int &out_len);
	static void decode(char* begin, char* end, char* out, int &out_len);

    static void decode_bin(char* begin, char* end, char* out, int &out_len);
};

#endif

