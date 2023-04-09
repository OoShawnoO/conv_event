#ifndef CONV_EVENT_CONN_H
#define CONV_EVENT_CONN_H

#include "socket_io.h"
#include <unistd.h>             /* close */
#include <arpa/inet.h>          /* socket */
#include <sys/epoll.h>          /* epoll */
#include <fcntl.h>              /* fcntl */
#include <deque>                /* deque */
#include <vector>               /* vector */
#include <algorithm>            /* find */
#include <memory>               /* unique_ptr */
#include <atomic>               /* atomic */
#include "safe_queue.h"
#include "lock_queue.h"

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
    static int epoll_add(int epoll_fd,int socket_fd,bool et,bool one_shot,bool none_block = true)
    {
        epoll_event ev{};
        ev.data.fd = socket_fd;
        ev.events = EPOLLIN | EPOLLRDHUP;
        if(et) ev.events = ev.events | EPOLLET;
        if(one_shot) ev.events = ev.events | EPOLLONESHOT;
        if(none_block) block_none(socket_fd);
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

    class conn : public socket_io{
        /* private member variable */
        bool ET{false};
        bool one_shot{true};
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
    protected:
        int epoll_fd{0};
        sockaddr_in sock_addr{};
        safe_queue<int>* close_queue{nullptr};
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
          * @brief register next event
          * @note None
          * @param event EPOLL_EVENTS
          * @retval success or not
          */
        int next(uint32_t event) const{
            return epoll_mod(epoll_fd,socket_fd,event,ET,one_shot);
        }
        /* thread safety */
        inline void notify_close() {
            status = CLOSE;
            if(close_queue) close_queue->push(socket_fd);
        }
        /* common virtual member methods */
        virtual void init(int _socket_fd,sockaddr_in* _addr)
        {
            status = OK;
            socket_fd = _socket_fd;
            sock_addr = *_addr;
        }
        virtual void init(int _epoll_fd,bool et,bool _one_shot,safe_queue<int>* cq)
        {
            status = OK;
            close_queue = cq;
            epoll_fd = _epoll_fd;
            ET = et;
            one_shot = _one_shot;
            epoll_add(epoll_fd,socket_fd,ET,one_shot);
        }
        virtual void init(int _socket_fd,sockaddr_in* _addr,int _epoll_fd,bool et,bool _one_shot)
        {
            status = OK;
            socket_fd = _socket_fd;
            sock_addr = *_addr;
            epoll_fd = _epoll_fd;
            ET = et;
            one_shot = _one_shot;
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
            LOG_MSG("client rdhup close");
            notify_close();
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
            notify_close();
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
                epoll_fd = -1;
                socket_fd = -1;
                status = OK;
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
        safe_queue<T*> q;
        size_t size;
    public:
        explicit connpool(size_t _size):size(_size)
        {
            for(size_t i=0;i<_size;i++)
            {
                q.push(new T);
            }
        }
        ~connpool()
        {
            T* t = nullptr;
            for(size_t i=0;i<size;i++)
            {
                if(q.pop(t))
                {
                    delete t;
                    t = nullptr;
                }
            }
            delete t;
        }
        connpool(const connpool<T>&) = delete;
        const connpool<T>& operator=(const connpool<T>&) = delete;

        T* acquire()
        {
            T* t;
            if(!q.pop(t))
            {
                t = new T;
                size++;
            }
            return t;
        }

        bool release(T* t)
        {
            q.push(t);
            return true;
        }
    };
}

#endif
