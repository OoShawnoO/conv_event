#ifndef CONV_EVENT_CONN_H
#define CONV_EVENT_CONN_H

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <ctime>
#include <csignal>
#include <sys/time.h>
#include <queue>
#include "ErrorLog/ErrorLog.h"
#include "json/json.h"
namespace hzd {

#define CONN_LOG_IP_PORT_FMT "client IP=%s client Port = %u",inet_ntoa(sock_addr.sin_addr),ntohs(sock_addr.sin_port)

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
        void time_out(int arg)
        {
            status = TIMEOUT;
        }
        inline void set_itimer()
        {
            setitimer(ITIMER_PROF,&time_out_timer,nullptr);
        }
        void send_heart_beat()
        {
            heart_beat_send_time = time(nullptr);
            json pack{
                {"type","heart_beat"},
                {"time",(double)heart_beat_send_time}
            };
            bzero(heart_beat_packet,64);
            sprintf(heart_beat_packet,"%s",pack.dump().c_str());
            if(send(socket_fd,heart_beat_packet,sizeof(heart_beat_packet),0) <= 0)
            {
                LOG_FMT(Conn_Send,"conn send error",CONN_LOG_IP_PORT_FMT);
                status = BAD;
                return;
            }
        }
        void recv_heart_beat()
        {
            if(heart_beat_already)
            {
                int ret = 0;
                json pack;
                bzero(heart_beat_packet,64);
                setsockopt(socket_fd,SOL_SOCKET,SO_RCVTIMEO,&time_out_timer,sizeof(time_out_timer));
                if((ret = recv(socket_fd,heart_beat_packet,sizeof(heart_beat_packet),MSG_PEEK)) <= 0)
                {
                    LOG_FMT(Conn_Recv,"conn recv error",CONN_LOG_IP_PORT_FMT);
                    if(ret == 0)
                    {
                        status = RDHUP;
                    }
                    else if(ret == -1)
                    {
                        status = TIMEOUT;
                    }
                    else
                    {
                        status = BAD;
                    }
                    return;
                }
                std::string s(heart_beat_packet);
                if(pack.load(s))
                {
                    if(pack["type"] == "heart_beat_ret")
                    {
                        recv(socket_fd,heart_beat_packet,s.size()+1,0);
                        last_in_time = time(nullptr);
                        status = NO;
                        heart_beat_already = false;
                        return;
                    }
                }
            }
        }
    protected:
        int socket_fd{-1};
        sockaddr_in sock_addr{};
        char read_buffer[4096] = {0};
        char write_buffer[4096] = {0};
        char heart_beat_packet[64] = {0};
        int read_cursor{0};
        int write_cursor{0};
        int write_total_bytes{0};
        time_t heart_beat_send_time{0};
        struct itimerval time_out_timer{2,0};
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
            HEARTBEAT,
            TIMEOUT,
        };
        /* static member variable*/
        static struct sigaction* time_out_sigaction;
        static int epoll_fd;
        /* static member methods */
        static void set_time_out_sigaction(struct sigaction* sigact)
        {
            if(!sigact) return;
            hzd::conn::time_out_sigaction->sa_flags = sigact->sa_flags;
            hzd::conn::time_out_sigaction->sa_mask = sigact->sa_mask;
            hzd::conn::time_out_sigaction->sa_handler = sigact->sa_handler;
            sigaction(SIGPROF,time_out_sigaction,nullptr);
        }
        /* common member variable */
        Status status;
        time_t last_in_time{time(nullptr)};
        bool heart_beat_already{false};
        /* common member methods */
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
        virtual int fd(){return socket_fd;}
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
            return hzd::epoll_del(_epoll_fd,_socket_fd);
        }
        virtual void set_time_out_timer(int sec,int usec)
        {
            if(sec >= 0 && usec >= 0)
            {
                time_out_timer.it_interval.tv_sec = sec;
                time_out_timer.it_interval.tv_usec = usec;
            }
        }
        virtual bool process_in() = 0;
        virtual bool process_out() = 0;
        virtual bool process_rdhup()
        {
            LOG_FMT(None,"client close",CONN_LOG_IP_PORT_FMT);
            return true;
        }
        virtual bool process_error()
        {
            LOG_FMT(Epoll_Error,"client error",CONN_LOG_IP_PORT_FMT);
            return true;
        }
        virtual bool process()
        {
            switch(status)
            {
                case HEARTBEAT : {
                    status = NO;
                    send_heart_beat();
                    recv_heart_beat();
                    return true;
                }
                case IN : {
                    status = NO;
                    recv_heart_beat();
                    if(!process_in())
                    {
                        status = BAD;
                        return false;
                    }
                    last_in_time = time(nullptr);
                    return true;
                }
                case OUT : {
                    status = NO;
                    if(!process_out())
                    {
                        status = BAD;
                        return false;
                    }
                    return true;
                }
                case RDHUP : {
                    status = NO;
                    return process_rdhup();
                }
                case ERROR : {
                    status = NO;
                    return process_error();
                }
                case BAD : {
                    return false;
                }
                default : {
                    status = NO;
                    return true;
                }
            }
        }
        virtual void close()
        {
            if(socket_fd != -1)
            {
                epoll_del(epoll_fd,socket_fd);
                ::close(socket_fd);
                socket_fd = -1;
            }
        }
        virtual ~conn()
        {
            conn::close();
        }
    };
    struct sigaction* conn::time_out_sigaction = nullptr;
    int conn::epoll_fd = -1;
}

#endif
