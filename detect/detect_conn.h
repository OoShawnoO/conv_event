#ifndef USE_CONV_DETECT_CONN_H
#define USE_CONV_DETECT_CONN_H

#include "core/include/conn.h"  /* conn */
#include "core/conv_multi.h"    /* conv_multi */
#include "core/conv_single.h"   /* conv_single */
#include "detector.h"           /* detector */
#include <algorithm>            /* find */

namespace hzd
{
    class detect_conn : public conn
    {
        static std::unordered_map<std::string,detector> detectors;
        json _data;

    public:
        bool process_in() override
        {
            std::string data;
            if(!recv_with_header(data))
            {
                return false;
            }
            _data.load(data);
            if(_data.has_key("type"))
            {
                if(!recv_file("tmp.jpg",(size_t)(int)_data["size"]))
                {
                    return false;
                }
                next(EPOLLOUT);
                return true;
            }
            return false;
        }

        bool process_out() override
        {

            if(detectors.find(_data["type"]) != detectors.end())
            {
                auto res = detectors.at(_data["type"]).predict("tmp.jpg");
                _data.clear();
                _data[res.first] = res.second;
                return send_with_header(_data.dump());
            }
            return false;
        }
    };

    std::unordered_map<std::string,detector> detect_conn::detectors{
            {"flower",detector{"../model/flower"}},
            {"culture_relic",detector{"../model/culture_relic"}},
            {"face_express",detector{"../model/face_express"}}
    };
}

#endif
