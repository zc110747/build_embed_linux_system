//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2022-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      fifo_manage.cpp
//
//  Purpose:
//      FIFO管理接口，用于提供外部，支持创建读写的接口
//
// Author:
//      @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/19/2022   Add explian and update structure.
/////////////////////////////////////////////////////////////////////////////
#include "fifo_manage.hpp"
#include "logger.hpp"

fifo_manage::fifo_manage(const std::string& fstr, int mode, ENUM_FIFO_MODE fifomode)
{
    fifo_path_ = std::move(fstr);
    mode_ = mode;
    fifo_mode_ = fifomode;
    readfd_ = -1;
    writefd_ = -1;
}

fifo_manage::~fifo_manage()
{
    release();
    fifo_path_.clear();
}

bool fifo_manage::create()
{
    //with create, will process fifo create
    if (fifo_mode_ == FIFO_MODE_WR_CREATE
    || fifo_mode_ == FIFO_MODE_R_CREATE
    || fifo_mode_ == FIFO_MODE_W_CREATE)
    {
        unlink(fifo_path_.c_str());
        if (mkfifo(fifo_path_.c_str(), mode_) < 0)
        {
            LOG_ERROR(0, "fifo {} make error!\n", fifo_path_.c_str());
            return false;
        }
        else
        {
            LOG_INFO(0, "fifo {} make success!\n", fifo_path_.c_str());
        }
    }

    if (fifo_mode_ == FIFO_MODE_WR_CREATE 
    || fifo_mode_ == FIFO_MODE_WR 
    || fifo_mode_ == FIFO_MODE_R_CREATE
    || fifo_mode_ == FIFO_MODE_R)
    {
        //for Read, need open as O_RDWR, otherwise will block
        readfd_ = open(fifo_path_.c_str(), O_RDWR);
        if (readfd_ < 0)
        {
            LOG_ERROR(0, "fifo open {} rx error:{}!\n", fifo_path_.c_str(), readfd_);
            return false;
        }
        else
        {
            LOG_INFO(0, "fifo open {} rx success:{}!\n", fifo_path_.c_str(), readfd_); 
        }
    }

    if (fifo_mode_ == FIFO_MODE_WR_CREATE 
    || fifo_mode_ == FIFO_MODE_WR 
    || fifo_mode_ == FIFO_MODE_W_CREATE
    || fifo_mode_ == FIFO_MODE_W)
    {
        writefd_ = open(fifo_path_.c_str(), O_RDWR);
        if (writefd_ < 0)
        {
            release();
            LOG_ERROR(0, "fifo open {} tx error:{}!\n", fifo_path_.c_str(), writefd_);
            return false;
        }
        else
        {
            LOG_INFO(0, "fifo open {} tx success:{}!\n", fifo_path_.c_str(), writefd_); 
        }
    }

    return true;
}

void fifo_manage::release()
{
    if (writefd_ >= 0)
    {
        close(writefd_);
        writefd_ = -1;
    }

    if (readfd_ >= 0)
    {
        close(readfd_);
        readfd_ = -1;
    }
}

int fifo_manage::read(char *buf, int bufsize)
{
    int readbytes = -1;
    if (readfd_ >= 0)
    {
        readbytes = ::read(readfd_, buf, bufsize);
    }
    
    return readbytes;
}

int fifo_manage::write(const char *buf, int bufsize)
{
    int writebytes = -1;
    if (writefd_ >= 0 && bufsize > 0)
    {
        writebytes = ::write(writefd_, buf, bufsize);
    }
    return writebytes;
}
