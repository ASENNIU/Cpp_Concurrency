//
// Created by 13345 on 2023/9/3.
//

#ifndef CPP_CONCURRENCY_INTERRUPTIBLE_THREAD_H
#define CPP_CONCURRENCY_INTERRUPTIBLE_THREAD_H

#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>

class interrupt_flag
{
private:
    std::atomic<bool> flag;
    std::condition_variable* thread_cond;
    std::condition_variable_any* thread_cond_any;
    std::mutex set_clear_mutex;
public:
    interrupt_flag() : thread_cond(0), thread_cond_any(0) {}

    void set()
    {
        flag.store(true, std::memory_order_relaxed);
        std::lock_guard<std::mutex> lk(set_clear_mutex);
        if (thread_cond)
            thread_cond->notify_all();
        else if (thread_cond_any)
            thread_cond_any->notify_all();
    }

    bool is_set() const
    {
        return flag.load(std::memory_order_relaxed);
    }

    void set_condition_variable(std::condition_variable& cv)
    {
        std::lock_guard<std::mutex> lk(set_clear_mutex);
        thread_cond = &cv;
    }

    void clear_condition_variable()
    {
        std::lock_guard<std::mutex> lk(set_clear_mutex);
        thread_cond = 0;
    }


};

thread_local interrupt_flag this_thread_interrupt_flag;

struct clear_cv_on_destruct
{
    ~clear_cv_on_destruct()
    {
        this_thread_interrupt_flag.clear_condition_variable();
    }
};

void interruption_point()
{
    if (this_thread_interrupt_flag.is_set())
        throw thread_interrupted();
}

void interruptible_wait(std::condition_variable& cv,
                        std::unique_lock<std::mutex>& lk)
{
    interruption_point();
    this_thread_interrupt_flag.set_condition_variable(cv);
    clear_cv_on_destruct guard;
    interruption_point();
    cv.wait_for(lk, std::chrono::milliseconds(1));
    interruption_point();
}

class interruptible_thread
{
    std::thread internal_thread;
    interrupt_flag* flag;
public:
    template<typename FunctionType>
    interruptible_thread(FunctionType f)
    {
        std::promise<interrupt_flag*> p;
        internal_thread = std::thread([f, &p] {
            p.set_value(&this_thread_interrupt_flag);
            try {
                f();
            }
            catch (thread_interrupted&)
            {}
        });
        flag = p.get_future().get();
    }
    void interrupt()
    {
        if (flag)
            flag->set();
    }
};


#endif //CPP_CONCURRENCY_INTERRUPTIBLE_THREAD_H
