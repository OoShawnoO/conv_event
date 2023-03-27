#ifndef CONV_EVENT_THREADPOOL_H
#define CONV_EVENT_THREADPOOL_H

#include <thread>               /* thread */
#include <queue>                /* queue */
#include "ErrorLog/ErrorLog.h"  /* LOG LOG_MSG LOG_FMT */
#include <unordered_map>        /* unordered_map */
#if __cplusplus <= 201703L
#include <condition_variable>   /* condition_variable */
#else
#include <semaphore>            /* mutex semaphore */
#endif

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
        std::mutex mutex;
        std::queue<T*> process_pool;
        std::thread* threads;
        #if __cplusplus > 201703L
        std::counting_semaphore<0> sem{0};
        #else
        hzd::counting_semaphore<0> sem{0};
        #endif
        bool stop{false};
    public:
        explicit threadpool(int _thread_count = 8,int _max_process_count = 10000)
        : thread_count(_thread_count),max_process_count(_max_process_count)
        {
            if(thread_count <= 0 || max_process_count <= 0)
            {
                LOG(Out_Of_Bound,"threads count or max process count <= 0");
                exit(-1);
            }
            threads = new std::thread[thread_count];
            if(!threads)
            {
                LOG(Bad_Malloc,"threads bad new");
                exit(-1);
            }
            for(int i=0;i<thread_count;i++) {
                threads[i] = std::thread(work, this);
                if (!&threads[i]) {
                    LOG(Bad_Malloc, "threads[i] bad new");
                    exit(-1);
                }
                threads[i].detach();
            }
        }
        ~threadpool()
        {
            delete []threads;
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
                mutex.lock();
                if(process_pool.empty())
                {
                    continue;
                }
                auto con = process_pool.front();
                process_pool.pop();
                mutex.unlock();
                if(!con)
                {
                    continue;
                }
                con->process();
            }
        }

        bool add(T* t)
        {
            mutex.lock();
            if(process_pool.size() > max_process_count)
            {
                mutex.unlock();
                return false;
            }
            process_pool.push(t);
            mutex.unlock();
            sem.release();
            return true;
        }
    };

}
#endif