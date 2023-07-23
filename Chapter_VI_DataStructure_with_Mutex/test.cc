//
// Created by 13345 on 2023/7/23.
//

#include "threadsafe_queue_complex.h"
#include "threadsafe_lookup_table.h"
#include "threadsafe_list.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <utility>




void test_threadsafe_queue();
void push_data_for_queue(std::shared_ptr<threadsafe_queue<int>> queue_ptr);
void pop_data_for_queue(std::shared_ptr<threadsafe_queue<int>> queue_ptr);

void test_threadsafe_lookup_table();
void push_data_for_lookup_table(std::shared_ptr<threadsafe_lookup_table<std::string, int>> map_ptr,
                                std::vector<std::pair<std::string, int>> const& data);
void lookup_data_for_lookup_table(std::shared_ptr<threadsafe_lookup_table<std::string, int>> map_ptr,
                                  std::vector<std::pair<std::string, int>> const& data);
void remove_data_for_lookup_table(std::shared_ptr<threadsafe_lookup_table<std::string, int>> map_ptr,
                                  std::vector<std::pair<std::string, int>> const& data);

void test_threadsafe_list();
void push_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr,
                                std::vector<std::string> const& data);
void lookup_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr,
                          std::vector<std::string> const& data);
void remove_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr,
                          std::vector<std::string> const& data);
void foreach_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr);




int main()
{
    //test_threadsafe_lookup_table();
    test_threadsafe_list();
    return 0;
}

void push_data_for_queue(std::shared_ptr<threadsafe_queue<int>> queue_ptr)
{
    int value = 0;
    for (unsigned int i = 0; i < 100; ++i)
    {
        queue_ptr->push(value);
        printf("push data %d.\n", value);
        ++value;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void pop_data_for_queue(std::shared_ptr<threadsafe_queue<int>> queue_ptr)
{
    for (unsigned int i = 0; i < 100; ++i)
    {
        std::shared_ptr<int> value = queue_ptr->wait_and_pop();
        printf("pop data %d.\n", *value);
        //std::this_thread::sleep_for(200ms);
    }
}

void test_threadsafe_queue()
{
    std::shared_ptr<threadsafe_queue<int>> queue_ptr(new threadsafe_queue<int>);
    std::thread t_push(push_data_for_queue, queue_ptr);
    std::thread t_pop(pop_data_for_queue, queue_ptr);
    t_push.join();
    t_pop.join();
}

void push_data_for_lookup_table(std::shared_ptr<threadsafe_lookup_table<std::string, int>> map_ptr,
                                std::vector<std::pair<std::string, int>> const& data)
{
    for (unsigned int i = 0; i < data.size(); ++i)
    {
        map_ptr->add_or_update_mapping(data[i].first, data[i].second);
        printf("push data (%s, %d)\n", data[i].first, data[i].second);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
void lookup_data_for_lookup_table(std::shared_ptr<threadsafe_lookup_table<std::string, int>> const map_ptr,
                                  std::vector<std::pair<std::string, int>> const& data)
{
    for (unsigned int i = 0; i < data.size(); ++i)
    {
        map_ptr->value_for(data[i].first, -1);
        printf("value for data (%s, %d)\n", data[i].first, data[i].second);
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}
void remove_data_for_lookup_table(std::shared_ptr<threadsafe_lookup_table<std::string, int>> map_ptr,
                                  std::vector<std::pair<std::string, int>> const& data)
{
    for (unsigned int i = 0; i < data.size(); ++i)
    {
        map_ptr->value_for(data[i].first, -1);
        printf("remove data (%s, %d)\n", data[i].first, data[i].second);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}
void test_threadsafe_lookup_table()
{
    std::vector<std::pair<std::string, int>> data{
            {"sssss", 111},
            {"yyyy", 222},
            {"jjjj", 333},
            {"jkjkj", 444},
            {"mmlk", 555}
    };
    std::shared_ptr<threadsafe_lookup_table<std::string, int>> map_ptr(new threadsafe_lookup_table<std::string, int>);
    std::thread t_push(push_data_for_lookup_table, map_ptr, std::ref(data));
    std::thread t_valuefor(lookup_data_for_lookup_table, map_ptr, std::ref(data));
    std::thread t_remove(remove_data_for_lookup_table, map_ptr, std::ref(data));
    t_push.join();
    t_valuefor.join();
    t_remove.join();
}

void test_threadsafe_list()
{
    std::vector<std::string> data{"ssss", "eefdf", "dsfdsdf", "sdeqradf", "erttysa", "retsadfqq"};
    std::shared_ptr<threadsafe_list<std::string>> list_ptr(new threadsafe_list<std::string>);
    std::thread t_push(push_data_for_list, list_ptr, std::ref(data));
    std::thread t_lookup(lookup_data_for_list, list_ptr, std::ref(data));
    std::thread t_remove(remove_data_for_list, list_ptr, std::ref(data));
    std::thread t_foreach(foreach_data_for_list, list_ptr);
    t_push.join();
    t_lookup.join();
    t_remove.join();
    t_foreach.join();
}
void push_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr,
                         std::vector<std::string> const& data)
{
    for (unsigned int i = 0; i < data.size(); ++i)
    {
        list_ptr->push_front(data[i]);
        printf("push data: %s\n", data[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
void lookup_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr,
                          std::vector<std::string> const& data)
{
    for (unsigned int i = 0; i < data.size(); ++i)
    {
        std::shared_ptr<std::string> res =  list_ptr->find_first_if([=](std::string const& item){
            return item == data[i];
        });
        if (res)
            printf("find data: %s\n", *res);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
void remove_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr,
                          std::vector<std::string> const& data)
{
    for (unsigned int i = 0; i < data.size(); ++i)
    {
        list_ptr->remove_if([=](std::string const& item){
            return item == data[i];
        });

        printf("remove data: %s\n", data[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
    }
}
void foreach_data_for_list(std::shared_ptr<threadsafe_list<std::string>> list_ptr)
{
    for (unsigned int i = 0; i < 5; ++i)
    {
        printf("scan table, times:%d: ");
        list_ptr->for_each([](std::string const& item){
            printf("%s ", item);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}