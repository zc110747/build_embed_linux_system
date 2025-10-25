
#pragma once

#include <unistd.h>
#include <cstring>
#include <thread>
#include <iostream>
#include <string>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <functional>
#include <stdint.h>

#define CAN_FRAME_EMPTY			    0
#define CAN_FRAME_RECEIVED			1
#define CAN_FRAME_COMPLETE			2

#define CAN_UI_HEADR				0x3C

#define CAN_HEAD_SIZE               5

#define CAN_PROTOCOL_VERSION        0x01

#define CAN_REQUEST_SHOW_INFO       0x01
#define CAN_REQUEST_NULL            0x02

#define CAN_RESPONSE_SHOW_INFO      0x01
#define CAN_RESPONSE_NULL           0x02

#define MAX_FRAME_BUFFER            548         //允许的最大包格式

#define CAN_STD_ID_DATA             0x00

/*
协议格式
head(1) | version(1) |  cmd(1) | size(2) | data(size) | crc16(2) |
*/
class can_protocol
{
public:
    /// \brief constructor
    can_protocol() = default;

    /// \brief destructor
    ~can_protocol() = default;

    int protocol_process(char *tx_buffer)
    {
        uint8_t version, cmd;
        int size = -1;
        
        version = rx_framebuffer_[1];
        cmd = rx_framebuffer_[2];
        
        if (version > CAN_PROTOCOL_VERSION)
        {
            std::cout<<"can frame version not support!"<<std::endl;
            return -1;
        }

        switch(cmd)
        {
            case CAN_REQUEST_SHOW_INFO:
                size = can_reqeust_show_info(&rx_framebuffer_[5], rx_framesize_-5, tx_buffer);
                break;
            default:
                break;
        }

        return size;
    }

    /// \brief process_rx_frame
    void process_rx_frame(char data)
    {
        if (rx_frame_state_ == CAN_FRAME_EMPTY) {
            if (data == CAN_UI_HEADR) {
                rx_framebuffer_[0] = data;
                rx_framesize_ = 1;
                rx_frame_state_ = CAN_FRAME_RECEIVED;
            }
        }
        else if (rx_frame_state_ == CAN_FRAME_RECEIVED) {
            rx_framebuffer_[rx_framesize_++] = data;

            if (rx_framesize_ > CAN_HEAD_SIZE) {
                int frame_size = (((uint16_t)rx_framebuffer_[3]<<8) | rx_framebuffer_[4]) + 7;
                if (frame_size > MAX_FRAME_BUFFER) {
                    rx_frame_state_ = CAN_FRAME_EMPTY;
                    std::cout << "rx_framesize_ > MAX_FRAME_BUFFER" << std::endl;
                } else {
                    if (rx_framesize_ >= frame_size) {
                        uint16_t checksum = crc16_modbus(rx_framebuffer_, frame_size - 2);
                        if (checksum == ((uint16_t)rx_framebuffer_[frame_size - 2] << 8) | rx_framebuffer_[frame_size - 1]) {
                            rx_frame_state_ = CAN_FRAME_COMPLETE;
                            rx_framesize_ -= 2;
                        }
                        else { 
                            rx_frame_state_ = CAN_FRAME_EMPTY;
                            std::cout << "checksum error" << std::endl;
                        }
                    }
                }
            }
        } else {
            // no process
        }
    }

    int get_frame_state(void) {
        return rx_frame_state_;
    }

    void clear_frame_state(void) {
        rx_frame_state_ = CAN_FRAME_EMPTY;
    }

    void clear_frame_size(void) {
        rx_framesize_ = 0;
    }

private:
    int can_reqeust_show_info(char *pbuffer, int len, char *tx_buffer)
    {
        int size = 0;

        std::cout<<"can_reqeust_show_info:"<<len<<std::endl;
        
        size = create_can_tx_buffer(CAN_RESPONSE_SHOW_INFO, pbuffer, len, tx_buffer);
        return size;
    }

    uint16_t create_can_tx_buffer(uint8_t cmd, const char *pbuffer, uint16_t len, char *out_buffer)
    {
        uint16_t size = 0;
        uint16_t crc_value;

        out_buffer[size++] = CAN_UI_HEADR;
        out_buffer[size++] = CAN_PROTOCOL_VERSION;
        out_buffer[size++] = cmd;
        out_buffer[size++] = len >> 8;
        out_buffer[size++] = len&0xff;

        if (len > 0)
        {
            memcpy(&out_buffer[size], pbuffer, len);
            size += len;
        }   
        
        crc_value = crc16_modbus(out_buffer, size);
        out_buffer[size++] = crc_value >> 8;
        out_buffer[size++] = crc_value&0xff;

        return size;
    }

    /// \brief crc16_modbus
    uint16_t crc16_modbus(char *data, uint16_t length)
    {
        uint8_t i;
        uint16_t crc = 0xffff;        // Initial value
        while(length--)
        {
            crc ^= *data++;            // crc ^= *data; data++;
            for (i = 0; i < 8; ++i)
            {
                if (crc & 1)
                    crc = (crc >> 1) ^ 0xA001;        // 0xA001 = reverse 0x8005
                else
                    crc = (crc >> 1);
            }
        }
        return crc;
    }

private:
    /// \brief rx_buffer_
    char rx_framebuffer_[MAX_FRAME_BUFFER];
    
    /// \brief rx_framesize_
    int rx_framesize_{0};

    /// \brief rx_frame_state_
    int rx_frame_state_{CAN_FRAME_EMPTY};
};

class can_socket
{
public:
    /// \brief constructor
    can_socket() {}

    /// \brief constructor
    can_socket(const std::string& str) 
    : interface_(str)
    {}

    /// \brief destructor
    ~can_socket() 
    {
        if (fd_ > 0) {
            close(fd_);
            fd_=-1;
        }
    }

    /// \brief init
    /// - This method is used for can socket init.
    /// \return whether success.
    bool init(void) 
    {
        fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (fd_ < 0) {
            std::cout << "socket error" << std::endl;
            return false;
        }

        strcpy(ifr_.ifr_name, interface_.c_str());
        ioctl(fd_, SIOCGIFINDEX, &ifr_);

        addr_.can_family = AF_CAN;
        addr_.can_ifindex = ifr_.ifr_ifindex;

        if (bind(fd_, (struct sockaddr *)&addr_, sizeof(addr_)) < 0) {
            std::cout << "bind error" << std::endl;
            close(fd_);
            return false;
        }

        std::thread(std::bind(&can_socket::run, this)).detach();
        return true;
    }

private:
    /// \brief run                
    /// - This method is used for thread run.
    void run(void) {
        int size;
        while (true) {
            if (read(fd_, &frame_, sizeof(struct can_frame)) > 0) {
                for (int i = 0; i < frame_.can_dlc; i++) {
                    can_protocol_.process_rx_frame(frame_.data[i]);
                    if (can_protocol_.get_frame_state() == CAN_FRAME_COMPLETE) {
                        can_protocol_.clear_frame_state();
                        size = can_protocol_.protocol_process(tx_buffer_);
                        if (size > 0) {
                            can_write_buffer(tx_buffer_, size);
                        }
                    } else if (can_protocol_.get_frame_state() == CAN_FRAME_EMPTY) {
                        can_protocol_.clear_frame_size();
                    } else {
                        // in receive， no process
                    }
                }
            }
        }
    }

    void can_write_buffer(char *buffer, int len) {
        int nums = len/CAN_MAX_DLEN;
        int offset = len%CAN_MAX_DLEN;
        char *ptr;

        ptr = buffer;

        // 按照CAN MAX_DLC 发送数据
        if (nums > 0) {
            for (int index=0; index<nums; index++) {
                frame_.can_id = id_;
                frame_.can_dlc = CAN_MAX_DLEN;
                memcpy(frame_.data, ptr, CAN_MAX_DLEN);
                ptr += 8;
                write(fd_, &frame_, sizeof(struct can_frame));
            }
        }

        if (offset > 0) {
            frame_.can_id = id_;
            frame_.can_dlc = offset;
            memcpy(frame_.data, ptr, offset);
            write(fd_, &frame_, sizeof(struct can_frame));
        }

        std::cout<<"can write buffer: "<<len<<" id: "<<id_<<std::endl;
    }
private:
    /// \brief fd_
    /// - can socket fd.
    int fd_{-1};

    /// \brief interface_
    /// - can interface
    std::string interface_{"can0"};

    /// \brief ifr_
    /// - can freq information.
    struct ifreq ifr_;

    /// \brief addr_
    /// - can address.
    struct sockaddr_can addr_;

    /// \brief frame_
    /// - can frame.
    struct can_frame frame_;

    /// \brief tx_buffer_
    /// - can tx buffer.
    char tx_buffer_[MAX_FRAME_BUFFER];

    can_protocol can_protocol_;

    int id_{CAN_STD_ID_DATA};
};