//
// Created by 13345 on 2023/7/22.
// 代码清单6.11 线程安全的查找表
//

#ifndef CPP_CONCURRENCY_THREADSAFE_LOOKUP_TABLE_H
#define CPP_CONCURRENCY_THREADSAFE_LOOKUP_TABLE_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <list>
#include <vector>
#include <map>
#include <algorithm>

template<typename Key, typename Value, typename Hash=std::hash<Key>>
class threadsafe_lookup_table
{
private:
    class bucket_type
    {
    private:
        typedef std::pair<Key, Value> bucket_value;
        typedef std::list<bucket_value> bucket_data;
        // 这里的 typename 让编译器知道 iterator 是一种类型，而不是 bucket_data 的静态成员。
        typedef typename bucket_data::iterator bucket_iterator;

        // find_entry_for这个函数不知道在哪个地方会修改data，所以这里要声明为mutable（因为find_entry_for是const的）
        mutable bucket_data data;
        mutable std::shared_mutex mutex;

        bucket_iterator find_entry_for(Key const& key) const
        {
            return std::find_if(data.begin(), data.end(),
                                [&](bucket_value const& item)
                                {return item.first == key;});
        }

    public:
        Value value_for(Key const& key, Value const& default_value) const
        {
            std::shared_lock<std::shared_mutex> lock(mutex);
            bucket_iterator const found_entry = find_entry_for(key);
            return (found_entry == data.end()) ? default_value : found_entry->second;
        }

        void add_or_update_mapping(Key const& key, Value const& value)
        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry == data.end())
            {
                data.push_back(bucket_value(key, value));
            }
            else
            {
                found_entry->second = value;
            }
        }

        void remove_mapping(Key const& key)
        {
            std::unique_lock<std::shared_mutex> lock(mutex);
            bucket_iterator const found_entry = find_entry_for(key);
            if (found_entry != data.end())
            {
                data.erase(found_entry);
            }
        }
    };

    std::vector<std::unique_ptr<bucket_type>> buckets;
    Hash hasher;
    bucket_type& get_bucket(Key const& key) const
    {
        std::size_t const bucket_index = hasher(key) % buckets.size();
        return *buckets[bucket_index];
    }

public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef Hash hash_type;

    threadsafe_lookup_table(unsigned num_buckets=19, Hash const& hasher_=Hash()) :
        buckets(num_buckets), hasher(hasher_)
    {
        for (unsigned i = 0; i < num_buckets; ++i)
        {
            buckets[i].reset(new bucket_type);
        }
    }

    threadsafe_lookup_table(threadsafe_lookup_table const&)=default;
    threadsafe_lookup_table& operator=(threadsafe_lookup_table const&)=delete;

    Value value_for(Key const& key, Value const& default_value=Value()) const
    {
        return get_bucket(key).value_for(key, default_value);
    }

    void add_or_update_mapping(Key const& key, Value const& value)
    {
        get_bucket(key).add_or_update_mapping(key, value);
    }

    void remove_mapping(Key const& key)
    {
        get_bucket(key).remove_mapping(key);
    }

    std::map<Key, Value> get_map() const
    {
        std::vector<std::unique_lock<std::shared_mutex>> locks;
        for (unsigned i = 0; i < buckets.size(); ++i)
        {
            locks.push_back(std::unique_lock<std::shared_mutex>(buckets[i].mutex));
        }
        std::map<Key, Value> res;
        for (unsigned i = 0; i < buckets.size(); ++i)
        {
            for (typename std::list<std::pair<Key, Value>>::iterator it = buckets[i].data.begin(); it != buckets[i].data.end(); ++it)
            {
                res.insert(*it);
            }
        }
        return res;
    }
};

#endif //CPP_CONCURRENCY_THREADSAFE_LOOKUP_TABLE_H
