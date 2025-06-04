#ifndef FB_GRAPHIC_H
#define FB_GRAPHIC_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <math.h>
#include <stdint.h>

typedef enum
{
    FB_PAGE_0 = 0,
    FB_PAGE_1,
    FB_PAGE_NUM
}FB_PAGE;

typedef struct
{
    int fd;                             // 帧缓冲设备文件描述符
    uint32_t *fbp;                      // 显存起始地址
    FB_PAGE page;                       // 显存页
    size_t page_size;                   // 单个页面显存大小(width*height*bytes_per_pixel)
    size_t width;                       // 界面宽度
    size_t height;                      // 界面高度
    size_t screen_size;                 // 单个页面的尺寸(width*height)
    struct fb_var_screeninfo fb_var;    // 显存变量信息
    struct fb_fix_screeninfo fb_fix;    // 显存固定信息
}FB_INFO;

void lcd_draw_string(FB_INFO *p_info, uint32_t x, uint32_t y, char *p, uint32_t color);
#endif
