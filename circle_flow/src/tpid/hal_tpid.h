
#ifndef TPID_HAL_H_
#define TPID_HAL_H_

#include "type_ext.h"

namespace CIRCLE_FLOW {
    UINT8 hal_tpid_max_id();

    void hal_tpid_port_set(UINT8 unit, UINT8 port_id, UINT8 bmp);
    void hal_tpid_unit_set(UINT8 unit, UINT8 index, UINT16 tpid);

    void hal_tpid_egr_outer_set(UINT8 unit, UINT16 tpid);
    void hal_tpid_inner_set(UINT8 unit, UINT16 tpid);
}

#endif /* TPID_HAL_H_ */
