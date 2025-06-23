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

#define FB_DEVICE        "/dev/fb0"

static size_t width_, height_, screen_size_;
static struct fb_var_screeninfo fb_var_;
static struct fb_fix_screeninfo fb_fix_;
static FB_PAGE page_ = FB_PAGE_1;

int lcd_fill_color(uint32_t *fbp_, uint32_t start_x, uint32_t end_x,
    uint32_t start_y, uint32_t end_y,
    uint32_t color)
{
    unsigned long temp = 0;
    size_t x, y;

    // 指针长度已经考虑4字节
    temp = width_*height_*page_;
    
    // 指针为4字节，已经考虑长度
    temp += start_y * width_;
    for (y = start_y; y < end_y; y++) {
        for (x = start_x; x < end_x; x++) {
            fbp_[temp + x] = color;
        }
        temp += width_;
    }    
}

void lcd_switch_hw_page(int fb)
{
    int ret = 0;

    // 界面切换到下一页
    fb_var_.yoffset = height_*page_;
    ret = ioctl(fb, FBIOPAN_DISPLAY, &fb_var_);
    if (ret < 0) {
        perror("ioctl() / FBIOPAN_DISPLAY");
    }

    // 等待vsync信号刷新
    ret = 0;
    ioctl(fb, FBIO_WAITFORVSYNC, &ret);
    if (ret < 0) {
        perror("ioctl() / FBIO_WAITFORVSYNC");
    }

    page_ += 1;
    if (page_ == FB_PAGE_NUM) {
        page_ = FB_PAGE_0;
    }
}

int main(int argc, char *argv[])
{
    int fb_;
    uint32_t *fbp_;

    // 1. 打开设备文件，获取UI显示信息
    fb_ = open(FB_DEVICE, O_RDWR); 
    if (fb_ < 0) {
        perror("open fb0 failed\n");
        return -1;
    }

    if (ioctl(fb_, FBIOGET_VSCREENINFO, &fb_var_) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO failed\n");
        close(fb_);
        return -1;
    }

    if (ioctl(fb_, FBIOGET_FSCREENINFO, &fb_fix_) < 0) {
        perror("ioctl FBIOGET_FSCREENINFO failed\n");
        close(fb_);
        return -1;
    }

    printf("xres:%d, yres:%d\n", fb_var_.xres, fb_var_.yres);
    printf("bits_per_pixel:%d, line_length:%d\n", fb_var_.bits_per_pixel, fb_fix_.line_length);
    
    // 2. 通过mmap将fb显存映射到用户空间, 这里映射两块缓存用于双缓冲显示
    width_ = fb_var_.xres;
    height_ = fb_var_.yres;
    screen_size_ = fb_fix_.line_length * height_;

    fbp_ = (uint32_t *)mmap(NULL, screen_size_*FB_PAGE_NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fb_, 0);
    if (fbp_ == MAP_FAILED) {
        perror("mmap failed\n");
        close(fb_);
        return -1;
    }

    printf("start fill color...\n");

    // 3. 双缓冲显示
    // 执行流程：
    // 填充缓冲1，显示切换到缓冲1，下一次填充切换到缓冲0
    // 填充缓冲0，显示切换到缓冲0，下一次填充切换到缓冲1
    // 重复流程
    page_ = FB_PAGE_1;
    for (int i=0; i<20; i++)
    {
        lcd_fill_color(fbp_, 0, width_, 0, height_, 0xFFFF0000);
        lcd_switch_hw_page(fb_);
        sleep(1);
        lcd_fill_color(fbp_, 0, width_, 0, height_, 0xFF00FF00);
        lcd_switch_hw_page(fb_);
        sleep(1);
        lcd_fill_color(fbp_, 0, width_, 0, height_, 0xFF0000FF);
        lcd_switch_hw_page(fb_);
        sleep(1);
    }

    // 4. 解除映射，关闭文件描述符
    munmap(fbp_, screen_size_*FB_PAGE_NUM);
    close(fb_);
}