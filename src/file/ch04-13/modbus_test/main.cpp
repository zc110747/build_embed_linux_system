
#include <thread>
#include <chrono>
#include <iostream>
#include <memory>

#include "fifo_manage.hpp"
#include "logger.hpp"

#define RX_BUFFER_SIZE 1024

static std::unique_ptr<fifo_manage> fifo_point_;
static char rx_buffer[RX_BUFFER_SIZE];

static void show_hex_string(const char *str, size_t len)
{
    printf("str = {");
    for (size_t i = 0; i < len; i++) {
        printf("0x%02x", (unsigned char)str[i]);
        if (i < len - 1) {
            printf(", ");
        }  
    }
    printf("}\n");
}

static void port_tty_rx_thread(void)
{
    ssize_t n_size;

    while (1) {
        n_size = fifo_point_->read(rx_buffer, RX_BUFFER_SIZE);
        if (n_size > 0) {         
            LOG_INFO(0, "read {} bytes", n_size);
            show_hex_string(rx_buffer, n_size);
        } else if ( n_size == 0) {
            continue;
        } else {
        }
    }
}

//01, 读线圈状态
static uint8_t feature_cmd_01[] = {
    0x01, 0x01, 0x00, 0x00, 0x00, 0x08, 0x3d, 0xcc
};

static uint8_t feature_cmd_05[] = {
    0x01, 0x05, 0x00, 0x01, 0xFF, 0x00, 0xdd, 0xfa
};

int main(void)
{
    fifo_point_ = std::make_unique<fifo_manage>("/tmp/fifo_modbus_slave", S_FIFO_WORK_MODE, FIFO_MODE_WR);
    if (fifo_point_ == nullptr) {
        LOG_ERROR(0, "fifo_manage new failed");
        return -1;
    }

    if (!fifo_point_->create()) {
        LOG_ERROR(0, "fifo_manage create failed");
        return -1;
    }

    std::thread(port_tty_rx_thread).detach();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    while(1)
    {
        fifo_point_->write((char *)feature_cmd_01, sizeof(feature_cmd_01));
        std::this_thread::sleep_for(std::chrono::seconds(3));
        fifo_point_->write((char *)feature_cmd_05, sizeof(feature_cmd_05));
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

}