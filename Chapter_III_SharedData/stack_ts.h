//
// Created by 13345 on 2023/7/7.
// 代码清单3.4 线程安全的stack
// 设计的目标是将top()和pop()合在一起（考虑数据结构本身的线程安全性），那么获取栈顶元素就有两种方式：
// 1）传一个参数进去，这要求提前构造一个栈元素的实例，且栈存储的元素本身课赋值
// 2) 以某种形式返回栈顶元素
//

#ifndef CPP_CONCURRENCY_STACK_TS_H
#define CPP_CONCURRENCY_STACK_TS_H

#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack: std::exception
{
    const char* what() const throw();
};

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    // mutable可以用来修饰一个类的成员变量。被 mutable 修饰的变量，将永远处于可变的状态，即使是 const 函数中也可以改变这个变量的值
    mutable std::mutex m;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};


#endif //CPP_CONCURRENCY_STACK_TS_H
