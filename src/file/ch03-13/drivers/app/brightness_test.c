//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      brightness_test.cpp
//
//  Purpose:
//     brightness测试代码
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      7/30/2024   Create New Version
//
/////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BRIGHTNESS          "/sys/class/backlight/usr-backlight/brightness"
#define ACTUAL_BRIGHTNESS   "/sys/class/backlight/usr-backlight/actual_brightness"

// 读取文件内容的函数
int read_file(const char *filename) 
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }

    int value;
    if (fscanf(file, "%d", &value) != 1) {
        perror("Failed to read value");
        fclose(file);
        return -1;
    }

    fclose(file);
    return value;
}


// 写入亮度值到文件的函数
int write_brightness(int brightness)
{
    FILE *file = fopen(BRIGHTNESS, "w");
    if (file == NULL) {
        perror("Failed to open brightness file for writing");
        return -1;
    }

    if (fprintf(file, "%d", brightness) < 0) {
        perror("Failed to write brightness value");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[]) 
{
    int set_brightness = 3;

    if (argc == 2) {
        set_brightness = atoi(argv[1]);
    }

    // 设置背光等级
    if (write_brightness(set_brightness) == 0) {
        printf("Successfully set brightness to %d\n", set_brightness);
    }

    // 读取设置的背光等级
    int brightness = read_file(BRIGHTNESS);
    if (brightness != -1) {
        printf("Brightness: %d\n", brightness);
    }

    // 读取当前的背光等级
    int actual_brightness = read_file(ACTUAL_BRIGHTNESS);
    if (actual_brightness != -1) {
        printf("Actual Brightness: %d\n", actual_brightness);
    }

    return 0;
}