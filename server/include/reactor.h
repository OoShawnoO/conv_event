#ifndef USE_CONV_REACTOR_H
#define USE_CONV_REACTOR_H

#include "conn.h"
#include "threadpool.h"

namespace hzd
{
    template <class T>
    class reactor {
        bool ET{false};
        bool one_shot{true};

        int epoll_fd{-1};
        epoll_event* events{nullptr};
        int max_events_count{1024};
        std::unordered_map<int,T*> connects;
        threadpool<T>* thread_pool{nullptr};
        connpool<T>* conn_pool{nullptr};

        safe_queue<T*>* conn_queue{nullptr};

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

#define CONNECTS_REMOVE_FD do                           \
        {                                               \
            connects[cur_fd]->close();                  \
            if(conn_pool)                               \
            {                                           \
                conn_pool->release(connects[cur_fd]);   \
            }                                           \
            else                                        \
            {                                           \
                delete connects[cur_fd];                \
                connects[cur_fd] = nullptr;             \
            }                                           \
            auto iter = connects.find(cur_fd);          \
            if(iter != connects.end())                  \
            {                                           \
                connects.erase(iter);                   \
            }                                           \
        }while(0)

        /**
  * @brief set max events count
  * @note None
  * @param size size
  * @retval None
  */
        void set_max_events_count(int size){if(size >= 0) max_events_count = size;}

        void init(bool _et,bool _one_shot,bool _thread_pool,connpool<T>* _conn_pool,safe_queue<T*>* _conn_queue)
        {
            ET = _et;
            one_shot = _one_shot;
            if(_thread_pool)
            {
                if(!thread_pool)
                    thread_pool = new threadpool<T>;
            }
            conn_pool = _conn_pool;
            conn_queue = _conn_queue;
        }

        void work(int time_out=100)
        {
            int ret,cur_fd;
            T* t = nullptr;
            while(run)
            {
                if((t = conn_queue->pop()) != nullptr)
                {
                    t->init(epoll_fd,ET,one_shot);
                    connects[t->fd()] = t;
                }
                if((ret = epoll_wait(epoll_fd,events,max_events_count,time_out)) < 0)
                {
                    LOG(Epoll_Wait,"epoll wait error");
                    break;
                }
                else if(ret == 0) continue;
                else
                {
                    for(int event_index = 0;event_index < ret; event_index++)
                    {
                        cur_fd = events[event_index].data.fd;
                        if(connects[cur_fd]->status == conn::CLOSE)
                        {
                            LOG_MSG("connect close");
                            CONNECTS_REMOVE_FD;
                        }
                        else if(events[event_index].events & EPOLLRDHUP)
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
                                CONNECTS_REMOVE_FD;
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
                                CONNECTS_REMOVE_FD;
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
                        else
                        {
                            LOG_FMT(None,"unknown error","client IP=%s client Port = %u",
                                    inet_ntoa(connects[cur_fd]->addr().sin_addr),ntohs(connects[cur_fd]->addr().sin_port));
                            CONNECTS_REMOVE_FD;
                        }
                    }
                }
            }
        }
    };
    template<class T>
    bool reactor<T>::run = false;
};

#endif