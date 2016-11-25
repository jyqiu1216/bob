#include "util_base64.h"

char CUtilBase64::g_to_table[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

char* CUtilBase64::g_to_table_beg = g_to_table;

char* CUtilBase64::g_to_table_end = g_to_table + sizeof(g_to_table);

char CUtilBase64::g_from_table[128] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, // 0
	-1, -1, -1, -1, -1, -1, -1, -1, // 8
	-1, -1, -1, -1, -1, -1, -1, -1, // 16
	-1, -1, -1, -1, -1, -1, -1, -1, // 24
	-1, -1, -1, -1, -1, -1, -1, -1, // 32
	-1, -1, -1, 62, -1, -1, -1, 63, // 40
	52, 53, 54, 55, 56, 57, 58, 59, // 48
	60, 61, -1, -1, -1,  0, -1, -1, // 56
	-1,  0,  1,  2,  3,  4,  5,  6, // 64
	7,  8,  9, 10, 11, 12, 13, 14, // 72
	15, 16, 17, 18, 19, 20, 21, 22, // 80
	23, 24, 25, -1, -1, -1, -1, -1, // 88
	-1, 26, 27, 28, 29, 30, 31, 32, // 96
	33, 34, 35, 36, 37, 38, 39, 40, // 104
	41, 42, 43, 44, 45, 46, 47, 48, // 112
	49, 50, 51, -1, -1, -1, -1, -1  // 120
};

void CUtilBase64::encode(char* begin, char* end, char* out, int &out_len)
{
	char* it = begin;
	char* out_beg = out;
	int lineSize = 0;

	int bytes;
	do
	{
		uint32 input = 0;
		// get the next three bytes into "in" (and count how many we actually get)
		bytes = 0;
		for(; (bytes < 3) && (it != end); ++bytes, ++it)
		{
			input <<= 8;
			input += static_cast<uint8>(*it);
		}
		// convert to base64
		int bits = bytes*8;
		while (bits > 0)
		{
			bits -= 6;
			const uint8 index = ((bits < 0) ? input << -bits : input >> bits) & 0x3F;
			*out = g_to_table_beg[index];
			++out;
			++lineSize;
		}
        
		/*if (lineSize >= 76) // ensure proper line length
		{
			*out = 13;
			++out;
			*out = 10;
			++out;
			lineSize = 0;
		}*/
         
	} while (bytes == 3);

	// add pad characters if necessary
	
		if (bytes > 0)
		for(int i=bytes; i < 3; ++i)
		{
		*out = '=';
		++out;
		}
		
	*out = 0;
	out_len = out - out_beg;
}

void CUtilBase64::decode(char* begin, char* end, char* out, int &out_len)
{
	char* it = begin;
	char* out_beg = out;
	int chars;
	do
	{
		uint8 input[4] = {0, 0, 0, 0};
		// get four characters
		chars=0;
		while((chars<4) && (it != end))
		{
			uint8 c = static_cast<char>(*it);
			if (c == '=') break; // pad character marks the end of the stream
			++it;
			if (std::find(g_to_table_beg, g_to_table_end, c) != g_to_table_end)
			{
				input[chars] = g_from_table[c];
				chars++;
			}
		}
		// output the binary data
		if (chars >= 2)
		{
			*out = static_cast<uint8>((input[0] << 2) + (input[1] >> 4));
			++out;
			if (chars >= 3)
			{
				*out = static_cast<uint8>((input[1] << 4) + (input[2] >> 2));
				++out;
				if (chars >= 4)
				{
					*out = static_cast<uint8>((input[2] << 6) + input[3]);
					++out;
				}
			}
		}
	} while (chars == 4);

	out_len = out - out_beg;
	out_beg[out_len] = 0;
}

void CUtilBase64::decode_bin( char* begin, char* end, char* out, int &out_len )
{
    char* it = begin;
    char* out_beg = out;
    int chars;
    do
    {
        uint8 input[4] = {0, 0, 0, 0};
        // get four characters
        chars=0;
        while((chars<4) && (it != end))
        {
            uint8 c = static_cast<char>(*it);
            if (c == '=') break; // pad character marks the end of the stream
            ++it;
            if (std::find(g_to_table_beg, g_to_table_end, c) != g_to_table_end)
            {
                input[chars] = g_from_table[c];
                chars++;
            }
        }
        // output the binary data
        if (chars >= 2)
        {
            *out = static_cast<uint8>((input[0] << 2) + (input[1] >> 4));
            ++out;
            if (chars >= 3)
            {
                *out = static_cast<uint8>((input[1] << 4) + (input[2] >> 2));
                ++out;
                if (chars >= 4)
                {
                    *out = static_cast<uint8>((input[2] << 6) + input[3]);
                    ++out;
                }
            }
        }
    } while (chars == 4);

    out_len = out - out_beg;
}
