
#include "fb_draw.hpp"

bool fb_manage::init(const char *dev) 
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

    printf("xres:%d, yres:%d\n", fb_var_.xres, fb_var_.yres);
    printf("bits_per_pixel:%d, line_length:%d\n",fb_var_.bits_per_pixel, fb_fix_.line_length);
    
    width_ = fb_var_.xres;
    height_ = fb_var_.yres;
    screen_size_ = width_*height_;
    page_size_ = screen_size_*fb_var_.bits_per_pixel/8;

    fbp_ = (uint32_t *)mmap(NULL, page_size_*FB_PAGE_NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fb_, 0);
    if (fbp_ == nullptr) {
        perror("mmap failed");
        release();
        return false;
    }

    return true; 
}

void fb_manage::release(void) 
{
    if (fbp_ != nullptr) {
        munmap(fbp_, page_size_*FB_PAGE_NUM);
        fbp_ = nullptr;
    }

    if (fb_ >= 0) {
        close(fb_);
        fb_ = -1;
    }
}

void fb_manage::lcd_switch_hw_page(void)
{
    // 硬件切换到当前页
    fb_var_.yoffset = height_*page_;
    ioctl(fb_, FBIOPAN_DISPLAY, &fb_var_);
    
    page_++;
    if (page_ == FB_PAGE_NUM) {
        page_ = FB_PAGE_0;
    }
}

bool fb_manage::lcd_fill(uint32_t start_x, uint32_t end_x,
    uint32_t start_y, uint32_t end_y,
    uint32_t color)
{
        unsigned long temp = 0;
        uint32_t x;

        // 起始显存地址偏移，第二块显存地址偏移
        temp = screen_size_*page_;
        temp += start_y * width_;

        for (; start_y < end_y; start_y++) {
            for (x = start_x; x < end_x; x++) {
                fbp_[temp + x] = color;
            }
            temp += width_;
        }
        return true;
}

void fb_manage::lcd_draw_antialiased_circle(uint32_t center_x, uint32_t center_y,
                                int radius, uint32_t color) 
{
    unsigned long temp = 0;
    int x, y;

    // 起始显存地址偏移，第二块显存地址偏移
    temp = screen_size_*page_;

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

                    fbp_[dest_index] = 0xFF<<24 | (final_r << 16) | (final_g << 8) | final_b;
                }
            }
        }
    }
}

void fb_manage::lcd_draw_line(uint32_t x, uint32_t y, int dir,
                            uint32_t length, uint32_t color)
{
    uint32_t end;
    unsigned long temp = 0;

    // 起始显存地址偏移，第二块显存地址偏移
    temp = screen_size_*page_;

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

void fb_manage::lcd_draw_rectangle(uint32_t start_x, uint32_t start_y,
    uint32_t end_x, uint32_t end_y,
    uint32_t color)
{
    int x_len = end_x - start_x + 1;
    int y_len = end_y - start_y - 1;

    lcd_draw_line(start_x,  start_y,     1, x_len, color); // 上边
    lcd_draw_line(start_x,  end_y,       1, x_len, color); // 下边
    lcd_draw_line(start_x,  start_y + 1, 0, y_len, color); // 左边
    lcd_draw_line(end_x,    start_y + 1, 0, y_len, color); // 右边
}

void fb_manage::lcd_draw_point(uint32_t x, uint32_t y, uint32_t color)
{
    unsigned long temp = 0;

    // 起始显存地址偏移，第二块显存地址偏移
    temp = screen_size_*page_;

    fbp_[temp + y * width_ + x] = color;
}
