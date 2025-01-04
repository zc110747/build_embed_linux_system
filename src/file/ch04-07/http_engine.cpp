////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      http_engine.cpp
//
//  Purpose:
//      http处理引擎
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/10/2024   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include "http_engine.hpp"
#include "http_resource.hpp"
#include <string_view>

const std::map<std::string, HTTP_FILE_TYPE> file_type_list = {
    {".jpg", FILE_TYPE_JPG},
    {".png", FILE_TYPE_PNG},
    {".html", FILE_TYPE_HTML},
    {".gif", FILE_TYPE_GIF},
    {".png", FILE_TYPE_PNG},
    {".js", FILE_TYPE_JS},
    {".asp", FILE_TYPE_ASP},
    {".css", FILE_TYPE_CSS},
};
static const std::string HTTP_NOT_FOUND_BODY("<!Doctype html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>File not found.\n</body></html>");

bool endsWith(std::string_view str, std::string_view suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool http_engine::process(char *rx_buffer, int rx_size)
{
    int ret;
    
    std::string buffer(rx_buffer, rx_size);
    std::string sub_str("\r\n");
    std::cout<<buffer<<std::endl;

    tx_size_ = 0;

    //decode get header string
    std::size_t pos = buffer.find(sub_str);
    std::string head_str = buffer.substr(0, pos);
    std::cout<<head_str<<std::endl;
    
    ret = decode_request_line(head_str);
    if (ret) {
        std::cout<<"method:"<<protocol_.method_<<std::endl;
        std::cout<<"url:"<<protocol_.url_<<std::endl;
        std::cout<<"ver:"<<protocol_.ver_<<std::endl;

        if (protocol_.is_dynamic) {
            engine_dynamic_process();
        } else {
            engine_static_process();
        }

    } else {
        handle_tx_(http_resp_header_resource[HEAD_RESP_404].c_str(), http_resp_header_resource[HEAD_RESP_404].size());
        auto &&header_extra_str = format_header_extra_info(FILE_TYPE_HTML, HTTP_NOT_FOUND_BODY.size());
        handle_tx_(header_extra_str.c_str(), header_extra_str.size());
        handle_tx_(HTTP_NOT_FOUND_BODY.c_str(), HTTP_NOT_FOUND_BODY.size());
    }
    return true;
}

bool http_engine::engine_dynamic_process()
{
    std::string dynamic_rep;

    handle_tx_(http_resp_header_resource[HEAD_RESP_200].c_str(), http_resp_header_resource[HEAD_RESP_200].size());

    auto &&header_extra_str = format_header_extra_info(FILE_TYPE_JSON, dynamic_rep.size());
    handle_tx_(header_extra_str.c_str(), header_extra_str.size());
    handle_tx_(dynamic_rep.c_str(), dynamic_rep.size());
    return true;
}

bool http_engine::engine_static_process()
{
    std::string url = "www/" + protocol_.url_;
    std::ifstream input_file(url, std::ios::binary);
    char buffer[1024];

    if (!input_file) {
        std::cerr << "无法打开文件: " << url << std::endl;
        return false;
    }

    input_file.seekg(0, std::ios::end);
    std::streamsize body_size = input_file.tellg();
    input_file.seekg(0, std::ios::beg);

    handle_tx_(http_resp_header_resource[HEAD_RESP_200].c_str(), http_resp_header_resource[HEAD_RESP_200].size());
    auto &&header_extra_str = format_header_extra_info(protocol_.type_, body_size);
    handle_tx_(header_extra_str.c_str(), header_extra_str.size());
    
    while (input_file) {
        input_file.read(buffer, 1024);
        std::streamsize bytesRead = input_file.gcount();
 
        if (bytesRead > 0) {
            handle_tx_(buffer, bytesRead);
        }

        if (input_file.bad()) {
            std::cerr << "读取文件时发生错误。" << std::endl;
            input_file.close();
            return 1;
        }

        if (input_file.eof() && bytesRead == 0) {
            break;
        }
    }

    input_file.close();
    return true;
}

std::string http_engine::format_header_extra_info(HTTP_FILE_TYPE type, int size)
{
    std::string base_string = "Content-type: ";

    switch ((type))
    {
    case FILE_TYPE_TEXT:
        base_string += "text/plain;charset=utf-8\r\n";
        break;
    
    case FILE_TYPE_JPG:
        base_string += "application/x-jpg;charset=utf-8\r\n";
        break;

    case FILE_TYPE_PNG:
        base_string += "image/png;charset=utf-8\r\n";
        break;

    case FILE_TYPE_HTML:
        base_string += "text/html;charset=utf-8\r\n";
        break;

    case FILE_TYPE_GIF:
        base_string += "image/gif;charset=utf-8\r\n";
        break;

    case FILE_TYPE_JS:
        base_string += "application/x-javascript;charset=utf-8\r\n";
        break;

    case FILE_TYPE_ASP:
        base_string += "text/asp;charset=utf-8\r\n";
        break;

    case FILE_TYPE_CSS:
        base_string += "text/css;charset=utf-8\r\n";
        break;

    case FILE_TYPE_JSON:
        base_string += "application/json;charset=utf-8\r\n";
        break;

    default:
        base_string += "text/html;charset=utf-8\r\n";   
        break;
    }

    base_string += "Connection: close\r\n";
    base_string += "Server: c++ sever\r\n";

    base_string += "Content-Length: ";
    base_string += std::to_string(size);
    base_string += "\r\n";

    base_string += "\r\n";
    return base_string;
}

bool http_engine::decode_request_line(const std::string &head_str)
{
    std::vector<std::string> tokens;
    std::istringstream tokenStream(head_str);
    std::string token;
    
    while (tokenStream >> token) {
        tokens.push_back(token);
    }
    
    if (tokens.size() != 3) {
        std::cout<<"decode header failed!"<<std::endl;
        return false;
    }

    //获取方法，参考《HTTP权威指南》第三章
    //常见请求码GET，HEAD，POST，PUT，TRACE，OPTIONS，DELETE
    //这里只实现GET和POST
    if (tokens[0] == "GET") {
        protocol_.method_ = METHOD_GET;
    } else if (tokens[0] == "POST") {
        protocol_.method_ = METHOD_POST;
    } else {
        return false;
    }
    
    protocol_.url_ = tokens[1];
    if (protocol_.url_ == "/") {
        protocol_.url_ = "index.html";
    }

    protocol_.ver_ = tokens[2];

    //解析文件后缀
    if (endsWith(protocol_.url_, ".json")) {
        protocol_.is_dynamic = true;
    } else {
        protocol_.is_dynamic = false;
        protocol_.type_ = FILE_TYPE_TEXT;

        for (const auto &file_type: file_type_list) {
            if(endsWith(protocol_.url_, file_type.first)) {
                protocol_.type_ = file_type.second;
                break;
            }
        }
    }

    return true;
}