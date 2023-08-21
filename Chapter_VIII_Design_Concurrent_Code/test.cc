//
// Created by 13345 on 2023/8/21.
//

#include <cstdio>
#include <thread>

struct callable
{
    void operator()(int a, int b)
    {
        printf("a + b = %d\n", a + b);
    }
};

void func(int a, int b)
{
    printf("thread: a + b = %d\n", a + b);
}

int main()
{
    int a = 1, b = 1;
    callable()(a, b);
    std::thread t(func, a, b);
    t.join();
    return 0;
}
