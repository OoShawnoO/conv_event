#ifndef CONV_EVENT_SOCKET_IO_H
#define CONV_EVENT_SOCKET_IO_H

#include <cstddef>                          /* def */
#include <cstring>                          /* c string */
#include <sys/socket.h>                     /* socket */
#include <fcntl.h>                          /* fcntl */
#include <sys/stat.h>                       /* fstat */
#include <sys/sendfile.h>                   /* sendfile */
#include <csignal>                          /* SIGNAL */
#include "utils.h"                          /* utils packet */
#include <unistd.h>                         /* close */
#include "async_logger/async_logger.hpp"    /* async_logger */

namespace hzd {

    class socket_io {
        /**
          * @brief base of send data
          * @note None
          * @param data data
          * @retval success or not
          */
        bool send_base(const char *data) {
            size_t send_count;
            size_t need_to_send;

            while (write_cursor < write_total_bytes) {
                bzero(write_buffer, sizeof(write_buffer));
                need_to_send = write_total_bytes - write_cursor;
                memcpy(write_buffer, data + write_cursor,
                       need_to_send > sizeof(write_buffer) ? sizeof(write_buffer) : need_to_send);
                if ((send_count = ::send(socket_fd, write_buffer,
                                         need_to_send > sizeof(write_buffer) ? sizeof(write_buffer) : need_to_send,
                                         MSG_NOSIGNAL)) <= 0) {

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
        bool recv_base(std::string &data) {
            if (read_total_bytes <= 0) {
                return false;
            }
            ssize_t read_count;
            while (read_cursor < read_total_bytes) {
                bzero(read_buffer, sizeof(read_buffer));
                read_count = (read_total_bytes - read_cursor) > sizeof(read_buffer) ? sizeof(read_buffer) : (read_total_bytes - read_cursor);
                if ((read_count = ::recv(socket_fd, read_buffer, read_count, 0)) <= 0) {
                    if (read_count == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            return true;
                        }
                        return false;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            return true;
                        }
                        return false;
                    }
                }
                data += std::string(read_buffer, read_count);
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
        bool already{true};
        /**
         * @brief send data by using hzd::header
         * @note None
         * @param data data
         * @param type data-type
         * @retval None
         */
    public:
        bool send_with_header(const char *data) {
            if (already) {
                write_total_bytes = strlen(data) + 1;
                write_cursor = 0;
                if (write_total_bytes <= 1) {
                    return false;
                }
                header h{write_total_bytes};
                if (::send(socket_fd, &h, HEADER_SIZE, 0) <= 0) {
                    return false;
                }
            }
            already = send_base(data);
            return already;
        }

        /**
          * @brief send data by using hzd::header
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
        bool send_with_header(std::string &data) {
            if (already) {
                write_total_bytes = data.size() + 1;
                write_cursor = 0;
                if (write_total_bytes <= 1) {
                    return false;
                }
                header h{write_total_bytes};
                if (::send(socket_fd, &h, HEADER_SIZE, 0) <= 0) {
                    return false;
                }
            }
            already = send_base(data.c_str());
            return already;
        }

        bool send_with_header(std::string &&data) {
            if (already) {
                write_total_bytes = data.size() + 1;
                write_cursor = 0;
                if (write_total_bytes <= 1) {
                    return false;
                }
                header h{write_total_bytes};
                if (::send(socket_fd, &h, HEADER_SIZE, 0) <= 0) {
                    return false;
                }
            }
            already = send_base(data.c_str());
            return already;
        }

        /**
          * @brief send data by given length of data
          * @note None
          * @param data data
          * @param size data size
          * @retval None
          */
        bool send(std::string &data, size_t size) {
            if (already) {
                write_total_bytes = size;
                write_cursor = 0;
                if (write_total_bytes <= 0) {
                    return false;
                }
            }
            already = send_base(data.c_str());
            return already;
        }

        bool send(std::string &&data, size_t size) {
            if (already) {
                write_total_bytes = size;
                write_cursor = 0;
                if (write_total_bytes <= 0) {
                    return false;
                }
            }
            already = send_base(data.c_str());
            return already;
        }

        bool send(const char *data, size_t size) {
            if (already) {
                write_total_bytes = size;
                write_cursor = 0;
                if (write_total_bytes <= 0) {
                    return false;
                }
            }
            already = send_base(data);
            return already;
        }

        bool send_file(const std::string &filename) {
            int file_fd = open(filename.c_str(), O_RDONLY);
            struct stat st{};
            fstat(file_fd, &st);
            write_cursor = 0;
            write_total_bytes = st.st_size;
            while (write_cursor < write_total_bytes) {
                auto offset = (off_t) write_cursor;
                size_t send_count = sendfile(socket_fd, file_fd, &offset, write_total_bytes - write_cursor);
                if (send_count == -1) {
                    if (errno == EAGAIN) {
                        return false;
                    }
                    close(file_fd);
                    file_fd = -1;
                    return false;
                }
                write_cursor += send_count;
            }
            close(file_fd);
            file_fd = -1;
            return true;
        }

        /**
          * @brief recv data and output type
          * @note None
          * @param data data
          * @param type data-type
          * @retval None
          */
        bool recv_with_header(std::string &data) {
            if (already) {
                header h{};
                if (::recv(socket_fd, &h, HEADER_SIZE, 0) <= 0) {
                    return false;
                }
                read_total_bytes = h.size;
                read_cursor = 0;
            }
            already = recv_base(data);
            return already;
        }

        /**
          * @brief recv data by given size
          * @note None
          * @param data data
          * @param size data-size
          * @retval None
          */
        bool recv(std::string &data, size_t size) {
            if (already) {
                read_total_bytes = size;
                read_cursor = 0;
            }
            already = recv_base(data);
            return already;
        }

        /**
        * @brief recv all data from system buffer
        * @note None
        * @param None
        * @retval None
        */
        bool recv_all(std::string &data) {
            if (already) {
                read_total_bytes = SIZE_MAX;
                read_cursor = 0;
            }
            already = recv_base(data);
            return already;
        }

        bool recv_file(const std::string &download_path, size_t size) {
            read_cursor = 0;
            read_total_bytes = size;
            size_t read_count;
            FILE *fp = fopen(download_path.c_str(), "wb");
            while (read_cursor < read_total_bytes) {
                bzero(read_buffer, sizeof(read_buffer));
                if ((read_count = ::recv(socket_fd, read_buffer,
                                         (read_total_bytes - read_cursor) > sizeof(read_buffer) ? sizeof(read_buffer)
                                                                                                : (read_total_bytes -
                                                                                                   read_cursor), 0)) <=
                    0) {
                    if (read_count == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            return true;
                        }
                        return false;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            return true;
                        }
                        return false;
                    }
                }
                fwrite(read_buffer, read_count, 1, fp);
                read_cursor += read_count;
            }
            fclose(fp);
            return true;
        }
    };
}

#endif
