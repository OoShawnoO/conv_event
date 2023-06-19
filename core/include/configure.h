#ifndef CONV_EVENT_CONFIGURE_H
#define CONV_EVENT_CONFIGURE_H

#include "json/json.h"

namespace hzd {

    class configure {
        configure() {
            if(!configs.load_by_file_name("conv_event/conf/conf.json"))
            {
                exit(-1);
            }
        }
        json configs;
    public:

        configure(const configure& ) = delete;
        configure& operator=(const configure&) = delete;

        json_val& require(const std::string & key)
        {
            if(configs.has_key(key))
                return configs[key];
            else
                throw std::exception();
        }

        json_val operator[](const std::string & key)
        {
            if(configs.has_key(key))
            {
                return configs[key];
            }
            else
            {
                return nullptr;
            }
        }

        static configure& get_config()
        {
            static configure conf;
            return conf;
        }
    };

}

#endif
