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

//linux c header
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//cpp header
#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <functional>
#include <new>
#include <algorithm>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
#include <string_view>
#include <mutex>

#define HEAD_RESP_200   0
#define HEAD_RESP_400   1
#define HEAD_RESP_403   2
#define HEAD_RESP_404   3
#define HEAD_RESP_500   4

namespace HTTP_SREVER
{
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

    class RESOURCE 
    {
    public:
       std::vector<std::string> http_resp_header_resource{
            {"HTTP/1.1 200 OK\r\n"},
            {"HTTP/1.1 400 Bad Request\r\n"},
            {"HTTP/1.1 403 Forbidden\r\n"},
            {"HTTP/1.1 404 Not Found\r\n"},
            {"HTTP/1.1 500 Internal Server Error\r\n"}
        };

        std::map<std::string, HTTP_FILE_TYPE> file_type_list{
            {".jpg", FILE_TYPE_JPG},
            {".png", FILE_TYPE_PNG},
            {".html", FILE_TYPE_HTML},
            {".gif", FILE_TYPE_GIF},
            {".png", FILE_TYPE_PNG},
            {".js", FILE_TYPE_JS},
            {".asp", FILE_TYPE_ASP},
            {".css", FILE_TYPE_CSS},
        };

        std::string HTTP_NOT_FOUND_BODY{"<!Doctype html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>File not found.\n</body></html>"};
    };

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

    using http_tx_func_type = std::function<void(const char *ptr, int size)>;
    using http_dynamic_func_type = std::function<bool(const std::string& url, const std::string& data, std::string& response)>;
    using http_dynamic_map_type = std::map<std::string, http_dynamic_func_type>;

    class logger final
    {
    public:
        enum class LOGGER_LEVEL {
            DEBUG = 0,
            INFO,
            WARN,
            ERROR,
            FATAL,
        };

        /// \brief constructor
        logger() {};

        /// \brief destructor
        ~logger() {};

        static logger *get_instance() {
            static logger instance;
            return &instance;
        }

        /// @brief logger_message
        /// - This method is used to print the log message.
        /// @param format - the string base format.
        /// @param args - args process int the format
        template<typename... Args>
        void logger_message(bool with_endl, const std::string& format, Args... args) const {
            std::ostringstream oss;
            format_message(oss, format, args...);

            if(with_endl){
                std::cout << oss.str() << std::endl;
            } else {
                std::cout << oss.str();
            }
        }

        /// @brief logger_message
        /// - This method is used to print the log message.
        /// @param format - the string base format.
        /// @param args - args process int the format   
        template<typename... Args>
        void logger_message(LOGGER_LEVEL level, const std::string& format, Args... args) const {
            if (level >= level_) {
                std::lock_guard<std::mutex> lock(log_mutex_);
                logger_message(false, "LOG_LEVEL:{} DATA: ", static_cast<int>(level));
                logger_message(true, format, args...);
            }
        }
    private:
        /// @brief error level
        /// - the error level for the log message.
        LOGGER_LEVEL level_{LOGGER_LEVEL::DEBUG};

        /// @brief log_mutex_
        /// - mutex for thread-safe logging
        mutable std::mutex log_mutex_;    
    private:   
        /// @brief format_message
        /// - This method is used to format message.
        /// @param oss - the stringstream.
        /// @param format - the string base format.
        /// @param value - the value to be formatted.
        /// @param args - args process int the format
        template<typename T, typename... Args>
        void format_message(std::ostringstream& oss, const std::string& format, T value, Args... args) const {
            size_t pos = format.find("{}");
            if(pos == std::string::npos) {
                oss << format;
                return;
            }
            oss << format.substr(0, pos);
            oss << value;
            format_message(oss, format.substr(pos + 2), args...);
        }

        // @brief format_message
        /// - This final method while format message
        /// @param oss - the stringstream.
        /// @param format - the string base format.
        void format_message(std::ostringstream& oss, const std::string& format) const {
            oss << format;
        }
    };

    #define LOG_MESSAGE(...)    do {  \
                                    logger::get_instance()->logger_message(true, __VA_ARGS__); \
                                }while(0);
    #define LOG_DEBUG(...)      do { \
                                    logger::get_instance()->logger_message(logger::LOGGER_LEVEL::DEBUG, __VA_ARGS__); \
                                }while(0);          
    #define LOG_INFO(...)       do {  \
                                    logger::get_instance()->logger_message(logger::LOGGER_LEVEL::INFO, __VA_ARGS__); \
                                }while(0);
    #define LOG_WARN(...)       do {  \
                                    logger::get_instance()->logger_message(logger::LOGGER_LEVEL::WARN, __VA_ARGS__); \
                                }while(0);
    #define LOG_ERROR(...)      do {  \
                                    logger::get_instance()->logger_message(logger::LOGGER_LEVEL::ERROR, __VA_ARGS__); \
                                }while(0);
    #define LOG_FATAL(...)      do {  \
                                    logger::get_instance()->logger_message(logger::LOGGER_LEVEL::FATAL, __VA_ARGS__); \
                                }while(0);

    class http_engine
    {
    public:
        /// \brief constructor
        http_engine(http_tx_func_type tx_process): handle_tx_(tx_process) {
        };

        /// \brief destructor 
        ~http_engine() {};

        /// \brief process
        /// - This method is used to process the http decode.
        /// \param rx_buffer - rx buffer process.
        /// \param rx_size - rx size.
        /// \return Wheather process success or failed.
        bool process(char *rx_buffer, int rx_size, http_dynamic_map_type map={}) {
            int ret;
            
            std::string buffer(rx_buffer, rx_size);
            std::string sub_str("\r\n");
            LOG_DEBUG(buffer);

            tx_size_ = 0;

            //decode get header string
            std::size_t pos = buffer.find(sub_str);
            std::string head_str = buffer.substr(0, pos);
            LOG_DEBUG(head_str);
            
            ret = decode_request_line(head_str);
            if (ret) {
                LOG_INFO("method: {}", protocol_.method_);
                LOG_INFO("url: {}", protocol_.url_);
                LOG_INFO("ver: {}", protocol_.ver_);

                if (protocol_.is_dynamic) {
                    std::size_t pos = buffer.find("\r\n\r\n");
                    std::string data_str = buffer.substr(pos + 4);
                    std::cout<<data_str<<std::endl;
                    engine_dynamic_process(data_str, map);
                } else {
                    if(!engine_static_process())
                    {
                        html404_translate();              
                    }
                }

            } else {
                html404_translate();
            }
            return true;           
        }

    private:
        /// \brief decode_request_line
        /// - This method is used to decode the http header.
        /// \param head_str - the substring data.
        /// \return Wheather process success or failed.   
        bool decode_request_line(const std::string &head_str)
        {
            std::vector<std::string> tokens;
            std::istringstream tokenStream(head_str);
            std::string token;

            while (tokenStream >> token) {
                tokens.push_back(token);
            }

            if (tokens.size() != 3) {
                LOG_ERROR("decode header failed!");
                return false;
            }

            //获取方法，参考《HTTP权威指南》第三章
            //常见请求码GET，HEAD，POST，PUT，TRACE，OPTIONS，DELETE
            //这里只实现GET和POST
            if (tokens[0] == "GET") {
                protocol_.method_ = METHOD_GET;
            } else if (tokens[0] == "POST") {
                protocol_.method_ = METHOD_POST;
            } else {
                return false;
            }

            protocol_.url_ = tokens[1];
            if (protocol_.url_ == "/") {
                protocol_.url_ = "/index.html";
            }

            protocol_.ver_ = tokens[2];

            //解析文件后缀
            if (endsWith(protocol_.url_, ".json")) {
                protocol_.is_dynamic = true;
            } else {
                protocol_.is_dynamic = false;
                protocol_.type_ = FILE_TYPE_TEXT;

                for (const auto &file_type: resource_.file_type_list) {
                    if(endsWith(protocol_.url_, file_type.first)) {
                        protocol_.type_ = file_type.second;
                        break;
                    }
                }
            }

            return true;
        }

        /// \brief format_web_header_info
        /// - This method is used to format extra header info
        /// \param type - file type.
        /// \param size - file size.
        /// \return string return for header
        std::string format_web_header_info(HTTP_FILE_TYPE type, uint32_t size)
        {
            static const std::map<HTTP_FILE_TYPE, std::string> mime_type_map = {
                {FILE_TYPE_TEXT, "text/plain"},
                {FILE_TYPE_JPG, "application/x-jpg"},
                {FILE_TYPE_PNG, "image/png"},
                {FILE_TYPE_HTML, "text/html"},
                {FILE_TYPE_GIF, "image/gif"},
                {FILE_TYPE_JS, "application/x-javascript"},
                {FILE_TYPE_ASP, "text/asp"},
                {FILE_TYPE_CSS, "text/css"},
                {FILE_TYPE_JSON, "application/json"}
            };

            // 通过ostringstream来构建发送首部字符串
            std::ostringstream oss;
            oss << "Content-type: ";
            auto it = mime_type_map.find(type);
            if (it != mime_type_map.end()) {
                oss << it->second;
            } else {
                oss << "text/html";
            }
            oss << ";charset=utf-8\r\n";
            
            oss << "Connection: close\r\n"
                << "Server: c++ sever\r\n"
                << "Content-length: " << size << "\r\n\r\n";
            
            return oss.str();         
        }

        void send_response_header(std::streamsize body_size) {
            handle_tx_(resource_.http_resp_header_resource[HEAD_RESP_200].c_str(), resource_.http_resp_header_resource[HEAD_RESP_200].size());
            auto &&header_extra_str = format_web_header_info(protocol_.type_, body_size);
            handle_tx_(header_extra_str.c_str(), header_extra_str.size());
        }

        /// \brief engine_static_process
        /// - This method is used to static engine process. 
        /// \return weather can find file  
        bool engine_static_process()
        {
            std::string url = "www" + protocol_.url_;
            std::ifstream input_file(url, std::ios::binary);
            char buffer[1024];

            if (!input_file.is_open()) {
                LOG_ERROR("Can't open file: {}", url);
                return false;
            }

            // 获取文件大小
            input_file.seekg(0, std::ios::end);
            std::streamsize body_size = input_file.tellg();
            input_file.seekg(0, std::ios::beg);

            // 发送响应头
            send_response_header(body_size);
            
            // 发送文件内容
            while (input_file) {
                input_file.read(buffer, 1024);
                std::streamsize bytesRead = input_file.gcount();
        
                if (bytesRead > 0) {
                    handle_tx_(buffer, bytesRead);
                }

                if (input_file.bad()) {
                    LOG_ERROR("read file {} error!", url);
                    input_file.close();
                    return false;
                }

                if (input_file.eof() && bytesRead == 0) {
                    break;
                }
            }

            // 关闭文件
            input_file.close();
            return true;            
        }

        /// \brief engine_static_process
        /// - This method is used to dynamic engine process. 
        /// \return the final for the process
        bool engine_dynamic_process(const std::string &data_str, http_dynamic_map_type map={})
        {
            std::string dynamic_rep;

            if (map.count(protocol_.url_))
            {
                auto dynamic_func = map[protocol_.url_];
                if (dynamic_func) {
                    if(!dynamic_func(protocol_.url_, data_str, dynamic_rep)) {
                        dynamic_rep = "dynamic process failed";
                    }
                } else {
                    dynamic_rep = "no support dynamic process";
                }
            } else {
                dynamic_rep = "404 Not Found";
            }

            handle_tx_(resource_.http_resp_header_resource[HEAD_RESP_200].c_str(), resource_.http_resp_header_resource[HEAD_RESP_200].size());

            auto &&header_extra_str = format_web_header_info(FILE_TYPE_JSON, dynamic_rep.size());
            handle_tx_(header_extra_str.c_str(), header_extra_str.size());
            handle_tx_(dynamic_rep.c_str(), dynamic_rep.size());
            return true;
        }

        /// \brief html404_translate
        /// - This method is used to translate the 404 html.
        void html404_translate(void)
        {
            handle_tx_(resource_.http_resp_header_resource[HEAD_RESP_404].c_str(), resource_.http_resp_header_resource[HEAD_RESP_404].size());
            auto &&header_extra_str = format_web_header_info(FILE_TYPE_HTML, resource_.HTTP_NOT_FOUND_BODY.size());
            handle_tx_(header_extra_str.c_str(), header_extra_str.size());
            handle_tx_(resource_.HTTP_NOT_FOUND_BODY.c_str(), resource_.HTTP_NOT_FOUND_BODY.size());
        }

        // 使用 C++11 实现 endsWith 函数
        bool endsWith(const std::string& str, const std::string& suffix) {
            if (suffix.size() > str.size()) return false;
            return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }
        
    private:
        /// \brief tx_size_
        /// - the tx size
        int tx_size_{0};

        /// \brief protocol_
        /// - the http protocol info  
        http_protocol protocol_;

        /// \brief handle_tx_
        /// - tx handler
        http_tx_func_type handle_tx_;

        /// \brief resource_
        /// - the resource for the http engine
        const RESOURCE resource_;
    };

    /// \brief http_server
    /// - This class is used to implement the http server.
    class http_server
    {
    public:
        /// \brief constructor
        http_server() {}

        /// \brief destructor   
        ~http_server() {}

        /// \brief init
        /// - This method is used to init the object.
        /// \param ip - ipaddr used for init.
        /// \param port - port used for init.
        /// \return Wheather initialization is success or failed.
        bool init (const std::string &ip, int port, http_dynamic_map_type map = {}, int maxl = 32) {
            ipaddr_ = ip;
            port_ = port;
            max_listen_ = maxl;
            dynamic_map_ = map;
            return true;
        }

        /// \brief register_dynamic_process
        /// - This method is used to register the dynamic process.
        /// \param url - url used for register.
        /// \param func - func used for register.
        /// \return Wheather initialization is success or failed.
        void register_dynamic_process(const std::string &url, http_dynamic_func_type func)
        {
            dynamic_map_[url] = func;
        }

        /// \brief start
        /// - This method is used to start the object.
        /// \param dynamic_process - dynamic process func.
        /// \return Wheather initialization is success or failed.
        bool start() 
        {
            socket_fd_ = socket(PF_INET, SOCK_STREAM, 0);
            if (socket_fd_ < 0) {
                LOG_ERROR("socket init error!");
                return false;
            }

            // 配置运行绑定在TIME_WAIT的接口
            int flag = 1;
            if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) {
                LOG_ERROR("setsockopt failed!");
                socket_fd_ = -1;
                return false;
            }

            // 启动线程
            std::thread(std::bind(&http_server::run, this)).join();

            return true;
        }

    private:
        /// \brief run
        /// - This method is for thread accept loop run.
        void run() 
        {
            struct sockaddr_in servaddr, clientaddr;

            // 绑定socket接口到指定的ip地址和端口(必须为本机IP或者127.0.0.1循环地址)
            memset(&servaddr, 0, sizeof(servaddr));    
            servaddr.sin_family = AF_INET;     
            servaddr.sin_addr.s_addr = inet_addr(ipaddr_.c_str());  
            servaddr.sin_port = htons(port_);

            do 
            {
                if (bind(socket_fd_, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {      
                    LOG_MESSAGE("socket bind failed, ip:{}, port:{}!", ipaddr_, port_);;
                    sleep(5);
                    continue;     
                } else {
                    LOG_MESSAGE("socket bind ok, ip:{}, port:{}!", ipaddr_, port_);
                    break;  
                }
            }while(1);

            //监听socket通讯
            listen(socket_fd_, max_listen_);

            while (1) {
                int client_fd;
                uint32_t client_size; 

                client_size = sizeof(clientaddr);
                
                //等待客户端的TCP进行三次握手连接
                client_fd = accept(socket_fd_, (struct sockaddr *)&clientaddr, &client_size);
                if (client_fd < 0) {
                    LOG_ERROR("socket accept failed!");
                    continue;
                } else {
                    // 启动线程
                    std::thread(std::bind(&http_server::recv_process_task, this, client_fd)).detach();
                }         
            }    
            close(socket_fd_);
        }

        /// \brief recv_process_task
        /// - This method is for thread recv loop run.
        /// \param fd - client fd.    
        void recv_process_task(int fd) 
        {
            char recvbuf[1024] = {0};
            int recv_len;
            http_engine engine([&](const char *ptr, int size){
                write(fd, ptr, size);
            });
            std::string out_str;

            memset(recvbuf, 0, sizeof(recvbuf));

            //接收客户端发送的数据
            recv_len = recv(fd, recvbuf, sizeof(recvbuf), 0);
            if (recv_len > 0) {  
                LOG_DEBUG("recv len: {}", recv_len);
                recvbuf[recv_len] = '\0';
                
                if(engine.process(recvbuf, recv_len, dynamic_map_)) {
                    LOG_INFO("server callback ok");;
                }
            } else if (recv_len == 0) {
                LOG_ERROR("server recv zero, break!");
            } else {
                LOG_ERROR("server recv failed, break!");
            }

            close(fd);
        }

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

        /// \brief dynamic_map_
        /// - dynamic map.
        http_dynamic_map_type dynamic_map_;
    };
}
