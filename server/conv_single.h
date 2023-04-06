#ifndef CONV_EVENT_CONV_H
#define CONV_EVENT_CONV_H

#include "server/include/conn.h"        /* conn */
#include "server/include/threadpool.h"  /* thread_pool */
#include <csignal>                      /* signal */

namespace hzd {
    template<class T>
    class conv_single {
        /* static assert*/
        #if __cplusplus > 201703L
        static_assert(std::is_base_of_v<conn,T>,"must derived from class hzd::conn.");
        #else
        static_assert(std::is_base_of<conn,T>::value,"must derived from class hzd::conn.");
        #endif

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
          * @brief prepare epoll event
          * @note None
          * @param None
          * @retval None
          */
        inline void _prepare_epoll_event_()
        {
            if(!events)
                events = new epoll_event[max_events_count];
            if(!events)
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
            if(epoll_add(epoll_fd,socket_fd,ET,false) < 0)
            {
                LOG(Epoll_Add,"epoll add error");
                close();
                perror("epoll_add");
                exit(-1);
            }
        }

    protected:
        /* protected member variable */
        bool ET{false};
        bool one_shot{false};
        std::string ip;
        short port;
        int socket_fd{-1};
        sockaddr_in my_addr{};
        int listen_queue_count{32};
        int epoll_fd{-1};
        epoll_event* events{nullptr};
        int max_events_count{1024};
        int max_connect_count{200000};
        int current_connect_count{0};
        std::unordered_map<int,T*> connects;
        threadpool<T>* thread_pool{nullptr};
        connpool<T>* conn_pool{nullptr};
    public:
        /* Constructor */
        conv_single(std::string _ip,short _port,bool _one_shot = false,bool ET = false)
        :ip(std::move(_ip)),port(_port),one_shot(_one_shot),ET(ET)
        {
            _create_socket_();
            _prepare_socket_address_();
            signal(SIGPIPE,SIG_IGN);
        }
        /* Destructor */
        virtual ~conv_single()
        {
            close();
        }
        /* define */
        #define CONNECTS_REMOVE_FD do                   \
        {                                               \
            connects[cur_fd]->close();                  \
            current_connect_count--;                    \
            if(conn_pool)                               \
            {                                           \
                conn_pool->release(connects[cur_fd]);   \
            }                                           \
            else                                        \
            {                                           \
                delete connects[cur_fd];                \
                connects[cur_fd] = nullptr;             \
            }                                           \
            connects.erase(cur_fd);                     \
        }while(0)

        #define CONNECTS_REMOVE_FD_OUT do               \
        {                                               \
            current_connect_count--;                    \
            T* tmp = it->second;                        \
            tmp->close();                               \
            it = connects.erase(it);                    \
            if(conn_pool)                               \
            {                                           \
                conn_pool->release(tmp);                \
                tmp = nullptr;                          \
            }                                           \
            else                                        \
            {                                           \
                delete tmp;                             \
                tmp = nullptr;                          \
            }                                           \
        }while(0)
        /* common member methods */
        /**
          * @brief close all fd and delete the allocated data
          * @note None
          * @param None
          * @retval None
          */
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
            delete []events;
            events = nullptr;
            if(!connects.empty())
            {
                connects.clear();
            }
            delete thread_pool;
            thread_pool = nullptr;
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
            int opt = 1;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&opt,sizeof(opt));
        }
        /**
          * @brief disable address reuse
          * @note None
          * @param None
          * @retval None
          */
        void disable_addr_reuse()
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
          * @brief enable using multi-thread
          * @note None
          * @param thread_count working thread count
          * @param max_process_count max process count
          * @retval None
          */
        void enable_multi_thread(int thread_count = 8,int max_process_count = 10000)
        {
            if(!thread_pool)
            {
                thread_pool = new threadpool<T>(thread_count,max_process_count);
            }
        }
        /**
          * @brief disable using multi-thread
          * @note None
          * @param None
          * @retval None
          */
        void disable_multi_thread()
        {
            delete thread_pool;
            thread_pool = nullptr;
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
                conn_pool = new connpool<T>(size);
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
        void set_max_events_count(int size){if(size >= 0) max_events_count = size;}
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
        void set_listen_queue_count(int size){if(size >= 0) listen_queue_count = size;}
        /**
          * @brief start epoll
          * @note None
          * @param time_out epoll_wait's time out
          * @retval None
          */
        void wait(int time_out = -1)
        {
            _bind_();
            _prepare_epoll_event_();
            _listen_();
            _register_listen_fd_();
            int ret;
            int cur_fd;
            while(true)
            {
                for(auto it = connects.begin();it != connects.end();)
                {
                    if(it->second->status == conn::CLOSE)
                    {
                        CONNECTS_REMOVE_FD_OUT;
                    }
                    else
                    {
                        it++;
                    }
                }
                if((ret = epoll_wait(epoll_fd,events,max_events_count,time_out)) < 0)
                {
                        LOG(Epoll_Wait,"epoll wait error");
                        break;
                }
                for(int event_index = 0;event_index < ret; event_index++)
                {
                    cur_fd = events[event_index].data.fd;
                    if(cur_fd == socket_fd) {
                        sockaddr_in client_addr{};
                        socklen_t len = sizeof(client_addr);
                        int client_fd = accept(socket_fd, (sockaddr *) &client_addr, &len);
                        if (current_connect_count >= max_connect_count) {
                            LOG(Out_Of_Bound,"connects maxed");
                            ::close(client_fd);
                            continue;
                        }
                        if(conn_pool)
                        {
                            T* t = conn_pool->acquire();
                            if(!t) { ::close(client_fd);continue; }
                            if(connects[client_fd] != nullptr)
                            {
                                LOG(Pointer_To_Null,"already exist");
                                conn_pool->release(t);
                                ::close(client_fd);
                                continue;
                            }
                            connects[client_fd] = t;
                        }
                        else
                        {
                            if(connects[client_fd] != nullptr)
                            {
                                LOG(Pointer_To_Null,"already exist");
                                ::close(client_fd);
                                continue;
                            }
                            connects[client_fd] = new T;
                        }
                        connects[client_fd]->init(client_fd, &client_addr,epoll_fd,ET,one_shot);
                        current_connect_count++;
                    }
                    else if(events[event_index].events & EPOLLRDHUP)
                    {
                        connects[cur_fd]->status = conn::RDHUP;
                        if(thread_pool)
                        {
                            thread_pool->add(connects[cur_fd]);
                        }
                        else
                        {
                            if(!connects[cur_fd]->process())
                            {

                            }
                            CONNECTS_REMOVE_FD;
                        }
                    }
                    else if(events[event_index].events & EPOLLERR)
                    {
                        connects[cur_fd]->status = conn::ERROR;
                        if(thread_pool)
                        {
                            thread_pool->add(connects[cur_fd]);
                        }
                        else
                        {
                            if(!connects[cur_fd]->process())
                            {

                            }
                            CONNECTS_REMOVE_FD;
                        }
                    }
                    else if(events[event_index].events & EPOLLOUT)
                    {
                        connects[cur_fd]->status = conn::OUT;
                        if(thread_pool)
                        {
                            thread_pool->add(connects[cur_fd]);
                        }
                        else
                        {
                            if(!connects[cur_fd]->process())
                            {
                                LOG_FMT(Epoll_Write,"epoll write error","client IP=%s client Port = %u",
                                        inet_ntoa(connects[cur_fd]->addr().sin_addr),ntohs(connects[cur_fd]->addr().sin_port));
                                CONNECTS_REMOVE_FD;
                            }
                        }
                    }
                    else if(events[event_index].events & EPOLLIN)
                    {
                        connects[cur_fd]->status = conn::IN;
                        if(thread_pool)
                        {
                            thread_pool->add(connects[cur_fd]);
                        }
                        else
                        {
                            if(!connects[cur_fd]->process())
                            {
                                LOG_FMT(Epoll_Read,"epoll read error","client IP=%s client Port = %u",
                                        inet_ntoa(connects[cur_fd]->addr().sin_addr),ntohs(connects[cur_fd]->addr().sin_port));
                                CONNECTS_REMOVE_FD;
                            }
                        }
                    }
                    else
                    {
                        LOG_FMT(None,"unknown error","client IP=%s client Port = %u",
                                inet_ntoa(connects[cur_fd]->addr().sin_addr),ntohs(connects[cur_fd]->addr().sin_port));
                        CONNECTS_REMOVE_FD;
                    }
                }
            }
            close();
        }
    };

}

#endif
