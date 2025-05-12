////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      can_test.c
//
//  Purpose:
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//      需要使能can支持(使用iproute2工具，busybox工具包中不支持can功能)
//      ip link set can0 type can bitrate 500000
//      ifconfig can0 up
//  Revision History:
//      12/19/2022  Create New Version.
//      23/08/202   Format update.     
/////////////////////////////////////////////////////////////////////////////

#include "can_socket.hpp"

#define CAN_INTERFACE   "can0"

int main(int argc, char *argv[]) 
{
    can_socket manage;
    
    if (!manage.init()) {
        std::cout << "can init failed" << std::endl;
        return -1;
    }

    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}