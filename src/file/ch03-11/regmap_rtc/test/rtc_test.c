/*
 * File      : key_test.c
 * test for key driver.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <poll.h>
#include <sys/stat.h>
#include <signal.h>

struct rtc_time rtc_time = {0};
struct rtc_wkalrm alarm_time = {0};
static unsigned int flag = 0;

#define RTC_DEVICE "/dev/rtc1"

static void alarm_signal_func(int signum)
{
    flag = 1;
    printf("alarm signal!\r\n");
}

int main(int argc, char *argv[])
{
    int device_fd;
    int retval, flags;

    device_fd = open(RTC_DEVICE, O_RDWR | O_NONBLOCK);

    if (device_fd>=0) {
        retval = ioctl(device_fd, RTC_RD_TIME, &rtc_time);
        if (retval >= 0) {
            printf("timer:%d:%d:%d\n", rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);
        }
        printf("retval:%d\n", retval);

        signal(SIGIO, alarm_signal_func);

        fcntl(device_fd, F_SETOWN, getpid());
        flags = fcntl(device_fd, F_GETFD);
        fcntl(device_fd, F_SETFL, flags | FASYNC);

        rtc_time.tm_min += 1;
        if (rtc_time.tm_min == 60) {
            rtc_time.tm_min = 0;
            rtc_time.tm_hour += 1;
        }
        alarm_time.enabled = 1;
        alarm_time.pending = 1;
        alarm_time.time = rtc_time;
        retval = ioctl(device_fd, RTC_WKALM_SET, &alarm_time);
        if (retval >= 0) {
            printf("set alarm success!\n");
        }
        printf("retval:%d\n", retval);   
    }

    while (!flag) {
        sleep(1);
    }

    close(device_fd);
    return 0;
}