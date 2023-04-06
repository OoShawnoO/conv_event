#ifndef USE_CONV_SOCKET_IO_H
#define USE_CONV_SOCKET_IO_H

#include <cstddef>
#include <cstring>
#include <sys/socket.h>
#include "ErrorLog/ErrorLog.h"
#include "utils.h"

namespace hzd {

    class socket_io {
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
                if((send_count = ::send(socket_fd,write_buffer,
                                        (write_total_bytes-write_cursor) > sizeof(write_buffer) ? sizeof(write_buffer) : (write_total_bytes-write_cursor)
                                        ,MSG_NOSIGNAL))<= 0)
                {
                    LOG(Conn_Send,"data send error");
                    return false;
                }
                write_cursor += send_count;
            }
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
                LOG(Conn_Recv,"recv size = 0");
                return false;
            }
            size_t read_count;
            read_cursor = 0;
            data.clear();
            while(read_cursor < read_total_bytes)
            {
                bzero(read_buffer,sizeof(read_buffer));
                if((read_count = ::recv(socket_fd,read_buffer,
                                        (read_total_bytes-read_cursor) > sizeof(read_buffer) ? sizeof(read_buffer) : (read_total_bytes-read_cursor)
                                        ,0))<=0)
                {
                    if(read_count == -1)
                    {
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            return true;
                        }
                        LOG(Conn_Recv,"data recv error");
                        return false;
                    }
                    else
                    {
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            return true;
                        }
                        LOG(Conn_Recv,"client close");
                        return false;
                    }
                }
                data += read_buffer;
                read_cursor += read_count;
            }
            return true;
        }
    protected:
        char read_buffer[2048] = {0};
        char write_buffer[2048] = {0};
        size_t read_cursor{0};
        size_t write_cursor{0};
        size_t write_total_bytes{0};
        size_t read_total_bytes{0};
        int socket_fd{-1};

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
                LOG(Conn_Send,"send data = null");
                return false;
            }
            header h{write_total_bytes};
            if(::send(socket_fd,&h,HEADER_SIZE,0) <= 0)
            {
                LOG(Conn_Send,"header send error");
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
                LOG(Conn_Send,"send data = null");
                return false;
            }
            header h{write_total_bytes};
            if(::send(socket_fd,&h,HEADER_SIZE,0) <= 0)
            {
                LOG(Conn_Send,"header send error");
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
                LOG(Conn_Send,"send data = null");
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
                LOG(Conn_Recv,"recv header error");
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
    };

}

#endif
