#ifndef USE_CONV_CONV_MULTI_H
#define USE_CONV_CONV_MULTI_H

#include "server/include/reactor.h"
#include "server/include/acceptor.h"
#include <csignal>

namespace hzd
{
    template <class T>
    class conv_multi{
        bool ET{false};
        bool one_shot{true};
        bool _thread_pool{false};
        std::string ip;
        short port;
        acceptor<T> _acceptor;
        safe_queue<T*> conn_queue;
        bool run{true};
        connpool<T>* conn_pool{nullptr};
        std::vector<reactor<T>> reactors;
        size_t max_connect_count{10000};

    public:
        conv_multi(std::string _ip,short _port,int reactor_count = 4):ip(std::move(_ip)),port(_port)
        {
            run = true;
            reactors.resize(reactor_count);
            signal(SIGPIPE,SIG_IGN);
            reactor<T>::set_run_true();
        };
        ~conv_multi()
        {
            close();
        }
        void close()
        {
            run = false;
            reactor<T>::set_run_false();
            delete conn_pool;
            conn_pool = nullptr;
        }
        /**
         * @brief enable address reuse
         * @note None
         * @param None
         * @retval None
         */
        void enable_addr_reuse()
        {
            _acceptor.enable_addr_reuse();
        }
        /**
          * @brief disable address reuse
          * @note None
          * @param None
          * @retval None
          */
        void disable_addr_reuse()
        {
            _acceptor.disable_add_reuse;
        }
        /**
          * @brief enable port reuse
          * @note None
          * @param None
          * @retval None
          */
        void enable_port_reuse()
        {
            _acceptor.enable_port_reuse();
        }
        /**
          * @brief disable port reuse
          * @note None
          * @param None
          * @retval None
          */
        void disable_port_reuse()
        {
            _acceptor.disable_port_reuse();
        }
        /**
          * @brief enable using multi-thread
          * @note None
          * @param thread_count working thread count
          * @param max_process_count max process count
          * @retval None
          */
        void enable_multi_thread(int thread_count = 8,int max_process_count = 10000)
        {
            _thread_pool = true;
        }
        /**
          * @brief disable using multi-thread
          * @note None
          * @param None
          * @retval None
          */
        void disable_multi_thread()
        {
            _thread_pool = false;
        }
        /**
        * @brief enable object pool
        * @note None
        * @param None
        * @retval None
        */
        void enable_object_pool(size_t size = 200)
        {
            if(!conn_pool)
            {
                conn_pool = new connpool<T>(size,max_connect_count);
            }
        }
        /**
        * @brief disable object pool
        * @note None
        * @param None
        * @retval None
        */
        void disable_object_pool()
        {
            delete conn_pool;
            conn_pool = nullptr;
        }
        /**
          * @brief enable et model
          * @note None
          * @param None
          * @retval None
          */
        void enable_et() { ET = true; }
        /**
          * @brief disable et model
          * @note None
          * @param None
          * @retval None
          */
        void disable_et() { ET = false; }
        /**
          * @brief enable one shot trigger
          * @note None
          * @param None
          * @retval None
          */
        void enable_one_shot() { one_shot = true; }
        /**
          * @brief disable one shot trigger
          * @note None
          * @param None
          * @retval None
          */
        void disable_one_shot() { one_shot = false; }
        /**
          * @brief set max events count
          * @note None
          * @param size size
          * @retval None
          */
        void set_max_events_count(int size)
        {
            if(size >= 0)
            {
                for(auto& r : reactors) r.set_max_events_count(size);
            }
        }
        /**
          * @brief set max connection count
          * @note None
          * @param size size
          * @retval None
          */
        void set_max_connect_count(int size){if(size >= 0) max_connect_count = size;}
        /**
          * @brief set listen queue size
          * @note None
          * @param size size
          * @retval None
          */
        void set_listen_queue_count(int size){if(size >= 0) _acceptor.set_listen_queue_count(size);}

        void wait(int time_out=-1)
        {
            _acceptor.init(ip,port,&conn_queue,conn_pool);
            for(auto& r : reactors)
            {
                r.init(run,ET,one_shot,conn_pool,&conn_queue);
                using func = void(*)(void*);
                std::thread t(static_cast<func>(&reactor<T>::work),(void*)&r);
                t.detach();
            }
            while(run)
            {
                _acceptor.work();
            }
        }
    };
}

#endif