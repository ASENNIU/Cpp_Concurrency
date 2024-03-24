//
// Created by 13345 on 2024/3/24.
//

#include "lock_free_stack.h"

#include <memory>
#include <iostream>

int main()
{
    lock_free_stack<int> st{};

    int a = 10, b = 30, c = 20;
    st.push(a);
    st.push(b);
    st.push(c);

    std::shared_ptr<int> p(st.pop()) ;

    std::cout << *p << std::endl;
//    std::shared_ptr<int> p2 = st.pop();
    p = st.pop();
    std::cout << *p << std::endl;

    return 0;
}