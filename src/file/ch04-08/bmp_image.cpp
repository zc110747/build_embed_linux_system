
#include "bmp_image.hpp"

using namespace bmp_img;

bool bmp_decoder::decode_bmp(std::string filename)
{
    std::filesystem::path p(filename);
    std::ifstream file(p, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "File open failed!" << std::endl;
        return false;
    }

    //解析bmp文件头数据
    auto bmp_header_buf = std::make_unique<uint8_t[]>(BMP_HEAD_LEN);       
    bmp_file_header *file_header;
    bmp_info_header *info_header;
    uint32_t offsets;

    file.read((char *)bmp_header_buf.get(), BMP_HEAD_LEN);
    file_header = reinterpret_cast<bmp_file_header *>(bmp_header_buf.get());
    info_header = reinterpret_cast<bmp_info_header *>((bmp_header_buf.get() + BMP_FILEHEADER_LEN));
    offsets = file_header->offsets;

    // 清理除头部信息外的额外bmp信息
    if (offsets > BMP_HEAD_LEN) {
        pattle_len_ = offsets - BMP_HEAD_LEN;
        palette_data_ = std::make_unique<uint8_t[]>(pattle_len_);
        file.read((char *)palette_data_.get(), pattle_len_);
    }

    width_ = info_header->bwidth;
    height_ = info_header->bheight;
    color_depth_ = info_header->bBitCount;
    bytes_per_pixel_ = color_depth_ / 8;
    if (bytes_per_pixel_ == 0) {
        bytes_per_pixel_ = 1;
    }
    compress_ = info_header->bCompression;

    image_data_size_ = width_ * height_ * bytes_per_pixel_;
    image_data_ = std::make_unique<uint8_t[]>(image_data_size_);
    file.read((char *)image_data_.get(), image_data_size_);
    
    reserval_image();

    file.close();
    return true;
}

void bmp_decoder::show_info(void)
{
    std::cout<<"width: "<<width_<<std::endl;
    std::cout<<"height: "<<height_<<std::endl;
    std::cout<<"color_depth: "<<color_depth_<<std::endl;
    std::cout<<"bytes_per_pixel: "<<bytes_per_pixel_<<std::endl;
    std::cout<<"image_data_size_: "<<image_data_size_<<std::endl;
    std::cout<<"compress: "<<compress_<<std::endl;
    std::cout<<"pattle_len_: "<<pattle_len_<<std::endl;
}

void bmp_decoder::reserval_image(void)
{
    uint32_t width_bytes = width_ * bytes_per_pixel_;
    std::unique_ptr<uint8_t[]> cache_buffer = std::make_unique<uint8_t[]>(image_data_size_);

    /*
        反转图像内数据所在行, 原始数据和bmp中的按行顺序相反
    */ 
    std::memcpy(cache_buffer.get(), image_data_.get(), image_data_size_);
    for(int i = 0; i < height_; i++)
    {
        uint8_t *p_cache = cache_buffer.get() + image_data_size_ - width_bytes * (i + 1);
        uint8_t *p_image = image_data_.get() + width_bytes * i;
        std::memcpy(p_image, p_cache, width_bytes);
    }

    std::cout << "reserval image" << std::endl;
}

bool bmp_decoder::save_to_bmp(const std::string filename, bmp_compress_state compress)
{
    uint8_t my_compress = get_compress_value(compress);
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    
    if (!file.is_open()) {
        std::cout << "File open failed!" << std::endl;
        return false; 
    }

    if (image_data_ == nullptr) {
        std::cout << "image_data_ is null!" << std::endl;
        return false; 
    }

    //添加bmp文件头
    bmp_file_header file_header;
    bmp_info_header info_header;
    file_header.bfType = BMP_FILE_TYPE;
    file_header.reserved0 = 0;
    file_header.reserved1 = 0;
    file_header.offsets = BMP_HEAD_LEN;

    //添加bmp信息头
    info_header.bsize = BMP_INFOHEADER_LEN;
    info_header.bwidth = width_;
    info_header.bheight = height_;
    info_header.bPlanes = 1;
    info_header.bBitCount = color_depth_;
    info_header.bCompression = my_compress;  //压缩方式
    info_header.bSizeImage = image_data_size_;
    info_header.bXPelsPerMeter = 0;
    info_header.bYPelsPerMeter = 0;
    info_header.bClrUsed = 0;
    info_header.bClrImportant = 0;

    //图像单色, 16色, 256色, 位图数据是调色板的索引值, 需要添加调色板(BytePerPix为1)
    //位图是16位, 24位和32位色, 不存在调色板, 图像的颜色直接在位图数据中给出
    if (bytes_per_pixel_ == 1) {
        file_header.offsets += pattle_len_;
    }
    //位图文件长度(包含图像数据信息，文件头和位图数据的总长度)
    file_header.bfSize = image_data_size_ + file_header.offsets;

    std::unique_ptr<uint8_t[]> p_header = std::make_unique<uint8_t[]>(BMP_HEAD_LEN);
    memcpy(p_header.get(), (char *)&file_header, BMP_FILEHEADER_LEN);
    memcpy(p_header.get() + BMP_FILEHEADER_LEN, (char *)&info_header, BMP_INFOHEADER_LEN);
    file.write((char *)p_header.get(), BMP_HEAD_LEN);
    
    //调位板定义(每个色彩4字节)
    //对于单色, 16色和256色
    if ( bytes_per_pixel_ == 1 ) {
        file.write((char *)palette_data_.get(), pattle_len_);
    }

    if ( my_compress == 0 ) {
        //不压缩,写入实际数据
        save_image_nocompress(file);
    } else if ( my_compress == 1 ) {
        //对于256色图像,采用RLE4压缩

    } else {

    }

    file.close();
    return true;
}