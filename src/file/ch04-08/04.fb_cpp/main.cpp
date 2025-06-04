#include "fb_draw.hpp"

int main(void)
{
    fb_manage fbm;
    bool res;
    uint8_t index;
    uint32_t width, height;

    res = fbm.init(FB_DEVICE);
    if (res == false) {
        return -1;
    }

    width = fbm.get_width();
    height = fbm.get_height();

    for (index=0; index=10; index++) {     
        // 绘制第一块显存, 切换到第一个界面
        fbm.lcd_fill(0, width, 0, height, 0xFF000000);
        for (index=0; index<10; index++) {
            fbm.lcd_draw_rectangle(width/2-index*20, height/2-index*20, width/2+index*20, height/2+index*20, 0xFFFFFFFF);
        }
        fbm.lcd_draw_line(0, height/2, 1, width, 0xFFFFFFFF);
        fbm.lcd_draw_line(width/2, 0, 0, height, 0xFFFFFFFF);
        fbm.lcd_draw_antialiased_circle(width/2, height/2, 150, 0xFFFFFFFF);
        fbm.lcd_draw_antialiased_circle(width/2, height/2, 200, 0xFFFFFFFF);
        fbm.lcd_switch_hw_page();
        sleep(1);

        // 绘制第二块显存, 切换到第二个界面
        fbm.lcd_fill(0, width, 0, height, 0xFFFFFF00);
        fbm.lcd_switch_hw_page();
        sleep(1);
    }
    
    fbm.release();

    return 0;
}
