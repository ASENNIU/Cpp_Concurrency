//
// Created by 13345 on 2023/9/1.
//

#ifndef CPP_CONCURRENCY_THREAD_POOL_H
#define CPP_CONCURRENCY_THREAD_POOL_H

#include "threadsafe_queue_complex.h"
#include "utils.h"

class thread_pool_naive
{
    /***
     * 不支持等待结果的线程池，任务提交只支持无参数的可调用对象
     */
    std::atomic_bool done;
    threadsafe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;

    void worker_thread()
    {
        while (!done)
        {
            std::function<void()> task;
            if(work_queue.try_pop(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
public:
    thread_pool_naive() : done(false), joiner(threads)
    {
        unsigned  const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                threads.push_back(std::thread(&thread_pool::worker_thread, this));
            }
        }
        catch (...)
        {
            done = true;
            throw;
        }
    }
    ~thread_pool()
    {
        done = true;
    }

    template<typename FunctionType>
    void submit(FunctionType f)
    {
        work_queue.push(std::function<void()>(f));
    }
};


class function_wrapper
{
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() {}
    };

    std::unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type : impl_base
    {
        F f;
        impl_type(F&& f_) : f(std::move(f_)) {}
        void call() {
            f();
        }
    };

public:
    template<typename F>
    function_wrapper(F&& f) : impl(new impl_type<F>(std::move(f))) {}
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other) : impl(std::move(other.impl)) {}
    function_wrapper& operator=(function_wrapper&& other)
    {
        impl = std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&)=delete;
    function_wrapper(function_wrapper&)=delete;
    function_wrapper& operator=(const function_wrapper&)=delete;

    void operator()() {
        impl->call();
    }
};

class thread_pool
{
    std::atomic_bool done;
    threadsafe_queue<function_wrapper> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;

    void worker_thread()
    {
        while (!done)
        {
            function_wrapper task;
            if (work_queue.try_pop(task))
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
public:
    thread_pool_naive() : done(false), joiner(threads)
    {
        unsigned  const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                threads.push_back(std::thread(&thread_pool::worker_thread, this));
            }
        }
        catch (...)
        {
            done = true;
            throw;
        }
    }

    ~thread_pool()
    {
        done = true;
    }

    template<typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        work_queue.push(std::move(task));
        return res;
    }

    void run_pending_task()
    {
        function_wrapper task;
        if (work_queue.try_pop(task))
        {
            task();
        }
        else
        {
            std::this_thread::yield();
        }
    }
};

#endif //CPP_CONCURRENCY_THREAD_POOL_H
