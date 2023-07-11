//
// Created by 13345 on 2023/7/11.
// 代码清单4.12 函数式编程风格的快排（串行实现）
//

#ifndef CPP_CONCURRENCY_QUICK_SORT_H
#define CPP_CONCURRENCY_QUICK_SORT_H

#include <list>
#include <algorithm>
#include <iostream>

std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list)
{
    for (auto& i : list)
        ostr << ' ' << i;

    return ostr;
}

template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input)
{
    if (input.empty())
        return input;
    std::list<T> result;
    // 将元素从一个列表转移到另一个列表。这些元素被插入到pos指向的元素之前。
    // 没有元素被复制或移动，只有列表节点的内部指针被重新指向。
    // 此处仅仅只转移了一个元素
    result.splice(result.begin(), input, input.begin());
    std::cout << "list: " << result << '\n';
    T const& pivot = *result.begin();

    //将范围[first, last]中的元素重新排序，使谓词p返回true的元素排在谓词p返回false的元素之前。元素的相对顺序不保留
    //返回第二组第一个元素的迭代器(第一个使断言失效的迭代器)
    auto divide_point = std::partition(input.begin(), input.end(),
                                       [&](T const& t) {return t < pivot;});
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    auto new_lower(sequential_quick_sort(std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}


#endif //CPP_CONCURRENCY_QUICK_SORT_H
