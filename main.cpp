#include <iostream>
#include "conv.h"

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
using namespace hzd;
using namespace std;
int main()
{
    conv<conn_a> base("127.0.0.1",9999);
    base.enable_multi_thread();
    base.enable_heart_beat();
    base.enable_addr_reuse();
    base.enable_port_reuse();
    base.wait();
}