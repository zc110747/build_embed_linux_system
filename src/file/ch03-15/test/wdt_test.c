/*
 * File      : wdt_test.c
 * test for key driver.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/watchdog.h>
#include <stdlib.h>
#include <string.h>

static unsigned int flag = 0;

#define WATCHDOG_DEVICE "/dev/watchdog1"

int main(int argc, char *argv[])
{
    int wdt_fd;
    int timeout = 5;
    int ioctl_alive = 0;
    int ops;

    if (argc > 1) {
        ioctl_alive = atoi(argv[1]);
    }
    printf("ioctl_alive:%d\n", ioctl_alive);

    // open会直接开启对应的watchdog
    wdt_fd = open(WATCHDOG_DEVICE, O_RDWR | O_NONBLOCK);

    if (wdt_fd >= 0)
    {
        ioctl(wdt_fd, WDIOC_SETTIMEOUT, &timeout);

        while (!flag)
        {
            if (ioctl_alive == 0) {
                ioctl(wdt_fd, WDIOC_KEEPALIVE, NULL);
            }

            sleep(1);
        }

        // 关闭watchdog设备(需要驱动支持)
        ops = WDIOS_DISABLECARD;
        ioctl(wdt_fd, WDIOC_SETOPTIONS, &ops);
        close(wdt_fd);
    }
    
    return 0;
}
