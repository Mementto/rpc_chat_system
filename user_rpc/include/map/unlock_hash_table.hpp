#pragma once
#include <queue>
#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <functional>
#include <condition_variable>
#include <shared_mutex>
#include <bits/stl_iterator_base_types.h>


namespace mementto {

const std::vector<unsigned long>primeList = {
  12289ul,      24593ul,      49157ul,     98317ul,     196613ul,    
  393241ul,     786433ul,     1572869ul,   3145739ul,   6291469ul,   
  12582917ul,   25165843ul,   50331653ul,  100663319ul, 201326611ul, 
  402653189ul,  805306457ul,  1610612741ul,3221225473ul,4294967291ul
};

// const std::vector<unsigned long>primeList = {
//   3ul,         13ul,         23ul,       33ul,       43ul,
//   53ul,       63ul,       73ul,      83ul,     93ul,
//   49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
//   1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
//   50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
//   1610612741ul, 3221225473ul, 4294967291ul
// };

inline unsigned long nextPrime(unsigned long n) {
    auto pos = std::lower_bound(primeList.begin(), primeList.end(), n);
    return pos == primeList.end() ? *(primeList.end() - 1) : *pos;
}

enum SYN_ASY_FLAG {
    SYN_RECOVERY = 0,
    ASY_RECOVERY = 1
};

/**
 * @brief 哈希表开链结点
 * 
 * @tparam __Value 
 */
template<typename __Value>
class HashTableNode {
    using _size = std::size_t;
public:
    HashTableNode(const __Value& val, HashTableNode* ptr = nullptr)
        : m_next(ptr)
        , m_val(val)
        , m_atom(0) {}

    HashTableNode* m_next;
    __Value m_val;
    std::atomic<_size> m_atom; 
};

template<typename __Value, typename __Key, 
    typename __ExtractKey,
    typename __HashFcn, 
    typename __EqualKey>
class HashTable;

/**
 * @brief 哈希表指针
 * 
 * @tparam __Value 
 * @tparam __Key 
 * @tparam __ExtractKey 
 * @tparam __HashFcn 
 * @tparam __EqualKey 
 */
template<typename __Value, typename __Key, 
    typename __ExtractKey,
    typename __HashFcn,
    typename __EqualKey>
class HashTableIterator {
public:
    using _HashTable = HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>;
    using _HashTablePtr = std::shared_ptr<HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>>;
    using iterator = HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>;
    using _Node = HashTableNode<__Value>;
    using _NodePtr = HashTableNode<__Value>*;
    using iterator_category = std::forward_iterator_tag;
    using value_type = __Value;
    using _size = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using pointer = value_type*;

    HashTableIterator(_NodePtr n, _HashTable* tab);
    ~HashTableIterator();
    reference operator*() const { return m_cur->m_val; }
    pointer operator->() const { return &(operator*()); }
    iterator& operator++();
    iterator operator++(int);
    bool operator==(const iterator& it) { return m_cur == it.m_cur; }
    bool operator!=(const iterator& it) { return m_cur != it.m_cur; }

public:
    _NodePtr m_cur;
    _HashTable* m_ht;

};

template<typename __Value, typename __Key, 
    typename __ExtractKey,
    typename __HashFcn,
    typename __EqualKey>
HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::HashTableIterator(_NodePtr n, _HashTable* tab)
    : m_cur(n), m_ht(tab) {
    
    if (m_cur) {
        ++m_cur->m_atom;
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey,
    typename __HashFcn,
    typename __EqualKey>
HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::~HashTableIterator() {
    if (m_cur) {
        --m_cur->m_atom;
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey,
    typename __HashFcn,
    typename __EqualKey>
    HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>&
    HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::operator++() {
    
    const _NodePtr old = m_cur;
    _size bucket = m_ht->m_bkt_num(old->m_val);

    // 自减迭代器跳转前的位置的引用计数
    --m_cur->m_atom;

    m_cur = m_cur->m_next;

    if (!m_cur) {
        while (!m_cur && ++bucket < m_ht->m_buckets.size()) {
            m_cur = m_ht->m_buckets[bucket];
        }

        // 更新迭代器所指位置
        if (m_cur) {
            ++m_cur->m_atom;
        }

    } else {
        ++m_cur->m_atom;
    }
    return *this;
}

template<typename __Value, typename __Key,
    typename __ExtractKey,
    typename __HashFcn, 
    typename __EqualKey>
    HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
    HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
    ::operator++(int) {

    iterator tmp = *this;
    ++*this;
    return tmp;
}

/**
 * @brief 哈希表
 * 
 * @tparam __Value 
 * @tparam __Key 
 * @tparam __ExtractKey 
 * @tparam __HashFcn 
 * @tparam __EqualKey 
 */
template<typename __Value, typename __Key, 
    typename __ExtractKey,
    typename __HashFcn, 
    typename __EqualKey>
class HashTable {

    friend class HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>;
public:

    using value_type = __Value;
    using _size = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator = HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>;

private:
    using _Node = HashTableNode<__Value>;
    using _NodePtr = HashTableNode<__Value>*;

    using CallbackFun = std::function<void()>;

public:

    HashTable(const _size n, 
        const __HashFcn& fcn, 
        const __ExtractKey& ext, 
        const __EqualKey& eql, 
        SYN_ASY_FLAG flag)
        : m_hash(fcn)
        , m_equals(eql)
        , m_getKey(ext)
        , m_numElements(0)
        , m_recTh(nullptr)
        , m_maxQueueSize(10000)
        , m_flag(flag) 
        , m_recStop(false) { m_initializeBuckets(n); }

    HashTable(const __ExtractKey& ext, const _size n,
        SYN_ASY_FLAG flag)
        : m_getKey(ext)
        , m_numElements(0)
        , m_recTh(nullptr)
        , m_maxQueueSize(10000)
        , m_flag(flag)
        , m_recStop(false) { m_initializeBuckets(n); }

    HashTable(const _size n, SYN_ASY_FLAG flag)
        : m_numElements(0)
        , m_recTh(nullptr)
        , m_maxQueueSize(10000)
        , m_flag(flag) 
        , m_recStop(false) { m_initializeBuckets(n); }

    HashTable(const HashTable& table);
    HashTable& operator=(const HashTable& table);
    ~HashTable() { clear(); }

    _size size() const { return m_numElements.load(); }
    bool empty() const { return size() == 0; }
    iterator begin();
    iterator end() { return iterator(nullptr, this); }
    _size bucketCount() const { return m_tableSize; }
    _size maxBucketCount() const { return *(primeList.end() - 1); }

    void insert_rcu(const __Value& value);
    void update_rcu(const __Value& value);
    void remove_rcu(const __Key& key);

    iterator find(const __Key& key);

    void clear();
    void test();

private:
    __HashFcn m_hash;
    __EqualKey m_equals;
    __ExtractKey m_getKey;
    std::vector<_NodePtr> m_buckets;
    std::vector<std::mutex*> m_atomicVec;
    std::atomic<_size> m_numElements;
    std::atomic<_size> m_tableSize;
    std::condition_variable m_condFull;
    std::condition_variable m_condEmpty;
    std::mutex m_globalRecMtx;
    std::mutex m_countMutex;
    std::shared_timed_mutex m_globalMtx;
    std::queue<_NodePtr> m_nodeQ;
    std::queue<std::vector<_NodePtr>*> m_tableQ;
    std::shared_ptr<std::thread> m_recTh;
    _size m_maxQueueSize;
    SYN_ASY_FLAG m_flag;
    bool m_recStop;
    
private:
    _size m_nextSize(_size n) const { return nextPrime(n); }

    void m_initializeBuckets(_size n);
    void m_startRecThread();

    _size m_bkt_num_key(const __Key& key) const { return m_bkt_num_key(key, m_tableSize.load()); }
    _size m_bkt_num(const __Value& value) const { return m_bkt_num_key(m_getKey(value)); }
    _size m_bkt_num_key(const __Key& key, _size n) const { return m_hash(key) % n; }
    _size m_bkt_num(const __Value& value, _size n) const { return m_bkt_num_key(m_getKey(value), n); }

    void update_rcu_syn(const __Value& value);
    void remove_rcu_syn(const __Key& key);
    void resize_rcu_syn(_size numElements);

    void update_rcu_asy(const __Value& value);
    void remove_rcu_asy(const __Key& key);
    void resize_rcu_asy(_size numElements);

    void recovery(std::vector<_NodePtr>* vec);
    void recovery(_NodePtr& node);
    void recovery();

    template<typename __Type>
    void producer(__Type value);
    template<typename __Ptr, typename __Vec>
    void consumer(__Ptr ptr, __Vec vec);

    void pushQ(std::vector<_NodePtr>* ptr) { m_tableQ.push(ptr); }
    void pushQ(_NodePtr ptr) { m_nodeQ.push(ptr); }

private:

};

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::HashTable(const HashTable& table) 
    : m_hash(table.m_hash)
    , m_equals(table.m_equals)
    , m_getKey(table.m_getKey)
    , m_buckets(table.m_buckets)
    , m_numElements(table.m_numElements.load())
    , m_tableSize(table.m_tableSize.load())
    , m_recTh(nullptr)
    , m_maxQueueSize(table.m_maxQueueSize)
    , m_flag(table.m_flag) 
    , m_recStop(false) { 

    m_atomicVec.resize(table.m_atomicVec.size(), new std::mutex);
    m_startRecThread(); 
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>& 
HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::operator=(const HashTable& table) {

    clear();
    m_hash = table.m_hash;
    m_equals = table.m_equals;
    m_getKey = table.m_getKey;
    m_buckets = table.m_buckets;
    m_numElements = table.m_numElements.load();
    m_tableSize = table.m_tableSize.load();
    m_maxQueueSize = table.m_maxQueueSize;
    m_flag = table.m_flag;
    m_recStop = table.m_recStop;

    m_atomicVec.resize(table.m_atomicVec.size(), new std::mutex);
    m_startRecThread();

    return *this;
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
typename HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::iterator 
HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::begin() {
    for (_size i = 0; i < m_buckets.size(); ++i) {

        if (m_buckets[i]) {
            return iterator(m_buckets[i], this);
        }
    }
    return end();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
typename HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::iterator 
HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::find(const __Key& key) {
    _size n = m_bkt_num_key(key);
    
    _NodePtr first = nullptr;
    for (first = m_buckets[n]; 
         first && !m_equals(m_getKey(first->m_val), key); 
         first = first->m_next) {
        
        ++first->m_atom;
        --first->m_atom;
    }
    
    return iterator(first, this);
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::clear() {

    m_globalMtx.lock();

    for (_size i = 0; i < m_buckets.size(); ++i) {
        _NodePtr cur = m_buckets[i];
        while (cur) {
            _NodePtr next = cur->m_next;
            delete cur;
            cur = next;
        }
        m_buckets[i] = nullptr;

        delete m_atomicVec[i];
    }
    m_numElements = 0;

    if (m_recTh) {
        m_recStop = true;
        while (!m_tableQ.empty()) {
            recovery(m_tableQ.front());
            m_tableQ.pop();
        }
        while (!m_nodeQ.empty()) {
            recovery(m_nodeQ.front());
            m_nodeQ.pop();
        }
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::test() {

}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::m_initializeBuckets(_size n) {
    const _size nBuckets = m_nextSize(n);

    m_buckets.resize(nBuckets, nullptr);
    m_atomicVec.resize(nBuckets, nullptr);
    m_tableSize = nBuckets;

    for (int i = 0; i < m_atomicVec.size(); ++i) {
        m_atomicVec[i] = new std::mutex;
    }

    m_startRecThread();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::m_startRecThread() {
    if (m_flag == ASY_RECOVERY) {
        CallbackFun fun = 
        std::bind(
            (void(HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::*)())
            &HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::recovery, 
            this);
        m_recTh = std::make_shared<std::thread>(fun);
        m_recTh->detach();
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::insert_rcu(const __Value& value) {
    
    if (m_flag == SYN_RECOVERY) {
        resize_rcu_syn(m_numElements.load() + 1);
    } else {
        resize_rcu_asy(m_numElements.load() + 1);
    }

    const _size n = m_bkt_num(value);
    m_globalMtx.lock_shared();
    m_atomicVec[n]->lock();
    _NodePtr first = m_buckets[n];
    for (auto cur = first; cur; cur = cur->m_next) {
        if (m_equals(m_getKey(cur->m_val), m_getKey(value))) {
            m_atomicVec[n]->unlock();
            m_globalMtx.unlock_shared();
            return;
        }
    }
    _NodePtr tmp = new _Node(value);
    tmp->m_next = first;
    m_buckets[n] = tmp;
    
    ++m_numElements;
    m_atomicVec[n]->unlock();
    m_globalMtx.unlock_shared();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::update_rcu(const __Value& value) {
    if (m_flag == SYN_RECOVERY) {
        update_rcu_syn(value);
    } else {
        update_rcu_asy(value);
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::remove_rcu(const __Key& key) {
    if (m_flag == SYN_RECOVERY) {
        remove_rcu_syn(key);
    } else {
        remove_rcu_asy(key);
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::update_rcu_syn(const __Value& value) {

    resize_rcu_syn(m_numElements.load() + 1);
    _size n = m_bkt_num(value);
    // std::cout << n << ": update lock" << std::endl;
    m_globalMtx.lock_shared();
    m_atomicVec[n]->lock();
    _NodePtr first = m_buckets[n];

    for (_NodePtr cur = first, pre = nullptr; cur; pre = cur, cur = cur->m_next) {
        if (m_equals(m_getKey(cur->m_val), m_getKey(value))) {
            _NodePtr tmp = new _Node(value);
            tmp->m_next = cur->m_next;
            if (!pre) {
                m_buckets[n] = tmp;
            } else {
                pre->m_next = tmp;
            }
            m_atomicVec[n]->unlock();
            m_globalMtx.unlock_shared();
            
            // 等待引用计数清零
            while(cur->m_atom.load() > 0);

            delete cur;
            cur = nullptr;

            return;
        }
    }

    _NodePtr tmp = new _Node(value);
    tmp->m_next = first;
    m_buckets[n] = tmp;
    ++m_numElements;
    m_atomicVec[n]->unlock();
    m_globalMtx.unlock_shared();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::remove_rcu_syn(const __Key& key) {

    const _size n = m_bkt_num_key(key);
    m_globalMtx.lock_shared();
    m_atomicVec[n]->lock();

    _NodePtr first = m_buckets[n];

    if (first) {
        _NodePtr cur = first;
        _NodePtr next = cur->m_next;
        while (next) {
            if (m_equals(m_getKey(next->m_val), key)) {
                cur->m_next = next->m_next;
                --m_numElements;
                m_atomicVec[n]->unlock();
                m_globalMtx.unlock_shared();

                // 等待引用计数清零
                while(next->m_atom.load() > 0);

                delete next;
                return;
            } else {
                cur = next;
                next = cur->m_next;
            }
        }
        if (m_equals(m_getKey(first->m_val), key)) {
            m_buckets[n] = first->m_next;
            --m_numElements;
            m_atomicVec[n]->unlock();
            m_globalMtx.unlock_shared();

            // 等待引用计数清零
            while(first->m_atom.load() > 0);

            delete first;
            first = nullptr;
            return;
        }
    }

    m_atomicVec[n]->unlock();
    m_globalMtx.unlock_shared();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::
resize_rcu_syn(_size numElements) {
    if (numElements > m_buckets.size()) {
        const _size n = nextPrime(numElements);
        m_globalMtx.lock();
        if (n > m_buckets.size()) {

            std::vector<std::mutex*> tmpLock(n, nullptr);
            std::vector<_NodePtr> tmp(n, nullptr);
            for (int i = 0; i < n; ++i) {
                tmpLock[i] = new std::mutex;
            }

            for (_size bucket = 0; bucket < m_buckets.size(); ++bucket) {
                _NodePtr first = m_buckets[bucket];
                while (first) {
                    _size new_bucket = m_bkt_num(first->m_val);
                    _NodePtr tmpNode = new _Node(first->m_val);
                    tmpNode->m_next = tmp[new_bucket];
                    tmp[new_bucket] = tmpNode;
                    first = first->m_next;
                }
            }

            m_tableSize = m_buckets.size();
            m_buckets.swap(tmp);
            m_atomicVec.swap(tmpLock);
            m_globalMtx.unlock();

            // 同步操作：该数组的引用计数为零以后销毁newVec与tmp
            auto getValue = [](_size init, _NodePtr hs) {
                _size nodeSum = 0;
                while (hs) {
                    nodeSum += hs->m_atom.load();
                    hs = hs->m_next;
                }
                return init + nodeSum;
            };
            while (std::accumulate(tmp.begin(), tmp.end(), 0, getValue) > 0);

            for (_size bucket = 0; bucket < tmp.size(); ++bucket) {
                while (tmp[bucket]) {
                    _NodePtr next = tmp[bucket]->m_next;
                    delete tmp[bucket];
                    tmp[bucket] = next;
                }
                delete tmpLock[bucket];
            }
            return;
        }
        m_globalMtx.unlock();
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::update_rcu_asy(const __Value& value) {
    resize_rcu_asy(m_numElements.load() + 1);
    _size n = m_bkt_num(value);
    m_globalMtx.lock_shared();
    m_atomicVec[n]->lock();
    _NodePtr first = m_buckets[n];

    for (_NodePtr cur = first, pre = nullptr; cur; pre = cur, cur = cur->m_next) {
        if (m_equals(m_getKey(cur->m_val), m_getKey(value))) {
            _NodePtr tmp = new _Node(value);
            tmp->m_next = cur->m_next;
            if (!pre) {
                m_buckets[n] = tmp;
            } else {
                pre->m_next = tmp;
            }
            m_atomicVec[n]->unlock();
            m_globalMtx.unlock_shared();
            
            // 等待引用计数清零
            {
                std::unique_lock<std::mutex> lock(m_globalRecMtx);     
                producer(cur);
            }

            return;
        }
    }

    _NodePtr tmp = new _Node(value);
    tmp->m_next = first;
    m_buckets[n] = tmp;
    ++m_numElements;
    m_atomicVec[n]->unlock();
    m_globalMtx.unlock_shared();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::remove_rcu_asy(const __Key& key) {
    const _size n = m_bkt_num_key(key);

    m_globalMtx.lock_shared();
    m_atomicVec[n]->lock();
    _NodePtr first = m_buckets[n];

    if (first) {
        _NodePtr cur = first;
        _NodePtr next = cur->m_next;
        while (next) {
            if (m_equals(m_getKey(next->m_val), key)) {
                cur->m_next = next->m_next;
                --m_numElements;
                m_atomicVec[n]->unlock();
                m_globalMtx.unlock_shared();

                // 等待引用计数清零
                {
                    std::unique_lock<std::mutex> lock(m_globalRecMtx);
                    producer(next);
                }
                return;
            } else {
                cur = next;
                next = cur->m_next;
            }
        }
        if (m_equals(m_getKey(first->m_val), key)) {
            m_buckets[n] = first->m_next;
            --m_numElements;
            m_atomicVec[n]->unlock();
            m_globalMtx.unlock_shared();

            // 等待引用计数清零
            {
                std::unique_lock<std::mutex> lock(m_globalRecMtx);
                producer(first);
            }
            return;
        }
    }
    m_atomicVec[n]->unlock();
    m_globalMtx.unlock_shared();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::resize_rcu_asy(_size numElements) {
    const _size old_n = m_buckets.size();
    if (numElements > old_n) {
        const _size n = nextPrime(numElements);
        m_globalMtx.lock();
        if (n > old_n) {
            std::vector<std::mutex*> tmpLock(n, nullptr);
            std::vector<_NodePtr>* tmp = new std::vector<_NodePtr>(n, nullptr);

            for (int i = 0; i < n; ++i) {
                tmpLock[i] = new std::mutex;
            }
            for (_size bucket = 0; bucket < old_n; ++bucket) {
                _NodePtr first = m_buckets[bucket];
                while (first) {
                    _size new_bucket = m_bkt_num(first->m_val);
                    _NodePtr tmpNode = new _Node(first->m_val);
                    tmpNode->m_next = (*tmp)[new_bucket];
                    (*tmp)[new_bucket] = tmpNode;
                    first = first->m_next;
                }
            }

            m_tableSize = m_buckets.size();
            m_buckets.swap(*tmp);
            m_atomicVec.swap(tmpLock);
            m_globalMtx.unlock();

            for (int i = 0; i < tmpLock.size(); ++i) {
                delete tmpLock[i];
            }

            // 同步操作：该数组的引用计数为零以后销毁newVec与tmp
            {
                std::unique_lock<std::mutex> lock(m_globalRecMtx);
                producer(tmp);
            }
            return;
        }
        m_globalMtx.unlock();
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::recovery(std::vector<_NodePtr>* tmp) {
    if (!tmp) {
        return;
    }

    auto getValue = [](_size init, _NodePtr hs) {
        _size nodeSum = 0;
        while (hs) {
            nodeSum += hs->m_atom.load();
            hs = hs->m_next;
        }
        return init + nodeSum;
    };

    while (std::accumulate(tmp->begin(), tmp->end(), 0, getValue) > 0);

    for (_size bucket = 0; bucket < tmp->size(); ++bucket) {
        while ((*tmp)[bucket]) {
            _NodePtr next = (*tmp)[bucket]->m_next;
            delete (*tmp)[bucket];
            (*tmp)[bucket] = next;
        }
    }
    delete tmp;
    tmp = nullptr;
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::recovery(_NodePtr& node) {

    if (!node) {
        return;
    }

    while (node->m_atom.load() > 0);
    delete node;
    node = nullptr;
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::recovery() {

    while (!m_recStop) {
        _NodePtr ptr = nullptr;
        std::vector<_NodePtr>* vec = nullptr;

        consumer(ptr, vec);
        if (ptr) {
            recovery(ptr);
        } 
        if (vec) {
            recovery(vec);
        }
    }
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
template<typename __Type>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::producer(__Type value) {

    std::unique_lock<std::mutex> lock(m_countMutex);
    while (m_nodeQ.size() == m_maxQueueSize || m_tableQ.size() == m_maxQueueSize) {
        m_condFull.wait(lock);
    }

    pushQ(value);

    m_condEmpty.notify_all();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
template<typename __Ptr, typename __Vec>
void HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>
::consumer(__Ptr ptr, __Vec vec) {
    std::unique_lock<std::mutex> lock(m_countMutex);
    while (m_nodeQ.empty() && m_tableQ.empty()) {
        m_condEmpty.wait(lock);
    }
    // std::cout << m_nodeQ.size() << ", " << m_tableQ.size() << std::endl;

    if (!m_nodeQ.empty()) {
        ptr = m_nodeQ.front();
        m_nodeQ.pop();
    } 
    if (!m_tableQ.empty()) {
        vec = m_tableQ.front();
        m_tableQ.pop();
    }

    m_condFull.notify_all();
}

template<typename __Value, typename __Key, 
    typename __ExtractKey, 
    typename __HashFcn, 
    typename __EqualKey>
inline std::forward_iterator_tag
iterator_category(const HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>&) {

    return std::forward_iterator_tag();
}

// template<typename __Value, typename __Key, 
//     typename __ExtractKey, 
//     typename __HashFcn, 
//     typename __EqualKey>
//     inline __Value*
//     value_type(const HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>&) {
//         return (__Value*) 0;
// }

// template<typename __Value, typename __Key, 
//     typename __ExtractKey, 
//     typename __HashFcn, 
//     typename __EqualKey>
//     inline typename HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::difference_type*
//     distance_type(const HashTableIterator<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>&) {
//         return (HashTable<__Value, __Key, __ExtractKey, __HashFcn, __EqualKey>::difference_type*) 0;
//     }

} // namespace mementto