
#include "udf/hal_udf.h"
#include "fp_rx_sdk_wrap.h"
#include "fp_rx.h"
#include "resource.h"
#include "fp_entry.h"
#include "fp_counter.h"
#include "fp_policer.h"
#include "vfp_rx.h"
#include "cpu.h"
#include "sdk.h"
#include "bytes_bits.h"

#include "utility/export/unique.h"


// group initialization
namespace CIRCLE_FLOW {

    void _hal_fp_create_group_rx_ifp(int unit)
    {
        bcm_field_qset_t qset;
        bcm_field_qset_t_init(&qset);

        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageIngress);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);

        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifySrcClassField);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstClassField);

        set_qset(qset, STAGE_IFP);

        hal_udf_qset(unit, &qset);

        ENSURE_OK(
            bcm_field_group_create_id(unit, qset, PRI_G_RX_IFP, GID_RX_IFP));
    }

    void _hal_fp_create_group_rx_vfp(int unit)
    {
        bcm_field_qset_t qset;
        bcm_field_qset_t_init(&qset);

        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageLookup);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);

        set_qset(qset, STAGE_VFP);

        ENSURE_OK(
            bcm_field_group_create_id(unit, qset, PRI_G_RX_VFP, GID_RX_VFP));
    }
}


class IFP_RX {
private:
    const int unit;
    FP_COUNTER cnt; //placed before entry, to ensure to be destroyed after entry
    FP_POLICER pol; //placed before entry, to ensure to be destroyed after entry
    FP_ENTRY entry;
    int eid;

public:
    IFP_RX(UINT8 _unit, const vector<UINT8> &ports,
           const QUAL::PDF_RULES &pdf_rules, const QUAL::MATCH &udf_match,
           VFP_CLASS_ID class_id, UINT8 pri)
    : unit(_unit),
      cnt(unit, GID_RX_IFP),
      pol(unit),
      entry(unit, GID_RX_IFP),
      eid(entry.get_eid())
    {
        ENSURE_OK(
            bcm_field_qualify_InPorts(unit, eid, pbmp(ports), pbmp_all(unit)));

        set_quals(unit, eid, pdf_rules);

        hal_udf_qualify(unit, eid, udf_match);

        if(NULL_VFP_CLASS_ID != class_id) {
            ENSURE_OK(
                bcm_field_qualify_SrcClassField(unit, eid, CLASS_SOURCE(class_id), 0x3f));
            ENSURE_OK(
                bcm_field_qualify_DstClassField(unit, eid, CLASS_DEST(class_id), 0x3f));
        }

        cnt.attach(eid);
        pol.attach(eid);

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionGpDrop, 0, 0));
        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionYpDrop, 0, 0));
        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionRpDrop, 0, 0));
        ENSURE_OK(
            bcm_field_entry_prio_set(unit, eid, pri));
        ENSURE_OK(
            bcm_field_entry_install(unit, eid));

    }

    void snoop_start(int max_pkt, CPU::QID qid)
    {
        //create a temporary entry, keep counter working during the reinstall break
        FP_ENTRY fp_copy(entry);

        ENSURE_OK(
            bcm_field_entry_install(unit, fp_copy.get_eid()));

        pol.set(-1);

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionCosQCpuNew, qid, 0));

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionGpTimeStampToCpu, 0, 0));

        ENSURE_OK(
            bcm_field_entry_reinstall(unit, eid));

        pol.set(max_pkt);

    }

    void snoop_stop()
    {
        //create a temporary entry, keep counter working during the reinstall break
        FP_ENTRY fp_copy(entry);

        ENSURE_OK(
            bcm_field_entry_install(unit, fp_copy.get_eid()));

        pol.set(-1);

        ENSURE_OK(
            bcm_field_action_remove(unit, eid, bcmFieldActionCosQCpuNew));

        ENSURE_OK(
            bcm_field_action_remove(unit, eid, bcmFieldActionGpTimeStampToCpu));

        ENSURE_OK(
            bcm_field_entry_reinstall(unit, eid));
    }

    COUNTER counter(bool clear)
    {
        return cnt.get(clear);
    }

};

/* to support all/arbitrary qual fields, combine VFP and IFP together
 * split part of qual fields into VFP group,
 * once VFP hit, a corresponded VFP class_id is placed into pkt context.
 * IFP qualify the rest of fields and the VFP class_id as well
 * */
struct HAL_FP_RX::IMPL {

    VFP_RX vfp;
    IFP_RX ifp;

    IMPL(const FP_QUAL &qual)
    :vfp(qual.unit, qual.in_ports, split_rules(qual.pdf_rules, STAGE_VFP), qual.pri),
     ifp(qual.unit, qual.in_ports,
         split_rules(qual.pdf_rules, STAGE_IFP), qual.udf_match,
         vfp.get_class_id(), qual.pri)
    { }

};


HAL_FP_RX::HAL_FP_RX(const FP_QUAL &qual)
{
    pimpl = new IMPL(qual);
}

HAL_FP_RX::~HAL_FP_RX()
{
    delete pimpl;
}

void HAL_FP_RX::snoop_start(UINT8 max_pkt, CPU::QID qid)
{
    pimpl->ifp.snoop_start(max_pkt, qid);
}
void HAL_FP_RX::snoop_stop(void)
{
    pimpl->ifp.snoop_stop();
}

COUNTER HAL_FP_RX::counter(bool clear)
{
    return pimpl->ifp.counter(clear);
}


