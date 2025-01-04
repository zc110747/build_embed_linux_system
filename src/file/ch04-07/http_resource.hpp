_Pragma("once");

#include "includes.hpp"

#define HEAD_RESP_200   0
#define HEAD_RESP_400   1
#define HEAD_RESP_403   2
#define HEAD_RESP_404   3
#define HEAD_RESP_500   4
const std::vector<std::string> http_resp_header_resource = {
    {"HTTP/1.1 200 OK\r\n"},
    {"HTTP/1.1 400 Bad Request\r\n"},
    {"HTTP/1.1 403 Forbidden\r\n"},
    {"HTTP/1.1 404 Not Found\r\n"},
    {"HTTP/1.1 500 Internal Server Error\r\n"}
};

