
////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      tcp_socket.hpp
//
//  Purpose:
//      实现tcp服务器的接口
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/10/2024   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include "includes/include.hpp"

class tcp_server
{
public:
    /// \brief constructor
    tcp_server() {}

    /// \brief destructor   
    ~tcp_server() {}

    /// \brief init
    /// - This method is used to init the object.
    /// \param ip - ipaddr used for init.
    /// \param port - port used for init.
    /// \return Wheather initialization is success or failed.
    bool init (const std::string &ip, int port, int maxl = 32) {
        ipaddr_ = ip;
        port_ = port;
        max_listen_ = maxl;
        return true;
    }

    /// \brief start
    /// - This method is used to start the object.
    /// \return Wheather initialization is success or failed.
    bool start();

private:
    /// \brief run
    /// - This method is for thread accept loop run.
    void run();

    /// \brief recv_task_run
    /// - This method is for thread recv loop run.    
    void recv_task_run(int fd);

private:
    /// \brief ipaddr_
    /// - ipaddr used for the server.
    std::string ipaddr_;

    /// \brief port_
    /// - port used for the server.
    int port_;

    /// \brief socket_fd_
    /// - socket fd for the device.    
    int socket_fd_{-1};

    /// \brief max_listen_
    /// - max listen port.  
    int max_listen_{32};
};
