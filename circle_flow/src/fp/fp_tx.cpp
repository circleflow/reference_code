#include "fp_tx.h"
#include "fp_entry.h"
#include "resource.h"
#include "sdk.h"
#include "hal.h"

extern "C" {
#include "bcm/field.h"
}

class HAL_FP_TX_STOP::IMPL {
public:
    IMPL(UINT8 unit, UINT8 engine, UINT16 ovid)
    :entry(unit, GID_FORK)
    {
        bcm_pbmp_t pbmp;
        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_PORT_ADD(pbmp,engine);

        int eid = entry.get_eid();

        ENSURE_OK(
                bcm_field_qualify_InPorts(unit, eid, pbmp, pbmp_all(unit)));

        ENSURE_OK(
                bcm_field_action_ports_add(unit, eid, bcmFieldActionRedirectPbmp, pbmp));

        ENSURE_OK(
                bcm_field_qualify_OuterVlanId(unit, eid, ovid, 0xfff));

        ENSURE_OK(
                bcm_field_action_add(unit, eid, bcmFieldActionGpDrop, 0, 0));
        ENSURE_OK(
                bcm_field_action_add(unit, eid, bcmFieldActionYpDrop, 0, 0));
        ENSURE_OK(
                bcm_field_action_add(unit, eid, bcmFieldActionRpDrop, 0, 0));

        ENSURE_OK(
                bcm_field_entry_prio_set(unit, eid, PRI_E_FLOW_STOP));

        ENSURE_OK(
                bcm_field_entry_install(unit, eid));

    }

private:

    FP_ENTRY entry;
};

HAL_FP_TX_STOP::HAL_FP_TX_STOP(UINT8 unit, UINT8 engine, UINT16 ovid)
{
    pimpl = new IMPL(unit, engine, ovid);
}

HAL_FP_TX_STOP::~HAL_FP_TX_STOP()
{
    delete pimpl;
}

