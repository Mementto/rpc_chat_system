#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

template<typename __Type>
class AsyQueue {

public:
    void push(const __Type& data);
    __Type pop();

private:
    std::queue<__Type> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cond;

};

template<typename __Type>
void AsyQueue<__Type>::push(const __Type& data) {
    std::lock_guard<std::mutex>lock(m_mutex);
    m_queue.push(data);
    m_cond.notify_one();
}

template<typename __Type>
__Type AsyQueue<__Type>::pop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty()) {
        m_cond.wait(lock);
    }

    __Type data = m_queue.front();
    m_queue.pop();
    return data;
}