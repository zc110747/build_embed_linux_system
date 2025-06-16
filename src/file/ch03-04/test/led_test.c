////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      led_test.c
//
//  Purpose:
//      LED应用层控制。
//      设备文件: /dev/led
//      配置pinctrl文件: /sys/devices/platform/20c406c.usr_led/pinctrl-led
//      类管理文件: /sys/class/leds/led0/brightness
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/19/2022  Create New Version.
//      23/08/202   Format update.     
/////////////////////////////////////////////////////////////////////////////
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define LED_DEVICE  "/dev/led"

int main(int argc, const char *argv[])
{
    unsigned char val = 1;
    int fd;

    fd = open(LED_DEVICE, O_RDWR | O_NDELAY);
    if (fd == -1) {
        printf("%s open error!\n", LED_DEVICE);
        exit(1);
    }

    if (argc > 1) {   
        val = atoi(argv[1]);
    }

    write(fd, &val, 1);

    close(fd); 

    exit(0);  
}
