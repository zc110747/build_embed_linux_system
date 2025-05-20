#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

static int index = 0;
sem_t semaphore;

void *thread_loop_func0(void *arg) 
{ 
    for (int i = 0; i < 10; i++) {
        sem_wait(&semaphore);
        index += 1;
        printf("thread0 index = %d\n", index);
        sem_post(&semaphore); 
        usleep(100);    
    }

    pthread_exit(NULL); //线程退出(非阻塞)
}

void *thread_loop_func1(void *arg) 
{ 
    for (int i = 0; i < 10; i++) {
        sem_wait(&semaphore);
        index += 1;
        printf("thread1 index = %d\n", index);
        sem_post(&semaphore);
        usleep(100);    
    }

    pthread_exit(NULL); //线程退出(非阻塞)
}


int main(int argc, char *argv[]) 
{
    pthread_t thread0, thread1;

    // 初始化信号量，初始值为 1
    if (sem_init(&semaphore, 0, 1) != 0) {
        perror("sem_init");
        return 1;
    }

    pthread_create(&thread0, NULL, thread_loop_func0, NULL);
    pthread_create(&thread1, NULL, thread_loop_func1, NULL);

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    // 删除信号量集
    printf("index = %d\n", index);
    if (sem_destroy(&semaphore) != 0) {
        perror("sem_destroy");
        return 1;
    }

    return 0;
}
