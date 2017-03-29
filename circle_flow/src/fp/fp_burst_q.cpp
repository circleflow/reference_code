
#include "fp_burst_q.h"
#include "fp_entry.h"
#include "resource.h"
#include "fp_vtag.h"
#include "fp_policer.h"
#include "sdk.h"

extern "C" {
#include "bcm/field.h"
}

static
void create_fp_default_cosq(int unit, int group, int prio, UINT16 vid, UINT16 mask, UINT8 cosq)
{
    int eid=0;

    ENSURE_OK(
        bcm_field_entry_create(unit, group, &eid));

    ENSURE_OK(
        bcm_field_qualify_InPorts(unit, eid, pbmp_e_all(unit), pbmp_all(unit)));

    ENSURE_OK(
        bcm_field_qualify_OuterVlanId(unit, eid, vid, mask));

    ENSURE_OK(
        bcm_field_action_add(unit, eid, bcmFieldActionGpCosQNew, cosq, 0));
    ENSURE_OK(
        bcm_field_action_add(unit, eid, bcmFieldActionYpDrop, 0, 0));
    ENSURE_OK(
        bcm_field_action_add(unit, eid, bcmFieldActionRpDrop, 0, 0));

    ENSURE_OK(
        bcm_field_entry_prio_set(unit, eid, PRI_E_DEFAULT_Q));

    ENSURE_OK(
        bcm_field_entry_install(unit, eid));

}

namespace CIRCLE_FLOW {
    void _hal_fp_create_group_burst_q(int unit)
    {
        bcm_field_qset_t qset;

        bcm_field_qset_t_init(&qset);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageIngress);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyOuterVlanId);

        ENSURE_OK(
            bcm_field_group_create_id(unit, qset, PRI_G_BURST_Q, GID_BURST_Q));

        vector<FP_SHARED_COSQ_VTAG::MAPPING> mapping = FP_SHARED_COSQ_VTAG::get_mapping();
        for(UINT32 i=0; i<mapping.size(); i++) {
            create_fp_default_cosq(unit, GID_BURST_Q, PRI_E_DEFAULT_Q,
                                   mapping[i].vid, mapping[i].mask, mapping[i].cosq);
        }
    }
}

class FP_BURST_COSQ::IMPL {
public:

    UINT8 unit;
    FP_POLICER pol; //placed before entry, to ensure destroy after entry
    FP_ENTRY entry;

    IMPL(UINT8 unit, UINT8 engine, UINT8 cosq, UINT16 flow_vid)
    : unit(unit),
      pol(unit),
      entry(unit, GID_BURST_Q)
    {
        int eid = entry.get_eid();

        ENSURE_OK(
            bcm_field_qualify_InPorts(unit, eid, pbmp(engine), pbmp_all(unit)));

        ENSURE_OK(
            bcm_field_qualify_OuterVlanId(unit, eid,
                                          flow_vid,
                                          FP_BRUST_Q_VTAG::get_vid_mask()));

        pol.attach(eid);

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionGpCosQNew, cosq, 0));
        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionYpDrop, 0, 0));
        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionRpDrop, 0, 0));

        ENSURE_OK(
            bcm_field_entry_prio_set(unit, eid, PRI_E_BURST_Q));

        ENSURE_OK(
            bcm_field_entry_install(unit, eid));

        pol.set(0);
    }
};

FP_BURST_COSQ::FP_BURST_COSQ(UINT8 unit, UINT8 engine, UINT8 cosq, UINT16 flow_vid)
{
    pimpl = new IMPL(unit, engine, cosq, flow_vid);
}

FP_BURST_COSQ::~FP_BURST_COSQ()
{
    delete pimpl;
}

void FP_BURST_COSQ::burst (const BURST &burst)
{
    ENSURE(BURST::PKT == burst.type,
            "invalid burst type, must in unit of pkt")

    ENSURE(burst.value<=pimpl->pol.max(),
            "burst value exceed the max limitation %d", pimpl->pol.max());

    pimpl->pol.set((int)burst.value);
}
