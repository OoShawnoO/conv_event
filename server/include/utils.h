#ifndef CONV_EVENT_UTILS_H
#define CONV_EVENT_UTILS_H

#include <cstdint> /* uint16_t uint64_t*/
namespace hzd
{
    #pragma pack(1)
    struct header   /* 8 byte */
    {
        uint64_t size;    /* 8 byte */
    };
    #define HEADER_SIZE sizeof(header)
    #pragma pack()
}

#endif
