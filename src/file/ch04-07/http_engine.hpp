////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      http_engine.hpp
//
//  Purpose:
//      http处理引擎
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

typedef enum
{
    METHOD_GET = 0,
    METHOD_POST,
    METHOD_PUT,  
}HTTP_METHOD;

struct http_protocol
{
    /// \brief method_
    /// - the http method for http1.1.
    HTTP_METHOD method_;

    /// \brief method_
    /// - the url for user ask.   
    std::string url_;

    /// \brief ver_
    /// - the version for the http request.   
    std::string ver_;   
};

class http_engine
{
public:
    /// \brief constructor
    http_engine() {
    };

    /// \brief destructor 
    ~http_engine() {
        release();
    };

    /// \brief process
    /// - This method is used to process the http decode.
    /// \param rx_buffer - rx buffer process.
    /// \param rx_size - rx size.
    /// \return Wheather process success or failed.
    bool process(char *rx_buffer, int rx_size, std::string& Str);

    /// \brief get_tx_buf_
    /// - This method is used to process output buffer.
    /// \return the tx buffer point.
    char *get_tx_buf_() {
        return ptxbuf_;
    }

    /// \brief get_tx_size
    /// - This method is used to process output buffer size.
    /// \return the tx buffer size. 
    int get_tx_size() {
        return tx_size_;
    }

    /// \brief get_tx_size
    /// - This method is used for static process.
    void static_process();

    void release() {
        if (!ptxbuf_) {
            delete ptxbuf_;
            ptxbuf_ = nullptr;
        }
    }

private:
     /// \brief decode_header
    /// - This method is used to decode the http header.
    /// \param head_str - the substring data.
    /// \return Wheather process success or failed.   
    bool decode_header(const std::string &head_str);

    bool create_out_buffer(const std::string& http_header, const std::string& body);
private:
    /// \brief ptxbuf_
    /// - the tx buffer point
    char *ptxbuf_{nullptr};

    /// \brief tx_size_
    /// - the tx size
    int tx_size_{0};

    /// \brief protocol_
    /// - the http protocol info  
    http_protocol protocol_;
};

