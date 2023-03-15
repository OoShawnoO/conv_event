#ifndef CONV_EVENT_THREADPOOL_H
#define CONV_EVENT_THREADPOOL_H

#include <thread>
#include <queue>
#include <mutex>
#include <semaphore>
#include "ErrorLog/ErrorLog.h"
namespace hzd
{
    template<class T>
    class threadpool {
    private:
        int thread_count;
        int max_process_count;
        std::mutex mutex;
        std::queue<T*> process_pool;
        std::thread* threads;
        std::counting_semaphore<0> sem{0};
        bool stop;
    public:
        threadpool(int _thread_count = 8,int _max_process_count = 10000)
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
                T* con = process_pool.front();
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
