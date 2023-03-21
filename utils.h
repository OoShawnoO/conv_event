#ifndef CONV_EVENT_UTILS_H
#define CONV_EVENT_UTILS_H

#include <cstdint> /* uint16_t uint64_t*/
namespace hzd
{
    enum class header_type : std::uint16_t
    {
        BYTE = 0,
        JSON,
    };
    #pragma pack(1)
    struct header   /* 10 byte */
    {
        header_type type; /* 2 byte */
        uint64_t size;    /* 8 byte */
    };
    #define HEADER_SIZE sizeof(header)
    #pragma pack()
}

#endif
