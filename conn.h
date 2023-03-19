#ifndef CONV_EVENT_CONN_H
#define CONV_EVENT_CONN_H

#include <unistd.h>             /* close */
#include <arpa/inet.h>          /* socket */
#include <sys/epoll.h>          /* epoll */
#include "ErrorLog/ErrorLog.h"  /* hzd:: LOG LOG_MSG LOG_FMT */
#include <cstring>              /* memcpy bzero memset */
#include "utils.h"              /* hzd::header */
#include <memory>               /* unique_ptr */
#include <fcntl.h>              /* fcntl */
namespace hzd {

#define CONN_LOG_IP_PORT_FMT "client IP=%s client Port = %u",inet_ntoa(sock_addr.sin_addr),ntohs(sock_addr.sin_port)

    static void block_none(int fd)
    {
        int option = fcntl(fd,F_GETFL);
        int new_option = option | O_NONBLOCK;
        fcntl(fd,F_SETFL,new_option);
    }

    static int epoll_add(int epoll_fd,int socket_fd,bool et,bool one_shot)
    {
        epoll_event ev{};
        ev.data.fd = socket_fd;
        ev.events = EPOLLIN | EPOLLRDHUP;
        if(et) ev.events = ev.events | EPOLLET;
        if(one_shot) ev.events = ev.events | EPOLLONESHOT;
        return epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&ev);
    }
    static int epoll_mod(int epoll_fd,int socket_fd,uint32_t ev,bool et,bool one_shot)
    {
        epoll_event event{};
        event.data.fd = socket_fd;
        event.events = ev | EPOLLRDHUP;
        if(et){ event.events = event.events | EPOLLET; }
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
        bool working{false};
        bool ET{false};
        bool one_shot{false};
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
        bool send_base(const char* data)
        {
            size_t send_count;
            write_cursor = 0;
            while(write_cursor < write_total_bytes)
            {
                bzero(write_buffer,sizeof(write_buffer));
                memcpy(write_buffer,data+write_cursor,sizeof(write_buffer));
                if((send_count = ::send(socket_fd,write_buffer,sizeof(write_buffer),0))<= 0)
                {
                    LOG(Conn_Send,"data send error");
                    return false;
                }
                write_cursor += send_count;
            }
            write_cursor = 0;
            write_total_bytes = 0;
            return true;
        }
        bool recv_base(std::string& data)
        {
            if(read_total_bytes <= 0)
            {
                LOG(Conn_Recv,"recv size = 0");
                return false;
            }
            size_t read_count;
            read_cursor = 0;
            data.clear();
            while(read_cursor < read_total_bytes)
            {
                bzero(read_buffer,sizeof(read_buffer));
                if((read_count = ::recv(socket_fd,read_buffer,sizeof(read_buffer),MSG_DONTWAIT))<=0)
                {
                    LOG(Conn_Recv,"data recv error");
                    return false;
                }
                data += read_buffer;
                read_cursor += read_count;
            }
            read_cursor = 0;
            read_total_bytes = 0;
            return true;
        }
    protected:
        int socket_fd{-1};
        int epoll_fd{0};
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
            CLOSE,
        };
        /* default constructor */
        conn() = default;
        /* virtual destructor */
        virtual ~conn()
        {
            conn::close();
        }
        /* static member variable*/
        /* static member methods */
        /* common member variable */
        Status status{OK};
        /* common base member methods */
        bool send(const char* data,header_type type = header_type::BYTE)
        {
            write_total_bytes = strlen(data) + 1;
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
            return send_base(data);
        }
        bool send(std::string& data,header_type type = header_type::BYTE)
        {
            write_total_bytes = data.size()+1;
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
            return send_base(data.c_str());
        }
        bool send(std::string& data,size_t size)
        {
            write_total_bytes = size;
            if(write_total_bytes <= 0)
            {
                LOG(Conn_Send,"send data = null");
                return false;
            }
            return send_base(data.c_str());
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
            return recv_base(data);
        }
        bool recv(std::string& data,size_t size)
        {
            read_total_bytes = size;
            return recv_base(data);
        }
        int next(EPOLL_EVENTS event) const{
            return epoll_mod(epoll_fd,socket_fd,event,ET,one_shot);
        }
        bool is_working() const{ return working; }
        void set_working() { working = true; }
        void cancel_working() { working = false; }
        /* common virtual member methods */
        virtual void init(int _socket_fd,sockaddr_in* _addr,int _epoll_fd,bool et,bool _one_shot)
        {
            socket_fd = _socket_fd;
            sock_addr = *_addr;
            epoll_fd = _epoll_fd;
            ET = et;
            one_shot = _one_shot;
            epoll_add(epoll_fd,socket_fd,ET,one_shot);
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
}

#endif
