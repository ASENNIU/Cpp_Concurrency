//
// Created by 13345 on 2023/7/6.
// 代码清单2.7 joining_thread类
// 执行析构函数时，线程即能自动汇合
//

#ifndef CPP_CONCURRENCY_JTHREAD_H
#define CPP_CONCURRENCY_JTHREAD_H

#include <thread>

class joining_thread
{
    std::thread t;
public:
    joining_thread() noexcept=default;
    template<typename Callable, typename ... Args>
    explicit joining_thread(Callable&& func, Args&& ... args):
        t(std::forward<Callable>(func), std::forward<Args>(args)...) {}
    //thread对象不可复制，只能转移控制权，所以这里的参数类型为std::thread t_，无论传入的参数类型是什么，资源控制权都将被转移
    explicit joining_thread(std::thread t_) noexcept : t(std::move(t_)) {}
    joining_thread(joining_thread&& other) noexcept : t(std::move(other.t)) {}
    joining_thread& operator=(joining_thread&& other) noexcept
    {
        if (joinable())
            join();
        t = std::move(other.t);
        return *this;
    }
    joining_thread& operator=(std::thread other) noexcept
    {
        if (joinable())
            join();
        t = std::move(other);
        return *this;
    }
    ~joining_thread() noexcept
    {
        if (joinable())
            join();
    }
    void swap(joining_thread& other) noexcept
    {
        t.swap(other.t);
    }
    //const后置修饰函数，显示告诉编译器，该函数内未对对象进行修改
    std::thread::id get_id() const noexcept
    {
        return t.get_id();
    }
    bool joinable() const noexcept
    {
        return t.joinable();
    }
    void join()
    {
        t.join();
    }
    void detach()
    {
        t.detach();
    }
    std::thread& as_thread() noexcept
    {
        return t;
    }
    const std::thread& as_thread() const noexcept
    {
        return t;
    }
};


#endif //CPP_CONCURRENCY_JTHREAD_H
