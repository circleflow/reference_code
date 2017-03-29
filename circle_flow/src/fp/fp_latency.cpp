
#include "udf/hal_udf.h"
#include "fp_rx_sdk_wrap.h"
#include "fp_rx.h"
#include "resource.h"
#include "fp_entry.h"
#include "sdk.h"



struct HAL_FP_LATENCY_RX::IMPL {

    int unit;
    FP_ENTRY entry;

    IMPL(const FP_QUAL &qual, CPU::QID qid)
    : unit(qual.unit),
      entry(unit, GID_FORK)
    {
        int eid = entry.get_eid();

        ENSURE_OK(
            bcm_field_qualify_InPorts(unit, eid, pbmp(qual.in_ports), pbmp_all(unit)));

        hal_udf_qualify(unit, eid, qual.udf_match);

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionCosQCpuNew, qid, 0));

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionGpTimeStampToCpu, 0, 0));

        ENSURE_OK(
                bcm_field_entry_prio_set(unit, eid, PRI_E_LATENCY_RX));

        ENSURE_OK(
            bcm_field_entry_install(unit, eid));

    }

};

HAL_FP_LATENCY_RX::HAL_FP_LATENCY_RX(const FP_QUAL &qual, CPU::QID qid)
{
    pimpl = new IMPL(qual, qid);
}

HAL_FP_LATENCY_RX::~HAL_FP_LATENCY_RX()
{
    delete pimpl;
}



