
namespace CIRCLE_FLOW {
    void _hal_fp_create_group_fork(int unit);
    void _hal_fp_create_group_burst_q(int unit);
    void _hal_fp_create_group_rx_ifp(int unit);
    void _hal_fp_create_group_rx_vfp(int unit);

    void _hal_udf_init(int unit);
}

#include "resource.h"
#include "hal.h"
#include "init.h"
#include "sdk.h"

#include "utility/export/error.h"

extern "C" {
#include "bcm/vlan.h"
}

void CIRCLE_FLOW::fp_init(void)
{
    int unit;
    UNIT_ALL_ITER(unit) {
        _hal_udf_init(unit);

        _hal_fp_create_group_fork(unit);
        _hal_fp_create_group_burst_q(unit);
        _hal_fp_create_group_rx_vfp(unit);
        _hal_fp_create_group_rx_ifp(unit);

        ENSURE_OK(bcm_vlan_port_remove(unit, 1, pbmp_all(unit)));

        for(bcm_vlan_t vid=2; vid<=4095; vid++) {
            ENSURE_OK(
                bcm_vlan_create(unit, vid));
        }
    }
}

