#ifndef CONV_EVENT_CONV_H
#define CONV_EVENT_CONV_H

#include <iostream>
#include <unordered_map>
#include <type_traits>
#include <utility>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "ErrorLog/ErrorLog.h"
#include "threadpool.h"
#include "conn.h"

namespace hzd {
    template<class T>
    class conv {
        static_assert(std::is_base_of_v<conn,T>,"must derived from class hzd::conn.");
    private:
        std::string ip;
        short port;
        int socket_fd{-1};
        sockaddr_in my_addr{};
        int epoll_fd{-1};
        epoll_event* events{nullptr};
        int max_events_count{1024};
        int listen_queue_count{32};
        int max_connect_count{512};
        int current_connect_count{0};
        bool heart_beat{false};
        std::unordered_map<int,T*> connects;
        threadpool<T>* thread_pool = nullptr;

    public:
        conv(std::string _ip,short _port):ip(std::move(_ip)),port(_port)
        {
            int temp_fd = socket(AF_INET,SOCK_STREAM,0);
            if(temp_fd < 0)
            {
                LOG(Socket_Create,"socket create error");
                close();
                perror("socket");
                exit(-1);
            }
            my_addr.sin_addr.s_addr = inet_addr(ip.c_str());
            my_addr.sin_port = htons(port);
            my_addr.sin_family = AF_INET;
            if(bind(temp_fd,(sockaddr*)&my_addr,sizeof(my_addr)) < 0)
            {
                LOG_FMT(Socket_Bind,"bind socket error","IP=%s,PORT=%u",ip.c_str(),port);
                close();
                perror("bind");
                exit(-1);
            }
            socket_fd = temp_fd;
            events = new epoll_event[max_events_count];
            if(!events)
            {
                LOG(Bad_Malloc,"epoll_event bad new");
                close();
                exit(-1);
            }
            epoll_fd = epoll_create(1024);
            T::epoll_fd = epoll_fd;
            if(epoll_fd < 0)
            {
                LOG(Epoll_Create,"epoll create error");
                close();
                perror("epoll_create");
                exit(-1);
            }
            if(epoll_add(epoll_fd,socket_fd,false) < 0)
            {
                LOG(Epoll_Add,"epoll add error");
                close();
                perror("epoll_add");
                exit(-1);
            }
        }
        ~conv()
        {
            close();
        }

        void close()
        {
            if(socket_fd != -1)
            {
                ::close(socket_fd);
            }
            if(epoll_fd != -1)
            {
                ::close(epoll_fd);
            }
            delete []events;
            events = nullptr;
            if(!connects.empty())
            {
                connects.clear();
            }
            delete thread_pool;
            thread_pool = nullptr;

        }
        void enable_addr_reuse()
        {
            int opt = 1;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&opt,sizeof(opt));
        }
        void disable_add_reuse()
        {
            int opt = 0;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&opt,sizeof(opt));
        }
        void enable_port_reuse()
        {
            int opt = 1;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEPORT,(const void*)&opt,sizeof(opt));
        }
        void disable_port_reuse()
        {
            int opt = 0;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEPORT,(const void*)&opt,sizeof(opt));
        }
        void enable_multi_thread(int thread_count = 8,int max_process_count = 10000)
        {
            if(!thread_pool)
            {
                thread_pool = new threadpool<T>(thread_count,max_process_count);
            }
        }
        void disable_multi_thread()
        {
            delete thread_pool;
            thread_pool = nullptr;
        }
        void enable_heart_beat(){heart_beat = true;}
        void disable_heart_beat(){heart_beat = false;}
        void set_max_events_count(int size){if(size >= 0) max_events_count = size;}
        void set_max_connect_count(int size){if(size >= 0) max_connect_count = size;}
        void set_listen_queue_count(int size){if(size >= 0) listen_queue_count = size;}

        #define CONNECTS_REMOVE_FD_OUT do{      \
            connect->second->close();           \
            current_connect_count--;            \
            connect = connects.erase(connect);  \
        }while(0)

        #define CONNECTS_REMOVE_FD do{          \
            connects[cur_fd]->close();          \
            current_connect_count--;            \
            auto iter = connects.find(cur_fd);  \
            if(iter != connects.end())          \
            {                                   \
                connects.erase(iter);           \
            }                                   \
        }while(0)
        void wait(int time_out = 0)
        {
            if(listen(socket_fd,listen_queue_count) < 0)
            {
                LOG(Socket_Listen,"listen socket error");
                close();
                exit(-1);
            }
            if(!events)
            {
                LOG(Pointer_To_Null,"epoll_events array is nullptr");
                close();
                exit(-1);
            }
            while(true)
            {
                int ret = 0;
                for(auto connect = connects.begin();connect != connects.end();)
                {
                    if(connect->second->fd() == socket_fd) continue;
                    if(!connect->second->heart_beat_already && time(nullptr) - connect->second->last_in_time > 4)
                    {
                        connect->second->status = conn::HEARTBEAT;
                        connect->second->heart_beat_already = true;
                    }
                    switch(connect->second->status)
                    {
                        case conn::TIMEOUT :
                        case conn::BAD : {
                            CONNECTS_REMOVE_FD_OUT;
                            break;
                        }
                        case conn::HEARTBEAT : {
                            if(heart_beat)
                            {
                                if(thread_pool)
                                {
                                    thread_pool->add(connect->second);
                                    connect++;
                                }
                                else
                                {
                                    if(!connect->second->process())
                                    {
                                        LOG_FMT(Epoll_Read,"epoll read error","client IP=%s client Port = %u",
                                                inet_ntoa(connect->second->addr().sin_addr),ntohs(connect->second->addr().sin_port));
                                        CONNECTS_REMOVE_FD_OUT;
                                    }
                                    else
                                    {
                                        connect++;
                                    }
                                }
                            }
                            else
                            {
                                connect->second->status = conn::NO;
                                connect++;
                            }
                            break;
                        }
                        default : {
                            connect++;
                        }
                    }
                }
                if((ret = epoll_wait(epoll_fd,events,max_events_count,time_out)) < 0)
                {
                    if(errno == EINTR)
                    {
                        LOG_MSG("user interrupt.");
                        break;
                    }
                    else
                    {
                        LOG(Epoll_Wait,"epoll wait error");
                        break;
                    }
                }
                for(int event_index = 0;event_index < ret; event_index++)
                {
                    int cur_fd = events[event_index].data.fd;
                    if(cur_fd == socket_fd) {
                        sockaddr_in client_addr{};
                        socklen_t len = sizeof(client_addr);
                        int client_fd = accept(socket_fd, (sockaddr *) &client_addr, &len);
                        if (current_connect_count >= max_connect_count) {
                            ::close(client_fd);
                            continue;
                        }
                        connects[client_fd] = new T;
                        connects[client_fd]->init(client_fd, &client_addr);
                        current_connect_count++;
                    }
                    else if(events[event_index].events & EPOLLRDHUP)
                    {
                        connects[cur_fd]->status = conn::RDHUP;
                        if(!connects[cur_fd]->process())
                        {

                        }
                        CONNECTS_REMOVE_FD;
                    }
                    else if(events[event_index].events & EPOLLERR)
                    {
                        connects[cur_fd]->status = conn::ERROR;
                        if(!connects[cur_fd]->process())
                        {

                        }
                        CONNECTS_REMOVE_FD;
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
