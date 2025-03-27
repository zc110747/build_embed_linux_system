/*
 * 多进程接口
 * fork 系统调用创建和原进程资源一致的的新进程(会复制资源重新分配)
 * exec 系列函数用于在进程上下文执行一个新程序，包含execl, execv, execle, execve, execlp和execvp.
 * wait和waitpid, 这两个函数用于让父进程等待子进程的结束。wait()函数会阻塞父进程，直到它的一个子进程结束。waitpid()函数提供了更多的选项，允许父进程等待特定的子进程或一组子进程
 * getpid/getppid 获取当前进程/父进程的pid信息
 * nice/setpriority 这些函数用于设置进程的调度优先级
 * exit()和_exit() 这两个函数用于终止进程。exit()函数在退出前会执行一些清理工作（如调用已注册的atexit()函数），而_exit()函数则立即终止进程
 * kill()：发送信号到进程。可以用来终止进程、请求进程执行某些操作等。
 * signal()和sigaction()：用于设置进程接收到特定信号时的处理动作
*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>