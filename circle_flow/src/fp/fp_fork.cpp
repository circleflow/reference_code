
#include "fp_fork.h"
#include "fp_entry.h"
#include "fp_counter.h"
#include "fp_policer.h"
#include "fp_vtag.h"
#include "resource.h"
#include "udf/hal_udf.h"
#include "sdk.h"
#include "hal.h"

extern "C" {
#include "bcm/field.h"
}

namespace CIRCLE_FLOW {

    void _hal_fp_create_group_fork(int unit)
    {
        bcm_field_qset_t qset;

        bcm_field_qset_t_init(&qset);

        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageIngress);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOuterVlanId);

        hal_udf_qset(unit, &qset);

        ENSURE_OK(
                bcm_field_group_create_id(unit, qset, PRI_G_FORK, GID_FORK));
    }
}


class FP_FORK_PORT::IMPL {

public:
    IMPL(UINT8 _unit,UINT8 engine, UINT8 front)
    : unit(_unit),
      entry(unit, GID_FORK)
    {

        bcm_pbmp_t pbmp;
        BCM_PBMP_CLEAR(pbmp);
        BCM_PBMP_PORT_ADD(pbmp,engine);

        int eid = entry.get_eid();

        ENSURE_OK(
                bcm_field_qualify_InPorts(unit, eid, pbmp, pbmp_all(unit)));

        BCM_PBMP_PORT_ADD(pbmp,front);
        ENSURE_OK(
                bcm_field_action_ports_add(unit, eid, bcmFieldActionRedirectPbmp, pbmp));

        ENSURE_OK(
                bcm_field_entry_prio_set(unit, eid, PRI_E_FORK_PORT));

        ENSURE_OK(
                bcm_field_entry_install(unit, eid));
    }

private:
    UINT8 unit;
    FP_ENTRY entry;

};

FP_FORK_PORT::FP_FORK_PORT(UINT8 unit,UINT8 engine, UINT8 front)
{
    pimpl = new IMPL(unit, engine, front);
}

FP_FORK_PORT::~FP_FORK_PORT()
{
    delete pimpl;
}


class FP_FORK_LATENCY::IMPL {
public:
    UINT8 unit;
    FP_ENTRY entry;

    IMPL(UINT8 unit, UINT8 engine, UINT8 front, UINT8 latency)
        :unit(unit),
         entry(unit, GID_FORK)
    {
        int eid = entry.get_eid();

        ENSURE_OK(
            bcm_field_qualify_InPorts(unit, eid, pbmp(engine), pbmp_all(unit)));

        ENSURE_OK(
            bcm_field_qualify_OuterVlanId(unit, eid,
                                          FP_LATENCY_VTAG::get_fp_data(),
                                          FP_LATENCY_VTAG::get_fp_mask()));

        ENSURE_OK(
            bcm_field_action_ports_add(unit, eid, bcmFieldActionRedirectPbmp, pbmp(front, latency)));

        ENSURE_OK(
            bcm_field_entry_prio_set(unit, eid, PRI_E_FORK_LATENCY));

        ENSURE_OK(
            bcm_field_entry_install(unit, eid));
    }
};

FP_FORK_LATENCY::FP_FORK_LATENCY(UINT8 unit, UINT8 engine, UINT8 front, UINT8 latency)
{
    pimpl = new IMPL(unit, engine, front, latency);
}

FP_FORK_LATENCY::~FP_FORK_LATENCY()
{
    delete pimpl;
}

struct FP_LATENCY_PORT::IMPL {
    UINT8 unit;
    FP_ENTRY entry;

    IMPL(UINT8 _unit, UINT8 port, CPU::QID qid)
    : unit(_unit),
      entry(unit, GID_FORK)
    {
        int eid = entry.get_eid();

        ENSURE_OK(
            bcm_field_qualify_InPorts(unit, eid, pbmp(port), pbmp_all(unit)));

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionCosQCpuNew, qid, 0));

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionGpTimeStampToCpu, 0, 0));

        ENSURE_OK(
                bcm_field_entry_prio_set(unit, eid, PRI_E_LATENCY_PORT));

        ENSURE_OK(
            bcm_field_entry_install(unit, eid));
    }
};

FP_LATENCY_PORT::FP_LATENCY_PORT(UINT8 unit, UINT8 port, CPU::QID qid)
{
    pimpl = new IMPL(unit, port, qid);
}

FP_LATENCY_PORT::~FP_LATENCY_PORT()
{
    delete pimpl;
}

