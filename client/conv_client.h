#ifndef USE_CONV_CONV_CLIENT_H
#define USE_CONV_CONV_CLIENT_H

#include "utils.h"
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
    class conv_client {
        /* private methods */
        /**
          * @brief base of send data
          * @note None
          * @param data data
          * @retval success or not
          */
        bool send_base(const char* data)
        {
            size_t send_count;
            write_cursor = 0;
            while(write_cursor < write_total_bytes)
            {
                bzero(write_buffer,sizeof(write_buffer));
                memcpy(write_buffer,data+write_cursor,sizeof(write_buffer));
                if((send_count = ::send(socket_fd,write_buffer,sizeof(write_buffer),0))<= 0)
                {
                    perror("send");
                    return false;
                }
                write_cursor += send_count;
            }
            write_cursor = 0;
            write_total_bytes = 0;
            return true;
        }
        /**
          * @brief base of recv data
          * @note None
          * @param data data save in this string
          * @retval success or not
          */
        bool recv_base(std::string& data)
        {
            if(read_total_bytes <= 0)
            {
                return false;
            }
            size_t read_count;
            read_cursor = 0;
            data.clear();
            while(read_cursor < read_total_bytes)
            {
                bzero(read_buffer,sizeof(read_buffer));
                if((read_count = ::recv(socket_fd,read_buffer,sizeof(read_buffer),0))<=0)
                {
                    perror("recv");
                    return false;
                }
                data += read_buffer;
                read_cursor += read_count;
            }
            read_cursor = 0;
            read_total_bytes = 0;
            return true;
        }
        /* protected variable */
    protected:
        int socket_fd{-1};
        sockaddr_in dest_addr;
        char write_buffer[1024];
        char read_buffer[1024];
        size_t read_cursor{0};
        size_t write_cursor{0};
        size_t write_total_bytes{0};
        size_t read_total_bytes{0};
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
        void connect()
        {
            if(::connect(socket_fd,(sockaddr*)&dest_addr,sizeof(dest_addr)) < 0)
            {
                ERROR("connect");
            }
//            int option = fcntl(socket_fd,F_GETFL);
//            int new_option = option | O_NONBLOCK;
//            fcntl(socket_fd,F_SETFL,new_option);
        }
        /**
          * @brief send data by using hzd::header
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
        bool send_with_header(const char* data)
        {
            write_total_bytes = strlen(data) + 1;
            if(write_total_bytes <= 1)
            {
                return false;
            }
            header h{write_total_bytes};
            if(::send(socket_fd,&h,HEADER_SIZE,0) <= 0)
            {
                perror("send");
                return false;
            }
            return send_base(data);
        }
        /**
          * @brief send data by using hzd::header
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
        bool send_with_header(std::string& data)
        {
            write_total_bytes = data.size()+1;
            if(write_total_bytes <= 1)
            {
                return false;
            }
            header h{write_total_bytes};
            if(::send(socket_fd,&h,HEADER_SIZE,0) <= 0)
            {
                perror("send");
                return false;
            }
            return send_base(data.c_str());
        }
        /**
          * @brief send data by given length of data
          * @note None
          * @param data data
          * @param size data size
          * @retval None
          */
        bool send(std::string& data,size_t size)
        {
            write_total_bytes = size;
            if(write_total_bytes <= 0)
            {
                return false;
            }
            return send_base(data.c_str());
        }
        /**
          * @brief recv data and output type
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
        bool recv_with_header(std::string& data)
        {
            header h{};
            if(::recv(socket_fd,&h,HEADER_SIZE,0) <= 0)
            {
                perror("recv");
                return false;
            }
            read_total_bytes = h.size;
            return recv_base(data);
        }
        /**
          * @brief recv data by given size
          * @note None
          * @param data data
          * @param size data-size
          * @retval None
          */
        bool recv(std::string& data,size_t size)
        {
            read_total_bytes = size;
            return recv_base(data);
        }
        /**
        * @brief recv all data from system buffer
        * @note None
        * @param None
        * @retval None
        */
        bool recv_all(std::string& data)
        {
            read_total_bytes = SIZE_MAX;
            return recv_base(data);
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
