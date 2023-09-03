//
// Created by 13345 on 2023/9/3.
//

//#include "thread_pool.h"
#include "thread_pool_stealing.h"
//#include <mutex>
#include <cstdio>
#include <memory>
#include <functional>

int func(int a, int b)
{
    printf("%d + %d = %d\n", a, b, a +b);
    return a + b;
}

int main()
{
    std::unique_ptr<thread_pool> pool_ptr(new thread_pool);
    std::vector<std::future<int>> res;
    for (unsigned i = 0;  i < 100; ++i) {
        std::future<int> ans = pool_ptr->submit([i, capture0 = i + 1] { return func(i, capture0); });
        res.push_back(std::move(ans));
    }
    for (auto & re : res)
        printf("%d\n", re.get());
    return 0;
}