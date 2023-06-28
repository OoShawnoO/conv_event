#ifndef CONV_EVENT_LOCK_QUEUE_H
#define CONV_EVENT_LOCK_QUEUE_H

#include <deque>
#include <mutex>
#if __cplusplus >= 201703L
#include <shared_mutex>
# endif

namespace hzd
{
    template <class T>
    class lock_queue {
        std::deque<T> q;
        #if __cplusplus >= 201703L
        std::shared_mutex mtx;
        #else
        std::mutex mtx;
        #endif
    public:
        void push(T& t)
        {
            #if __cplusplus >= 201703L
            std::unique_lock<std::shared_mutex> lock(mtx);
            #else
            std::lock_guard<std::mutex> guard(mtx);
            #endif
            q.push_back(t);
        }
        bool pop(T& t)
        {
            #if __cplusplus >= 201703L
            std::unique_lock<std::shared_mutex> lock(mtx);
            #else
            std::lock_guard<std::mutex> guard(mtx);
            #endif
            if(q.empty()) { t = nullptr; return false; }
            t = q.front();
            q.pop_front();
            return true;
        }
        bool empty()
        {
            #if __cplusplus >= 201703L
            std::shared_lock<std::shared_mutex> lock(mtx);
            #else
            std::lock_guard<std::mutex> guard(mtx);
            #endif
            return q.empty();
        }
        size_t size() {
            return q.size();
        }
    };
    template<>
    bool lock_queue<int>::pop(int& t)
    {
        #if __cplusplus >= 201703L
        std::unique_lock<std::shared_mutex> lock(mtx);
        #else
        std::lock_guard<std::mutex> guard(mtx);
        #endif
        if(q.empty()) { t = -1; return false; }
        t = q.front();
        q.pop_front();
        return true;
    }
}

#endif
