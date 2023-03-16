#include "conv.h"
#include <iostream>
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

int main()
{
    hzd::conv<conn_a> base("127.0.0.1",9999);
    base.enable_multi_thread();
    base.wait();
    return 0;
}