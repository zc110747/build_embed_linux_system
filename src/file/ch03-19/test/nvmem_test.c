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
    char w_buffer[64];
    char r_buffer[64] = {0};
    int fd;
    int ret;

    fd = open(NVMEM_DEVICE_FILE, O_RDWR | O_NDELAY);
    if (fd == -1) {
        printf("open %s error", NVMEM_DEVICE_FILE);
        return -1;
    }

    // 写入数据
    for (int i = 0; i < 64; i++) {
        w_buffer[i] = i;
    }
    lseek(fd, 100, SEEK_SET);
    ret = write(fd, w_buffer, 64);
    if (ret < 0) {
        printf("write failed:%d\n", ret);
        goto __exit;
    }

    // 读取数据
    lseek(fd, 100, SEEK_SET);
    ret = read(fd, r_buffer, sizeof(r_buffer));
    if (ret < 0) {
        printf("read failed:%d\n", ret);
        goto __exit;
    }

    // 比较数据
    for (int i = 0; i < 64; i++) {
        if (r_buffer[i] != w_buffer[i]) {
            printf("no equal, %d:%d\n", r_buffer[i], w_buffer[i]);
            goto __exit;
        }
    }

    printf("nvmem test success\n");

__exit:
    close(fd); 
    return 0;
}