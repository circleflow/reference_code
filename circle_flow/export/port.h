
#ifndef CIRCLE_FLOW_PORT_H_
#define CIRCLE_FLOW_PORT_H_

#include "type.h"
#include "port_type.h"

namespace CIRCLE_FLOW {

    namespace PORT{

        PORT_NAME_SET get_port_all();

        ITF_SET   get_itf_set(const PORT_NAME &);
        void      set_itf(const PORT_NAME &, INTERFACE );
        INTERFACE get_itf(const PORT_NAME &);

        void set_auto_nego(const PORT_NAME &, const ADVERT &);    //half duplex not supported
        void set_forced(const PORT_NAME &, const MODE &);    //half duplex not supported

        ABILITY get_ability(const PORT_NAME &);
        STATUS  get_status(const PORT_NAME &);

        TRX_CNT get_cnt (const PORT_NAME &, bool is_reset=false);
        TRX_CNT get_rate(const PORT_NAME &);
    }

}

#endif /* PORT_H_ */
