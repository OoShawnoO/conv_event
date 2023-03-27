#ifndef USE_CONV_SAFE_QUEUE_H
#define USE_CONV_SAFE_QUEUE_H

#include <queue>
#include <mutex>
namespace hzd
{
    template<class T>
    class safe_queue {
        std::queue<T> q;
        std::mutex mtx;
    public:
        void push(T t)
        {
            std::lock_guard<std::mutex> guard(mtx);
            q.push(t);
        }
        T pop()
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(!q.empty())
            {
                T ret = q.front();
                q.pop();
                return ret;
            }
            else
            {
                return nullptr;
            }
        }
    };
}

#endif
