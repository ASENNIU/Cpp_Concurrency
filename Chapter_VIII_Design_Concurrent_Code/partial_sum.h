//
// Created by 13345 on 2023/8/21.
//

#ifndef CPP_CONCURRENCY_PARTIAL_SUM_H
#define CPP_CONCURRENCY_PARTIAL_SUM_H

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

struct barrier
{
    std::atomic<unsigned> count;
    std::atomic<unsigned> spaces;
    std::atomic<unsigned> generation;
    barrier(unsigned count_) : count(count_), spaces(count), generation(0) {}

    void wait()
    {
        unsigned const gen = generation.load();
        if (!--spaces)
        {
            spaces = count.load();
            ++generation;
        } else
        {
            while (generation.load() == gen)
                std::this_thread::yield();
        }
    }

    void done_waiting()
    {
        --count;
        if (!--spaces)
        {
            spaces = count.load();
            ++generation;
        }
    }
};

template<typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last)
{
    typedef typename Iterator::value_type value_type;

    struct process_element
    {
        void operator()(Iterator first, Iterator last,
                        std::vector<value_type>& buffer,
                        unsigned i, barrier& b)
        {
            value_type& ith_element = *(first + i);
            bool update_source = false;

            for (unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2)
            {
                bool read_flag = step &  1;
                value_type const& source = read_flag ? buffer[i] : ith_element;
                value_type& dest = read_flag ? ith_element : buffer[i];
                value_type const& addend = read_flag == 0 ? buffer[i - stride] : *(first + i - stride);

                dest = source + addend;
                update_source = !read_flag;
                b.wait();
            }

            if (update_source)
                ith_element = buffer[i];
            b.done_waiting();
        }
    };

    unsigned long const length = std::distance(first, last);
    if (length <= 1)
        return;

    std::vector<value_type> buffer(length);
    barrier b(length);

    std::vector<std::thread> threads(length - 1);
    join_threads joiner(threads);

    Iterator block_start = first;
    for (unsigned long i = 0; i < (length - 1); ++i)
    {
        threads[i] = std::thread(process_element(), first, last, std::ref(buffer), i, std::ref(b));
    }
    process_element()(first, last, buffer, length - 1, b);
}


#endif //CPP_CONCURRENCY_PARTIAL_SUM_H
