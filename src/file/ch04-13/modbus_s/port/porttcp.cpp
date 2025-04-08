/*
  * FreeModbus Libary: LPC214X Port
  * Copyright (C) 2007 Tiago Prado Lone <tiago@maxwellbohr.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"
#include "mb.h"
#include "mbconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> 
#include <arpa/inet.h>
#include <thread>

#include "logger.hpp"

/*
| tid(2) | pid(2) | len(2) | uid(1) | function(1) | data(len-2)，格式根据function |
00 00 00 00 00 06 01 01 00 00 00 0a
00 00 00 00 00 05 01 01 02 10 01
*/
/* ----------------------- Defines  -----------------------------------------*/
#define TCP_RX_BUFFER_SIZE      1024

static int server_fd, client_fd;
static UCHAR rx_buffer[TCP_RX_BUFFER_SIZE];
static int rx_size;
static USHORT port;

void mb_tcp_process_task(void)
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    int opt = 1;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) 
    {
        LOG_ERROR(0, "can't create socket\n");
        return;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 定义服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    do 
    {
        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {      
            printf("socket server bind error:%d\n", port);
            sleep(5);
            continue;     
        } else {
            printf("bind ok, net_port:%d\n", port);
            break;
        }
    }while(1);

    // 监听socket
    listen(server_fd, 3);

    LOG_INFO(0, "server listen success, fd:{}, port:{}", server_fd, port);

    while (1) 
    {
        //接收客户端连接
        client_addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            LOG_WARN(0, "client accept failed!");
            continue;
        }

        LOG_INFO(0, "client connect success!");

        while (1) 
        {
            rx_size = recv(client_fd, rx_buffer, TCP_RX_BUFFER_SIZE, 0);
            if (rx_size < 0) 
            {
                LOG_ERROR(0, "recv data failed");
                break;
            } 
            else if (rx_size == 0) 
            {
                LOG_ERROR(0, "client client failed");
                break;
            }
            else
            {
                //接收到数据，通知应用层处理
                xMBPortEventPost(EV_FRAME_RECEIVED);
            }
        }
        
        //检测失败，断开现有连接
        close(client_fd);
        client_fd = -1;
    }
}

BOOL
xMBTCPPortInit(USHORT usTCPPort)
{
    port = usTCPPort;

    std::thread(mb_tcp_process_task).detach();
    return TRUE;
}

void
vMBTCPPortDisable(void)
{
    close(client_fd);
    client_fd = -1;
}

BOOL
xMBTCPPortGetRequest(UCHAR **ppucMBTCPFrame, USHORT *usTCPLength)
{
    *ppucMBTCPFrame = &rx_buffer[0];
    *usTCPLength = rx_size;
    return TRUE;
}

BOOL
xMBTCPPortSendResponse(const UCHAR *pucMBTCPFrame, USHORT usTCPLength)
{
    int ret;
    BOOL bFrameSent = FALSE;

    if (client_fd)
    {
        LOG_INFO(0, "send size:{}", usTCPLength);
        ret = send(client_fd, (void *)pucMBTCPFrame, usTCPLength, 0);
        if (ret == usTCPLength)
        {
            bFrameSent = TRUE;
        }
    }
    return bFrameSent;
}
