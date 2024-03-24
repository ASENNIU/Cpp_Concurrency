//
// Created by 13345 on 2023/8/3.
//

#ifndef CPP_CONCURRENCY_LOCK_FREE_STACK_H
#define CPP_CONCURRENCY_LOCK_FREE_STACK_H

#include <atomic>
#include <memory>
#include <iostream>

template<typename T>
class lock_free_stack
{
private:
    struct node; // 前向声明，定义栈节点
    struct counted_node_ptr
    {
        int external_count; // 外部计数器，用于管理对node的引用计数
        node* ptr; // 指向实际节点的指针
    };
    struct node
    {
        std::shared_ptr<T> data; // 节点数据，使用智能指针管理
        std::atomic<int> internal_count; // 内部计数器，用于无锁操作中的安全删除
        counted_node_ptr next; // 指向下一个节点的指针，含外部计数
        node(T const& data) : data(std::make_shared<T>(data)), internal_count(0) {}
    };
    std::atomic<counted_node_ptr> head; // 栈顶元素，使用原子操作保证线程安全

    void increase_head_count(counted_node_ptr& old_counter)
    {
        // 尝试安全地增加head节点的外部计数器
        counted_node_ptr new_counter;
        do {
            new_counter = old_counter; // 复制旧计数器状态
            ++new_counter.external_count; // 增加外部计数
        } while (!head.compare_exchange_strong(old_counter, new_counter,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }
public:
    ~lock_free_stack()
    {
        // 析构时清空栈
        while (pop());
    }
    void push(T const& data)
    {
        // 向栈中推入一个元素
        counted_node_ptr new_node;
        new_node.ptr = new node(data); // 创建新节点
        new_node.external_count = 1; // 初始化外部计数器
        new_node.ptr->next = head.load(); // 设置新节点的下一个节点为当前头节点
        // 尝试将head指向新节点，直到成功
        while (!head.compare_exchange_weak(new_node.ptr->next, new_node,
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    }
    std::shared_ptr<T> pop()
    {
        // 从栈中弹出一个元素
        counted_node_ptr old_head = head.load(std::memory_order_relaxed);
        for (;;)
        {
            increase_head_count(old_head); // 增加头节点的外部计数器
            node* const ptr = old_head.ptr;
            if (!ptr)
            {
                // 如果头节点为空，返回空指针
                return std::shared_ptr<T>();
            }
            if (head.compare_exchange_strong(old_head, ptr->next, std::memory_order_relaxed))
            {
                // 尝试将头节点指向下一个节点
                std::shared_ptr<T> res;
                res.swap(ptr->data); // 获取数据
                int const count_increase = old_head.external_count - 2;
                if (ptr->internal_count.fetch_add(count_increase,
                                                  std::memory_order_release) == -count_increase)
                {
                    // 如果内部计数器达到0，安全删除节点
                    delete ptr;
                }
                return res; // 返回弹出的数据
            }
            else if (ptr->internal_count.fetch_add(-1, std::memory_order_relaxed) == 1)
            {
                // 如果减少内部计数器后达到0，安全删除节点
                ptr->internal_count.load(std::memory_order_acquire);
                delete ptr;
            }
        }
    }
};


#endif //CPP_CONCURRENCY_LOCK_FREE_STACK_H
