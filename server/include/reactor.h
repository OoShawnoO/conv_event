#ifndef USE_CONV_REACTOR_H
#define USE_CONV_REACTOR_H

#include "conn.h"
#include "threadpool.h"

namespace hzd
{
    template <class T>
    class conv_multi;
    template <class T>
    class reactor {
        void close()
        {
            if(epoll_fd != -1)
            {
                ::close(epoll_fd);
                epoll_fd = -1;
            }
            delete []events;
            events = nullptr;
            delete thread_pool;
            thread_pool = nullptr;
            parent = nullptr;
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
    protected:
        bool ET{false};
        bool one_shot{true};
        int epoll_fd{-1};
        epoll_event* events{nullptr};
        int max_events_count{1024};
        std::unordered_map<int,T*> connects;
        threadpool<T>* thread_pool{nullptr};
        connpool<T>* conn_pool{nullptr};
        safe_queue<T*>* conn_queue{nullptr};
        conv_multi<T>* parent{nullptr};

#define CONNECTS_REMOVE_FD_REACTOR do                   \
        {                                               \
            parent->current_connect_count--;            \
            T* tmp = connects[cur_fd];                  \
            connects.erase(cur_fd);                     \
            tmp->close();                               \
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

#define CONNECTS_REMOVE_FD_REACTOR_OUT do{              \
            parent->current_connect_count--;            \
            T* tmp = it->second;                        \
            it = connects.erase(it);                    \
            tmp->close();                               \
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

    public:
        static void work(void* r)
        {
            auto* reac = (reactor<T>*)r;
            reac->work();
        }
        static bool run;
        static void set_run_false()
        {
            run = false;
        }
        static void set_run_true()
        {
            run = true;
        }
        reactor()
        {
            _prepare_epoll_event_();
        }
        ~reactor()
        {
            close();
        }
        /**
  * @brief set max events count
  * @note None
  * @param size size
  * @retval None
  */
        void set_max_events_count(int size){if(size >= 0) max_events_count = size;}
        void init(conv_multi<T>* _parent)
        {
            parent = _parent;
            ET = parent->ET;
            one_shot = parent->one_shot;
            if(parent->_thread_pool)
            {
                if(!thread_pool)
                    thread_pool = new threadpool<T>;
            }
            conn_pool = parent->conn_pool;
            conn_queue = &parent->conn_queue;
        }
        void work(int time_out=0)
        {
            int ret,cur_fd;
            T* t = nullptr;
            while(run)
            {
                for(auto it = connects.begin();it != connects.end();)
                {
                    if(it->second->status == conn::CLOSE)
                    {
                        CONNECTS_REMOVE_FD_REACTOR_OUT;
                    }
                    else
                    {
                        it++;
                    }
                }
                if((t = conn_queue->pop()) != nullptr)
                {
                    t->init(epoll_fd,ET,one_shot);
                    if(connects[t->fd()] != nullptr)
                    {
                        LOG(Pointer_To_Null,"already exist");
                    }
                    connects[t->fd()] = t;
                }
                if((ret = epoll_wait(epoll_fd,events,max_events_count,time_out)) == 0)
                {
                    continue;
                }
                else if(ret < 0)
                {
                    LOG(Epoll_Wait,"epoll wait error");
                    break;
                }
                else
                {
                    for(int event_index = 0;event_index < ret; event_index++)
                    {
                        cur_fd = events[event_index].data.fd;
                        if(events[event_index].events & EPOLLRDHUP)
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
                                CONNECTS_REMOVE_FD_REACTOR;
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
                                CONNECTS_REMOVE_FD_REACTOR;
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
                                    CONNECTS_REMOVE_FD_REACTOR;
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
                                    CONNECTS_REMOVE_FD_REACTOR;
                                }
                            }
                        }
                        else
                        {
                            LOG_FMT(None,"unknown error","client IP=%s client Port = %u",
                                    inet_ntoa(connects[cur_fd]->addr().sin_addr),ntohs(connects[cur_fd]->addr().sin_port));
                            CONNECTS_REMOVE_FD_REACTOR;
                        }
                    }
                }
            }
            close();
        }
    };
    template<class T>
    bool reactor<T>::run = false;
};

#endif
