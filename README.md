
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