/*

*/
#pragma once

#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>

namespace bmp_img
{

enum class bmp_compress_state
{
    IMAGE_NOCOMPRESS = 0,
    IMAGE_COMPRESS,
};

#define BMP_FILEHEADER_LEN  14
#define BMP_INFOHEADER_LEN  40
#define BMP_HEAD_LEN        (BMP_FILEHEADER_LEN + BMP_INFOHEADER_LEN)

#define BMP_FILE_TYPE      0x4d42

#pragma pack(push, 1)
struct bmp_file_header
{
    uint16_t bfType;      //文件类型
    uint32_t bfSize;      //位图文件大小(此处不一定准确)
    uint16_t reserved0;
    uint16_t reserved1;
    uint32_t offsets;     //实际数据偏移值
};

struct bmp_info_header
{
    uint32_t bsize;           //信息头
    uint32_t bwidth;          //图像每行像素点
    uint32_t bheight;         //图像行数
    uint16_t bPlanes;         //颜色平面数(总为1)
    uint16_t bBitCount;       //说明比特数/像素(每个像素点占有bit数)
    uint32_t bCompression;    //数据压缩类型
    uint32_t bSizeImage;      //图像数据的大小
    uint32_t bXPelsPerMeter;  //水平分辨率
    uint32_t bYPelsPerMeter;  //垂直分辨率
    uint32_t bClrUsed;        //颜色索引数
    uint32_t bClrImportant;   //说明对图像有重要影响的颜色索引 
};
#pragma pack(pop)

class bmp_decoder
{
public:
    bmp_decoder() = default;
    bmp_decoder(const bmp_decoder&) = default;

    bool decode_bmp(std::string filename);

    void show_info(void);

    bool save_to_bmp(const std::string filename, bmp_compress_state compress = bmp_compress_state::IMAGE_NOCOMPRESS);

private:
    // 获取压缩方式
    uint8_t get_compress_value(bmp_compress_state compress)
    {
        uint8_t my_compress = 0;

        if (compress == bmp_compress_state::IMAGE_COMPRESS) {
            switch (color_depth_) {
               case 4:
                    my_compress = 2;
                    break;
               case 8:
                    my_compress = 1;
                    break;
               default:
                    std::cout << "not supoort compress, use default!" << std::endl;
                    break;
            }
        }
        return my_compress;   
    }

    void save_image_nocompress(std::ofstream &file)
    {
        uint32_t width_bytes = width_ * bytes_per_pixel_;
        std::unique_ptr<uint8_t[]> cache_buffer = std::make_unique<uint8_t[]>(image_data_size_);

        for(int i = 0; i < height_; i++)
        {
            uint8_t *p_cache = cache_buffer.get() + image_data_size_ - width_bytes * (i + 1);
            uint8_t *p_image = image_data_.get() + width_bytes * i;
            std::memcpy(p_cache, p_image, width_bytes);   
        }

        file.write((char *)cache_buffer.get(), image_data_size_);
    }

    // 反转图像内各行数据，获取真实数据
    void reserval_image(void);

public:
    uint32_t get_width(void) { return width_; }
    uint32_t get_height(void) { return height_; }
    uint16_t get_color_depth(void) { return color_depth_; }
    uint16_t get_bytes_per_pixel(void) { return bytes_per_pixel_; }
    uint64_t get_image_data_size(void) { return image_data_size_; }

private:
    // 图像数据
    std::unique_ptr<uint8_t[]> image_data_{nullptr};

    std::unique_ptr<uint8_t[]> palette_data_{nullptr};

    uint32_t pattle_len_{0};

    // 图形宽度
    uint32_t width_;

    // 图形高度         
    uint32_t height_;

    //每个像素点占的Byte数
    uint16_t bytes_per_pixel_;

    // 数据总数
    uint64_t image_data_size_;

    //图像的色深    
    uint16_t color_depth_;

    // 压缩方式
    uint32_t compress_;
};
}
