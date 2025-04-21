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
#define TCP_RX_BUFFER_SIZE      1024

typedef struct porttcp
{
    int client_fd = -1;
    UCHAR rx_buffer[TCP_RX_BUFFER_SIZE];
    size_t rx_size;
    UCHAR rx_frame_buffer[TCP_RX_BUFFER_SIZE];
    size_t rx_frame_size;
}PORTTCP_INFO;

static PORTTCP_INFO porttcp_info;

static void tcp_rx_process(int client_fd)
{
    porttcp_info.rx_frame_size = 0;

    while (1) {
        porttcp_info.rx_size = recv(client_fd, porttcp_info.rx_buffer, TCP_RX_BUFFER_SIZE, 0);
        if (porttcp_info.rx_size < 0) {
            break;
        } else if (porttcp_info.rx_size == 0) {
            break;
        } else {
            //小于范围才进行拷贝
            if (porttcp_info.rx_frame_size + porttcp_info.rx_size < TCP_RX_BUFFER_SIZE) {
                memcpy(&porttcp_info.rx_frame_buffer[porttcp_info.rx_frame_size], porttcp_info.rx_buffer, porttcp_info.rx_size);
            } else { // 超出范围则直接丢弃
                LOG_ERROR(0, "rx_frame_size:{} > rx_size:{}", porttcp_info.rx_frame_size, porttcp_info.rx_size);
                porttcp_info.rx_frame_size = 0;
                continue; 
            }
            porttcp_info.rx_frame_size += porttcp_info.rx_size;

            // 判断完整帧
            if (porttcp_info.rx_frame_size >= 6) {
                uint16_t len = porttcp_info.rx_frame_buffer[4]*256 + porttcp_info.rx_frame_buffer[5] + 6;

                // 长度过长, 数据丢弃
                if (len > TCP_RX_BUFFER_SIZE) {
                    LOG_ERROR(0, "rx_frame_size:{} > TCP_RX_BUFFER_SIZE:{}", len, TCP_RX_BUFFER_SIZE);
                    porttcp_info.rx_frame_size = 0;
                    continue;
                }

                if (porttcp_info.rx_frame_size >= len) {
                    xMBPortEventPost(EV_FRAME_RECEIVED);
                }
            }
        }
    }

    printf("client disconnect!\n");
    close(client_fd);
    client_fd = -1;
}

void mb_tcp_process_task(USHORT port)
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    int opt = 1;
    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        LOG_ERROR(0, "can't create socket\n");
        return;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 定义服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    do {
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
    listen(server_fd, 1);

    LOG_INFO(0, "server listen success, fd:{}, port:{}", server_fd, port);

    while (1) {
        client_addr_len = sizeof(client_addr);
        porttcp_info.client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (porttcp_info.client_fd < 0) {
            LOG_WARN(0, "client accept failed!");
            continue;
        }

        LOG_INFO(0, "client connect success!");
        tcp_rx_process(porttcp_info.client_fd);
    }
}

BOOL
xMBTCPPortInit(USHORT usTCPPort)
{
    memset((char *)&porttcp_info, 0, sizeof(porttcp_info));
    porttcp_info.client_fd = -1;

    std::thread(mb_tcp_process_task, usTCPPort).detach();
    return TRUE;
}

void
vMBTCPPortDisable(void)
{
    close(porttcp_info.client_fd);
    porttcp_info.client_fd = -1;
}

BOOL
xMBTCPPortGetRequest(UCHAR **ppucMBTCPFrame, USHORT *usTCPLength)
{
    *ppucMBTCPFrame = &porttcp_info.rx_frame_buffer[0];
    *usTCPLength = porttcp_info.rx_frame_size;
    porttcp_info.rx_frame_size = 0;
    return TRUE;
}

BOOL
xMBTCPPortSendResponse(const UCHAR *pucMBTCPFrame, USHORT usTCPLength)
{
    int ret;
    BOOL bFrameSent = FALSE;

    if (porttcp_info.client_fd >= 0)
    {
        ret = send(porttcp_info.client_fd, (void *)pucMBTCPFrame, usTCPLength, 0);
        if (ret == usTCPLength)
        {
            bFrameSent = TRUE;
        }
    }
    return bFrameSent;
}
