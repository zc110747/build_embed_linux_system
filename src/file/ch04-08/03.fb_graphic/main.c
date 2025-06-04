
#include "fb_graphic.h"

#define FB_DEVICE        "/dev/fb0"

int lcd_fill_color(FB_INFO *p_info, uint32_t start_x, uint32_t end_x,
    uint32_t start_y, uint32_t end_y,
    uint32_t color)
{
    unsigned long temp = 0;
    size_t x, y;

    //起始显存地址偏移(指针已经考虑了每个像素点的字节数)
    temp = p_info->screen_size*p_info->page;
    temp += start_y * p_info->width;

    for (y = start_y; y < end_y; y++) {
        for (x = start_x; x < end_x; x++) {
            p_info->fbp[temp + x] = color;
        }
        temp += p_info->width;
    }    
}

void lcd_draw_line(FB_INFO *p_info, uint32_t x, uint32_t y, int dir, uint32_t length, uint32_t color)
{
    uint32_t end;
    unsigned long temp = 0;
    
    //起始显存地址偏移(指针已经考虑了每个像素点的字节数)
    temp = p_info->screen_size*p_info->page;

    // 定位到起点
    temp += y * p_info->width + x; 
    if (dir) {  // 水平线
        end = x + length - 1;
        if (end >= p_info->width) {
            end = p_info->width - 1;
        }

        for (; x <= end; x++, temp++) {
            p_info->fbp[temp] = color;
        }
    } else {  // 垂直线
        end = y + length - 1;
        if (end >= p_info->height) {
            end = p_info->height - 1;
        }

        for (; y <= end; y++, temp += p_info->width) {
            p_info->fbp[temp] = color;
        }
    }
}

void lcd_draw_rectangle(FB_INFO *p_info, uint32_t start_x, uint32_t start_y,
                        uint32_t end_x, uint32_t end_y,
                        uint32_t color)
{
    int x_len = end_x - start_x + 1;
    int y_len = end_y - start_y - 1;

    lcd_draw_line(p_info, start_x,  start_y,     1, x_len, color); // 上边
    lcd_draw_line(p_info, start_x,  end_y,       1, x_len, color); // 下边
    lcd_draw_line(p_info, start_x,  start_y + 1, 0, y_len, color); // 左边
    lcd_draw_line(p_info, end_x,    start_y + 1, 0, y_len, color); // 右边
}

void lcd_draw_antialiased_circle(FB_INFO *p_info, uint32_t center_x, uint32_t center_y,
                                int radius, uint32_t color) 
{
    unsigned long temp = 0;
    int x, y;

    //起始显存地址偏移(指针已经考虑了每个像素点的字节数)
    temp = p_info->screen_size*p_info->page;

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

                if (screen_x >= 0 && screen_x < p_info->width && screen_y >= 0 && screen_y < p_info->height) {
                    unsigned long dest_index = temp + screen_y * p_info->width + screen_x;
                    uint32_t dest_color = p_info->fbp[dest_index];
                    uint32_t dest_r = (dest_color >> 16) & 0xFF;
                    uint32_t dest_g = (dest_color >> 8) & 0xFF;
                    uint32_t dest_b = dest_color & 0xFF;

                    uint32_t final_r = (uint32_t)(dest_r * (1 - alpha) + src_r * alpha);
                    uint32_t final_g = (uint32_t)(dest_g * (1 - alpha) + src_g * alpha);
                    uint32_t final_b = (uint32_t)(dest_b * (1 - alpha) + src_b * alpha);

                    p_info->fbp[dest_index] = 0xFF<<24 | (final_r << 16) | (final_g << 8) | final_b;
                }
            }
        }
    }
}

void lcd_switch_hw_page(FB_INFO *p_info)
{
    // 硬件切换到当前页
    p_info->fb_var.yoffset = p_info->height*p_info->page;
    ioctl(p_info->fd, FBIOPAN_DISPLAY, &(p_info->fb_var));
    
    p_info->page += 1;
    if (p_info->page == FB_PAGE_NUM) {
        p_info->page = FB_PAGE_0;
    }
}

int main(int argc, char *argv[])
{
    FB_INFO fb_info;

    fb_info.fd = open(FB_DEVICE, O_RDWR); 
    if (fb_info.fd < 0) {
        perror("open fb0 failed\n");
        return -1;
    }

    // 获取显存信息
    if (ioctl(fb_info.fd, FBIOGET_VSCREENINFO, &fb_info.fb_var) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO failed\n");
        close(fb_info.fd);
        return -1;
    }

    if (ioctl(fb_info.fd, FBIOGET_FSCREENINFO, &fb_info.fb_fix) < 0) {
        perror("ioctl FBIOGET_FSCREENINFO failed\n");
        close(fb_info.fd);
        return -1;
    }

    printf("xres:%d, yres:%d\n", fb_info.fb_var.xres, fb_info.fb_var.yres);
    printf("bits_per_pixel:%d, line_length:%d\n", fb_info.fb_var.bits_per_pixel, fb_info.fb_fix.line_length);
    
    fb_info.width = fb_info.fb_var.xres;
    fb_info.height = fb_info.fb_var.yres;
    fb_info.screen_size = fb_info.width * fb_info.height;
    fb_info.page_size = fb_info.screen_size * fb_info.fb_var.bits_per_pixel / 8;

    // 申请fbp图形对应缓存，并映射到用户空间
    fb_info.fbp = (uint32_t *)mmap(NULL, fb_info.page_size*FB_PAGE_NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fb_info.fd, 0);
    if (fb_info.fbp == MAP_FAILED) {
        perror("mmap gui memory failed\n");
        close(fb_info.fd);
        return -1;
    }

    printf("start fill color...\n");

    for (int i=0; i<5; i++)
    {
        lcd_fill_color(&fb_info, 0, fb_info.width, 0, fb_info.height, 0xFFFF0000);
        lcd_switch_hw_page(&fb_info);
        sleep(1);
        lcd_fill_color(&fb_info, 0, fb_info.width, 0, fb_info.height, 0xFF000000);
        lcd_draw_rectangle(&fb_info, fb_info.width/2-40*(i+1), fb_info.height/2-40*(i+1), fb_info.width/2+40*(i+1), fb_info.height/2+40*(i+1), 0xFFFFFFFF);
        lcd_draw_rectangle(&fb_info, fb_info.width/2-50*(i+1), fb_info.height/2-50*(i+1), fb_info.width/2+50*(i+1), fb_info.height/2+50*(i+1), 0xFFFFFFFF);
        lcd_draw_antialiased_circle(&fb_info, fb_info.width/2, fb_info.height/2, 200, 0xFFFFFFFF);
        lcd_draw_string(&fb_info, 0, 0, "Hello World!", 0xFFFFFFFF);
        lcd_draw_string(&fb_info, 0, 12, "This is for fb test!", 0xFFFFFFFF);
        lcd_switch_hw_page(&fb_info);
        sleep(1);
        lcd_fill_color(&fb_info, 0, fb_info.width/4, 0, fb_info.height, 0xFFFF0000);
        lcd_fill_color(&fb_info, fb_info.width/4, fb_info.width/2, 0, fb_info.height, 0xFF00FF00);
        lcd_fill_color(&fb_info, fb_info.width/2, fb_info.width*3/4, 0, fb_info.height, 0xFF0000FF);
        lcd_fill_color(&fb_info, fb_info.width*3/4, fb_info.width, 0, fb_info.height, 0xFFFFFF00);
        lcd_switch_hw_page(&fb_info);
        sleep(1);
    }

    //释放申请的缓存
    munmap(fb_info.fbp, fb_info.page_size*FB_PAGE_NUM);
    close(fb_info.fd);
}