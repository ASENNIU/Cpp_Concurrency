//
// Created by 13345 on 2023/7/10.
// 代码清单3.11 线程安全的延迟初始化
//

#ifndef CPP_CONCURRENCY_LAZY_INITIALIZATION_H
#define CPP_CONCURRENCY_LAZY_INITIALIZATION_H

#include <memory>
#include <mutex>

std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;
void init_resource()
{
    resource_ptr.reset(new some_resource);
}

void foo()
{
    // 如果init_resource是类成员函数，则还需传入类的this指针
    // std::call_once(resource_flag, &X::init_resource, this)
    std::call_once(resource_flag, init_resource);
    resource_ptr->do_something();
}

#endif //CPP_CONCURRENCY_LAZY_INITIALIZATION_H
