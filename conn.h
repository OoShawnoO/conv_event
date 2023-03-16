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
        if(one_shot) ev.events = ev.events | EPOLLONESHOT;
        return epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&ev);
    }
    static int epoll_mod(int epoll_fd,int socket_fd,uint32_t ev,bool one_shot)
    {
        epoll_event event{};
        event.data.fd = socket_fd;
        event.events = ev | EPOLLRDHUP;
        if(one_shot)
        {
            event.events = event.events | EPOLLONESHOT;
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
        bool set_recv_time_out(int sec,int usec)
        {
            set_time_out_timer(sec,usec);
            if(setsockopt(socket_fd,SOL_SOCKET,SO_RCVTIMEO,&time_out_timer,sizeof(time_out_timer)) < 0)
            {
                LOG(Socket_Set_Opt,"set sock opt error");
                return false;
            }
            return true;
        }
        void prepare_heart_beat()
        {
            heart_beat_send_time = time(nullptr);
            json pack{
                {"type","heart_beat"},
                {"time",(double)heart_beat_send_time}
            };
            bzero(heart_beat_buffer,sizeof(heart_beat_buffer));
            sprintf(heart_beat_buffer,"%s",pack.dump().c_str());
        }
        bool process_out_base()
        {
            if(status == HEARTBEAT)
            {
                prepare_heart_beat();
                if(send(socket_fd,heart_beat_buffer,sizeof(heart_beat_buffer),0) <= 0)
                {
                    status = BAD;
                    return false;
                }
                heart_beat_already = true;
                status = WAIT;
                return true;
            }
            else if(status == OUT)
            {
                return process_out();
            }
            else
            {
                status = BAD;
                return false;
            }
        }
        bool process_in_base()
        {
            if(status == WAIT)
            {
                set_recv_time_out(2,0);
                size_t ret = 0;
                bzero(heart_beat_buffer,sizeof(heart_beat_buffer));
                if((ret = recv(socket_fd,heart_beat_buffer,sizeof(heart_beat_buffer),MSG_PEEK)) <= 0)
                {
                    if(ret == -1 && errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        LOG_FMT(Heart_Beat_Timeout,"heart beat time out",CONN_LOG_IP_PORT_FMT);
                        status = BAD;
                        return false;
                    }
                    else if(ret == 0)
                    {
                        status = RDHUP;
                        return false;
                    }
                    else
                    {
                        status = BAD;
                        return false;
                    }
                }
                if(heart_beat_buffer[0] == '{')
                {
                    json pack;
                    std::string s(heart_beat_buffer);
                    if(pack.load(s))
                    {
                        if(recv(socket_fd,heart_beat_buffer,sizeof(heart_beat_buffer),0) <= 0)
                        {
                            if(ret == -1 && errno == EAGAIN || errno == EWOULDBLOCK)
                            {
                                LOG_FMT(Heart_Beat_Timeout,"heart beat time out",CONN_LOG_IP_PORT_FMT);
                                status = BAD;
                                return false;
                            }
                            else if(ret == 0)
                            {
                                status = RDHUP;
                                return false;
                            }
                            else
                            {
                                status = BAD;
                                return false;
                            }
                        }
                        if(pack["type"] == "heart_beat_ret")
                        {
                            last_in_time = time(nullptr);
                            heart_beat_already = false;
                            status = IN;
                            set_recv_time_out(0,0);
                            return true;
                        }
                    }
                    status = BAD;
                    return false;
                }
                status = BAD;
                return false;
            }
            else if(status == IN)
            {
                if(process_in())
                {
                    last_in_time = time(nullptr);
                    return true;
                }
                return false;
            }
            else
            {
                return false;
            }
        }
    protected:
        int socket_fd{-1};
        sockaddr_in sock_addr{};
        char read_buffer[4096] = {0};
        char write_buffer[4096] = {0};
        char heart_beat_buffer[64] = {0};
        int read_cursor{0};
        int write_cursor{0};
        int write_total_bytes{0};
        time_t heart_beat_send_time{0};
        struct timeval time_out_timer{2,0};
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
            WAIT,
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
                time_out_timer.tv_sec = sec;
                time_out_timer.tv_usec = usec;
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
                case WAIT :
                case IN : {
                    return process_in_base();
                }
                case HEARTBEAT:
                case OUT : {
                    return process_out_base();
                }
                case RDHUP : {
                    return process_rdhup();
                }
                case ERROR : {
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
