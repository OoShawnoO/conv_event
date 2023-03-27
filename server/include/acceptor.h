#ifndef USE_CONV_ACCEPTOR_H
#define USE_CONV_ACCEPTOR_H

#include <utility>

#include "conn.h"

namespace hzd
{
    template <class T>
    class acceptor {
        void close()
        {
            if(socket_fd != -1)
            {
                ::close(socket_fd);
                socket_fd = -1;
            }
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

        void _accept_()
        {
            sockaddr_in client_addr{};
            socklen_t len = sizeof(client_addr);
            int fd = accept(socket_fd,(sockaddr*)&client_addr,&len);
            T* t = nullptr;
            if(conn_pool)
            {
                t = conn_pool->acquire();
                if(!t){::close(fd);return;}
            }
            else
            {
                t = new T;
            }
            t->init(socket_fd,&client_addr);
            conn_queue.push(t);
        }
    protected:
        std::string ip{};
        short port{};
        int socket_fd{-1};
        sockaddr_in my_addr{};
        int listen_queue_count{32};
        bool& run;
        safe_queue<T>& conn_queue;
        connpool<T>* conn_pool{nullptr};
    public:
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

        void init(std::string _ip,short _port,safe_queue<T*>& _conn_queue,bool& _run,connpool<T>* cp = nullptr)
        {
            ip = std::move(_ip);
            port = _port;
            conn_queue = _conn_queue;
            run = _run;
            conn_pool = cp;
            _create_socket_();
            _prepare_socket_address_();
            _bind_();
        }
        void work()
        {
            while(run)
            {
                _accept_();
            }
        }
    };
}

#endif
