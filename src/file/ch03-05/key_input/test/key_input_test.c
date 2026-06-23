////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      key_test.cpp
//          GPIO1_18
//
//  Purpose:
//     测试按键输入
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      4/3/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sys/select.h>

#define KEY_DEV_NAME "/dev/input/event0"

static int check_button_pressed(int fd) {

    /* wait button being pressed or released. */
    fd_set input;

    if (fd < 0) {
        return -1;
    }

    FD_ZERO(&input);
    FD_SET(fd, &input);
    int ret = select(fd + 1, &input, NULL, NULL, NULL);
    if (ret < 0) {
        printf("%s", strerror(errno));
        return -1;
    }

    /* read event */
    struct input_event buf;
    if (read(fd, &buf, sizeof(struct input_event)) < 0) {
        printf("%s", strerror(errno));
        return -1;
    }

    switch (buf.type) {
        case EV_KEY:
            if (buf.code < BTN_MISC) {
                printf("key %d %s\n", buf.code, buf.value ? "press" : "release");
            } else {
                printf("button %d %s\n", buf.code, buf.value ? "press" : "release");
            }
            break;
        case EV_REL:
            break;
        case EV_ABS:
            break;
        case EV_MSC:
            break;
        case EV_SW:
            break;
        case EV_SYN:
            printf("sync\n");
            buf.code = 0;
            break;
        default:
            break;
    }
    return buf.code;
}

int main(int argc, const char *argv[])
{
    unsigned char val = 1;
    int fd;
    struct input_event inputevent;

    fd = open(KEY_DEV_NAME, O_RDONLY);
    if (fd < 0) {
        printf("%s open failed!\n", KEY_DEV_NAME);
        return -1;
    } else {
        while (1) {
            int key_code = check_button_pressed(fd);
            if (key_code <= 0)
                continue;
            else {
                printf("key code:%d\n", key_code);
            }
        }
    }

    close(fd);
    return 0;
}
