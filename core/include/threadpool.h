#ifndef CONV_EVENT_THREADPOOL_H
#define CONV_EVENT_THREADPOOL_H

#include <thread>                       /* thread */
#include <queue>                        /* queue */
#include <unordered_map>                /* unordered_map */
#if __cplusplus <= 201703L
#include <condition_variable>           /* condition_variable */
#else
#include <semaphore>                    /* mutex semaphore */
#endif
#include "async_logger/async_logger.hpp" /* async_logger */

namespace hzd
{

    #if __cplusplus <= 201703L
    template<size_t _least_max_value = SIZE_MAX>
    class counting_semaphore{
        static_assert(_least_max_value >= 0,"least max value must more than 0");
        static_assert(_least_max_value <= SIZE_MAX,"least max value must lower than size_t max");
        std::mutex mtx;
        std::condition_variable cv;
        size_t count{0};
    public:
        explicit counting_semaphore(size_t desire):count(desire){}
        void acquire()
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock,[&]{return count > 0;});
            --count;
        }
        void release()
        {
            std::unique_lock<std::mutex> lock(mtx);
            ++count;
            cv.notify_one();
        }
    };
    #endif

    template<class T>
    class threadpool {
    private:
        int thread_count;
        int max_process_count;
        lock_queue<T*> process_pool{};
        std::vector<std::shared_ptr<std::thread>> threads;
        #if __cplusplus > 201703L
        std::counting_semaphore<0> sem{0};
        #else
        hzd::counting_semaphore<0> sem{0};
        #endif
        bool stop{false};
    public:
        explicit threadpool(int _thread_count = 8,int _max_process_count =40000)
        : thread_count(_thread_count),max_process_count(_max_process_count)
        {
            if(thread_count <= 0 || max_process_count <= 0)
            {
                LOG_FATAL("thread count or max process count should not <= 0");
                exit(-1);
            }
            for(int i=0;i<thread_count;i++) {
                std::shared_ptr<std::thread> t = std::make_shared<std::thread>(work,this);
                threads.emplace_back(t);
                t->detach();
            }
            LOG_TRACE(
                    "thread pool init success,thread count ="
                    + std::to_string(thread_count)
                    + " max_process_count = "
                    + std::to_string(max_process_count)
                    );
        }
        ~threadpool()
        {
            stop = true;
        }
        static void* work(void* arg)
        {
            auto pool = (threadpool*)arg;
            pool->run();
            return pool;
        }
        void run()
        {
            while(!stop)
            {
                sem.acquire();
                T* con = nullptr;
                if(process_pool.empty())
                {
                    continue;
                }
                process_pool.pop(con);
                if(!con)
                {
                    continue;
                }
                con->process();
            }
        }

        bool add(T* t)
        {
            if(process_pool.size() >= max_process_count)
            {
                LOG_WARN("thread pool overload");
                return false;
            }
            process_pool.push(t);
            sem.release();
            return true;
        }
    };

}
#endif
