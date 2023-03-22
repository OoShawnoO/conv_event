
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
    bool process_error(); /* process the EPOLLERR */
    ```
- Use multi-thread
    ```c++
    hzd::conv<conn_a> base;
    base.multi_thread(); /* 创建线程池 */
    base.wait();
    ```
- Manual register event
    ```c++
    bool process_in/process_out()
   {
        next(EPOLLIN); /* register next event EPOLLIN */
        next(EPOLLOUT); /* register next event EPOLLOUT */
   }
    ```
- Edge Triggered
   ```c++
    /* it works well when you read or write all data form buffer,
     * but when you need read a part of buffer,it will not trigger
     * process_in or process_out until new data coming.*/
    conv.enable_et(); /* enable ET */
    conv.disable_et(); /* disable ET */
   ```
- One shot (Recommended)
   ```c++
    /* pay attention,because of using one_shot need manual register 
     * next event,and using system call epoll_ctl or my next() method.*/
    conv.enable_one_shot();  /* enable one_shot */
    conv.disable_one_shot(); /* disable one_shot */
   ```
- Send && Recv
   ```c++
    /* overload send and recv,you can use my send or recv
     * by this way*/
    header_type type;
    type = header_type::BYTE;
    send("message",type); /* automatic send sizeof your msg */
    send(string&,type); /* send by std::string */
    send(char*,size); /* send char* but need you give size*/
    
    recv(string&,header_type&); /* recv string& and type&*/
    recv(string&,size); /* recv such size msg*/
   ```