#ifndef USE_CONV_CONV_CLIENT_H
#define USE_CONV_CONV_CLIENT_H

#include "utils.h"
#include "socket_io.h"
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>

#define ERROR(method) do{ \
    perror(method);       \
    close();              \
    exit(-1);             \
}while(0)

namespace hzd
{
    class conv_client : public socket_io{
        /* private methods */

        /* protected variable */
    protected:
        sockaddr_in dest_addr{};
    public:
        /* Constructor */
        conv_client(const std::string& IP,short PORT)
        {
            socket_fd = socket(AF_INET,SOCK_STREAM,0);
            if(socket_fd < 0)
            {
                ERROR("socket");
            }
            dest_addr.sin_addr.s_addr = inet_addr(IP.c_str());
            dest_addr.sin_port = htons(PORT);
            dest_addr.sin_family = AF_INET;
        }
        /* virtual Destructor */
        virtual ~conv_client()
        {
            close();
        }
        /* common public methods */
        /**
        * @brief connect server
        * @note None
        * @param None
        * @retval None
        */
        void connect() {
            if (::connect(socket_fd, (sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
                ERROR("connect");
            }
        }
        /**
        * @brief close socket
        * @note None
        * @param None
        * @retval None
        */
        void close() {
            if (socket_fd != -1)
            {
                ::close(socket_fd);
            }
        }
    };
}

#endif
