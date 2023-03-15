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
        std::unordered_map<int,T*> connects;

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
            if(events)
            {
                delete []events;
            }
            if(!connects.empty())
            {
                connects.clear();
            }
        }
        void addr_reuse()
        {
            int opt = 1;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&opt,sizeof(opt));
        }
        void port_reuse()
        {
            int opt = 1;
            setsockopt(socket_fd,SOL_SOCKET,SO_REUSEPORT,(const void*)&opt,sizeof(opt));
        }

        void wait(int time_out = 0)
        {
            if(listen(socket_fd,listen_queue_count) < 0)
            {
                LOG(Socket_Listen,"listen socket error");
                close();
                exit(-1);
            }
            while(true)
            {
                int ret = 0;
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
                    }
                    #define CONNECTS_REMOVE_FD do{                  \
                                connects[cur_fd]->close();           \
                                auto iter = connects.find(cur_fd);  \
                                if(iter != connects.end())          \
                                {                                   \
                                    connects.erase(iter);           \
                                }                                   \
                    }while(0)
                    else if(events[event_index].events & EPOLLRDHUP)
                    {
                        if(!connects[cur_fd]->process_rdhup())
                        {

                        }
                        CONNECTS_REMOVE_FD;
                    }
                    else if(events[event_index].events & EPOLLERR)
                    {
                        if(!connects[cur_fd]->process_error())
                        {

                        }
                        CONNECTS_REMOVE_FD;
                    }
                    else if(events[event_index].events & EPOLLIN)
                    {
                        LOG_MSG("123");
                        if(!connects[cur_fd]->process_in())
                        {
                            LOG_FMT(Epoll_Read,"epoll read error","client IP=%s client Port = %u",
                                    inet_ntoa(connects[cur_fd]->addr().sin_addr),ntohs(connects[cur_fd]->addr().sin_port));
                            CONNECTS_REMOVE_FD;
                        }
                    }
                    else if(events[event_index].events & EPOLLOUT)
                    {
                        if(!connects[cur_fd]->process_out())
                        {
                            LOG_FMT(Epoll_Write,"epoll write error","client IP=%s client Port = %u",
                                    inet_ntoa(connects[cur_fd]->addr().sin_addr),ntohs(connects[cur_fd]->addr().sin_port));
                            CONNECTS_REMOVE_FD;
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
