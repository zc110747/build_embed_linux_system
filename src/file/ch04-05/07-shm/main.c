#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define SHM_SIZE    1024        // 共享内存大小
#define SHM_KEY     0x1234      // 共享内存键值
sem_t semaphore;

void *thread_shm_read(void *arg)
{
    int shmid = *(int *)arg;

    // 连接共享内存
    char *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (char *)-1) {
        perror("shmat");
        return NULL;
    }

    // 读取共享内存中的数据
    for (int i = 0; i < 26; i++) {
        sem_wait(&semaphore);
        printf("Shared memory data: %s\n", shmaddr);
    }

    // 断开共享内存连接
    if (shmdt(shmaddr) == -1) {
        perror("shmdt");
        return NULL;
    }
    pthread_exit(NULL);
}

void *thread_shm_write(void *arg)
{
    int shmid = *(int *)arg;
    char buffer[1024] = {0};

    // 连接共享内存
    char *shmaddr = shmat(shmid, NULL, 0);

    // 写入共享内存中的数据
    for (int i = 0; i < 26; i++) {
        sem_post(&semaphore);
        shmaddr[i] = 'a' + i;
        shmaddr[i+1] = '\0';
        sleep(1);
    }

    // 断开共享内存连接
    if (shmdt(shmaddr) == -1) {
        perror("shmdt");
        return NULL;
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int shmid;
    pthread_t tid1, tid2;

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // 初始化信号量，初始值为0
    if (sem_init(&semaphore, 0, 0) != 0) {
        perror("sem_init");
        return 1;
    }

    // 创建线程
    pthread_create(&tid1, NULL, thread_shm_read, &shmid);
    pthread_create(&tid2, NULL, thread_shm_write, &shmid);

    // 等待线程结束
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // 删除共享内存
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    if (sem_destroy(&semaphore) != 0) {
        perror("sem_destroy");
        return 1;
    }
}

