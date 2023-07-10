//
// Created by 13345 on 2023/7/10.
// 读写锁（共享锁和排他锁）的简单应用，基于C++ 14（shared_mutex是C++14的特性）
//

#ifndef CPP_CONCURRENCY_READ_WRITE_LOCK_H
#define CPP_CONCURRENCY_READ_WRITE_LOCK_H

#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>

class dns_entry;
class dns_cache
{
    std::map<std::string, dns_entry> entries;
    mutable std::shared_mutex entry_mutex;
public:
    dns_entry find_entry(std::string const& domain) const
    {
        std::shared_lock<std::shared_mutex> lk(entry_mutex);
        std::map<std::string, dns_entry>::const_iterator const it = entries.find(domain);
        return (it == entries.end() ? dns_entry() : it->second);
    }
    void update_or_add_entry(std::string const& domain, dns_entry const& dns_detail)
    {
        std::lock_guard<std::shared_mutex> lk(entry_mutex);
        entries[domain] = dns_detail;
    }
};


#endif //CPP_CONCURRENCY_READ_WRITE_LOCK_H
