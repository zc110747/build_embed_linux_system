//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2022-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      fifo_manage.hpp
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
_Pragma("once")

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include <string>

#define S_FIFO_WORK_MODE               (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)  

typedef enum
{
    FIFO_MODE_WR_CREATE = 0,
    FIFO_MODE_R_CREATE,
    FIFO_MODE_W_CREATE, 
    FIFO_MODE_WR,
    FIFO_MODE_R,
    FIFO_MODE_W,
}ENUM_FIFO_MODE;

class fifo_manage final
{
public:
    /// \brief constructor
    fifo_manage(const std::string& fstr, int mode, ENUM_FIFO_MODE fifomode=FIFO_MODE_WR_CREATE);

    /// \brief destructor
    ~fifo_manage();

    /// \brief create
    /// - This method is used to create and open fifo.
    /// \return Wheather create and open is success or fail.
    bool create(void);                   
    
    /// \brief read
    /// - This method is used to read from the fifo.
    /// \param buf - pointer to the start of read data.
    /// \param bufsize - max size of the data can be read.
    /// \return data alread read from the fifo
    int read(char *buf, int bufsize);

    /// \brief write
    /// - This method is used to write to fifo.
    /// \param buf - pointer to the start of write data.
    /// \param bufsize -size of the data write to fifo.
    /// \return data alread write to fifo
    int write(const char *buf, int bufsize);

    /// \brief release
    /// - This method is used to close the fifo already open.
    void release();   

    /// \brief is_write_valid
    /// - This method is used to check wheather write valid.
    bool is_write_valid()
    {
        if (writefd_ < 0)
        {
            return false;
        }

        return true;
    }

private:
    /// \brief fifo_path_
    /// - the path of fifo for management.
    std::string  fifo_path_;

    /// \brief mode_
    /// - the open mode of the fifo
    int mode_;

    /// \brief mode_
    /// - the fifo run mode
    ENUM_FIFO_MODE fifo_mode_;

    /// \brief readfd_
    /// - the fd of the fifo to read.
    int readfd_{-1};

    /// \brief writefd_
    /// - the fd of the fifo to write.
    int writefd_{-1};
};
