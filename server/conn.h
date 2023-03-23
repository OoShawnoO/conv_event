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
#include <deque>                /* deque */
#include <vector>               /* vector */
#include <algorithm>
namespace hzd {

#define CONN_LOG_IP_PORT_FMT "client IP=%s client Port = %u",inet_ntoa(sock_addr.sin_addr),ntohs(sock_addr.sin_port)
    /**
      * @brief set fd none-blocking
      * @note None
      * @param fd fd for set none-blocking
      * @retval None
      */
    static void block_none(int fd)
    {
        int option = fcntl(fd,F_GETFL);
        int new_option = option | O_NONBLOCK;
        fcntl(fd,F_SETFL,new_option);
    }
    /**
      * @brief for epoll add event
      * @note None
      * @param epoll_fd epoll fd
      * @param et enable et model or not
      * @param one_shot enable one_shot or not
      * @param socket_fd socket fd
      * @retval None
      */
    static int epoll_add(int epoll_fd,int socket_fd,bool et,bool one_shot)
    {
        epoll_event ev{};
        ev.data.fd = socket_fd;
        ev.events = EPOLLIN | EPOLLRDHUP;
        if(et) ev.events = ev.events | EPOLLET;
        if(one_shot) ev.events = ev.events | EPOLLONESHOT;
        return epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&ev);
    }
    /**
      * @brief for epoll modify event
      * @note None
      * @param epoll_fd epoll fd
      * @param et enable et model or not
      * @param one_shot enable one_shot or not
      * @param socket_fd socket fd
      * @retval success or not
      */
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
    /**
      * @brief for epoll delete event
      * @note None
      * @param epoll_fd epoll fd
      * @param et enable et model or not
      * @param one_shot enable one_shot or not
      * @param socket_fd socket fd
      * @retval success or not
      */
    static int epoll_del(int epoll_fd,int socket_fd)
    {
        int ret = epoll_ctl(epoll_fd,EPOLL_CTL_DEL,socket_fd,nullptr);
        ::close(socket_fd);
        return ret;
    }

    class conn {
        /* private member variable */
        bool ET{false};
        bool one_shot{false};
        char read_buffer[2048] = {0};
        char write_buffer[2048] = {0};
        size_t read_cursor{0};
        size_t write_cursor{0};
        size_t write_total_bytes{0};
        size_t read_total_bytes{0};
        /* private member method */
        /**
          * @brief base of process out data
          * @note None
          * @param None
          * @retval success or not
          */
        bool process_out_base()
        {
            return process_out();
        }
        /**
          * @brief base of process in data
          * @note None
          * @param None
          * @retval success or not
          */
        bool process_in_base()
        {
            return process_in();
        }
        /**
          * @brief base of send data
          * @note None
          * @param data data
          * @retval success or not
          */
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
            return true;
        }
        /**
          * @brief base of recv data
          * @note None
          * @param data data save in this string
          * @retval success or not
          */
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
                if((read_count = ::recv(socket_fd,read_buffer,sizeof(read_buffer),0))<=0)
                {
                    if(read_count == -1)
                    {
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            return true;
                        }
                        LOG(Conn_Recv,"data recv error");
                        return false;
                    }
                    else
                    {
                        LOG(Conn_Recv,"client close");
                        return false;
                    }
                }
                data += read_buffer;
                read_cursor += read_count;
            }
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
        /**
          * @brief send data by using hzd::header
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
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
        /**
          * @brief send data by using hzd::header
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
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
        /**
          * @brief send data by given length of data
          * @note None
          * @param data data
          * @param size data size
          * @retval None
          */
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
        /**
          * @brief recv data and output type
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
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
        /**
          * @brief recv data by given size
          * @note None
          * @param data data
          * @param size data-size
          * @retval None
          */
        bool recv(std::string& data,size_t size)
        {
            read_total_bytes = size;
            return recv_base(data);
        }
        /**
        * @brief recv all data from system buffer
        * @note None
        * @param None
        * @retval None
        */
        bool recv_all(std::string& data)
        {
            read_total_bytes = SIZE_MAX;
            return recv_base(data);
        }
        /**
          * @brief register next event
          * @note None
          * @param event EPOLL_EVENTS
          * @retval success or not
          */
        int next(EPOLL_EVENTS event) const{
            return epoll_mod(epoll_fd,socket_fd,event,ET,one_shot);
        }
        /* thread safety */
        void notify_close() {
            next(EPOLLRDHUP);
            status = CLOSE;
        }
        /* common virtual member methods */
        virtual void init(int _socket_fd,sockaddr_in* _addr,int _epoll_fd,bool et,bool _one_shot)
        {
            socket_fd = _socket_fd;
            sock_addr = *_addr;
            epoll_fd = _epoll_fd;
            ET = et;
            one_shot = _one_shot;
//            block_none(socket_fd);
            epoll_add(epoll_fd,socket_fd,ET,one_shot);
        }
        /**
          * @brief get socket address for this connect
          * @note None
          * @param None
          * @retval socket address
          */
        virtual sockaddr_in& addr()
        {
            return sock_addr;
        }
        /**
          * @brief get socket file-descriptor
          * @note None
          * @param None
          * @retval file-descriptor
          */
        virtual int fd(){return socket_fd;}
        /**
          * @brief method must be override for process event EPOLLIN
          * @note None
          * @param None
          * @retval success or not
          */
        virtual bool process_in() = 0;
        /**
          * @brief method must be override for process event EPOLLOUT
          * @note None
          * @param None
          * @retval success or not
          */
        virtual bool process_out() = 0;
        /**
          * @brief method optional be override for process event EPOLLRDHUP
          * @note None
          * @param None
          * @retval success or not
          */
        virtual bool process_rdhup()
        {
            LOG_FMT(None,"client close",CONN_LOG_IP_PORT_FMT);
            return true;
        }
        /**
          * @brief method optional be override for process event EPOLLERROR
          * @note None
          * @param None
          * @retval success or not
          */
        virtual bool process_error()
        {
            LOG_FMT(Epoll_Error,"client error",CONN_LOG_IP_PORT_FMT);
            return true;
        }
        virtual bool process()
        {
            switch(status)
            {
                case CLOSE : {
                    return false;
                }
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

    template<class T>
    class connpool
    {
        #if __cplusplus > 201703L
                static_assert(std::is_base_of_v<conn,T>,"must derived from class hzd::conn.");
        #else
                static_assert(std::is_base_of<conn,T>::value,"must derived from class hzd::conn.");
        #endif

        size_t size{0};
        size_t max_count;
        std::vector<T*> object_vector;
        std::deque<T*> object_queue;
        std::mutex mtx;
    public:
        connpool(size_t _size,size_t _max_count=10000)
        :size(_size),max_count(_max_count)
        {
            object_vector.reserve(size);
            T* t = nullptr;
            for(int i=0;i<size;i++)
            {
                t = new T;
                object_vector[i] = t;
                object_queue.push_back(t);
            }
        }
        ~connpool()
        {
            for(int i=0;i<object_vector.size();i++)
            {
                delete object_vector[i];
                object_vector[i] = nullptr;
            }
        }
        connpool(const connpool<T>&) = delete;
        const connpool<T>& operator=(const connpool<T>&) = delete;

        T* acquire()
        {
            if(object_vector.size() >= max_count) return nullptr;
            std::lock_guard<std::mutex> guard(mtx);
            if(object_queue.empty())
            {
                    T* t = new T;
                    object_vector.push_back(t);
                    object_queue.push_back(t);
            }
            T* t =object_queue.front();
            object_queue.pop_front();
            return t;
        }

        bool release(T* t)
        {
            if(find(object_vector.begin(),object_vector.end(),t) == object_vector.end()) return false;
            std::lock_guard<std::mutex> guard(mtx);
            object_queue.push_back(t);
            return true;
        }
    };
}

#endif
