
#ifndef PORT_HAL_H_
#define PORT_HAL_H_

#include "port.h"
using namespace CIRCLE_FLOW::PORT;

#include "type_ext.h"

namespace CIRCLE_FLOW {
    void hal_port_auto_nego(UINT8 unit, UINT8 port, const ADVERT &);

    void hal_port_forced(UINT8 unit, UINT8 port, const MODE &);

    ABILITY hal_port_ability_get(UINT8 unit, UINT8 port);

    STATUS hal_port_status_get(UINT8 unit, UINT8 port);

    typedef void (*HAL_LINK_CB)(UINT8 unit, UINT8 port, bool link);
    void hal_port_link_callback_register(HAL_LINK_CB cb);

    TRX_CNT hal_port_cnt_get   (UINT8 unit, UINT8 port);
    void    hal_port_cnt_clear (UINT8 unit, UINT8 port);

    void hal_port_max_frame_size_set(UINT32 max_frame_size);
}

#endif /* PORT_HAL_H_ */
