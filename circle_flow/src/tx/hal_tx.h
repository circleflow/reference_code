
#ifndef TX_HAL_H_
#define TX_HAL_H_

#include "type_ext.h"

namespace CIRCLE_FLOW {
    void hal_init_front_port(UINT8 unit, UINT8 port);
    void hal_init_engine_port(UINT8 unit, UINT8 port);
    void hal_init_latency_port(UINT8 unit, UINT8 port);
}

#endif /* TX_HAL_H_ */
