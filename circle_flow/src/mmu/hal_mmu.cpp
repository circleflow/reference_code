
#include "hal_mmu.h"
#include "defs.h"
#include "sdk.h"

extern "C" {
#include "soc/drv.h"
}

const HAL_MMU_SPECS & CIRCLE_FLOW::hal_mmu_specs(UINT8 unit)
{
    static HAL_MMU_SPECS specs;
    static bool initialized = false;

    if(false == initialized) {

#if defined(CF_BCM_56334)

        specs.byte_per_cell = 128;
        specs.total_cell    = 16*1024-1; //fit the size of Q_SHARED_LIMIT_CELL
        specs.total_pkt     = 1 * 1024;

        initialized = true;

#else
        "chip type not suppoted yet, pls complete it here."
#endif

    }

    return specs;
}


/* ingress drop threshold
 * no SDK API available, refer _mmu_init for implementation*/
void CIRCLE_FLOW::hal_mmu_ingress_set(UINT8 unit, UINT8 port, UINT32 max_cell, UINT32 max_pkt)
{
#if defined(CF_BCM_56334)

    uint32 rval = 0;
    soc_reg_field_set(unit, PORT_SHARED_LIMIT_CELLr, &rval,
                      PORT_SHARED_LIMITf, max_cell);
    soc_reg_field_set(unit, PORT_SHARED_LIMIT_CELLr, &rval,
                      PORT_SHARED_DYNAMICf, 0);
    ENSURE_OK(WRITE_PORT_SHARED_LIMIT_CELLr(unit, port, rval));

    rval = 0;
    soc_reg_field_set(unit, PORT_SHARED_LIMIT_PACKETr, &rval,
                      PORT_SHARED_LIMITf, max_pkt);
    soc_reg_field_set(unit, PORT_SHARED_LIMIT_PACKETr, &rval,
                      PORT_SHARED_DYNAMICf, 0);
    ENSURE_OK(WRITE_PORT_SHARED_LIMIT_PACKETr(unit, port, rval));

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}

/* egress port drop threshold
 * no SDK API available, refer _mmu_init for implementation */
void CIRCLE_FLOW::hal_mmu_egress_set (UINT8 unit, UINT8 port, UINT32 max_cell, UINT32 max_pkt)
{
#if defined(CF_BCM_56334)

    uint32 rval = 0;
    soc_reg_field_set(unit, OP_PORT_CONFIG_CELLr, &rval,
                      OP_SHARED_LIMIT_CELLf, max_cell);
    soc_reg_field_set(unit, OP_PORT_CONFIG_CELLr, &rval,
                      OP_SHARED_RESET_VALUE_CELLf, (max_cell / 2));
    soc_reg_field_set(unit, OP_PORT_CONFIG_CELLr, &rval,
                      PORT_LIMIT_ENABLE_CELLf, 1);

    ENSURE_OK(WRITE_OP_PORT_CONFIG_CELLr(unit, port, rval));

    rval = 0;
    soc_reg_field_set(unit, OP_PORT_CONFIG_PACKETr, &rval,
                      OP_SHARED_LIMIT_PACKETf, max_pkt);
    soc_reg_field_set(unit, OP_PORT_CONFIG_PACKETr, &rval,
                      OP_SHARED_RESET_VALUE_PACKETf, (max_pkt / 2));
    soc_reg_field_set(unit, OP_PORT_CONFIG_PACKETr, &rval,
                      PORT_LIMIT_ENABLE_PACKETf, 1);
    ENSURE_OK(WRITE_OP_PORT_CONFIG_PACKETr(unit, port, rval));

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}

/* egress queue drop threshold
 * no SDK API available, refer _mmu_init for implementation */
void CIRCLE_FLOW::hal_mmu_egress_q_set(UINT8 unit, UINT8 port, UINT8 queue,
                                       UINT32 min_cell, UINT32 min_pkt,
                                       UINT32 max_cell, UINT32 max_pkt)
{
#if defined(CF_BCM_56334)

    uint32 rval = 0;
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_CELLr, &rval,
                      Q_MIN_CELLf, min_cell);
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_CELLr, &rval,
                      Q_LIMIT_ENABLE_CELLf, 0x1);
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_CELLr, &rval,
                      Q_LIMIT_DYNAMIC_CELLf, 0x0);
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_CELLr, &rval,
                      Q_SHARED_LIMIT_CELLf, max_cell);
    ENSURE_OK(
        WRITE_OP_QUEUE_CONFIG_CELLr(unit, port, queue, rval));

    rval = 0;
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_PACKETr, &rval,
                      Q_MIN_PACKETf, min_pkt);
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_PACKETr, &rval,
                      Q_LIMIT_ENABLE_PACKETf, 0x1);
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_PACKETr, &rval,
                      Q_LIMIT_DYNAMIC_PACKETf, 0x0);
    soc_reg_field_set(unit, OP_QUEUE_CONFIG_PACKETr, &rval,
                      Q_SHARED_LIMIT_PACKETf, max_pkt);

    ENSURE_OK(
        WRITE_OP_QUEUE_CONFIG_PACKETr(unit, port, queue, rval));

#else
    "chip type not suppoted yet, pls complete it here."
#endif
}

/* global drop threshold
 * no SDK API available, refer _mmu_init for implementation */
void CIRCLE_FLOW::hal_mmu_share_set(UINT8 unit,
                                    UINT32 ing_max_cell, UINT32 ing_limit_pkt,
                                    UINT32 egr_max_cell, UINT32 egr_max_pkt)
{
#if defined(CF_BCM_56334)

    uint32 rval = 0;
    soc_reg_field_set(unit, TOTAL_SHARED_LIMIT_CELLr, &rval,
                      TOTAL_SHARED_LIMITf, ing_max_cell);
    ENSURE_OK(WRITE_TOTAL_SHARED_LIMIT_CELLr(unit, rval));

    rval = 0;
    soc_reg_field_set(unit, TOTAL_SHARED_LIMIT_PACKETr, &rval,
                      TOTAL_SHARED_LIMITf, ing_limit_pkt);
    ENSURE_OK(WRITE_TOTAL_SHARED_LIMIT_PACKETr(unit, rval));

    rval = 0;
    soc_reg_field_set(unit, OP_BUFFER_SHARED_LIMIT_CELLr, &rval,
                      OP_BUFFER_SHARED_LIMIT_CELLf, egr_max_cell);
    ENSURE_OK(WRITE_OP_BUFFER_SHARED_LIMIT_CELLr(unit, rval));

    rval = 0;
    soc_reg_field_set(unit, OP_BUFFER_SHARED_LIMIT_PACKETr, &rval,
                      OP_BUFFER_SHARED_LIMIT_PACKETf, egr_max_pkt);
    ENSURE_OK(WRITE_OP_BUFFER_SHARED_LIMIT_PACKETr(unit, rval));

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}
