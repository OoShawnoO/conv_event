#ifndef CONV_EVENT_ACCEPTOR_H
#define CONV_EVENT_ACCEPTOR_H

#include "conn.h"       /* conn */
#include "configure.h"  /* configure */

namespace hzd
{
    template <class T>
    class conv_multi;
    template <class T>
    class acceptor {
        void close()
        {
            if(socket_fd != -1)
            {
                ::close(socket_fd);
                socket_fd = -1;
            }
            if(epoll_fd != -1)
            {
                ::close(epoll_fd);
                epoll_fd = -1;
            }
            delete event;
            reactor<T>::set_run_false();
            event = nullptr;
            parent = nullptr;
        }

        /**
        * @brief create socket
        * @note None
        * @param None
        * @retval None
        */
        inline void _create_socket_()
        {
            int temp_fd = socket(AF_INET,SOCK_STREAM,0);
            if(temp_fd < 0)
            {
                close();
                perror("socket");
                LOG_ERROR("create socket failed");
                exit(-1);
            }
            socket_fd = temp_fd;
        }
        /**
        * @brief prepare socket address
        * @note None
        * @param None
        * @retval None
        */
        inline void _prepare_socket_address_()
        {
            my_addr.sin_addr.s_addr = inet_addr(ip.c_str());
            my_addr.sin_port = htons(port);
            my_addr.sin_family = AF_INET;
        }
        /**
        * @brief bind socket
        * @note None
        * @param None
        * @retval None
        */
        inline void _bind_()
        {
            if(bind(socket_fd,(sockaddr*)&my_addr,sizeof(my_addr)) < 0)
            {
                close();
                perror("bind");
                LOG_ERROR("bind failed");
                exit(-1);
            }
        }
        /**
         * @brief listen socket
         * @note None
         * @param None
         * @retval None
         */
        inline void _listen_()
        {
            if(listen(socket_fd,listen_queue_count) < 0)
            {
                close();
                exit(-1);
            }
        }
        /**
          * @brief register listen fd to epoll event
          * @note None
          * @param None
          * @retval None
          */
        inline void _register_listen_fd_()
        {
            if(epoll_add(epoll_fd,socket_fd,false,false,false) < 0)
            {
                close();
                perror("epoll_add");
                LOG_ERROR("epoll add listen fd failed");
                exit(-1);
            }
        }
        /**
          * @brief prepare epoll event
          * @note None
          * @param None
          * @retval None
          */
        inline void _prepare_epoll_event_()
        {
            if(!event)
                event = new epoll_event[max_event_count];
            if(!event)
            {
                close();
                exit(-1);
            }
            if(epoll_fd == -1)
                epoll_fd = epoll_create(1024);
            if(epoll_fd < 0)
            {
                close();
                perror("epoll_create");
                LOG_ERROR("epoll create failed");
                exit(-1);
            }
        }
        /**
        * @brief accept new connect
        * @note None
        * @param None
        * @retval None
        */
        void _accept_()
        {
            sockaddr_in client_addr{};
            socklen_t len = sizeof(client_addr);
            int fd = accept(socket_fd,(sockaddr*)&client_addr,&len);
            if(fd < 0)
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK) return;
                return;
            }
            int least = INT32_MAX;
            int least_index = 0;
            for(int i=0;i<parent->reactors.size();i++)
            {
                if(least > parent->reactors[i].connects.size()) {
                    least = parent->reactors[i].connects.size();
                    least_index = i;
                }
            }
            parent->reactors[least_index].add_conn(fd);
        }

    protected:
        std::string ip{};
        short port{};
        int socket_fd{-1};
        sockaddr_in my_addr{};
        int listen_queue_count{1024};
        int epoll_fd{-1};
        int max_event_count{4096};
        epoll_event* event{nullptr};
        conv_multi<T>* parent{nullptr};
        connpool<T>* conn_pool{nullptr};
    public:
        acceptor() = default;
        ~acceptor()
        {
            close();
        }
        /**
         * @brief enable address reuse
         * @note None
         * @param None
         * @retval None
         */
        void enable_addr_reuse()
        {
            int opt = 1;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&opt,sizeof(opt));
        }
        /**
          * @brief disable address reuse
          * @note None
          * @param None
          * @retval None
          */
        void disable_add_reuse()
        {
            int opt = 0;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&opt,sizeof(opt));
        }
        /**
          * @brief enable port reuse
          * @note None
          * @param None
          * @retval None
          */
        void enable_port_reuse()
        {
            int opt = 1;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEPORT,(const void*)&opt,sizeof(opt));
        }
        /**
          * @brief disable port reuse
          * @note None
          * @param None
          * @retval None
          */
        void disable_port_reuse()
        {
            int opt = 0;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEPORT,(const void*)&opt,sizeof(opt));
        }
        /**
          * @brief set listen queue size
          * @note None
          * @param size size
          * @retval None
          */
        void set_listen_queue_count(int size){if(size >= 0) listen_queue_count = size;}
        /**
        * @brief set connect pool
        * @note None
        * @param None
        * @retval None
        */
        void set_conn_pool(connpool<T>* cp)
        {
            conn_pool = cp;
        }
        void init(conv_multi<T>* _parent)
        {
            parent = _parent;
            conn_pool = parent->conn_pool;
            ip = parent->ip;
            port = parent->port;

            configure& conf = configure::get_config();
            listen_queue_count = conf["listen_queue_count"].type == JSON_NULL ? 1024 : (int32_t)conf["listen_queue_count"];
            max_event_count = conf["max_events_count"].type == JSON_NULL ? 4096 : (int32_t)conf["max_events_count"];

            _create_socket_();
            _prepare_socket_address_();

            LOG_TRACE("acceptor init success");
        }
        static bool run;

        void work()
        {
            _bind_();
            _prepare_epoll_event_();
            _register_listen_fd_();
            _listen_();

             LOG_INFO("socket already listening at " + ip + ":" + std::to_string(port));

            int ret;
            run = true;
            while(run)
            {
                if((ret = epoll_wait(epoll_fd,event,max_event_count,-1)) >= 0)
                {
                    for(int i=0;i<ret;i++)
                    {
                        _accept_();
                    }
                }
                else
                {
                    if(errno == EINTR) continue;
                    break;
                }
            }
            close();
        }
    };
    template<class T>
    bool acceptor<T>::run = false;
}

#endif
