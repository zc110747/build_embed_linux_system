
#include "fb_draw.hpp"
#include "bmp_image.hpp"

int main(void)
{
    fb_manage fbm;
    bool res;
    uint8_t index;
    uint32_t width, height;

    res = fbm.init(FB_DEVICE, 2);
    if (res == false) {
        return -1;
    }

    width = fbm.get_width();
    height = fbm.get_height();

    // 绘制第一块显存
    fbm.lcd_fill(fb_manage::CACHE_PAGE_0, 0, width-1, 0, height-1, 0x00000000);
    for (index=0; index<10; index++) {
        fbm.lcd_draw_rectangle(fb_manage::CACHE_PAGE_0, width/2-index*20, height/2-index*20, width/2+index*20, height/2+index*20, 0x00FFFFFF);
    }
    fbm.lcd_draw_line(fb_manage::CACHE_PAGE_0, 0, height/2, 1, width, 0x00FFFFFF);
    fbm.lcd_draw_line(fb_manage::CACHE_PAGE_0, width/2, 0, 0, height, 0x00FFFFFF);
    fbm.lcd_draw_antialiased_circle(fb_manage::CACHE_PAGE_0, width/2, height/2, 150, 0x00FFFFFF);
    fbm.lcd_draw_antialiased_circle(fb_manage::CACHE_PAGE_0, width/2, height/2, 200, 0x00FFFFFF);

    fbm.lcd_fill(fb_manage::CACHE_PAGE_1, 0, width-1, 0, height-1, 0x00FF0000);

    sleep(1);

    fbm.lcd_set_page(fb_manage::CACHE_PAGE_1);

    for (index=0; index<10; index++) {
        fbm.lcd_set_page(fb_manage::CACHE_PAGE_1);
        sleep(1);
        fbm.lcd_set_page(fb_manage::CACHE_PAGE_0);
        sleep(1);
    }

    sleep(2);
    
    fbm.release();

    return 0;
}
