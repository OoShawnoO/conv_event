#ifndef CONV_EVENT_CONV_MULTI_H
#define CONV_EVENT_CONV_MULTI_H

#include "include/reactor.h"            /* reactor */
#include "include/acceptor.h"           /* acceptor */
#include <atomic>                       /* atomic */
#include "include/configure.h"          /* configure */
#include "include/conv_base.h"          /* conv_base */

namespace hzd
{

    template <class T>
    class conv_multi : public conv_base{
        friend class reactor<T>;
        friend class acceptor<T>;
    protected:
        bool _thread_pool{false};
        bool run{true};
        acceptor<T> _acceptor;
        connpool<T>* conn_pool{nullptr};
    public:
        std::vector<reactor<T>> reactors;
        explicit conv_multi(int reactor_count = 4)
        {
            configure& conf = configure::get_config();
            ip = (const char*)conf.require("ip");
            port = conf.require("port");
            max_connect_count = (int32_t)conf.require("max_connect_count");
            if(conf["multi_thread"].type != JSON_NULL)
                conv_multi::enable_multi_thread();
            if(conf["object_pool"].type != JSON_NULL)
                conv_multi::enable_object_pool((int32_t)conf.require("object_pool_size"));
            if(conf["one_shot"].type != JSON_NULL)
                one_shot = conf["one_shot"];
            if(conf["et"].type != JSON_NULL)
                ET = conf["et"];
            if(conf["reactor_count"].type != JSON_NULL)
                reactor_count = conf["reactor_count"];

            run = true;
            reactors.resize(reactor_count);
            reactor<T>::set_run_true();
            _acceptor.init(this);

            if(conf["port_reuse"].type != JSON_NULL && conf["port_reuse"]) conv_multi::enable_port_reuse();
            if(conf["address_reuse"].type != JSON_NULL && conf["address_reuse"]) conv_multi::enable_addr_reuse();

            signal(SIGPIPE,SIG_IGN);
        };
        virtual ~conv_multi()
        {
            close();
        }
        void close() override
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
        void enable_addr_reuse() override
        {
            _acceptor.enable_addr_reuse();
        }
        /**
          * @brief disable address reuse
          * @note None
          * @param None
          * @retval None
          */
        void disable_addr_reuse() override
        {
            _acceptor.disable_add_reuse();
        }
        /**
          * @brief enable port reuse
          * @note None
          * @param None
          * @retval None
          */
        void enable_port_reuse() override
        {
            _acceptor.enable_port_reuse();
        }
        /**
          * @brief disable port reuse
          * @note None
          * @param None
          * @retval None
          */
        void disable_port_reuse() override
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
        void enable_multi_thread() override
        {
            _thread_pool = true;
        }
        /**
          * @brief disable using multi-thread
          * @note None
          * @param None
          * @retval None
          */
        void disable_multi_thread() override
        {
            _thread_pool = false;
        }
        /**
        * @brief enable object pool
        * @note None
        * @param None
        * @retval None
        */
        void enable_object_pool(size_t size) override
        {
            if(!conn_pool)
            {
                conn_pool = new connpool<T>(size);
            }
        }
        /**
        * @brief disable object pool
        * @note None
        * @param None
        * @retval None
        */
        void disable_object_pool() override
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
        void enable_et() override { ET = true; }
        /**
          * @brief disable et model
          * @note None
          * @param None
          * @retval None
          */
        void disable_et() override { ET = false; }
        /**
          * @brief enable one shot trigger
          * @note None
          * @param None
          * @retval None
          */
        void enable_one_shot() override { one_shot = true; }
        /**
          * @brief disable one shot trigger
          * @note None
          * @param None
          * @retval None
          */
        void disable_one_shot() override { one_shot = false; }
        /**
          * @brief set max events count
          * @note None
          * @param size size
          * @retval None
          */
        void set_max_events_count(int size) override
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
        void set_max_connect_count(int size) override{if(size >= 0) max_connect_count = size;}
        /**
          * @brief set listen queue size
          * @note None
          * @param size size
          * @retval None
          */
        void set_listen_queue_count(int size) override{if(size >= 0) _acceptor.set_listen_queue_count(size);}

        void wait(int time_out=5)
        {
            if(conn_pool) _acceptor.set_conn_pool(conn_pool);
            reactor<T>::set_run_true();
            for(auto& r : reactors)
            {
                r.init(this);
                using func = void(*)(void*,int);
                std::thread t(static_cast<func>(reactor<T>::work),(void*)&r,time_out);
                t.detach();
            }
            _acceptor.work();
            close();
        }
    };
}

#endif
