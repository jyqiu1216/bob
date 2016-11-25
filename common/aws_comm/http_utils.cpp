#include <string.h>
#include <stdio.h>

#include "http_utils.h"

char * CHttpUtils::url_encode(const char *input_url, char *output_url, size_t buf_size)
{
    size_t src_pos = 0;
    size_t dst_pos = 0;
    while (input_url[src_pos] && buf_size - dst_pos > 1)
    {
        if (is_url_char(input_url[src_pos]))
        {
            output_url[dst_pos++] = input_url[src_pos];
        }
        else if (input_url[src_pos] == ' ')
        {
            output_url[dst_pos++] = '+';
        }
        else
        {
            if (buf_size - dst_pos < 4)
                break;
            output_url[dst_pos++] = '%';
            output_url[dst_pos++] = char2hex(input_url[src_pos] >> 4);
            output_url[dst_pos++] = char2hex(input_url[src_pos] & 15);
        }
        src_pos++;
    }
    output_url[dst_pos] = '\0';

    return output_url;
}

char * CHttpUtils::url_decode(char *url)
{
    int src_pos = 0;
    int dst_pos = 0;
    int len = strlen(url);
    char c;
    bool cvt_left_ok = true;
    bool cvt_right_ok = true;

    while (src_pos < len)
    {
        c = (url[src_pos] == '+') ? ' ' : url[src_pos];
        if (c == '%' && src_pos + 2 < len)
        {
            c = (hex2char(url[src_pos + 1], &cvt_left_ok) << 4)
                + hex2char(url[src_pos + 2], &cvt_right_ok);

            // error escape format, use space instead and skip %
            if (cvt_left_ok && cvt_right_ok)
            {
                src_pos += 2;
            }
            else
            {
                c = ' ';
            }
        }

        src_pos++;
        url[dst_pos++] = c;
    }

    url[dst_pos] = '\0';

    return url;
}

int CHttpUtils::add_http_result_head(const char *body_content, int body_len, char *http_result,
    size_t buf_size, unsigned int &http_result_len, const char *code_type, const char* content_type)
{
	// head
    http_result_len = sprintf(http_result, 
		"HTTP/1.1 200 0K\r\n"
		"Connection:close\r\n"
		"Content-Length:%u\r\n"
		"Content-Type:%s;charset=%s\r\n\r\n",
        body_len, content_type, code_type);

	// content
	memcpy(http_result + http_result_len, body_content, body_len);
	http_result_len += body_len;

	// end
	http_result[http_result_len] = 0;
	
    return http_result_len;
}

int CHttpUtils::add_http_result_head(const char *body_content, int body_len, char *http_result, size_t buf_size, int &http_result_len, const char *code_type, const char* content_type)
{
    // head
    http_result_len = sprintf(http_result,
        "HTTP/1.1 200 0K\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection:close\r\n"
        "Content-Length:%u\r\n"
        "Content-Type:%s;charset=%s\r\n\r\n",
        body_len, content_type, code_type);

    // content
    memcpy(http_result + http_result_len, body_content, body_len);
    http_result_len += body_len;

    // end
    http_result[http_result_len] = 0;

    return http_result_len;
}
