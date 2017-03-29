
#include "hal_tpid.h"
#include "hal.h"

#include "utility/export/error.h"

extern "C" {
#include "bcm/types.h"
#include "bcm/port.h"
#include "soc/drv.h"
#include "soc/mem.h"
}

/* SDK provided TPID API, but it has problem to meet our requirement
 * TPID is used for RX flow qualify, not for TX flow tagging
 * bcm_port_tpid_add both set ingress and egress, but we only need ingress
 */

/* dimension of TPID value
 * refer _port_tpid_add */
UINT8 CIRCLE_FLOW::hal_tpid_max_id()
{
#if defined(CF_BCM_56334)
    return 4;
#else
    "chip type not suppoted yet, pls complete it here."
#endif

}


/* specify TPID selection for ingress
 * refer _port_tpid_add */
void CIRCLE_FLOW::hal_tpid_port_set(UINT8 unit, UINT8 port, UINT8 bmp)
{

#if defined(CF_BCM_56334)

    ENSURE_OK(
        soc_mem_field32_modify(unit,
                               PORT_TABm,
                               port,
                               OUTER_TPID_ENABLEf,
                               bmp));

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}

/* set TPID value on specific unit
 * refer _port_tpid_add */
void CIRCLE_FLOW::hal_tpid_unit_set(UINT8 unit, UINT8 index, UINT16 tpid)
{
#if defined(CF_BCM_56334)

    uint32 tpid_reg = 0;
    soc_reg_field_set(unit, ING_OUTER_TPIDr, &tpid_reg, TPIDf, tpid);

    ENSURE_OK(
        WRITE_ING_OUTER_TPIDr(unit, index, tpid_reg));

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}

/* specify TPID selection for egress
 * refer _port_tpid_add */
void CIRCLE_FLOW::hal_tpid_egr_outer_set(UINT8 unit, UINT16 tpid)
{

#if defined(CF_BCM_56334)

    uint32 tpid_reg = 0;
    soc_reg_field_set(unit, EGR_OUTER_TPIDr, &tpid_reg, TPIDf, tpid);
    ENSURE_OK(
        WRITE_EGR_OUTER_TPIDr(unit, 0, tpid_reg));

    int port;
    PBMP_E_ITER(unit, port) {
        ENSURE_OK(
            soc_reg_field32_modify(unit, EGR_PORT_1r, port,
                                   OUTER_TPID_ENABLEf, 0x1));
    }

#else
    "chip type not suppoted yet, pls complete it here."
#endif

}


void CIRCLE_FLOW::hal_tpid_inner_set(UINT8 unit, UINT16 tpid)
{
    int port;
    PBMP_E_ITER(unit, port) {
        ENSURE_OK(
            bcm_port_inner_tpid_set(unit, port, tpid));
    }

}


