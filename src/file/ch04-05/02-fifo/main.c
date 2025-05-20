/**********************************************************
 * 说明: 用于进程间通讯的FIFO接口
 * FIFO指先进先出的管道，不过FIFO有名称，又被称为named pipe
 * mkfifo 创建FIFO
 * open   根据名称打开FIFO通道
 * write  向FIFO通道内写入数据
 * read   从FIFO通道内读取数据
 * close  关闭FIFO通道
 * unlink 删除FIFO通道
************************************************************/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define FIFO_NAME "/tmp/1.fifo"
#define FIFO_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define STR_TEST "hello world"

int main(int argc, char* argv[])
{
    pid_t system_pid;
    int fd;
    int bytes;
    char buffer[40] = {0};

    unlink(FIFO_NAME);
    if (mkfifo(FIFO_NAME, FIFO_MODE) < 0) {
        if (errno != EEXIST) {
            printf("create fifo %s error:%s\n", FIFO_NAME, strerror(errno));
        }
        return -1;
    }

    system_pid = fork();
    if (system_pid < 0) {
        printf("system fork error\n");
    }
    else if (system_pid == 0) {
        // child process
        // 打开fifo，以可读写的方式(O_RDONLY也可)
        fd = open(FIFO_NAME, O_RDWR);
        if (fd < 0) {
            printf("open fifo %s error:%s\n", FIFO_NAME, strerror(errno));
            return -1;
        }

        // 从fifo中读取数据
        bytes = read(fd, buffer, 40);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("fifo %d read:%s, read size:%d\n", fd, buffer, bytes);
            close(fd);
        } else {
            printf("fifo read %s error:%s\n", FIFO_NAME, strerror(errno));
        }
    } else {
        // 打开fifo，以可读写的方式(O_WRONLY也可)
        fd = open(FIFO_NAME, O_RDWR);
        if (fd < 0) {
            printf("open fifo %s error:%s\n", FIFO_NAME, strerror(errno));
            return -1;
        }

        // 向fifo中写入数据
        bytes = write(fd, STR_TEST, strlen(STR_TEST));
        printf("fifo %d send:%s, size:%d\n", fd, STR_TEST, bytes);
        close(fd);
    }

    printf("fifo test end\n");
    return 0;
}
