/*
 * File      : nvmem_test.c
 * test for key driver.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define NVMEM_DEVICE_FILE "/sys/devices/platform/usr_nvmem/eeprom"

int main(int argc, const char *argv[])
{
    char buffer[64];
    int fd;
    int size = 0;

    fd = open(NVMEM_DEVICE_FILE, O_RDWR | O_NDELAY);
    if (fd == -1) {
        printf("open %s error", NVMEM_DEVICE_FILE);
        return -1;
    }

    // 移动指针位置
    lseek(fd, 100, SEEK_SET);

    size = read(fd, buffer, sizeof(buffer));
    if (size >= 0) {
        printf("read size:%d\n", size);
    } else {
        printf("read failed:%d\n", size);
    }

    close(fd); 

    return 0;
}