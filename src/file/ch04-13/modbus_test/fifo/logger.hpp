
#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <fstream>
#include <iomanip>

#define PROGRAM_NAME    "modbus_test"
#define LOG_MODULE_ON   1

class logger_manage
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
    logger_manage() {};

    /// \brief destructor
    ~logger_manage() {};

    /// @brief oss_
    /// - stringostream for formatting the log message.
    std::ostringstream oss_;

    static logger_manage *get_instance() {
        static logger_manage instance;
        return &instance;
    }

    /// @brief logger_message
    /// - This method is used to print the log message.
    /// @param format - the string base format.
    /// @param args - args process int the format
    template<typename... Args>
    void logger_message(bool with_endl, const std::string& format, Args... args) {
        format_message(format, args...);
        if(with_endl){
            oss_ << "\n";
        } 
    }

    /// @brief logger_message
    /// - This method is used to print the log message.
    /// @param level - the logger_manage level.
    /// @param time - the time of the log message.
    /// @param format - the string base format.
    /// @param args - args process int the format   
    template<typename... Args>
    void logger_message(LOGGER_LEVEL level, uint32_t time, const std::string& format, Args... args) {
        if (level >= level_) {
            std::lock_guard<std::mutex> lock(log_mutex_);
            oss_.str("");
            logger_message(false, "[{}][{}][{}]:", PROGRAM_NAME, convert_to_clock_day(time), static_cast<int>(level));
            logger_message(true, format, args...);
            write_oss_info();
        }
    }

    /// @brief write_oss_info
    /// - This method is used to write the log message to the output stream.
    virtual void write_oss_info(void) {
        std::cout << oss_.str();
    }

    /// @brief logger_message
    /// - This method is used to update logger_manage level
    /// @param level - the logger_manage level.
    void set_log_level(LOGGER_LEVEL level) {
        level_ = level;
    }
private:
    /// @brief error level
    /// - the error level for the log message.
    LOGGER_LEVEL level_{LOGGER_LEVEL::DEBUG};

    /// @brief log_mutex_
    /// - mutex for thread-safe logging
    mutable std::mutex log_mutex_;

    /// @brief format_message
    /// - This method is used to format message.
    /// @param format - the string base format.
    /// @param value - the value to be formatted.
    /// @param args - args process int the format
    template<typename T, typename... Args>
    void format_message(const std::string& format, T value, Args... args) {
        size_t pos = format.find("{}");
        if(pos == std::string::npos) {
            oss_ << format;
            return;
        }
        oss_ << format.substr(0, pos);
        oss_ << value;
        format_message(format.substr(pos + 2), args...);
    }

    // @brief format_message
    /// - This final method while format message
    /// @param oss - the stringstream.
    /// @param format - the string base format.
    void format_message(const std::string& format) {
        oss_ << format;
    }

    /// \brief convert_timer
    /// - This method convert second to home.
    /// \param timer - timer by second to convert.
    /// \return string after convert.
    std::string convert_to_clock_day(uint32_t timer)
    {
        uint32_t days = timer / (60 * 60 * 24);
        timer %= (60 * 60 * 24);
        uint32_t hours = timer / (60 * 60);
        timer %= (60 * 60);
        uint32_t minutes = timer / 60;
        uint32_t seconds = timer % 60;
    
        std::ostringstream oss;
        oss << days << " "
            << std::setw(2) << std::setfill('0') << hours << ":" 
            << std::setw(2) << std::setfill('0') << minutes << ":"
            << std::setw(2) << std::setfill('0') << seconds;
        return oss.str();
    }
};

#if LOG_MODULE_ON == 1
#define LOG_MESSAGE(level, time, ...)    do {  \
    logger_manage::get_instance()->logger_message(level, time, __VA_ARGS__); \
}while(0);
#define LOG_DEBUG(time, ...)      do { \
    LOG_MESSAGE(logger_manage::LOGGER_LEVEL::DEBUG, time, __VA_ARGS__); \
}while(0);          
#define LOG_INFO(time, ...)       do {  \
    LOG_MESSAGE(logger_manage::LOGGER_LEVEL::INFO, time, __VA_ARGS__); \
}while(0);
#define LOG_WARN(time, ...)       do {  \
    LOG_MESSAGE(logger_manage::LOGGER_LEVEL::WARN, time, __VA_ARGS__); \
}while(0);
#define LOG_ERROR(time, ...)      do {  \
    LOG_MESSAGE(logger_manage::LOGGER_LEVEL::ERROR, time, __VA_ARGS__); \
}while(0);
#define LOG_FATAL(time, ...)      do {  \
    LOG_MESSAGE(logger_manage::LOGGER_LEVEL::FATAL, time, __VA_ARGS__); \
}while(0);
#else
#define LOG_MESSAGE(...)
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...)
#endif