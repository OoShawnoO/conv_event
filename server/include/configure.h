#include "json/json.h"

#ifndef USE_CONV_CONFIGURE_H
#define USE_CONV_CONFIGURE_H

namespace hzd {

    class configure {
        configure() {
            if(!configs.load_by_file_name("conv_event/server/conf/conf.json"))
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
