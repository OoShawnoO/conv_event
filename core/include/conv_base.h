#ifndef USE_CONV_CONV_BASE_H
#define USE_CONV_CONV_BASE_H

#include "conn.h"               /* conn */
#include "threadpool.h"         /* thread_pool */
#include <iostream>
namespace hzd
{
    class conv_base {
    public:
        conv_base() {
            std::cout << "\n"
                 " ██████╗ ██████╗ ███╗   ██╗██╗   ██╗       ███████╗██╗   ██╗███████╗███╗   ██╗████████╗\n"
                 "██╔════╝██╔═══██╗████╗  ██║██║   ██║       ██╔════╝██║   ██║██╔════╝████╗  ██║╚══██╔══╝\n"
                 "██║     ██║   ██║██╔██╗ ██║██║   ██║       █████╗  ██║   ██║█████╗  ██╔██╗ ██║   ██║   \n"
                 "██║     ██║   ██║██║╚██╗██║╚██╗ ██╔╝       ██╔══╝  ╚██╗ ██╔╝██╔══╝  ██║╚██╗██║   ██║   \n"
                 "╚██████╗╚██████╔╝██║ ╚████║ ╚████╔╝███████╗███████╗ ╚████╔╝ ███████╗██║ ╚████║   ██║   \n"
                 " ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝  ╚═══╝ ╚══════╝╚══════╝  ╚═══╝  ╚══════╝╚═╝  ╚═══╝   ╚═╝   \n"
                 "                                                                                       \n"
            << std::endl;
            system("ulimit -n 65535");
            LOG_INFO("ulimit file descriptor nums = 65535");
        }
    protected:
        bool ET{false};
        bool one_shot{true};
        std::string ip{};
        short port{0};
        int max_connect_count{200000};
        std::atomic<int> current_connect_count{0};

        virtual void close() = 0;
        virtual void enable_addr_reuse() = 0;
        virtual void disable_addr_reuse() = 0;
        virtual void enable_port_reuse() = 0;
        virtual void disable_port_reuse() = 0;
        virtual void enable_multi_thread() {}
        virtual void enable_multi_thread(int thread_count,int max_process_count) {}
        virtual void disable_multi_thread() = 0;
        virtual void enable_object_pool(size_t size) = 0;
        virtual void disable_object_pool() = 0;
        virtual void enable_et() { ET = true; }
        virtual void disable_et() { ET = false; }
        virtual void enable_one_shot() { one_shot = true; }
        virtual void disable_one_shot() { one_shot = false; }
        virtual void set_max_events_count(int size) = 0;
        virtual void set_max_connect_count(int size){if(size >= 0) max_connect_count = size;}
        virtual void set_listen_queue_count(int size) = 0;
    };
}

#endif
