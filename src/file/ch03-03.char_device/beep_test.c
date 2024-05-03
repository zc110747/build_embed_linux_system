/*
 * File      : beep_test.c
 * test for beep driver.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define DEVICE_BEEP     "/dev/miscbeep"

int main(int argc, const char *argv[])
{
    unsigned char val = 1;
    int fd;

    fd = open(DEVICE_BEEP, O_RDWR | O_NDELAY);
    if (fd == -1)
    {
        printf("open %s error\r\n", DEVICE_BEEP);
        return -1;
    }

    if (argc > 1)
    {
        val = atoi(argv[1]);
    }

    write(fd, &val, 1);

    close(fd); 

    exit(0);  
}
