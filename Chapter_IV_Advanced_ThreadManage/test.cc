//
// Created by 13345 on 2023/9/2.
//

#include <thread>
#include <vector>
#include <atomic>
#include <cstdio>

class A
{
    std::vector<std::thread> threads;
public:
    static thread_local int* p;
    std::atomic_bool done;
    A() : done(false) {
        for(unsigned i = 0; i < 5; ++i)
            threads.push_back(std::thread(&A::fun, this, i));
    };
    ~A() {
        for (unsigned i = 0; i < threads.size(); ++i)
            threads[i].join();
    }
    void fun(int v)
    {
        *p = v;
        while (!done)
            continue;
    }
};

int main()
{
    A a;

    for (unsigned i = 0; i < 1000; ++i) {
        if (a.p)
            printf("%d\n", *a.p);
        else
            printf("-1\n");
    }
    a.done = true;
    return 0;
}
