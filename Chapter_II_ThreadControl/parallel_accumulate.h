//
// Created by 13345 on 2023/7/6.
// 代码清单2，9 并行版的std::accumulate()的简单实现
//

#ifndef CPP_CONCURRENCY_PARALLEL_ACCUMULATE_H
#define CPP_CONCURRENCY_PARALLEL_ACCUMULATE_H

#include <thread>
#include <numeric>
#include <iterator>
#include <vector>

template<typename Iterator, typename T>
struct accumulate_block
{
    void operator()(Iterator first, Iterator last, T& result)
    {
        result = std::accmulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    //Returns the number of hops from first to last.
    unsigned long const length = std::distance(first, last);
    if (!length)
        return init;
    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads -1);
    Iterator block_start = first;
    for (unsigned long i = 0; i <  (num_threads - 1); ++i)
    {
        Iterator block_end = block_start;
        //Increments given iterator it by n elements.
        std::advance(block_end, block_size);
        threads[i] = std::thread(
                //显式的向线程传递引用，以获取计算结果
                accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i])
                );
        block_start = block_end;
    }
    accumulate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);

    for (auto& entry: threads)
        entry.join();
    return std::accumulate(results.begin(), results.end(), init);
}

#endif //CPP_CONCURRENCY_PARALLEL_ACCUMULATE_H
