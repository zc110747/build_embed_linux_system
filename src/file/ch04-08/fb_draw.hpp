#pragma once

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <math.h>
#include <stdint.h>
#include <iostream>

#define FB_DEVICE        "/dev/fb0"

/*
第一页: (0, 0) ~ (width_-1, height_-1)
第二页: (0, height) ~ (width_-1, 2*height_-1)
*/
class fb_manage {
public: 
    typedef enum {
        CACHE_PAGE_0 = 0,
        CACHE_PAGE_1,
    }CACHE_PAGE;

    /// @brief fb_manage
    /// - 构造函数
    fb_manage() {};

    /// @brief ~fb_manage
    /// - 析构函数
    ~fb_manage() {};

    /// @brief init
    /// - 初始化帧缓冲区
    /// @param dev 屏幕设备名称，默认使用"/dev/fb0"
    /// @param page_num 支持的缓存区个数，用于缓冲
    /// @return 初始化状态，成功返回true，失败返回false
    bool init(const char *dev, uint8_t page_num);

    /// @brief release
    /// - 释放帧缓冲区资源
    void release(void);

    /// @brief lcd_set_page
    /// - 设置当前显示的缓存区
    /// @param page 缓存区编号，0表示第一页，1表示第二页
    void lcd_set_page(CACHE_PAGE page);

    /// @brief lcd_fill
    /// - 填充指定区域的颜色
    /// @param page 缓存区编号，0表示第一页，1表示第二页
    /// @param start_x 起始x坐标
    /// @param end_x 结束x坐标
    /// @param start_y 起始y坐标
    /// @param end_y 结束y坐标
    /// @param color 颜色值，单位RGB888
    /// @return 填充状态，成功返回true，失败返回false
    bool lcd_fill(CACHE_PAGE page, uint32_t start_x, uint32_t end_x,
        uint32_t start_y, uint32_t end_y,
        uint32_t color);

    /// @brief  lcd_draw_antialiased_circle
    /// 绘制带抗锯齿的圆形
    /// @param page 缓存区编号，0表示第一页，1表示第二页
    /// @param center_x 圆心x坐标
    /// @param center_y 圆心y坐标
    /// @param radius 圆的半径
    /// @param color 颜色值，单位RGB888
    void lcd_draw_antialiased_circle(fb_manage::CACHE_PAGE page, uint32_t center_x, uint32_t center_y,
        int radius, uint32_t color);
    
    /// @brief lcd_draw_line
    /// @param page 显示的缓存区编号，0表示第一页，1表示第二页
    /// @param x 起始点x坐标
    /// @param y 起始点y坐标
    /// @param dir 线段方向，0表示垂直方向，1表示水平方向
    /// @param length 线段的长度
    /// @param color 线段的颜色，单位RGB888
    void lcd_draw_line(CACHE_PAGE page, uint32_t x, uint32_t y, int dir,
        uint32_t length, uint32_t color);

    /// @brief lcd_draw_rectangle
    /// @param page 显示的缓存区编号，0表示第一页，1表示第二页
    /// @param start_x 矩形左上角x坐标
    /// @param start_y 矩形左上角y坐标
    /// @param end_x 矩形右下角x坐标
    /// @param end_y 矩形右下角y坐标
    /// @param color 矩形的颜色，单位RGB888
    void lcd_draw_rectangle(CACHE_PAGE page, uint32_t start_x, uint32_t start_y, uint32_t end_x, uint32_t end_y, uint32_t color);

    /// @brief lcd_draw_point
    /// @param page 显示的缓存区编号，0表示第一页，1表示第二页
    /// @param x 点的x坐标
    /// @param y 点的y坐标
    /// @param color 点的颜色，单位RGB888
    void lcd_draw_point(CACHE_PAGE page, uint32_t x, uint32_t y, uint32_t color);

public:
    /// \brief get_width
    /// - 获取屏幕宽度
    /// \return 屏幕宽度，单位像素
    uint32_t get_width()    { return width_; }

    /// \brief get_height
    /// - 获取屏幕高度
    /// \return 屏幕高度，单位像素
    uint32_t get_height()   { return height_; }
private:
    /// \brief fb_
    /// - 帧缓冲文件描述符
    int fb_{-1};
    
    // \brief fbp_
    /// - 帧缓冲区指针
    uint32_t *fbp_{nullptr};

    /// \brief screen_size_
    /// - 屏幕显存对应大小，单位字节
    uint32_t screen_size_{0};

    /// \brief width_
    /// - 屏幕宽度，单位像素
    uint32_t width_;

    /// \brief height_
    /// - 屏幕高度，单位像素
    uint32_t height_;

    /// \brief fb_var_
    /// - 帧缓冲区可变变量信息结构体
    struct fb_var_screeninfo fb_var_;

    /// \brief fb_fix_
    /// - 帧缓冲区固定变量信息结构体
    struct fb_fix_screeninfo fb_fix_;
};
