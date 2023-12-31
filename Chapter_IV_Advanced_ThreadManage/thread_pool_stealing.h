//
// Created by 13345 on 2023/9/2.
// thread_local
// 1. thread_local描述的对象在thread开始时分配，而在thread结束时分解。
// 2. 一般在声明时赋值，在本thread中只执行一次。
// 3. 描述的对象依然只在作用域内有效。
// 4. 描述类成员变量时，必须是static的。
//

#ifndef CPP_CONCURRENCY_THREAD_POOL_STEALING_H
#define CPP_CONCURRENCY_THREAD_POOL_STEALING_H

#include "threadsafe_queue_complex.h"
#include "utils.h"
#include <atomic>
#include <memory>
#include <vector>
#include <thread>
#include <deque>
#include <future>
#include <cstdio>

class function_wrapper
{
    struct impl_base {
        virtual void call() = 0;
        virtual ~impl_base() = default;
    };

    std::unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type : impl_base
    {
        F f;
        impl_type(F&& f_) : f(std::move(f_)) {}
        void call() override {
            f();
        }
    };

public:
    template<typename F>
    function_wrapper(F&& f) : impl(new impl_type<F>(std::move(f))) {}
    function_wrapper() = default;
    function_wrapper(function_wrapper&& other)  noexcept : impl(std::move(other.impl)) {}
    function_wrapper& operator=(function_wrapper&& other)
 noexcept     {
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

class work_stealing_queue
{
private:
    typedef function_wrapper data_type;
    std::deque<data_type> the_queue;
    mutable std::mutex the_mutex;
public:
    work_stealing_queue() = default;
    work_stealing_queue(const work_stealing_queue&)=delete;
    work_stealing_queue& operator=(const work_stealing_queue&)=delete;
    void push(data_type data)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        the_queue.push_front(std::move(data));
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        return the_queue.empty();
    }
    bool try_pop(data_type& res)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        if (the_queue.empty())
            return false;
        res = std::move(the_queue.front());
        the_queue.pop_front();
        return true;
    }
    bool try_steal(data_type& res)
    {
        std::lock_guard<std::mutex> lock(the_mutex);
        if (the_queue.empty())
            return false;
        res = std::move(the_queue.back());
        the_queue.pop_back();
        return true;
    }
};

class thread_pool
{
    typedef function_wrapper task_type;
    std::atomic_bool done;
    threadsafe_queue<task_type> pool_work_queue;
    std::vector<std::unique_ptr<work_stealing_queue>> queues;
    std::vector<std::thread> threads;
    join_threads joiner;
    // thread_local作为类成员变量时必须是static的
    static thread_local work_stealing_queue* local_work_queue;
    static thread_local unsigned my_index;

    void worker_thread(unsigned index)
    {
        my_index = index;
        local_work_queue = queues[index].get();
        while (!done)
        {
            run_pending_task();
        }
    }

    bool pop_task_from_local_queue(task_type& task)
    {
        if (local_work_queue && local_work_queue->try_pop(task)){
            printf("thread %d pop task from local queue\n", my_index);
            return true;
        }

        return false;
    }

    bool pop_task_from_pool_queue(task_type& task)
    {
        if (pool_work_queue.try_pop(task)) {
            printf("thread %d pop task from pool queue\n", my_index);
            return true;
        }

        return false;
    }

    bool pop_task_from_other_thread_queue(task_type& task)
    {
        for (unsigned i = 0; i < queues.size(); ++i)
        {
            unsigned const index = (my_index + i + 1) % queues.size();
            if (queues[index]->try_steal(task)){
                printf("thread %d pop task from %d thread_queue\n", my_index, index);
                return true;
            }

        }
        return false;
    }

public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try
        {
            for (unsigned i = 0; i < thread_count; ++i)
            {
                queues.push_back(std::make_unique<work_stealing_queue>());
            }
            for (unsigned i = 0; i < thread_count; ++i)
            {
                threads.emplace_back(&thread_pool::worker_thread, this, i);
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
        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());

        // 这里的实现任务始终只会提交到pool_work_queue，如果想要提交到各个线程的队列，需要进行适当的设计，提交到各线程的队列
        if (local_work_queue)
            local_work_queue->push(std::move(task));
        else
            pool_work_queue.push(std::move(task));
//        if (my_index != -1)
//        {
//            printf("queue %d push task\n", my_index);
//            queues[my_index]->push(std::move(task));
//        }
//        else
//            pool_work_queue.push(std::move(task));
        return res;
    }

    void run_pending_task()
    {
        task_type task;
        if (pop_task_from_local_queue(task) ||
            pop_task_from_pool_queue(task) ||
            pop_task_from_other_thread_queue(task))
        {
            task();
        }
        else
            std::this_thread::yield();
    }
};

thread_local work_stealing_queue* thread_pool::local_work_queue = nullptr;
thread_local unsigned thread_pool::my_index = -1;

#endif //CPP_CONCURRENCY_THREAD_POOL_STEALING_H
