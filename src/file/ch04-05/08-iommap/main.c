#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#define FILE_SIZE 1024
#define FILE_NAME "mmap_file.txt"
sem_t semaphore;

void *thread_mmap_read(void *arg)
{
    int fd = *(int *)arg;
    char *map_addr;

    // 内存映射文件
    map_addr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_addr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    for (int i = 0; i < 26; i++) {
        sem_wait(&semaphore);
        printf("Read from shared memory: %s\n", map_addr);
    }

    munmap(map_addr, FILE_SIZE);
    pthread_exit(NULL);
}

void *thread_mmap_write(void *arg)
{
    int fd = *(int *)arg;
    char *map_addr;

    // 内存映射文件
    map_addr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_addr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    for (int i = 0; i < 26; i++) {
        map_addr[i] = 'a' + i;
        map_addr[i+1] = '\0';
        sem_post(&semaphore);
        usleep(100); // 等待100ms
    }

    munmap(map_addr, FILE_SIZE);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int fd;
    pthread_t tid1, tid2;

    // 打开或创建文件
    fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 扩展文件大小
    if (ftruncate(fd, FILE_SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    // 初始化信号量，初始值为0
    if (sem_init(&semaphore, 0, 0) != 0) {
        perror("sem_init");
        return 1;
    }

    // 创建线程
    pthread_create(&tid1, NULL, thread_mmap_read, &fd);
    pthread_create(&tid2, NULL, thread_mmap_write, &fd);

    // 等待线程结束
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // 关闭文件
    close(fd);

    if (sem_destroy(&semaphore) != 0) {
        perror("sem_destroy");
        return 1;
    }
}

