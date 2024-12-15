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

static const std::string HTTP_NoFound_HEADER("HTTP/1.1 404 Not Found\r\n");
static const std::string HTTP_NoFound_HTML("<!Doctype html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>File not found.\n</body></html>");

bool http_engine::process(char *rx_buffer, int rx_size, std::string& Str)
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
    
    ret = decode_header(head_str);
    if (ret) {
        std::cout<<"method:"<<protocol_.method_<<std::endl;
        std::cout<<"url:"<<protocol_.url_<<std::endl;
        std::cout<<"ver:"<<protocol_.ver_<<std::endl;
        Str = HTTP_NoFound_HEADER + HTTP_NoFound_HTML;
    } else {
        Str = HTTP_NoFound_HEADER + HTTP_NoFound_HTML;
    }
    
    return true;
}

bool http_engine::create_out_buffer(const std::string& http_header, const std::string& body)
{
    int head_size = http_header.size();
    int body_size = body.size();

    tx_size_ = head_size+body_size;
    ptxbuf_ = new(std::nothrow) char[tx_size_];
    if(ptxbuf_ == nullptr) {
        return false;
    }

    memcpy(ptxbuf_, http_header.c_str(), head_size);
    memcpy(ptxbuf_+head_size, body.c_str(), body_size);
    return true;
}

bool http_engine::decode_header(const std::string &head_str)
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
    return true;
}