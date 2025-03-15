#include <thread>
#include <iostream>
#include <pthread.h>
#include <atomic>
#include <functional>

class TIME_MANAGE_POSIX
{
public:
    /// \brief constructor
    TIME_MANAGE_POSIX() = default;
    TIME_MANAGE_POSIX(const TIME_MANAGE_POSIX &time) = delete;

    /// \brief destructor
    ~TIME_MANAGE_POSIX(){
        stop();

        pthread_cond_destroy(&m_cond_);
        pthread_mutex_destroy(&m_mutex_);
    }

    /// \brief start
    /// - This method is used to start TIME_MANAGE_POSIX thread
    /// \param interval - time for the thread run period
    /// \param task - task when the thread run.
    void start(int interval, std::function<void()> task)
    {
        pthread_condattr_t attr;

        millisecond_ = interval;
        timeout_handler_ = task;

        pthread_mutex_init(&m_mutex_, NULL);
        pthread_condattr_init(&attr);
        pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
        pthread_cond_init(&m_cond_, &attr); 

        //start thread
        std::thread(std::bind(&TIME_MANAGE_POSIX::run, this)).detach();
    }

    /// \brief stop
    /// - This method is used to stop one TIME_MANAGE_POSIX thread
    void stop()
    {
        if (stop_loop_)
            return;
        
        stop_loop_ = true;
        pthread_mutex_lock(&m_mutex_);
        pthread_cond_broadcast(&m_cond_);
        pthread_mutex_unlock(&m_mutex_);
    }

private:
    /// \brief run
    /// - This method is used to run the koop by thread
    void run()
    {
        struct timespec ts;
        uint64_t millisecond;

        stop_loop_ = false;

        while (!stop_loop_)
        {
            clock_gettime(CLOCK_MONOTONIC, &ts);
            millisecond = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000) + millisecond_;
            ts.tv_sec = millisecond / 1000;
            ts.tv_nsec = (millisecond - ((millisecond / 1000) * 1000)) * 1000 * 1000;

            pthread_mutex_lock(&m_mutex_);
            auto stauts = pthread_cond_timedwait(&m_cond_, &m_mutex_, &ts);
            pthread_mutex_unlock(&m_mutex_);

            if (stauts == ETIMEDOUT)
            {
                if (timeout_handler_)
                {
                    timeout_handler_();
                }
            }
            else
            {
                std::cout<<"TIME_MANAGE_POSIX finished\n";
            }
        }
    }

private:
    /// \brief m_cond_
    /// - cond used to wait timeout.
    pthread_cond_t m_cond_;

    /// \brief m_mutex
    /// - mutex protect
    pthread_mutex_t m_mutex_;

    /// \brief millisecond_
    /// - ms wait for timeout.
    uint64_t millisecond_{1};

    /// \brief timeout_handler_
    /// - function run when timeout.
    std::function<void()> timeout_handler_;

    /// \brief stop_loop_
    /// - loop flag until stop.
    std::atomic<bool> stop_loop_{false};
};

int main(int argc, char* argv[])
{
    int index = 0;
    TIME_MANAGE_POSIX tmc;

    tmc.start(1000, [&](){
        std::cout<<"posix timer loop: "<<index++<<std::endl;
    });

    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (index == 5) {
            tmc.stop();
            index++;
        } else if (index > 5) {                  
            index = 0;
            tmc.start(1000, [&](){
                std::cout<<"posix new timer loop: "<<index++<<std::endl;
            });
        }
    }
    return 0;
}
