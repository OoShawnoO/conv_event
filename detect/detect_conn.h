#ifndef USE_CONV_DETECT_CONN_H
#define USE_CONV_DETECT_CONN_H

#include "core/include/conn.h"  /* conn */
#include "core/conv_multi.h"    /* conv_multi */
#include "core/conv_single.h"   /* conv_single */
#include "detector.h"           /* detector */

namespace hzd
{
    class detect_conn : public conn
    {
        static detector flower_detector;
        static detector culture_relic_detector;

        json _data;

    public:
        bool process_in() override
        {
            std::string data;
            if(!recv_with_header(data))
            {
                notify_close();
                return false;
            }
            _data.load(data);
            if(_data.has_key("type"))
            {
                if(!recv_file("tmp.jpg",(size_t)(int)_data["size"]))
                {
                    notify_close();
                    return false;
                }
                next(EPOLLOUT);
                return true;
            }
            notify_close();
            return false;
        }

        bool process_out() override
        {
            if(_data["type"] == "flower")
            {
                auto res = flower_detector.predict("tmp.jpg");
                _data.clear();
                _data[res.first] = res.second;
                return send_with_header(_data.dump());
            }
            else if(_data["type"] == "culture_relic")
            {
                auto res = culture_relic_detector.predict("tmp.jpg");
                _data.clear();
                _data[res.first] = res.second;
                return send_with_header(_data.dump());
            }
            return false;
        }
    };

    detector detect_conn::flower_detector("../model/flower");
    detector detect_conn::culture_relic_detector("../model/culture_relic");

}

#endif
