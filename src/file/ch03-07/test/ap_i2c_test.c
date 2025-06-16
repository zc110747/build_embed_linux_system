/*
 * File      : ap_i2c_test.c
 * test for ap i2c driver.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */
#include<fcntl.h>
#include<stdio.h>
#include<unistd.h>

#define I2C_DRIVER_NAME "/dev/ap3216"

int main(int argc, char *argv[])
{
    int fd;
    unsigned short databuf[3];
    unsigned short ir, als, ps;
    int nSize = 0;

    fd = open(I2C_DRIVER_NAME, O_RDWR);
    if (fd < 0)
    {
        printf("can't open file %s\r\n", I2C_DRIVER_NAME);
        return -1;
    }

    while (1)
    {
        nSize = read(fd, databuf, sizeof(databuf));
        if (nSize >= 0)
        {
            ir =  databuf[0]; /* ir传感器数据 */
            als = databuf[1]; /* als传感器数据 */
            ps =  databuf[2]; /* ps传感器数据 */
            printf("read size:%d\r\n", nSize);
            printf("ir = %d, als = %d, ps = %d\r\n", ir, als, ps);
        }
        usleep(200000); /*100ms */
    }

    close(fd);
    return 0;
}
