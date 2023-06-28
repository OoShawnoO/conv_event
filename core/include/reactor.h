#ifndef CONV_EVENT_REACTOR_H
#define CONV_EVENT_REACTOR_H

#include "conn.h"           /* conn */
#include "threadpool.h"     /* thread pool */
#include "configure.h"      /* configure */

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
            delete close_queue;
            close_queue = nullptr;
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
                close();
                exit(-1);
            }
            if(epoll_fd == -1)
                epoll_fd = epoll_create(1024);
            if(epoll_fd < 0)
            {
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
        int max_events_count{4096};

        threadpool<T>* thread_pool{nullptr};
        connpool<T>* conn_pool{nullptr};
        conv_multi<T>* parent{nullptr};
        lock_queue<int>* close_queue{nullptr};
#define CONNECTS_REMOVE_FD_REACTOR do                   \
        {                                               \
            parent->current_connect_count--;            \
            T* tmp = connects[cur_fd];                  \
            connects[cur_fd] = nullptr;                 \
            tmp->close();                               \
            if(conn_pool)                               \
            {                                           \
                conn_pool->release(tmp);                \
            }                                           \
            else                                        \
            {                                           \
                delete tmp;                             \
            }                                           \
        }while(0)

#define CONNECTS_REMOVE_FD_REACTOR_OUT do{              \
            parent->current_connect_count--;            \
            T* tmp = it->second;                        \
            it->second = nullptr;                       \
            it++;                                       \
            tmp->close();                               \
            if(conn_pool)                               \
            {                                           \
                conn_pool->release(tmp);                \
            }                                           \
            else                                        \
            {                                           \
                delete tmp;                             \
            }                                           \
        }while(0)

    public:
        std::unordered_map<int,T*> connects;
        static void work(void* r,int time_out)
        {
            auto* reac = (reactor<T>*)r;
            reac->work(time_out);
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
            configure& conf = configure::get_config();
            max_events_count = conf["max_events_count"].type == JSON_NULL ? 4096 : (int32_t)max_events_count;


            parent = _parent;
            ET = parent->ET;
            one_shot = parent->one_shot;
            if(parent->_thread_pool)
            {
                if(!thread_pool)
                    thread_pool = new threadpool<T>(conf["thread_count"].type == JSON_NULL ? 8 : (int32_t)conf["thread_count"]);
            }
            close_queue = new lock_queue<int>();
            conn_pool = parent->conn_pool;

            LOG_INFO("reactor init success");
        }
        void add_conn(int fd)
        {
            epoll_add(epoll_fd,fd,ET,one_shot);
        }
        void work(int time_out=1)
        {
            int ret,cur_fd;
            T* t;
            while(run)
            {
                while(!close_queue->empty())
                {
                    close_queue->pop(cur_fd);
                    if(cur_fd == -1) continue;
                    if(connects[cur_fd] == nullptr) continue;
                    CONNECTS_REMOVE_FD_REACTOR;
                }

                if((ret = epoll_wait(epoll_fd,events,max_events_count,time_out)) == 0)
                {
                    continue;
                }
                else if(ret < 0)
                {
                    if(errno == EINTR) { continue; }
                    break;
                }
                else
                {
                    for(int event_index = 0;event_index < ret; event_index++)
                    {
                        cur_fd = events[event_index].data.fd;

                        if(connects[cur_fd] == nullptr)
                        {
                            if(conn_pool)
                            {
                                t = conn_pool->acquire();
                            }
                            else
                            {
                                t = new T;
                            }
                            if(!t)
                            {
                                ::close(cur_fd);
                                continue;
                            }
                            sockaddr_in addr{};
                            socklen_t len;
                            getsockname(cur_fd,(sockaddr*)&addr,&len);
                            t->init(cur_fd,&addr,epoll_fd,ET,one_shot,close_queue,false);
                            parent->current_connect_count++;
                            connects[cur_fd] = t;
                        }

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
                                    CONNECTS_REMOVE_FD_REACTOR;
                                }
                            }
                        }
                        else
                        {
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
