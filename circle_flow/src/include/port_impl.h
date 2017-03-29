
#ifndef PORT_EXT_H_
#define PORT_EXT_H_

#include "type_ext.h"

#include "port_type.h"
using namespace CIRCLE_FLOW::PORT;

#include "device.h"

#include "utility/export/function_bind.h"
#include "utility/export/smart_ptr_u.h"

namespace CIRCLE_FLOW {

    /* implementation class of PORT,
     * more func, without mutex, for internal only */

    namespace PORT_IMPL {

        PORT_NAME_SET get_port_all(void);

        PORT_ID get_front (const PORT_NAME &);
        PORT_ID get_engine(const PORT_NAME &);

        struct PAIR {
            PORT_ID engine;
            PORT_ID front;
            PAIR(const PORT_ID &_engine = PORT_ID(), const PORT_ID &_front=PORT_ID()) : engine(_engine), front(_front) {}
        };

        PAIR get_pair(const PORT_NAME &, INTERFACE);
        PAIR get_pair(const PORT_NAME &);

#define PORT_NAME_NULL PORT_NAME("")
        PORT_NAME get_port_name(const PORT_ID &);

        ITF_SET   get_itf_set(const PORT_NAME &);
        INTERFACE get_itf(const PORT_NAME &);
        void      set_itf(const PORT_NAME &, INTERFACE);

        bool is_valid(const PORT_NAME &);
        bool is_valid(const PORT_NAME_SET &);

        ABILITY get_ability(const PORT_ID &);
        STATUS  get_status (const PORT_ID &);

        void set_auto_nego(const PORT_ID &, const ADVERT &);
        void set_forced   (const PORT_ID &, const MODE &);

        TRX_CNT get_cnt  (const PORT_ID &);
        TRX_CNT get_rate (const PORT_NAME &);

        typedef function< void (const PORT_NAME &port, bool link) > LINK_CB;

        void link_notify_bind(const weak_ptr<LINK_CB> &cb,
                              const PORT_NAME_SET &port_names=get_port_all(),
                              bool notify_now=true);

        typedef function< void (const PORT_NAME &port_name, const PORT_ID &front, const PORT_ID &engine) > PAIR_CB;

        void pair_notify_bind(const weak_ptr<PAIR_CB> &cb,
                              const PORT_NAME_SET &port_names=get_port_all(),
                              bool notify_now=true);

        //helper
        PORT_NAME_SET make_ports(const PORT_NAME &port);
        PORT_NAME_SET make_ports(const PORT_NAME_SET &ports, const PORT_NAME &port);

        bool is_same_unit(const PORT_NAME_SET &ports);

        bool port_link(const PORT_ID &port);

    }
}

using namespace CIRCLE_FLOW::PORT_IMPL;

#endif /* PORT_EXT_H_ */
