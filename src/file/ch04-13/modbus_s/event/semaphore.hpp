//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2022-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      semaphore.hpp
//
//  Purpose:
//      信号量, 用于线程间通讯的触发, 支持长等待和超时模式
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/19/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
_Pragma("once")

#include <mutex>
#include <condition_variable>

#define TIME_ACTION_ALWAYS 0xFFFFFFFF

namespace EVENT
{

class semaphore
{
public:
    /// \brief constructor
    semaphore(int count = 0):wakeups_(count) {}

    /// \brief destructor
    virtual ~semaphore() {}
    
    /// \brief signal
    /// - This method is used to trigger signal.
    void signal()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++wakeups_;
        cv_.notify_one();
    }
    
    /// \brief wait
    /// - This method is used to wait signal with timeout.
    /// \param timeout - signal timeout
    /// \return wheather success wait the singal
    bool wait(uint32_t timeout)
    {
        bool ret;

        std::unique_lock<std::mutex> lock(mutex_);
        ret = cv_.wait_for(lock, std::chrono::milliseconds(timeout), [this]()->bool{
            return wakeups_ > 0;
        });
        if (ret)
            --wakeups_;
        
        return ret;
    }

    /// \brief wait
    /// - This method is used to wait signal forever.
    /// \return wheather success wait the singal
    bool wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        
        //在这一步释放了lock, 同时进行解锁
        cv_.wait(lock, [this]()->bool{
            return wakeups_ > 0;
        });

        --wakeups_;

        return true;
    }

private:
    /// \brief mutex_
    /// - the mutex to protect signal.
    std::mutex mutex_;

    /// \brief cv_
    /// - signal process object.
    std::condition_variable cv_;

    /// \brief wakeups_
    /// - signal protect data.
    int wakeups_;
};

}
