
#include "vfp_entry.h"
#include "fp_rx_sdk_wrap.h"
#include "resource.h"
#include "fp_entry.h"

#include "utility/export/unique.h"

extern "C" {
#include "bcm/field.h"
}

class VFP_ENTRY::IMPL{
    int unit;
    FP_ENTRY entry;

public:
    IMPL(UINT8 _unit, UINT8 port,
         VFP_CLASS_ID class_id,
         const QUAL::PDF_RULES &rules,
         UINT8 pri)
    :unit(_unit),
     entry(unit, GID_RX_VFP)
    {
        int eid = entry.get_eid();

        set_quals(unit, eid, rules);

        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionClassSourceSet, CLASS_SOURCE(class_id), 0));
        ENSURE_OK(
            bcm_field_action_add(unit, eid, bcmFieldActionClassDestSet, CLASS_DEST(class_id), 0));

        ENSURE_OK(
            bcm_field_entry_prio_set(unit, eid, pri));
        ENSURE_OK(
            bcm_field_entry_install(unit, eid));
    }
};

VFP_ENTRY::VFP_ENTRY(UINT8 unit, UINT8 port,
                     VFP_CLASS_ID class_id,
                     const QUAL::PDF_RULES &rules,
                     UINT8 pri)
{
    pimpl = new IMPL(unit, port, class_id, rules, pri);
}

VFP_ENTRY::~VFP_ENTRY()
{
    delete pimpl;
}
