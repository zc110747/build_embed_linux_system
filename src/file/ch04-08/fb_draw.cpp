
#include "fb_draw.hpp"

bool fb_manage::init(const char *dev, uint8_t page_num) 
{
    fb_ = open(dev, O_RDWR); 
    if (fb_ < 0) {
        perror("open fb0 failed");
        return false;
    }

    if (ioctl(fb_, FBIOGET_VSCREENINFO, &fb_var_) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO failed");
        return false;
    }

    if (ioctl(fb_, FBIOGET_FSCREENINFO, &fb_fix_) < 0) {
        perror("ioctl FBIOGET_FSCREENINFO failed");
        return false;
    }

    printf("xres*yres:%d*%d-%d\n" 
        "bits_per_pixel:%d\n"
        "line_length:%d\n"
        "ywrapstep:%d\n"
        "ypanstep:%d\n",
        fb_var_.xres,
        fb_var_.yres, fb_var_.yres_virtual,
        fb_var_.bits_per_pixel,
        fb_fix_.line_length,
        fb_fix_.ywrapstep,
        fb_fix_.ypanstep);
    
    width_ = fb_var_.xres;
    height_ = fb_var_.yres;
    screen_size_ = fb_fix_.line_length * fb_var_.yres;

    fbp_ = (uint32_t *)mmap(NULL, screen_size_*page_num + 4, PROT_READ | PROT_WRITE, MAP_SHARED, fb_, 0);
    if (fbp_ == nullptr) {
        perror("mmap failed");
        release();
        return false;
    }

    // 修改虚拟显存大小，实现双屏显示
    fb_var_.yres_virtual = fb_var_.yres * page_num;
    if (ioctl(fb_, FBIOPUT_VSCREENINFO, &fb_var_) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO failed");
        return false;
    }
    return true; 
}

void fb_manage::release(void) 
{
    if (fbp_ != nullptr) {
        munmap(fbp_, screen_size_);
        fbp_ = nullptr;
    }

    if (fb_ >= 0) {
        close(fb_);
        fb_ = -1;
    }
}

void fb_manage::lcd_set_page(fb_manage::CACHE_PAGE page) {

    int ret;

    if (page == CACHE_PAGE_1) {
        fb_var_.yoffset = height_;
        ret = ioctl(fb_, FBIOPAN_DISPLAY, &fb_var_);
        if (ret < 0) {
            printf("set page failed:%d\n", ret);
            fprintf(stderr, "Error code: %d\n", errno);
        }
    } else {
        fb_var_.yoffset = 0;
        ioctl(fb_, FBIOPAN_DISPLAY, &fb_var_);           
    }
    printf("set page offset:%d, %d\n", fb_var_.xoffset, fb_var_.yoffset);
}

bool fb_manage::lcd_fill(fb_manage::CACHE_PAGE page, uint32_t start_x, uint32_t end_x,
    uint32_t start_y, uint32_t end_y,
    uint32_t color)
{
        unsigned long temp = 0;
        uint32_t x;

        if (fbp_ == nullptr) {
            perror("lcd_fill failed, fbp_ is nullptr");
            return false;
        }

        if (page == fb_manage::CACHE_PAGE_1) {
            temp = width_*height_;  //起始显存地址偏移，第二块显存地址偏移
        }

        temp += start_y * width_;
        for (; start_y <= end_y; start_y++) {
            temp += width_;
            for (x = start_x; x <= end_x; x++) {
                fbp_[temp + x] = color;
            }
        }
        return true;
}

void fb_manage::lcd_draw_antialiased_circle(fb_manage::CACHE_PAGE page, uint32_t center_x, uint32_t center_y,
                                int radius, uint32_t color) 
{
    unsigned long temp = 0;
    int x, y;

    if (page == CACHE_PAGE_1) {
        temp = width_ * height_;  // 起始显存地址偏移，第二块显存地址偏移
    }

    for (y = -radius; y <= radius; y++) {
        for (x = -radius; x <= radius; x++) {
            float dist = sqrtf(x * x + y * y);

            if (dist <= radius + 0.6f) {
                float alpha = 1.8f - fabsf(dist - radius);
                if (alpha < 0) alpha = 0;
                if (alpha > 1) alpha = 1;

                uint32_t src_r = (color >> 16) & 0xFF;
                uint32_t src_g = (color >> 8) & 0xFF;
                uint32_t src_b = color & 0xFF;

                int screen_x = center_x + x;
                int screen_y = center_y + y;

                if (screen_x >= 0 && screen_x < width_ && screen_y >= 0 && screen_y < height_) {
                    unsigned long dest_index = temp + screen_y * width_ + screen_x;
                    uint32_t dest_color = fbp_[dest_index];
                    uint32_t dest_r = (dest_color >> 16) & 0xFF;
                    uint32_t dest_g = (dest_color >> 8) & 0xFF;
                    uint32_t dest_b = dest_color & 0xFF;

                    uint32_t final_r = (uint32_t)(dest_r * (1 - alpha) + src_r * alpha);
                    uint32_t final_g = (uint32_t)(dest_g * (1 - alpha) + src_g * alpha);
                    uint32_t final_b = (uint32_t)(dest_b * (1 - alpha) + src_b * alpha);

                    fbp_[dest_index] = (final_r << 16) | (final_g << 8) | final_b;
                }
            }
        }
    }
}

void fb_manage::lcd_draw_line(fb_manage::CACHE_PAGE page, uint32_t x, uint32_t y, int dir,
                                uint32_t length, uint32_t color)
{
    uint32_t end;
    unsigned long temp = 0;

    if (page == fb_manage::CACHE_PAGE_1) {
        temp = width_*height_;       //起始显存地址偏移，第二块显存地址偏移
    }

    temp += y * width_ + x; // 定位到起点
    if (dir) {  // 水平线
        end = x + length - 1;
        if (end >= width_) {
            end = width_ - 1;
        }

        for (; x <= end; x++, temp++) {
            fbp_[temp] = color;
        }
    } else {  // 垂直线
        end = y + length - 1;
        if (end >= height_) {
            end = height_ - 1;
        }

        for (; y <= end; y++, temp += width_) {
            fbp_[temp] = color;
        }
    }
}

void fb_manage::lcd_draw_rectangle(fb_manage::CACHE_PAGE page, 
    uint32_t start_x, uint32_t start_y,
    uint32_t end_x, uint32_t end_y,
    uint32_t color)
{
    int x_len = end_x - start_x + 1;
    int y_len = end_y - start_y - 1;

    lcd_draw_line(page, start_x,  start_y,     1, x_len, color); // 上边
    lcd_draw_line(page, start_x,  end_y,       1, x_len, color); // 下边
    lcd_draw_line(page, start_x,  start_y + 1, 0, y_len, color); // 左边
    lcd_draw_line(page, end_x,    start_y + 1, 0, y_len, color); // 右边
}

void fb_manage::lcd_draw_point(fb_manage::CACHE_PAGE page,
    uint32_t x, uint32_t y, uint32_t color)
{
    unsigned long temp = 0;

    if (page == fb_manage::CACHE_PAGE_1) {
        temp = width_*height_;  //起始显存地址偏移，第二块显存地址偏移
    }

    fbp_[temp + y * width_ + x] = color;
}
