#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <unistd.h>

class TIME_MANAGE_CV
{
public:
    TIME_MANAGE_CV() = default;
    TIME_MANAGE_CV(const TIME_MANAGE_CV& timer) = delete;

    ~TIME_MANAGE_CV() {
        stop();
    }

    /// \brief start
    /// 启动线程运行，实现定时器功能
    /// \param interval - 定时器执行的间隔时间
    /// \param task - 定时器执行的回调函数
    void start(int interval, std::function<void()> task)
    {
        if (is_running_)
            return;

        millisecond_ = interval;
        timeout_handler_ = task;
        is_running_ = true;

        //start thread
        std::thread(std::bind(&TIME_MANAGE_CV::run, this)).detach();
    }

    /// @brief stop
    /// 运行停止函数，结束循环执行
    void stop() {
        if(!is_running_) {
            return;
        }

        is_running_ = false;
        cond_.notify_all();
    }

private:
    /// \brief run
    /// 线程执行函数，该函数在独立线程中执行，用于实现超时循环
    void run() {
        while(is_running_) {
            std::unique_lock<std::mutex> lock{mutex_};
            auto stauts = cond_.wait_for(lock, std::chrono::milliseconds(millisecond_));
            if (stauts == std::cv_status::timeout) {
                if (timeout_handler_) {
                    timeout_handler_();
                }
            } else {
                break;
            }
        }
        std::cout<<"thread finished!\n";
    }

private:
    /// \brief is_running_
    /// - 判断线程释放允许
    std::atomic<bool> is_running_ = false;

    /// \brief mutex_
    /// - mutex_ used for condition.
    std::mutex mutex_;

    /// \brief cond_
    /// - 线程通讯变量
    std::condition_variable cond_;

    /// \brief timeout_handler_
    /// - 超时执行函数
    std::function<void()> timeout_handler_;

    /// \brief millisecond_
    /// - 等待的毫秒超时时间
    uint32_t millisecond_{1};
};

int main(int argc, char* argv[])
{
    // 创建定时器对象
    TIME_MANAGE_CV mcv;
    int index = 0;

    // 实现定时器和回调函数
    mcv.start(1000, [&](){
        std::cout<<"cv timer loop: "<<index++<<std::endl;
    });

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (index == 5) {
            mcv.stop();
            index++;
        } else if (index > 5) {                  
            index = 0;
            mcv.start(1000, [&](){
                std::cout<<"cv new timer loop: "<<index++<<std::endl;
            });
        }
    }
    return 0;
}
