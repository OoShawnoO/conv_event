#ifndef CONV_EVENT_CONN_H
#define CONV_EVENT_CONN_H

#include <arpa/inet.h>          /* socket */
#include <sys/epoll.h>          /* epoll */
#include "ErrorLog/ErrorLog.h"  /* hzd:: LOG LOG_MSG LOG_FMT */
#include "json/json.h"          /* hzd::json */
#include "utils.h"              /* hzd::header */
#include <memory>               /* unique_ptr */
namespace hzd {

#define CONN_LOG_IP_PORT_FMT "client IP=%s client Port = %u",inet_ntoa(sock_addr.sin_addr),ntohs(sock_addr.sin_port)

    static int epoll_add(int epoll_fd,int socket_fd,bool one_shot)
    {
        epoll_event ev{};
        ev.data.fd = socket_fd;
        ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
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
        /* private member variable */
        char read_buffer[4096] = {0};
        char write_buffer[4096] = {0};
        size_t read_cursor{0};
        size_t write_cursor{0};
        size_t write_total_bytes{0};
        size_t read_total_bytes{0};
        /* private member method */
        bool process_out_base()
        {
            return process_out();
        }
        bool process_in_base()
        {
            return process_in();
        }
        bool send_base(const char* data,header_type type)
        {
            if(write_total_bytes <= 1)
            {
                LOG(Conn_Send,"send data = null");
                return false;
            }
            header h{type,write_total_bytes};
            if(::send(socket_fd,&h,HEADER_SIZE,0) <= 0)
            {
                LOG(Conn_Send,"header send error");
                return false;
            }
            size_t send_count;
            write_cursor = 0;
            while(write_cursor < write_total_bytes)
            {
                bzero(write_buffer,sizeof(write_buffer));
                memcpy(write_buffer,data+write_cursor,sizeof(write_buffer));
                send_count = ::send(socket_fd,write_buffer,sizeof(write_buffer),0);
                if(send_count <= 0)
                {
                    LOG(Conn_Send,"data send error");
                    return false;
                }
                write_cursor += send_count;
            }
            return true;
        }
    protected:
        int socket_fd{-1};
        sockaddr_in sock_addr{};
    public:
        enum Status
        {
            OK,
            BAD,
            IN,
            OUT,
            RDHUP,
            ERROR,
        };
        /* default constructor */
        conn() = default;
        /* virtual destructor */
        virtual ~conn()
        {
            conn::close();
        }
        /* static member variable*/
        static int epoll_fd;
        /* static member methods */
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
        /* common member variable */
        Status status{OK};
        /* common base member methods */
        bool send(const char* data,header_type type = header_type::BYTE)
        {
            write_total_bytes = strlen(data) + 1;
            return send_base(data,type);
        }
        bool send(std::string& data,header_type type = header_type::BYTE)
        {
            write_total_bytes = data.size()+1;
            return send_base(data.c_str(),type);
        }
        bool recv(std::string& data,header_type& type)
        {
            header h{};
            if(::recv(socket_fd,&h,HEADER_SIZE,0) <= 0)
            {
                LOG(Conn_Recv,"recv header error");
                return false;
            }
            read_total_bytes = h.size;
            type = h.type;
            size_t read_count;
            read_cursor = 0;
            data.clear();
            while(read_cursor < read_total_bytes)
            {
                bzero(read_buffer,sizeof(read_buffer));
                read_count = ::recv(socket_fd,read_buffer,sizeof(read_buffer),0);
                if(read_count <= 0)
                {
                    LOG(Conn_Recv,"data recv error");
                    return false;
                }
                data += read_buffer;
                read_cursor += read_count;
            }
            return true;
        }
        /* common virtual member methods */
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
                case IN : {
                    return process_in_base();
                }
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
                    status = OK;
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

    };

    int conn::epoll_fd = -1;
}

#endif
