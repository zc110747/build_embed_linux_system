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
#include "http_server.hpp"
#include "semaphore.hpp"

#define SERVER_IPADDR   "172.27.83.254"
#define SERVER_PORT     8080

static semaphore global_exit_sem;

int main(int argc, char *argv[])
{
    http_server server;
    server.init(std::string(SERVER_IPADDR), SERVER_PORT);
    server.start();

    for (;;)
    {
        if (global_exit_sem.wait())
        {
            break;
        }
    }
    return 0;
}