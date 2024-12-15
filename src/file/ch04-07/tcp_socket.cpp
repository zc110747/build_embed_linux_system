
////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      tcp_socket.cpp
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

#include "tcp_socket.hpp"
#include "http_engine.hpp"

bool tcp_server::start()
{
    socket_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        std::cout<<"socket init error!"<<std::endl;
        return false;
    }

    // 配置运行绑定在TIME_WAIT的接口
    int flag = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) {
        std::cout<<"setsockopt failed."<<std::endl;
        socket_fd_ = -1;
        return false;
    }

    // 启动线程
    std::thread(std::bind(&tcp_server::run, this)).detach();

    return true;
}

void tcp_server::run()
{   
    struct sockaddr_in servaddr, clientaddr;

    // 绑定socket接口到指定的ip地址和端口(必须为本机IP或者127.0.0.1循环地址)
    memset(&servaddr, 0, sizeof(servaddr));    
    servaddr.sin_family = AF_INET;     
    servaddr.sin_addr.s_addr = inet_addr(ipaddr_.c_str());  
    servaddr.sin_port = htons(port_);

    do 
    {
        if (bind(socket_fd_, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {      
            std::cout<<"socket bind failed!"<<ipaddr_<<" "<<port_<<std::endl;
            sleep(5);
            continue;     
        } else {
            std::cout<<"socket bind ok, "<<ipaddr_<<" "<<port_<<std::endl;
            break;  
        }
    }while(1);

    //监听socket通讯
    listen(socket_fd_, max_listen_);

    while (1) {
        int client_fd;
        uint32_t client_size; 

        client_size = sizeof(clientaddr);
        
        //等待客户端的TCP进行三次握手连接
        client_fd = accept(socket_fd_, (struct sockaddr *)&clientaddr, &client_size);
        if (client_fd < 0) {
            std::cout<<"socket accept failed!"<<std::endl;
            continue;
        } else {
            // 启动线程
            std::thread(std::bind(&tcp_server::recv_task_run, this, client_fd)).detach();
        }         
    }    
    close(socket_fd_);
}

void tcp_server::recv_task_run(int fd)
{
    char recvbuf[1024] = {0};
    int recv_len;
    http_engine enginee;
    std::string out_str;

    while(1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));

        //接收客户端发送的数据
        recv_len = recv(fd, recvbuf, sizeof(recvbuf), 0);
        if (recv_len < 0) {        
            std::cout<<"server recv failed, break!"<<std::endl;
            break;
        } else if (recv_len == 0) {
            std::cout<<"server recv zero, break!"<<std::endl;
            break;
        } else {
            std::cout<<"recv len:"<<recv_len<<std::endl;
            recvbuf[recv_len] = '\0';

            if(enginee.process(recvbuf, recv_len, out_str)) {
                std::cout<<out_str<<std::endl;
                write(fd, out_str.c_str(), out_str.size());
            }
        }
    }

    close(fd);
}