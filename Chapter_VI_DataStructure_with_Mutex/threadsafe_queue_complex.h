//
// Created by 13345 on 2023/7/22.
//

/***
 * 在函数声明时，为了让编译器能够正确识别 node 类型，应该使用 typedef 定义的别名 node。这样，函数签名中的类型名就会变成 node 而不是完整的作用域限定名。
 * 在函数实现时，为了让编译器能够正确找到 node 类型的定义，需要使用完整的作用域限定名 typename threadsafe_queue<T>::node。这样，编译器就能知道 node 的确切类型是 threadsafe_queue<T>::node。
 * 所以，为了保持代码的可读性和一致性，一般来说：在函数声明中，使用 node 类型的 typedef 别名；
 * 在函数实现中，使用完整的作用域限定名 typename threadsafe_queue<T>::node。
 * 这样的代码设计能够确保在整个代码中，编译器都能正确地识别并使用 node 类型。
 */

#ifndef CPP_CONCURRENCY_THREADSAFE_QUEUE_COMPLEX_H
#define CPP_CONCURRENCY_THREADSAFE_QUEUE_COMPLEX_H

#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue
{
private:
    struct node
    {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };
    // 一般来说，在 C++ 中，嵌套结构体在外部访问时需要带有其所属类的作用域
    // 在函数声明时可以使用typedef出来的node，函数实现时需要使用完整的作用域 threadsafe_queue<T>::node
    typedef typename threadsafe_queue<T>::node node;
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    typename threadsafe_queue<T>::node* tail;
    std::condition_variable data_cond;

    typename threadsafe_queue<T>::node* get_tail();
    std::unique_ptr<node> pop_head();
    std::unique_lock<std::mutex> wait_for_data();

    std::unique_ptr<node> wait_pop_head();
    std::unique_ptr<node> wait_pop_head(T& value);
    std::unique_ptr<node> try_pop_head();
    std::unique_ptr<node> try_pop_head(T& value);
public:
    threadsafe_queue() : head(new node), tail(head.get()) {}
    threadsafe_queue(const threadsafe_queue&)=delete;
    threadsafe_queue& operator=(const threadsafe_queue&)=delete;
    std::shared_ptr<T> try_pop();
    bool try_pop(T& value);
    std::shared_ptr<T> wait_and_pop();
    void wait_and_pop(T& value);
    void push(T new_value);
    bool empty();
};

template<typename T>
typename threadsafe_queue<T>::node* threadsafe_queue<T>::get_tail()
{
    std::lock_guard<std::mutex> tail_lock(tail_mutex);
    return tail;
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::pop_head()
{
    std::unique_ptr<node> old_head = std::move(head);
    head = std::move(old_head->next);
    return old_head;
}

template<typename T>
std::unique_lock<std::mutex> threadsafe_queue<T>:: wait_for_data()
{
    /***
     * 等待数据，这个函数抽象的很好
     */
    std::unique_lock<std::mutex> head_lock(head_mutex);
    data_cond.wait(head_lock, [&]{return head.get() != get_tail();});
    return std::move(head_lock);
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::wait_pop_head()
{
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    return pop_head();
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>:: wait_pop_head(T& value)
{
    /**
     * 第一行代码控锁，确保队列里有资源
     * 第二行代码，拿到资源（移动语义），第三行代码弹出队列头元素（队头元素已经不拥有资源了）
     */
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    value = std::move(*head->data);
    return pop_head();
}

template<typename T>
void threadsafe_queue<T>::push(T new_value)
{
    std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
    std::unique_ptr<node> p(new node);
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        tail->data = new_data;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
    data_cond.notify_one();
}

template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
    std::unique_ptr<node> const old_head = wait_pop_head();
    return old_head->data;
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::try_pop_head()
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if (head.get() == get_tail())
    {
        return std::unique_ptr<node>();
    }
    return pop_head();
}

template<typename T>
std::unique_ptr<typename threadsafe_queue<T>::node> threadsafe_queue<T>::try_pop_head(T &value)
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if (head.get() == get_tail())
    {
        return std::unique_ptr<node>();
    }
    value = std::move(*head->data);
    return pop_head();
}

template<typename T>
std::shared_ptr<T> threadsafe_queue<T>::try_pop() {
    std::unique_ptr <node> old_head = try_pop_head();
    return old_head ? old_head->data : std::shared_ptr<T>();
}

template<typename T>
bool threadsafe_queue<T>::try_pop(T &value)
{
    std::unique_ptr<node> const old_head = try_pop_head(value);
    return old_head != nullptr;
}

template<typename T>
bool threadsafe_queue<T>::empty()
{
    std::lock_guard<std::mutex> head_lock(head_mutex);
    return (head.get() == get_tail());
}

#endif //CPP_CONCURRENCY_THREADSAFE_QUEUE_COMPLEX_H
