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
    public:
        json configs;

        configure(const configure& ) = delete;
        configure& operator=(const configure&) = delete;

        static configure& get_config()
        {
            static configure conf;
            return conf;
        }
    };

}

#endif
