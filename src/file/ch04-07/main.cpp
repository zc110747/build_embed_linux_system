////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      main.c
//
//  Purpose:
//      实现最简单的web服务器，支持静态页面读取和动态处理
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/10/2024   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <fcntl.h>
#include <mutex>
#include "http_server.hpp"

#define SERVER_IPADDR   "172.27.83.254"
#define SERVER_PORT     8080
#define LED_FILE_PATH   "/dev/led"

void led_process(int status);

std::map<std::string, HTTP_SREVER::http_dynamic_func_type> process_map = {
    {
        "/submit.json", [](const std::string& url, const std::string &data, std::string &response) {response = data; return true;},
    }
};

int main(int argc, char *argv[])
{
    HTTP_SREVER::http_server server;
    server.init(std::string(SERVER_IPADDR), SERVER_PORT, process_map);
    server.register_dynamic_process("/led_push_on.json", [](const std::string& url, const std::string &data, std::string &response) {
            led_process(1); 
            response="led on"; 
            return true;});
    server.register_dynamic_process("/led_push_off.json", [](const std::string& url, const std::string &data, std::string &response) {
            led_process(1); 
            response="led off";
            return true;}); 
    server.start();

    return 0;
}

static std::mutex led_mutex;

void led_process(int status)
{
    int fd;
    std::lock_guard<std::mutex> lock(led_mutex);

    fd = open(LED_FILE_PATH, O_RDWR);
    if(fd < 0) {
        printf("open %s failed\n", LED_FILE_PATH);
        return;
    }

    write(fd, &status, sizeof(status));
    close(fd);
}
