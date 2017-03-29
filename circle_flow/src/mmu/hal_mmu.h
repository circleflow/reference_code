
#ifndef MMU_HAL_H_
#define MMU_HAL_H_

#include "hal.h"
#include "type_ext.h"

namespace CIRCLE_FLOW {
    typedef struct  {

        UINT32 byte_per_cell;
        UINT32 total_cell;
        UINT32 total_pkt;

    } HAL_MMU_SPECS;

    const HAL_MMU_SPECS &hal_mmu_specs(UINT8 unit=DEFAULT_UNIT);

    void hal_mmu_ingress_set(UINT8 unit, UINT8 port, UINT32 max_cell, UINT32 max_pkt);

    void hal_mmu_egress_set (UINT8 unit, UINT8 port, UINT32 max_cell, UINT32 max_pkt);

    void hal_mmu_egress_q_set(UINT8 unit, UINT8 port, UINT8 queue,
                          UINT32 min_cell, UINT32 min_pkt,
                          UINT32 max_cell, UINT32 max_pkt);

    void hal_mmu_share_set(UINT8 unit,
                       UINT32 ing_max_cell, UINT32 ing_limit_pkt,
                       UINT32 egr_max_cell, UINT32 egr_max_pkt);

}

#endif /* MMU_HAL_H_ */
