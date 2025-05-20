#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    int fd[2];
    int ret, size;
    pid_t pid;
    char buf[512]= {0};

    // 初始化创建匿名管道
    ret = pipe(fd);
    if (ret < 0) {
        printf("pipe init failed\n");
    }

    // 创建子进程
    pid = fork();

    if (pid < 0) {
         printf("fork failed\n");
    } else if (pid == 0) {
        //child process
        //read pipe fd[0]
        size = read(fd[0], buf, sizeof(buf));
        if (size <= 0) {
            printf("read pipe failed\n");
        } else {
            buf[size] = '\0';
            printf("child read pipe success, readsize=%d, buf=%s\n", size, buf);
        } 

        //write pipe fd[1]
        if (write(fd[1], "test for pipe", strlen("test for pipe")) > 0) {
            wait(NULL);
        } else {
            printf("child write pipe failed\n");
        }

        close(fd[0]);
        close(fd[1]);
    }
    else
    {
        //father process
        //write pipe fd[1]
        if (write(fd[1], "child test for pipe", strlen("child test for pipe\n"))>0) {
            wait(NULL);
        } else {
            printf("father write pipe failed\n");
        }

        //read pipe fd[1]
        size = read(fd[0], buf, sizeof(buf));
        if (size <= 0) {
            printf("father read pipe failed\n");
        } else {
            buf[size] = '\0';
            printf("father read pipe success, readsize=%d, buf=%s\n", size, buf);
        }

        close(fd[0]);
        close(fd[1]);
    }
    
    printf("test pipe end\n");
    return 0;
}
