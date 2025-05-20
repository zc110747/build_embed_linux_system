/**********************************************************
 * 说明: 消息队列的运用(服务器端)
 * 消息队列可以用于所有进程间的数据通讯，即使是非公共祖先，
 * 消息队列是消息的列表，存储在内核中
 * msgget 创建消息队列
 * msgrcv 从消息队列中读取数据
 * msgsnd  向消息队列中写入数据
 * msgctrl 操作消息队列接口
 * 删除消息的脚本:
 * ipcs -q | grep '^[0-9]' | awk '{system("ipcrm msg "$2"")}'
 * 消息队列代码在WSL上测试不成功，WSL不支持该接口，
 * WSL2上以修改该bug
************************************************************/
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> //for strerror

#define MESSAGE_QUEUE_ID  2345

struct msgsys
{
    long mtype;
    int size;
    char text[512];
};

void update_msg(struct msgsys *msg, long mtype, char *text, int size)
{
    msg->mtype = mtype;
    msg->size = size;
    memcpy(msg->text, text, size);
}

int main(int argc, char *argv)
{
    int  mq_id;   
    struct msgsys msg_val;
    int msg_size;
    pid_t system_pid;

    system_pid = fork();
    if (system_pid < 0) {
        printf("system fork error\n");
    } else if (system_pid == 0) {  
        //msg queue server
        mq_id = msgget((key_t)MESSAGE_QUEUE_ID, 0666 | IPC_CREAT);
        if (mq_id < 0) {
            printf("msg queue create failed, error:%s", strerror(errno));
            return -1;
        }

        //type: 0返回消息队列中的第一条消息，>0返回队列中mtype等于该值的消息
        //<0返回mtype绝对值最小的消息
        if ((msg_size = msgrcv(mq_id, (void *)&msg_val, BUFSIZ, 0, 0)) >= 0) {
            printf("server queue recv data: %s, size:%d\n", msg_val.text, msg_size);
            msg_val.text[msg_size] = '\0';
            update_msg(&msg_val, 1, "server recv queue msg", strlen("server recv queue msg"));
            msgsnd(mq_id, (void *)&msg_val, msg_size, 0);
        }

        sleep(1);

        //控制删除消息队列
        msgctl(mq_id, IPC_RMID, 0);
    } else {
        //msg queue client
        sleep(1);
        
        //申请消息队列
        mq_id = msgget((key_t)MESSAGE_QUEUE_ID, 0666);
        if (mq_id < 0) {
            printf("msg queue request failed");
            return -1;
        }

        update_msg(&msg_val, 1, "client send queue msg", strlen("client send queue msg"));

        //消息数据发送  
        msgsnd(mq_id, (void *)&msg_val, msg_val.size+8, 0);
        if ((msg_size = msgrcv(mq_id, (void *)&msg_val, BUFSIZ, 0, 0)) != -1) {
            msg_val.text[msg_size] = '\0';
            printf("client recv data: %s, size:%d\n", msg_val.text, msg_size);
        }
    }

    printf("msg queue test end\n");
    return 0;
}