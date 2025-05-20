#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

/*
SIGUSR1 用户定义的信号，可用于应用程序
SIGUSR2 另一个用户定义的信号，可用于应用程序
SIGALRM 当用alarm函数或settimer设置的定时器超时，产生此信号

kill -USR1 <id> 发送SIGUSR1信号
kill -(signo) <id> 向指定进程发送指定信号
*/

static int count = 0;
static void alarm_handler_callback(int signo)
{
    printf("alarm_handler_callback:%d\n", ++count);
}

static void virtual_alarm_callback(int signo)
{
    printf("virtual_alarm_callback:%d\n", signo);
}

static void signal_usr_callback(int signo) 
{
    switch (signo)
    {
    case SIGUSR1:
        printf("sigusr1 action\n");
        break;
    case SIGUSR2:
        printf("sigusr2 action\n");
        break;   
    default:
        break;
    }
}

int main(int argc, char* argv[])
{
    struct itimerval tick = {0};
    
    signal(SIGALRM, alarm_handler_callback);
    signal(SIGVTALRM, virtual_alarm_callback);
    signal(SIGUSR1, signal_usr_callback);
    signal(SIGUSR2, signal_usr_callback);

    kill(getpid(), SIGUSR1);
    raise(SIGUSR2);
    alarm(1);

    pause();

    printf("start setitimer test!\n");

    // 设置定时器初次触发时间为1秒，间隔时间也为1秒
    tick.it_value.tv_sec = 1;
    tick.it_value.tv_usec = 0;
    tick.it_interval.tv_sec = 1;
    tick.it_interval.tv_usec = 0;

    /*
    ITIMER_REAL 系统真实的时间来计算，它送出SIGALRM信号
    ITIMER_VIRTUAL 以该进程在用户态下花费的时间来计算，它送出SIGVTALRM信号。
    ITIMER_PROF 以该进程在用户态下和内核态下所费的时间来计算，它送出SIGPROF信号
    */
    if (setitimer(ITIMER_REAL, &tick, NULL) < 0) {
        printf("Set timer failed!\n");
    }

    while (1) {
        pause();
    }
    return 0;
}
