#ifndef USE_CONV_SAFE_QUEUE_H
#define USE_CONV_SAFE_QUEUE_H

#include <deque>        /* deque */
#include <mutex>        /* mutex */
namespace hzd
{
    template<class T>
    class safe_queue {
        std::deque<T> q;
        std::mutex mtx;
    public:
        ~safe_queue()
        {
            q.clear();
        }
        void push(T t)
        {
            std::lock_guard<std::mutex> guard(mtx);
            q.push_back(t);
        }
        void push_front(T t)
        {
            std::lock_guard<std::mutex> guard(mtx);
            q.push_front(t);
        }
        T pop()
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(!q.empty())
            {
                T ret = q.front();
                q.pop_front();
                return ret;
            }
            else
            {
                return nullptr;
            }
        }
        bool empty()
        {
            std::lock_guard<std::mutex> guard(mtx);
            return q.empty();
        }
    };
}

#endif
