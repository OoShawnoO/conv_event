#ifndef USE_CONV_LOCK_QUEUE_H
#define USE_CONV_LOCK_QUEUE_H

#include <deque>
#include <mutex>

namespace hzd
{
    template <class T>
    class lock_queue {
        std::deque<T> q;
        std::mutex mtx;
    public:
        void push(T& t)
        {
            std::lock_guard<std::mutex> guard(mtx);
            q.push_back(t);
        }
        bool pop(T& t)
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(q.empty()) { t = nullptr; return false; }
            t = q.front();
            q.pop_front();
            return true;
        }
        bool empty()
        {
            std::lock_guard<std::mutex> guard(mtx);
            return q.empty();
        }
    };
    template<>
    bool lock_queue<int>::pop(int& t)
    {
        std::lock_guard<std::mutex> guard(mtx);
        if(q.empty()) { t = -1; return false; }
        t = q.front();
        q.pop_front();
        return true;
    }
}

#endif
