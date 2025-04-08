//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2022-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      tty.hpp
//
//  Purpose:
//      tty interface manage module.
//
// Author:
//      @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//     15/10/2024    Create New Version
/////////////////////////////////////////////////////////////////////////////
_Pragma("once")

#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include <string>
#include <cstring>

class tty_control 
{
public:
    /// \brief constructor
    tty_control() = default;

    /// \brief destructor, delete not allow for singleton pattern.
    ~tty_control() {}

    /// \brief init
    /// - This method is used to init the object.
    /// \return Wheather initialization is success or failed.
    bool init(const std::string& path, int flags) {
        fd_ = ::open(path_.c_str(), flags);
        if (fd_ < 0) {
            return false;
        }
        return true;
    }

    /// \brief set_opt
    /// - This method is used to option the uart informate.
    /// \param nBaud - baud rate of the uart option
    /// \param nDataBits - data bits of the uart option
    /// \param cParity - parity of the uart option
    /// \param nStopBits - stop bits of the uart option
    /// \return wheather the option is sucess or failed
    bool set_opt(int nBaud, int nDataBits, const std::string cParity, int nStopBits)
    {
        struct termios newtio;
        struct termios oldtio;

        baud_ = nBaud;
        databits_ = nDataBits;
        parity_ = std::move(cParity);
        stopbits_ = nStopBits;

        if (tcgetattr(fd_, &oldtio)  !=  0)
            return false;

        memset((char *)&newtio, 0, sizeof(newtio));
        newtio.c_cflag |= (CLOCAL|CREAD);
        newtio.c_cflag &= ~CSIZE;

        //init databits flags
        switch (databits_)
        {
            case 7:
                newtio.c_cflag |= CS7;
            break;
            case 8:
                newtio.c_cflag |= CS8;
            break;
            default:
            break;
        }

        //init parity flags
        switch (parity_.c_str()[0])
        {
            case 'O':
            case 'o':
                newtio.c_cflag |= PARENB;
                newtio.c_cflag |= PARODD;
                newtio.c_iflag |= (INPCK | ISTRIP);
            break;
            case 'E':
            case 'e':
                newtio.c_iflag |= (INPCK | ISTRIP);
                newtio.c_cflag |= PARENB;
                newtio.c_cflag &= ~PARODD;
            break;
            case 'N':
            case 'n':
                newtio.c_cflag &= ~PARENB;
            break;
        }

        //init baud flags
        switch (baud_)
        {
            case 2400:
                cfsetispeed(&newtio, B2400);
                cfsetospeed(&newtio, B2400);
            break;
            case 4800:
                cfsetispeed(&newtio, B4800);
                cfsetospeed(&newtio, B4800);
            break;
            case 9600:
                cfsetispeed(&newtio, B9600);
                cfsetospeed(&newtio, B9600);
            break;
            case 115200:
                cfsetispeed(&newtio, B115200);
                cfsetospeed(&newtio, B115200);
            break;
            case 460800:
                cfsetispeed(&newtio, B460800);
                cfsetospeed(&newtio, B460800);
            break;
            case 921600:
                cfsetispeed(&newtio, B921600);
                cfsetospeed(&newtio, B921600);
            break;
            default:
                cfsetispeed(&newtio, B9600);
                cfsetospeed(&newtio, B9600);
            break;
        }

        // init stopbits
        if (stopbits_ == 1)
        {
            newtio.c_cflag &=  ~CSTOPB;
        }
        else if (stopbits_ == 2)
        {
            newtio.c_cflag |=  CSTOPB;
        }
        newtio.c_cc[VTIME]  = 0;
        newtio.c_cc[VMIN] = 0;

        // set config to tty.
        tcflush(fd_, TCIFLUSH);
        if ((tcsetattr(fd_, TCSANOW, &newtio))!=0)
        {
            return false;
        }

        return true;
    }

    /// \brief write
    /// - This method is used to write data to tty.
    /// \param buffer - buffer used to write
    /// \param len - buffer length used to send
    /// \return the size already write, -1 means failed.
    int write(char *buffer, int len) {
        if (fd_ < 0) {
            return -1;
        }

        return ::write(fd_, buffer, len);
    }

    /// \brief read
    /// - This method is used to read data from tty.
    /// \param buffer - buffer used to read
    /// \param len - buffer max length used to read
    /// \return the size already read, -1 means failed.
    int read(char *buffer, int len) {
        if (fd_ < 0) {
            return -1;
        }

        return ::read(fd_, buffer, len);
    }

    /// \brief close
    /// - This method is used to close tty
    void close() {
        if (fd_ > 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

private:
    /// \brief path_
    /// - path for save the device path.
    std::string path_;

    /// \brief fd_
    /// - fd for the device.
    int fd_{-1};

    /// \brief baud_
    /// - baud for tty.
    int baud_{115200};

    /// \brief parity_
    /// - parity for tty.
    std::string parity_;

    /// \brief stopbits_
    /// - stopbits for tty.
    int stopbits_;

    /// \brief databits_
    /// - databits for tty.
    int databits_;
};
