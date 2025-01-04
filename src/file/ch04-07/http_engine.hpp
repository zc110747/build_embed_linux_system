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
_Pragma("once");

#include "includes.hpp"
#include "http_resource.hpp"

typedef enum
{
    METHOD_GET = 0,
    METHOD_POST,
    METHOD_PUT,  
}HTTP_METHOD;

typedef enum
{
    FILE_TYPE_TEXT = 0,
    FILE_TYPE_JPG,
    FILE_TYPE_PNG,
    FILE_TYPE_HTML,
    FILE_TYPE_GIF,
    FILE_TYPE_JS,
    FILE_TYPE_ASP,
    FILE_TYPE_CSS,
    FILE_TYPE_JSON,
}HTTP_FILE_TYPE;

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

    /// \brief is_dynamic
    /// - wheather the process is dynamic
    bool is_dynamic{false};

    /// \brief type_
    /// - file type
    HTTP_FILE_TYPE type_;
};

class http_engine
{
public:
    /// \brief constructor
    http_engine(std::function<void(const char *ptr, int size)> func):handle_tx_(func) 
    {
    };

    /// \brief destructor 
    ~http_engine() {
    };

    /// \brief process
    /// - This method is used to process the http decode.
    /// \param rx_buffer - rx buffer process.
    /// \param rx_size - rx size.
    /// \return Wheather process success or failed.
    bool process(char *rx_buffer, int rx_size);

    /// \brief get_tx_size
    /// - This method is used for static process.
    void static_process();

private:
    /// \brief decode_request_line
    /// - This method is used to decode the http header.
    /// \param head_str - the substring data.
    /// \return Wheather process success or failed.   
    bool decode_request_line(const std::string &head_str);

    /// \brief format_header_extra_info
    /// - This method is used to format extra header info
    /// \param type - file type.
    /// \param size - file size.
    /// \return string return for header
    std::string format_header_extra_info(HTTP_FILE_TYPE type, int size);

    /// \brief engine_static_process
    /// - This method is used to static engine process. 
    /// \return weather can find file  
    bool engine_static_process();

    /// \brief engine_static_process
    /// - This method is used to dynamic engine process. 
    /// \return the final for the process
    bool engine_dynamic_process();

private:
    /// \brief tx_size_
    /// - the tx size
    int tx_size_{0};

    /// \brief protocol_
    /// - the http protocol info  
    http_protocol protocol_;

    /// \brief handle_tx_
    /// - tx handler
    std::function<void(const char *ptr, int size)> handle_tx_;
};

