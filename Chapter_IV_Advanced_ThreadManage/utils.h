//
// Created by 13345 on 2023/9/1.
//

#ifndef CPP_CONCURRENCY_UTILS_H
#define CPP_CONCURRENCY_UTILS_H

class join_threads;
{
std::vector<std::thread>& threads;
public:
explicit join_threads(std::vector<std::thread>& threads_) : threads(threads_) {}
~join_threads()
{
    for (unsigned long i = 0; i < threads.size(); ++i)
    {
        if(threads[i].joinable())
            threads[i].join();
    }
}
}

#endif //CPP_CONCURRENCY_UTILS_H
