#ifndef CONV_EVENT_CONN_H
#define CONV_EVENT_CONN_H

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "ErrorLog/ErrorLog.h"
namespace hzd {

    static int epoll_add(int epoll_fd,int socket_fd,bool one_shot)
    {
        epoll_event ev{};
        ev.data.fd = socket_fd;
        ev.events = EPOLLIN | EPOLLRDHUP;
        if(one_shot) ev.events |= EPOLLONESHOT;
        return epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&ev);
    }
    static int epoll_mod(int epoll_fd,int socket_fd,uint32_t ev,bool one_shot)
    {
        epoll_event event{};
        event.data.fd = socket_fd;
        event.events = ev | EPOLLRDHUP;
        if(one_shot)
        {
            event.events |= EPOLLONESHOT;
        }
        return epoll_ctl(epoll_fd,EPOLL_CTL_MOD,socket_fd,&event);
    }
    static int epoll_del(int epoll_fd,int socket_fd)
    {
        int ret = epoll_ctl(epoll_fd,EPOLL_CTL_DEL,socket_fd,nullptr);
        ::close(socket_fd);
        return ret;
    }

    class conn {
    protected:
        int socket_fd{-1};
        sockaddr_in sock_addr{};
        char read_buffer[4096] = {0};
        char write_buffer[4096] = {0};
        int read_cursor{0};
        int write_cursor{0};
        int write_total_bytes{0};
    public:
        conn() = default;
        enum Status
        {
            NO,
            BAD,
            IN,
            OUT,
            RDHUP,
            ERROR,
        };
        Status status;
        virtual void init(int _socket_fd,sockaddr_in* _addr)
        {
            socket_fd = _socket_fd;
            sock_addr = *_addr;
            epoll_add(epoll_fd,socket_fd,false);
        }
        virtual sockaddr_in& addr()
        {
            return sock_addr;
        }
        static int epoll_add(int _epoll_fd,int _socket_fd,bool _one_shot)
        {
            return hzd::epoll_add(_epoll_fd,_socket_fd,_one_shot);
        }
        static int epoll_mod(int _epoll_fd,int _socket_fd,uint32_t _ev,bool _one_shot)
        {
            return hzd::epoll_mod(_epoll_fd,_socket_fd,_ev,_one_shot);
        }
        static int epoll_del(int _epoll_fd,int _socket_fd)
        {
            return epoll_del(_epoll_fd,_socket_fd);
        }
        virtual bool process_in() = 0;
        virtual bool process_out() = 0;
        virtual bool process_rdhup()
        {
            LOG_FMT(None,"client close","client IP=%s client Port = %u",
                    inet_ntoa(sock_addr.sin_addr),ntohs(sock_addr.sin_port));
            return true;
        }
        virtual bool process_error()
        {
            LOG_FMT(Epoll_Error,"client error","client IP=%s client Port = %u",
                    inet_ntoa(sock_addr.sin_addr),ntohs(sock_addr.sin_port));
            return true;
        }
        virtual bool process()
        {
            switch(status)
            {
                case IN:{
                    status = NO;
                    if(!process_in())
                    {
                        status = BAD;
                        return false;
                    }
                    return true;
                }
                case OUT:{
                    status = NO;
                    if(!process_out())
                    {
                        status = BAD;
                        return false;
                    }
                    return true;
                }
                case RDHUP:{
                    status = NO;
                    return process_rdhup();
                }
                case ERROR:{
                    status = NO;
                    return process_error();
                }
                case BAD:{
                    return false;
                }
                default:{
                    status = NO;
                    return true;
                }
            }
        }
        virtual void close()
        {
            if(socket_fd != -1)
            {
                ::close(socket_fd);
                socket_fd = -1;
            }
        }
        virtual ~conn()
        {
            conn::close();
        }
        static int epoll_fd;
    };
    int conn::epoll_fd = -1;
}

#endif
