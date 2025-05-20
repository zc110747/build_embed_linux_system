#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define KEY         1234
#define SEM_NUM     1
static int index = 0;

// semaphore_p 操作
void semaphore_p(int semid) {
    struct sembuf sop = {0, -1, 0};
    semop(semid, &sop, 1);
}

// semaphore_v 操作
void semaphore_v(int semid) {
    struct sembuf sop = {0, 1, 0};
    semop(semid, &sop, 1);
}

void *thread_loop_func0(void *arg) { 
    int semid = *(int *)arg;

    for (int i = 0; i < 10; i++) {
        semaphore_p(semid);
        index += 1;
        printf("thread0: index = %d\n", index);
        semaphore_v(semid);
        usleep(100);
    }
}

void *thread_loop_func1(void *arg) { 
    int semid = *(int *)arg;

    for (int i = 0; i < 10; i++) {
        semaphore_p(semid);
        index += 1;
        printf("thread1: index = %d\n", index);
        semaphore_v(semid);
        usleep(100);       
    }
}

int main(int argc, char *argv[]) 
{
    int semid;
    pid_t pid;
    pthread_t thread0, thread1;

    // 创建信号量集
    semid = semget(KEY, SEM_NUM, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    // 初始化信号量值为 1
    if (semctl(semid, 0, SETVAL, 1) == -1) {
        perror("semctl");
        return 1;
    }

    pthread_create(&thread0, NULL, thread_loop_func0, &semid);
    pthread_create(&thread1, NULL, thread_loop_func1, &semid);

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    // 删除信号量集
    printf("index = %d\n", index);
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
        return 1;
    }

    return 0;
}
