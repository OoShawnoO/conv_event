#ifndef USE_CONV_ACCEPTOR_H
#define USE_CONV_ACCEPTOR_H

#include "conn.h"   /* conn */

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
                LOG(Socket_Create,"socket create error");
                close();
                perror("socket");
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
                LOG_FMT(Socket_Bind,"bind socket error","IP=%s,PORT=%u",ip.c_str(),port);
                close();
                perror("bind");
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
                LOG(Socket_Listen,"listen socket error");
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
                LOG(Epoll_Add,"epoll add error");
                close();
                perror("epoll_add");
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
                event = new epoll_event[1024];
            if(!event)
            {
                LOG(Bad_Malloc,"epoll_event bad new");
                close();
                exit(-1);
            }
            if(epoll_fd == -1)
                epoll_fd = epoll_create(1024);
            if(epoll_fd < 0)
            {
                LOG(Epoll_Create,"epoll create error");
                close();
                perror("epoll_create");
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
                LOG(Socket_Accept,"socket accept error");
                return;
            }
            T* t;
            if(conn_pool)
            {
                t = conn_pool->acquire();
                if(!t){::close(fd);return;}
            }
            else
            {
                t = new T;
                if(!t){::close(fd);return;}
            }
            parent->current_connect_count++;
            t->init(fd,&client_addr);
            static size_t count = 0;
            parent->reactors[count%parent->reactors.size()].add_conn(t);
            count++;
        }

    protected:
        std::string ip{};
        short port{};
        int socket_fd{-1};
        sockaddr_in my_addr{};
        int listen_queue_count{32};
        int epoll_fd{-1};
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
            _create_socket_();
            _prepare_socket_address_();
        }
        static bool run;

        void work()
        {
            _bind_();
            _prepare_epoll_event_();
            _register_listen_fd_();
            _listen_();

            int ret;
            run = true;
            while(run)
            {
                if((ret = epoll_wait(epoll_fd,event,1024,-1)) >= 0)
                {
                    for(int i=0;i<ret;i++)
                    {
                        _accept_();
                    }
                }
                else
                {
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
