#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <math.h>
#include <stdint.h>

#define FB_DEVICE        "/dev/fb0"

static size_t width_, height_, screen_size_;
static struct fb_var_screeninfo fb_var_;
static struct fb_fix_screeninfo fb_fix_;

int lcd_fill_color(uint32_t *fbp_, uint32_t start_x, uint32_t end_x,
    uint32_t start_y, uint32_t end_y,
    uint32_t color)
{
    unsigned long temp = 0;
    size_t x, y;

    // 指针为4字节，已经考虑长度
    temp += start_y * width_;
    for (y = start_y; y < end_y; y++) {
        for (x = start_x; x < end_x; x++) {
            fbp_[temp + x] = color;
        }
        temp += width_;
    }    
}

int main(int argc, char *argv[])
{
    int fb_;
    uint32_t *fbp_;

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
    
    width_ = fb_var_.xres;
    height_ = fb_var_.yres;
    screen_size_ = fb_fix_.line_length * height_;

    fbp_ = (uint32_t *)mmap(NULL, screen_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fb_, 0);
    if (fbp_ == MAP_FAILED) {
        perror("mmap failed\n");
        close(fb_);
        return -1;
    }

    printf("start fill color...\n");
    lcd_fill_color(fbp_, 0, width_, 0, height_, 0xFF00FF00);

    sleep(1);

    munmap(fbp_, screen_size_);
    close(fb_);
}