
# <font face="Agave Nerd Font">conv_event
### Full name:convenient epoll event

> Usage
> 
>[clone]:git clone --recursive https://github.com/OoShawnoO/conv_event.git

- Derived from class hzd::conn
- Methods that must be inherited
    ```c++
    bool process_in(); /* process the EPOLLIN */
    bool process_out(); /* process the EPOLLOUT */
  
  /* template */
  class conn_a : public hzd::conn
  {
  public:
    bool process_in() override
    {
        read(socket_fd,read_buffer,sizeof(read_buffer));
        std::cout << read_buffer<<std::endl;
        epoll_mod(epoll_fd,socket_fd,EPOLLOUT,false);
        return true;
    }
    bool process_out() override
    {
        sprintf(write_buffer,"i am server");
        write(socket_fd, write_buffer,sizeof(write_buffer));
        return true;
    }
  
  };
    ```
- Methods that optional be inherited
    ```c++
    bool process_rdhup(); /* process the EPOLLRDHUP */
    boll process_error(); /* process the EPOLLERR */
    ```
- Class hzd::conn have such protected member variable
    ```c++
    int socket_fd{-1};
    sockaddr_in sock_addr{};
    char read_buffer[4096] = {0};
    char write_buffer[4096] = {0};
    int read_cursor{0};
    int write_cursor{0};
    int write_total_bytes{0};
    ```
- Use multi-thread
    ```c++
    hzd::conv<conn_a> base;
    base.multi_thread(); /* 创建线程池 */
    base.wait();
    ```