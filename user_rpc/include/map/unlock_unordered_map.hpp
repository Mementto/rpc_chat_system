#pragma once
#include "unlock_hash_table.hpp"
#include <functional>

namespace mementto {

template<typename __Key, typename __T, 
    typename __HashFcn = std::hash<__Key>, 
    typename __EqualKey = std::equal_to<__Key>>
class unlock_unordered_map {

private:

    using ht = HashTable<std::pair<const __Key, __T>, __Key, 
                         std::_Select1st<std::pair<const __Key, __T>>, 
                         __HashFcn, __EqualKey>;
    ht rep;

public:
    using __Value = typename ht::value_type;
    using _size = typename ht::_size;
    using difference_type = typename ht::difference_type;
    using pointer = typename ht::pointer;
    using reference = typename ht::reference;
    using iterator = typename ht::iterator;

public:
    unlock_unordered_map(SYN_ASY_FLAG i = SYN_RECOVERY) : rep(100, i) {}
    unlock_unordered_map(_size n, SYN_ASY_FLAG i = SYN_RECOVERY) : rep(n, i) {}
    unlock_unordered_map(const unlock_unordered_map& map) 
        : rep(map.rep) {}
    unlock_unordered_map& operator=(const unlock_unordered_map& map) { 
        rep = map.rep;
        return *this;
    }

    // template<typename InputIterator>
    // unlock_unordered_map(InputIterator f, InputIterator l)
    //     : rep(100) { rep.insertUnique(f, l); }

    // template<typename InputIterator>
    // unlock_unordered_map(InputIterator f, InputIterator l, _size n)
    //     : rep(n) { rep.insertUnique(f, l); }

public:
    _size size() const { return rep.size(); }
    _size max_size() const { return rep.max_size(); }
    bool empty() const { return rep.empty(); }
    iterator begin() { return rep.begin(); }
    iterator end() { return rep.end(); }
    // swap()

public:

    /**
     * @brief 查找
     * 
     * @param key 键
     * @return iterator 迭代器
     */
    iterator find(const __Key& key) { return rep.find(key); }

    /**
     * @brief 删除
     * 
     * @param key 键
     */
    void erase(const __Key& key) { rep.remove_rcu(key); }

    /**
     * @brief 更新或添加
     * 
     * @param key 键
     * @param value 值
     */
    void update(const __Key& key, const __T& value) { rep.update_rcu(std::make_pair(key, value)); }

    /**
     * @brief 添加
     * 
     * @param key 键
     * @param value 值
     */
    void insert(const __Key& key, const __T& value) { rep.insert_rcu(std::make_pair(key, value)); }

    void test() { rep.test(); }

public:
    void resize(_size h) { rep.resize(h); }
    _size bucketCount() const { return rep.bucketCount(); }
    _size maxBucketCount() const { return rep.maxBucketCount(); }
    
};
    
} // namespace mementto