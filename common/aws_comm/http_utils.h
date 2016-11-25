#ifndef TSHP_HU_HTTP_UTILS_H
#define TSHP_HU_HTTP_UTILS_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

class CHttpUtils
{
public:
    /*
     * Http url encode & decode
     */
    static char * url_encode(const char *input_url, char *output_url, size_t buf_size);
    static char * url_decode(char *url);

	static int add_http_result_head(const char *body_content, int body_len, char *http_result,
		size_t buf_size, unsigned int &http_result_len, const char *code_type, const char* content_type);
    static int add_http_result_head(const char *body_content, int body_len, char *http_result,
        size_t buf_size, int &http_result_len, const char *code_type, const char* content_type);

	static inline void UpperHeadChar(char* pszStr)
	{
		int dwStrLen = strlen(pszStr);
		for(int idx = 0; idx < dwStrLen; idx++)
		{
			if(isalnum(pszStr[idx]) != 0)
			{
				pszStr[idx] = toupper(pszStr[idx]);
				break;
			}
		}
	}
private:
    static inline char hex2char(char c, bool *cvt_ok)
    {
        *cvt_ok = true;
        if (c >= 'a' && c <= 'z')
            return c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z')
            return c - 'A' + 10;
        else if (c >= '0' && c <= '9')
            return c - '0';
        else
            *cvt_ok = false;

        return c;
    }

    static inline char char2hex(char c)
    {
        static char m_hex_table[] = "0123456789ABCDEF";
        return m_hex_table[c & 15];
    }

    /*
     * RFC 2396 says unreserved characters including alpha(a-zA-Z), num(0-9) and the
     * following: - _ . ! ~ * ' ( )
     */
    static inline bool is_url_char(unsigned char c)
    {
        // exclude control char and non-ascii char
        return (c < 128) && (c > 31) &&
            ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9') || (c == '-') || (c == '_') ||
             (c == '.') || (c == '!') || (c == '~') || (c == '*') ||
             (c == '\'') || (c == '(') || (c == ')'));
    }
};

#endif

