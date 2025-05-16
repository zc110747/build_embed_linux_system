//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2022-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      pwm_test.cpp
//
//  Purpose:
//      pwm测试代码，用于测试pwm应用
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      7/30/2022   Create New Version
//
/////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#define PWM_EXPORT  "/sys/class/pwm/pwmchip1/export"
#define PWM_ENABLE  "/sys/class/pwm/pwmchip1/pwm0/enable"
#define PWM_PEROID  "/sys/class/pwm/pwmchip1/pwm0/period"
#define PWM_DUTYCLE "/sys/class/pwm/pwmchip1/pwm0/duty_cycle"

#define MAX_SIZE    64

int pwm_setup(int state, unsigned int peroid, unsigned int duty_cycle)
{
    int fd_state, fd_peroid, fd_duty;
    int ret = 0, num;
    char str[MAX_SIZE];

    fd_state = open(PWM_ENABLE, O_RDWR);
    fd_peroid = open(PWM_PEROID, O_RDWR);
    fd_duty = open(PWM_DUTYCLE, O_RDWR);

    if (fd_state<0 || fd_peroid<0 || fd_duty<0) {
        ret = -1;
        printf("fd open failed\n");
        goto __exit;
    }

    num = snprintf(str, MAX_SIZE, "%d", peroid);
    ret = write(fd_peroid, str, num);
    if (ret < 0) {
        printf("fd_peroid write failed\n");
        goto __exit;       
    }

    num = snprintf(str, MAX_SIZE, "%d", duty_cycle);
    ret = write(fd_duty, str, num);
    if (ret < 0) {
        printf("fd_duty write failed\n");
        goto __exit;       
    }

    num = snprintf(str, MAX_SIZE, "%d", state);
    ret = write(fd_state, str, num);
    if(ret < 0) {
        printf("fd_state write failed\n");
        goto __exit;       
    }

__exit:
    close(fd_state);
    close(fd_peroid);
    close(fd_duty);
    return ret;
}

int main(int argc, char *argv[])
{
    int fd, ret;
    int peroid, duty;

    fd = open(PWM_EXPORT, O_WRONLY);
    if (fd < 0) {
        printf("export open failed\n");
        return -1;
    }

    ret = write(fd, "0", strlen("0"));
    if (ret < 0) {
        printf("export failed\n");
        return -1;
    }
    close(fd);
    
    if (argc > 2) {   
        peroid = atoi(argv[1]);
        duty = atoi(argv[2]);
        pwm_setup(1, peroid, duty);
    }
    
    return 0;
}